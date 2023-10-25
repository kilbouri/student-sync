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
	serverSocket = acquiredSocket;
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

void Client::SendString(const std::string& str) {
    std::unique_ptr<Message> messageToSend = std::make_unique<Message>(str);
    int sendStatus = messageToSend->Send(serverSocket);
    // Handle sendStatus as needed
}

void Client::SendNumber(int64_t number) {
    std::unique_ptr<Message> messageToSend = std::make_unique<Message>(number);
    int sendStatus = messageToSend->Send(serverSocket);
    // Handle sendStatus as needed
}

void Client::SendScreenshot(const DisplayCapturer::Format format) {
    auto captureResult = DisplayCapturer::CaptureScreen(format);
    if (!captureResult) {
        std::cerr << "Failed to capture screen\n";
        return;
    }

    std::vector<char> captureData = std::move(*captureResult);
    std::unique_ptr<Message> messageToSend = std::make_unique<Message>(
        (format == DisplayCapturer::Format::JPG) ? Message::Type::IMAGE_JPG : Message::Type::IMAGE_PNG,
        captureData
    );

    int sendStatus = messageToSend->Send(serverSocket);
    // Handle sendStatus as needed
}



int Client::Connect() {
	if (connect(serverSocket, serverAddress->ai_addr, (int)serverAddress->ai_addrlen) == SOCKET_ERROR) {
		closesocket(serverSocket);
		serverSocket = INVALID_SOCKET;

		freeaddrinfo(serverAddress);
		serverAddress = nullptr;

		return 1;
	}

	freeaddrinfo(serverAddress);
	serverAddress = nullptr;

	auto invalidInput = []() {
		std::cout << "Invalid input, try again: ";
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	};

	enum Choice : int {
		STRING = 0,
		NUMBER = 1,
		SCREENSHOT_JPG = 2,
		SCREENSHOT_PNG = 3,
		GOODBYE = 4
	};

	int choice;
	do {
		std::cout
			<< "What data would you like to send?\n"
			<< "STRING = " << Choice::STRING << "\n"
			<< "NUMBER = " << Choice::NUMBER << "\n"
			<< "SCREENSHOT (JPG) = " << Choice::SCREENSHOT_JPG << "\n"
			<< "SCREENSHOT (PNG) = " << Choice::SCREENSHOT_PNG << "\n"
			<< "GOODBYE = " << Choice::GOODBYE << "\n";

		while (!(std::cin >> choice)) { invalidInput(); }

		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		std::unique_ptr<Message> messageToSend = nullptr;

		switch (choice) {
			case Choice::STRING: {
				std::string userString;

				std::cout << "Please type your String: ";
				std::getline(std::cin, userString);

				messageToSend = std::make_unique<Message>(userString);
				break;
			}

			case Choice::NUMBER: {
				int64_t userNumber;

				std::cout << "Please type your Number: ";
				while (!(std::cin >> userNumber)) { invalidInput(); }

				messageToSend = std::make_unique<Message>(userNumber);
				break;
			}

			case Choice::SCREENSHOT_JPG:
			case Choice::SCREENSHOT_PNG: {
				DisplayCapturer::Format format;
				Message::Type messageType;

				if (choice == Choice::SCREENSHOT_JPG) {
					format = DisplayCapturer::Format::JPG;
					messageType = Message::Type::IMAGE_JPG;
				}
				else {
					format = DisplayCapturer::Format::PNG;
					messageType = Message::Type::IMAGE_PNG;
				}

				auto captureResult = DisplayCapturer::CaptureScreen(format);
				if (!captureResult) {
					std::cerr << "Failed to capture screen\n";
					messageToSend = nullptr;
					break;
				}

				std::vector<char> captureData = std::move(*captureResult);
				messageToSend = std::make_unique<Message>(messageType, captureData);

				break;
			}

			case Choice::GOODBYE:
				messageToSend = std::make_unique<Message>(Message::Goodbye());
				break;

			default:
				invalidInput();
				continue;
		}

		if (messageToSend == nullptr) {
			std::cerr << "No message to send!\n";
			continue;
		}

		int sendStatus = (*messageToSend).Send(serverSocket);
		if (sendStatus != 0) {
			std::cerr << "Failed to send message with error code " << GetLastError() << "\n";
		}
	} while (choice != Choice::GOODBYE);

	shutdown(serverSocket, SD_BOTH);
	return 0;
}

Client::~Client() {
	std::cout << "Destroying client!\n";

	// clean up listen socket
	if (serverSocket != INVALID_SOCKET) {
		closesocket(serverSocket);
		serverSocket = INVALID_SOCKET;
	}

	if (serverAddress != nullptr) {
		freeaddrinfo(serverAddress);
		serverAddress = nullptr;
	}
}