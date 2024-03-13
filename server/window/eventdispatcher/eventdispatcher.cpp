#include "eventdispatcher.hpp"

#include <format>
#include <wx/rawbmp.h>

using namespace StudentSync::Networking;

namespace StudentSync::Server {
	EventDispatcher::EventDispatcher(Window* window)
		: window{ window }
		, decoder{ AVPixelFormat::AV_PIX_FMT_BGR24 }
	{}

	void EventDispatcher::SessionStarted(Session const& session) {
		wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_CONNECT);
		event->SetPayload(session.identifier);
		wxQueueEvent(window, event);
	}

	void EventDispatcher::SessionEnded(Session const& session) {
		wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_DISCONNECT);
		event->SetPayload(session.identifier);
		wxQueueEvent(window, event);
	}

	void EventDispatcher::ClientRegistered(Session const& session, Message::Hello const& message) {
		wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_REGISTERED);
		Window::ClientInfo payload{
			.identifier = session.identifier,
			.username = message.username
		};

		event->SetPayload(payload);
		wxQueueEvent(window, event);
	}

	void EventDispatcher::ClientFrameReceived(Session const& session, Networking::Message::StreamFrame const& message) {
		wxMemoryInputStream imageDataStream{ message.imageData.data(), message.imageData.size() };
		wxBitmap image{ wxImage{imageDataStream} }; // todo: do we need to cram this into a wxBitmap? Maybe we can draw wxImages directly?

		wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_STREAM_FRAME_RECEIVED);
		event->SetPayload(image);
		wxQueueEvent(window, event);
	}

	void EventDispatcher::ClientH264PacketRecieved(Session const& session, Networking::Message::H264Packet& packet) {
		using H264Decoder = Common::FFmpeg::Decoders::H264Decoder;

		auto sendResult = decoder.SendPacket(packet.imageData);
		if (sendResult != H264Decoder::SendPacketResult::Success) {
			return;
		}

		auto readResult = decoder.ReadFrame();
		if (!readResult) {
			return;
		}

		auto& rgbPixelData = readResult.value();

		size_t pixelDataByteLength = rgbPixelData.size() * sizeof(uint8_t);
		void* bmpDataPtr = std::malloc(pixelDataByteLength);
		if (bmpDataPtr == nullptr) {
			throw "Failed to allocate bitmap";
		}

		std::memcpy(bmpDataPtr, rgbPixelData.data(), pixelDataByteLength);

		wxImage image{ 2256, 1504, reinterpret_cast<unsigned char*>(bmpDataPtr), false };
		wxBitmap bmp{ image };

		if (!bmp.IsOk()) {
			wxLogFatalError("Invalid bitmap!");
			return;
		}

		wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_STREAM_FRAME_RECEIVED);
		event->SetPayload(bmp);
		wxQueueEvent(window, event);
	}
}