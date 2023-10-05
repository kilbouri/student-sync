#include "server.h"

#include <string>
#include <WS2tcpip.h>

/**
 * Creates a server that will listen on the specified port
 */
Server::Server(int portNumber)
	: port{ std::to_string(portNumber) }, hints{}, listenSocket{ INVALID_SOCKET }
{
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // this already has TCP implied for IPv4 but...
	hints.ai_protocol = IPPROTO_TCP; // ...the docs say to also include this
	hints.ai_flags = AI_PASSIVE; // indicates will call bind() with this socket
}

/**
 * Prepares the server to start listening for connections.
 */
int Server::Initialize() {
	// get granted address information
	struct addrinfo* resolvedAddress;

	if (getaddrinfo(nullptr, port.c_str(), &hints, &resolvedAddress) != 0) {
		return 1;
	}

	// try to obtain a socket
	SOCKET acquiredSocket = socket(resolvedAddress->ai_family, resolvedAddress->ai_socktype, resolvedAddress->ai_protocol);
	if (acquiredSocket == INVALID_SOCKET) {
		freeaddrinfo(resolvedAddress);
		return 1;
	}

	// try to bind the socket
	if (bind(acquiredSocket, resolvedAddress->ai_addr, resolvedAddress->ai_addrlen) == SOCKET_ERROR) {
		freeaddrinfo(resolvedAddress);
		closesocket(acquiredSocket);

		return 1;
	}

	freeaddrinfo(resolvedAddress);
	listenSocket = acquiredSocket;
	return 0;
}

bool Server::sendMessage(SOCKET socket, const std::string& message) {
	int result = send(socket, message.c_str(), message.length(), 0);
	return result != SOCKET_ERROR;
}

std::string Server::receiveMessage(SOCKET socket) {
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

int Server::Start() {
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		return 1;
	}

	SOCKET clientSocket = INVALID_SOCKET;
	while (true) {
		clientSocket = accept(listenSocket, NULL, NULL);

		if (clientSocket == INVALID_SOCKET) {
			continue;
		}

		const char* message = "Hello from the server!";
		if (!sendMessage(clientSocket, message)) {
			std::cout << "WARNING: send() failed with code " << GetLastError() << "\n";
			continue;
		}

		_flushall();
		std::string buffer;
		std::cin.ignore();
		do {
			std::cout << "Enter a message (or 'Exit' to quit)";
			std::getline(std::cin, buffer);

			if (!sendMessage(clientSocket, buffer)) {
				std::cout << "WARNING: send() failed with code " << GetLastError() << "\n";
				closesocket(clientSocket);
				continue;
			}

			std::cout << std::flush;

		} while (buffer != "Exit");

		shutdown(clientSocket, SD_SEND);
		closesocket(clientSocket);
	}
}

Server::~Server() {
	std::cout << "Destroying server!\n";

	// clean up listen socket
	if (listenSocket != INVALID_SOCKET) {
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
	}
}