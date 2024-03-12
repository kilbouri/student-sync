#pragma once

#include "../window.hpp"
#include "../../session/session.hpp"
#include "../../../common/ffmpeg/decoders/h264Decoder.hpp"

namespace StudentSync::Server {
	struct EventDispatcher : Session::EventDispatcher {
		EventDispatcher(Window* window);

		// Inherited via EventDispatcher
		void SessionStarted(Session const& session) override;
		void SessionEnded(Session const& session) override;
		void ClientRegistered(Session const& session, Networking::Message::Hello const& message) override;
		void ClientFrameReceived(Session const& session, Networking::Message::StreamFrame const& message) override;
		void ClientH264PacketRecieved(Session const& session, Networking::Message::H264Packet& packet) override;

	private:
		Window* window;
		Common::FFmpeg::Decoders::H264Decoder decoder;
	};
}