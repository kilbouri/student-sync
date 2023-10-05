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
		_flushall();
	} while (recvResult > 0);

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