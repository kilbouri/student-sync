#include "server.h"
#include "../common/message/message.h"

#include <string>
#include <WS2tcpip.h>
#include <optional>

void HandleConnection(SOCKET server, SOCKET client);

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

		HandleConnection(listenSocket, clientSocket);
	}
}

void HandleConnection(SOCKET server, SOCKET client) {
	shutdown(client, SD_SEND);

	// receive all messages regardless of type,
	// echo to screen. Breaks when GOODBYE is received,
	// or socket is closed
	while (true) {
		std::optional<Message> messageOpt = Message::TryReceive(client);
		if (!messageOpt.has_value()) {
			// no message, socket closed
			std::cout << "Malformed/no message received\n";
			break;
		}

		Message message = messageOpt.value();
		if (message.type == Message::Type::GOODBYE) {
			// goodbye message received, client is hecking off to god knows where else
			std::cout << "Received GOODBYE\n";
			break;
		}

		if (message.type == Message::Type::NUMBER_64) {
			int64_t receivedNumber;
			memcpy(&receivedNumber, message.data.data(), sizeof(int64_t));

			// convert from network to host byte order
			receivedNumber = ntohll(receivedNumber);
			std::cout << "Received NUMBER_64: " << receivedNumber << "\n";
		}
		else if (message.type == Message::Type::STRING) {
			std::string receivedString(message.data.data(), message.length);
			std::cout << "Received STRING: " << receivedString << "\n";
		}
		else {
			std::cout << "Invalid message type received " << message.type << "\n";
		}
	}

	std::cout << "Client connection closed\n";

	shutdown(client, SD_RECEIVE);
	closesocket(client);
}

Server::~Server() {
	std::cout << "Destroying server!\n";

	// clean up listen socket
	if (listenSocket != INVALID_SOCKET) {
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
	}
}