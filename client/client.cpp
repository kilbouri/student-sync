#include "client.h"

#include <fstream>
#include <optional>
#include <string>

#include "../common/message/message.h"

Client::Client(std::string_view serverHostname, int serverPort)
	: hostname{ std::string(serverHostname) }, port{ std::to_string(serverPort) }, hints{},
	serverSocket{ INVALID_SOCKET }, serverAddress{ nullptr }
{
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
}

bool Client::Initialize() {
	// resolve server address
	struct addrinfo* resolvedAddress;
	if (getaddrinfo(hostname.c_str(), port.c_str(), &hints, &resolvedAddress) != 0) {
		return false;
	}

	// try to obtain a socket
	SOCKET acquiredSocket = socket(resolvedAddress->ai_family, resolvedAddress->ai_socktype, resolvedAddress->ai_protocol);
	if (acquiredSocket == INVALID_SOCKET) {
		freeaddrinfo(resolvedAddress);
		return false;
	}

	serverAddress = resolvedAddress;
	serverSocket = acquiredSocket;
	return true;
}

std::optional<std::string> ReceiveMessage(SOCKET socket) {
	std::optional<Message> messageOpt = Message::TryReceive(socket);
	if (!messageOpt.has_value()) {
		return std::nullopt;
	}

	Message message = messageOpt.value();
	return std::string{ message.data.data() };
}

bool Client::SendString(const std::string& str) {
	return Message(str).Send(serverSocket) == 0;
}

bool Client::SendNumber(int64_t number) {
	return Message(number).Send(serverSocket) == 0;
}

bool Client::SendScreenshot(const DisplayCapturer::Format format) {
	auto captureResult = DisplayCapturer::CaptureScreen(format);
	if (!captureResult) {
		return false;
	}

	std::vector<char> captureData = std::move(*captureResult);
	Message::Type messageType = (format == DisplayCapturer::Format::JPG) ? Message::Type::IMAGE_JPG : Message::Type::IMAGE_PNG;

	return Message(messageType, captureData).Send(serverSocket) == 0;
}

bool Client::Connect() {
	if (connect(serverSocket, serverAddress->ai_addr, (int)serverAddress->ai_addrlen) == SOCKET_ERROR) {
		closesocket(serverSocket);
		serverSocket = INVALID_SOCKET;

		freeaddrinfo(serverAddress);
		serverAddress = nullptr;

		return false;
	}

	return true;
}

Client::~Client() {
	// clean up listen socket
	if (serverSocket != INVALID_SOCKET) {
		closesocket(serverSocket);
		serverSocket = INVALID_SOCKET;
	}

	// clean up server address info
	if (serverAddress != nullptr) {
		freeaddrinfo(serverAddress);
		serverAddress = nullptr;
	}
}