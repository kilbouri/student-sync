// included in clientwindow.cpp

#include "window.hpp"

#include <thread>
#include <format>

#include "../preferencesmanager/preferencesmanager.hpp"
#include "../../networking/message/message.hpp"

using namespace StudentSync::Networking;

namespace StudentSync::Client {
	void Window::ThreadEntry() {
		this->client->Run();
	}

	void Window::ConnectionHandler(Client::Connection connection) {
		std::string username = PreferencesManager::GetInstance().GetPreferences().displayName;

		TCPSocket::SocketInfo peerInfo = connection.socket
			.GetPeerSocketInfo()
			.value_or(TCPSocket::SocketInfo{ .Address = "unknown",.Port = 0 });

		PushLogMessage(std::format("Connected to {}:{}", peerInfo.Address, peerInfo.Port));

		if (!Message::Hello{ username }.ToTLVMessage().Send(connection.socket)) {
			PushLogMessage("Registration failed (failed to send Hello)");
			return wxQueueEvent(this, new wxThreadEvent(CLIENT_EVT_REGISTRATION_FAILED));
		}

		auto okReply = Message::TryReceive<Message::Ok>(connection.socket);
		if (!okReply) {
			PushLogMessage("Registration failed (reply was not Ok)");
			return wxQueueEvent(this, new wxThreadEvent(CLIENT_EVT_REGISTRATION_FAILED));
		}

		PushLogMessage(std::format("Registered successfully as {}", username));

		while (connection.socket.IsValid()) {
			auto message = TLVMessage::TryReceive(connection.socket);
			if (!message) {
				PushLogMessage("Failed to recieve message");
				connection.socket.Close();
			}

			MessageReceived(connection, *message);
		}
	}

	void Window::MessageReceived(Client::Connection& connection, TLVMessage& message) {
		PushLogMessage(std::format(
			"Received {} message ({} bytes)",
			TLVMessage::TagName(message.tag),
			message.data.size()
		));

		if (message.tag == TLVMessage::Tag::GetStreamParams) {
			// server is asking to start streaming
			ThreadStreaming(connection);
		}
	}

	void Window::ThreadStreaming(Client::Connection& connection) {
		auto& prefs = PreferencesManager::GetInstance().GetPreferences();
		Message::StreamParams params{
			.frameRate = prefs.maxFrameRate,
			.resolution = prefs.maxStreamResolution
		};

		PushLogMessage(std::format(
			"Sending StreamParams ({} @ {}fps)",
			prefs.maxStreamResolution.ToString(),
			prefs.maxFrameRate
		));

		if (!params.ToTLVMessage().Send(connection.socket)) {
			PushLogMessage("Failed to send StreamParams reply to GetStreamParams");
			return;
		}

		auto reply = Message::TryReceive<Message::InitializeStream>(connection.socket);
		if (!reply) {
			PushLogMessage("Server reply was not InitializeStream");
			return;
		}

		PushLogMessage(std::format(
			"Received InitializeStream({} @ {}fps)",
			reply->resolution.ToString(),
			reply->frameRate
		));

		int width = reply->resolution.width;
		int height = reply->resolution.height;
		int fps = reply->frameRate;
		GDIPlusUtil::PixelFormat pixelFormat = GDIPlusUtil::PixelFormat::RGB_24bpp;

		using Common::FFmpeg::Encoders::H264Encoder;
		H264Encoder encoder{ width, height, fps };

		auto bitmap = GDIPlusUtil::GetBitmap(width, height, pixelFormat);

		// Begin sending frames at regular interval on a background thread
		Common::Timer streamTimer{
			[&]() {
				auto capture = GDIPlusUtil::CaptureScreen(pixelFormat, Gdiplus::Color::Black, bitmap);
				if (!capture) {
					PushLogMessage(std::format("Failed to capture screen: %d", static_cast<int>(capture.error())));
					return;
				}

				bitmap = *capture;

				auto bitmapData = GDIPlusUtil::GetPixelData(bitmap, pixelFormat);
				if (!bitmapData) {
					PushLogMessage(std::format("Failed to get bitmap data: %d", static_cast<int>(bitmapData.error())));
					return;
				}

				auto encodeResult = encoder.SendFrame(*bitmapData, true);
				if (encodeResult != H264Encoder::SendFrameResult::Success) {
					PushLogMessage(std::format("Failed to send bitmap to encoder: %d", static_cast<int>(encodeResult)));

					if (encodeResult != H264Encoder::SendFrameResult::OutputBufferFull) {
						PushLogMessage("Output buffer full!");
						return;
					}
				};

				auto packet = encoder.ReceivePacket();
				if (!packet) {
					PushLogMessage(std::format("Failed to read packet from encoder: %d", static_cast<int>(packet.error())));
					if (packet.error() != H264Encoder::ReceivePacketError::InsufficientInput) {
						PushLogMessage("Insufficient input!");
						return;
					}
				}

				Message::H264Packet message{
					.imageData = *packet,
					.frameWidth = width,
					.frameHeight = height
				};

				if (!message.ToTLVMessage().Send(connection.socket)) {
					PushLogMessage("Failed to send H264Packet message");
				}

				PushLogMessage("Sent frame!");
			},
			std::chrono::milliseconds(1000 / reply->frameRate)
		};

		// Block this thread until we receive a stop message
		auto stop = Message::TryReceive<Message::EndStream>(connection.socket);
		streamTimer.Stop();
		PushLogMessage("Streaming stopped");
	}

	void Window::PushLogMessage(std::string message) {
		wxThreadEvent* event = new wxThreadEvent(CLIENT_EVT_PUSH_LOG);
		event->SetPayload(wxString(message));
		wxQueueEvent(this, event);
	}
}