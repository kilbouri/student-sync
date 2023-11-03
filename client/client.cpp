#include "client.h"

#include <fstream>
#include <optional>
#include <string>

#include "../common/socket/socket.h"
#include "../common/message/message.h"

Client::Client() : socket(TCPSocket{}) {}

bool Client::Connect(std::string_view hostname, int portNumber) {
	return socket.Connect(hostname, portNumber);
}

bool Client::Disconnect() {
	return socket.Close();
}

bool Client::SendString(const std::string& str) {
	return Message(str).Send(socket);
}

bool Client::SendNumber(int64_t number) {
	return Message(number).Send(socket);
}

bool Client::StartVideoStream() {
	// video frames are required to be in PNG format. Maybe in the future we will swap over to BMP to perform temporal compression
	std::optional<std::vector<byte>> firstFrame = DisplayCapturer::CaptureScreen(DisplayCapturer::Format::PNG);
	if (!firstFrame) {
		return false;
	}

	return Message(Message::Type::StartVideoStream, std::move(*firstFrame)).Send(socket);
}

bool Client::SendVideoFrame() {
	// video frames are required to be in PNG format. Maybe in the future we will swap over to BMP to perform temporal compression
	std::optional<std::vector<byte>> firstFrame = DisplayCapturer::CaptureScreen(DisplayCapturer::Format::PNG);
	if (!firstFrame) {
		return false;
	}

	return Message(Message::Type::VideoFramePNG, std::move(*firstFrame)).Send(socket);
}

bool Client::EndVideoStream() {
	return Message(Message::Type::EndVideoStream).Send(socket);
}

Client::~Client() {
	socket.Close();
}