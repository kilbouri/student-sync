#include "client.h"

#include <fstream>
#include <optional>
#include <string>

#include "../common/socket/socket.h"
#include "../common/messages/hellomessage.h"
#include "../common/networkmessage/networkmessage.h"
#include "../common/messages/streamframemessage.h"

Client::Client() : socket(TCPSocket{}) {}

bool Client::Connect(std::string_view hostname, int portNumber) {
	return socket.Connect(hostname, portNumber);
}

bool Client::Disconnect() {
	return socket.Close();
}

bool Client::StartVideoStream() {
	// video frames are required to be in PNG format. Maybe in the future we will swap over to BMP to perform temporal compression
	std::optional<std::vector<byte>> firstFrame = DisplayCapturer::CaptureScreen(DisplayCapturer::Format::PNG);
	if (!firstFrame) {
		return false;
	}

	return NetworkMessage(NetworkMessage::Tag::StartStream, std::move(*firstFrame)).Send(socket);
}

bool Client::Register(std::string& username) {
	if (!HelloMessage{ username }.ToNetworkMessage().Send(socket)) {
		return false;
	}

	auto reply = NetworkMessage::TryReceive(socket);
	return reply && (reply->tag == NetworkMessage::Tag::Ok);
}

bool Client::SendVideoFrame() {
	// video frames are required to be in PNG format. Maybe in the future we will swap over to BMP to perform temporal compression
	std::optional<StreamFrameMessage> message = StreamFrameMessage::FromDisplay(DisplayCapturer::Format::PNG);
	if (!message) {
		return false;
	}

	return std::move(*message).ToNetworkMessage().Send(socket);
}

bool Client::EndVideoStream() {
	return NetworkMessage(NetworkMessage::Tag::StopStream).Send(socket);
}

bool Client::RequestVideoStream() {
	return true;
}

Client::~Client() {
	socket.Close();
}