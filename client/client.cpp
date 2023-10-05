#include "client.h"

#include <string>
#include <WS2tcpip.h>

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

bool Client::SendMessage(SOCKET socket, const std::string& message) {
	int result = send(socket, message.c_str(), message.length()+1, 0);
	return result != SOCKET_ERROR;
}

bool Client::SendMessage(SOCKET socket, int64_t message) {
	const char* data = reinterpret_cast<const char*>(&message);
	int dataSize = sizeof(message)+1;

	int result = send(socket, data, dataSize, 0);
	return result != SOCKET_ERROR;
}

std::string Client::ReceiveMessage(SOCKET socket) {
	constexpr auto BUFF_SIZE = 256;
	char recvBuffer[BUFF_SIZE + 1];

	int recvResult = recv(socket, recvBuffer, BUFF_SIZE, 0);
	if (recvResult > 0) {
		recvBuffer[std::min<size_t>(static_cast<size_t>(recvResult), BUFF_SIZE)] = '\0';
		return recvBuffer;
	}
	else {
		return "";
	}
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

	std::string Buffer;
	int64_t Number = 0;
	int Choice;

	do {

		std::cout << "Would you like to send a string or a number\nSTRING = 0,\nNUMBER = 1,\nGOODBYE = 3\n";

		while (!(std::cin >> Choice) || (Choice != 0 && Choice != 1 && Choice != 3)) {
			// Handle invalid input
			std::cout << "Invalid input. Please enter a valid number.\n";
			std::cin.clear(); // Clear the error flag

			// Clear the input buffer
			while (std::cin.get() != '\n');
		}

		if (Choice == 0) {//STRING
			std::cout << "Please type your String: ";
			std::cin >> Buffer;

			if (!SendMessage(connectSocket, Buffer)) {
				std::cout << "WARNING: send() failed with code " << GetLastError() << "\n";
				closesocket(connectSocket);
				continue;
			}
		}
		else if (Choice == 1)//NUMBER
		{
			std::cout << "Please type your Number: ";
			std::cin >> Number;

			if (!SendMessage(connectSocket, Number)) {
				std::cout << "WARNING: send() failed with code " << GetLastError() << "\n";
				closesocket(connectSocket);
				continue;
			}
		}
		else if (Choice != 3)
		{
			std::cout << "Invalid choice. Please enter a valid option.\n";
			std::cin.clear(); // Clear the error flag
			std::cin.ignore(10000, '\n'); // Use a large constant value to clear the input buffer
			continue;
		}

		std::cout << std::flush;

	} while (Choice != 3);


	/*
	constexpr auto BUFF_SIZE = 255;
	char recvBuffer[BUFF_SIZE] = { 0 };

	int recvResult; // recieve the "Hello from the server" message.

	do {
		recvResult = recv(connectSocket, recvBuffer, BUFF_SIZE, 0);
		if (recvResult > 0) {
			// Null-terminate the received buffer
			recvBuffer[std::min<size_t>(recvResult, BUFF_SIZE)] = '\0'; // make sure to use std::min<size_t> so it doesnt overrun the buffer 
			std::cout << recvBuffer << "\n";
		}
		else if (recvResult == 0) {
			std::cout << "\nConnection closed.\n";
		}
		else {
			std::cout << "recv failed\n";
		}

	} while (recvResult > 0);*/

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