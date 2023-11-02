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

bool Client::SendScreenshot(const DisplayCapturer::Format format) {
	using Type = Message::Type;
	using Format = DisplayCapturer::Format;

	Type messageType;
	switch (format) {
		case Format::PNG: messageType = Type::ImagePNG; break;
		case Format::JPG: messageType = Type::ImageJPG; break;
		default: return false;
	}

	std::optional<std::vector<byte>> captureData = DisplayCapturer::CaptureScreen(format);
	if (!captureData) {
		return false;
	}

	return Message(messageType, std::move(*captureData)).Send(socket);
}

Client::~Client() {
	socket.Close();
}