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
				continue;
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

		// todo: this is a hack, lets not do that

		if (message.tag != TLVMessage::Tag::GetStreamParams) {
			return;
		}

		// the server is starting a stream
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
			PushLogMessage("Sending failed");
			return;
		}

		auto reply = Message::TryReceive<Message::InitializeStream>(connection.socket);
		if (!reply) {
			PushLogMessage("Server reply invalid");
			return;
		}

		PushLogMessage(std::format(
			"Received InitializeStream({} @ {}fps)",
			reply->resolution.ToString(),
			reply->frameRate
		));

		Common::Timer streamTimer{
			[&connection]() { // sending a message may mutate state, thus we need the closure to also be mutable
				auto message = Message::StreamFrame::FromDisplay(Common::DisplayCapturer::Format::PNG);
				if (message) {
					message->ToTLVMessage().Send(connection.socket);
				}
			},
			std::chrono::milliseconds(1000 / reply->frameRate)
		};

		// Await stop message
		auto stop = Message::TryReceive<Message::EndStream>(connection.socket);
		if (stop) {
			streamTimer.Stop();
		}
	}

	void Window::PushLogMessage(std::string message) {
		wxThreadEvent* event = new wxThreadEvent(CLIENT_EVT_PUSH_LOG);
		event->SetPayload(wxString(message));
		wxQueueEvent(this, event);
	}
}