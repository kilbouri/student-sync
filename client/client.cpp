#include "client.h"

#include <fstream>
#include <optional>
#include <string>

#include "../common/message/message.h"
#include "../common/displaycapturer/displaycapturer.h"

Client::Client(std::string_view serverHostname, int serverPort)
	: hostname{ std::string(serverHostname) }, port{ std::to_string(serverPort) }, hints{},
	connectSocket{ INVALID_SOCKET }, serverAddress{ nullptr }
{
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
}

int Client::Initialize() {
	// resolve server address
	struct addrinfo* resolvedAddress;
	if (getaddrinfo(hostname.c_str(), port.c_str(), &hints, &resolvedAddress) != 0) {
		return 1;
	}

	// try to obtain a socket
	SOCKET acquiredSocket = socket(resolvedAddress->ai_family, resolvedAddress->ai_socktype, resolvedAddress->ai_protocol);
	if (acquiredSocket == INVALID_SOCKET) {
		freeaddrinfo(resolvedAddress);
		return 1;
	}

	serverAddress = resolvedAddress;
	connectSocket = acquiredSocket;
	return 0;
}

std::optional<std::string> ReceiveMessage(SOCKET socket) {
	std::optional<Message> messageOpt = Message::TryReceive(socket);
	if (!messageOpt.has_value()) {
		return std::nullopt;
	}

	Message message = messageOpt.value();
	return std::string{ message.data.data() };
}

int Client::Connect() {
	if (connect(connectSocket, serverAddress->ai_addr, (int)serverAddress->ai_addrlen) == SOCKET_ERROR) {
		closesocket(connectSocket);
		connectSocket = INVALID_SOCKET;

		freeaddrinfo(serverAddress);
		serverAddress = nullptr;

		return 1;
	}

	freeaddrinfo(serverAddress);
	serverAddress = nullptr;

	std::string buffer;
	int64_t number = 0;
	int choice;

	auto invalidInput = []() {
		std::cout << "Invalid input, try again: ";
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	};

	do {
		std::cout
			<< "What data would you like to send?\n"
			<< "STRING = 0\n"
			<< "NUMBER = 1\n"
			<< "SCREENSHOT = 2\n"
			<< "GOODBYE = 3\n";

		while (!(std::cin >> choice)) {
			invalidInput();
		}

		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		if (choice == 0) { //STRING
			std::cout << "Please type your String: ";
			std::getline(std::cin, buffer);

			if (Message(buffer).Send(connectSocket)) {
				std::cout << "WARNING: send() failed with code " << GetLastError() << "\n";
				closesocket(connectSocket);
				continue;
			}
		}
		else if (choice == 1) { // NUMBER
			std::cout << "Please type your Number: ";

			while (!(std::cin >> number)) {
				invalidInput();
			};

			if (Message(number).Send(connectSocket)) {
				std::cout << "WARNING: send() failed with code " << GetLastError() << "\n";
				closesocket(connectSocket);
				continue;
			}
		}
		else if (choice == 2) { // SCREENSHOT
			std::optional<std::vector<char>> screenshotData = DisplayCapturer::CaptureScreen(DisplayCapturer::Format::PNG);
			if (!screenshotData.has_value()) {
				std::cerr << "Failed to capture screenshot\n";
				continue;
			}
			std::vector<char> data = screenshotData.value();

			std::ofstream outFile("./screenshot.png", std::ios::binary);
			if (!outFile.is_open()) {
				std::cout << "Failed to open ./screenshot.png\n";
			}

			outFile.write(data.data(), data.size());
			outFile.close();

			std::cout << "Write complete. Check ./screenshot.png!\n";
		}
		else if (choice != 3) {
			invalidInput();
			continue;
		}

		std::cout << std::flush;

	} while (choice != 3);

	// this may fail but who cares
	Message::Goodbye().Send(connectSocket);
	shutdown(connectSocket, SD_SEND);

	return 0;
}

Client::~Client() {
	std::cout << "Destroying client!\n";

	// clean up listen socket
	if (connectSocket != INVALID_SOCKET) {
		closesocket(connectSocket);
		connectSocket = INVALID_SOCKET;
	}

	if (serverAddress != nullptr) {
		freeaddrinfo(serverAddress);
		serverAddress = nullptr;
	}
}