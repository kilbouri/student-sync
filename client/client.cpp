#include "client.h"

#include <string>
#include <WS2tcpip.h>
#include <optional>
#include "../common/message/message.h"

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
	if (connect(connectSocket, serverAddress->ai_addr, serverAddress->ai_addrlen) == SOCKET_ERROR) {
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

	do {

		std::cout << "Would you like to send a string or a number\nSTRING = 0,\nNUMBER = 1,\nGOODBYE = 3\n";

		while (!(std::cin >> choice) || !(choice == 0 || choice == 1 || choice == 3)) {
			// Handle invalid input
			std::cout << "Invalid input. Please enter a valid number.\n";
			std::cin.clear(); // Clear the error flag
			std::cin.ignore(MAX_STREAM_SIZE, '\n');
		}

		if (choice == 0) { //STRING
			std::cout << "Please type your String: ";
			std::cin >> buffer;

			if (Message(buffer).Send(connectSocket)) {
				std::cout << "WARNING: send() failed with code " << GetLastError() << "\n";
				closesocket(connectSocket);
				continue;
			}
		}
		else if (choice == 1)//NUMBER
		{
			std::cout << "Please type your Number: ";
			std::cin >> number;

			if (Message(number).Send(connectSocket)) {
				std::cout << "WARNING: send() failed with code " << GetLastError() << "\n";
				closesocket(connectSocket);
				continue;
			}
		}
		else if (choice != 3)
		{
			std::cout << "Invalid choice. Please enter a valid option.\n";
			std::cin.clear(); // Clear the error flag
			std::cin.ignore(MAX_STREAM_SIZE, '\n');
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