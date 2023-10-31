#include "server.h"
#include "../common/message/message.h"

#include <string>
#include <optional>
#include <fstream>

#include <chrono>
#include <iostream>
#include <thread>

#include <wx/wx.h>
/**
 * Creates a server that will listen on the specified interface and port
 */
Server::Server(std::string& ipAddress, int portNumber)
	: ipAddress{ std::string(ipAddress) }, portNumber{ portNumber }, listenSocket{ INVALID_SOCKET }, messageHandler{ std::nullopt }, flag{ 0 }
{}


std::string Server::GetExternalAddress() {
	return std::string(ipAddress);
}

int Server::GetPortNumber() {
	if (listenSocket == INVALID_SOCKET) {
		// This shouldn't happen unless Initialize() wasn't called first.
		// Which makes this a pretty exceptional circumstance.
		throw "GetPortNumber called while Server has invalid socket";
	}

	// if port is non-zero, and we are successfully bound, then the port is the same
	if (portNumber != 0) {
		return portNumber;
	}

	// if the port is zero, and we are successfully bound, then the port is unknown and must be retrieved

	struct sockaddr_in socketInfo = {};
	int socketInfoLength = sizeof(socketInfo);

	if (getsockname(listenSocket, (struct sockaddr*)&socketInfo, &socketInfoLength) != 0) {
		throw "Failed to get server socket info";
	}

	portNumber = ntohs(socketInfo.sin_port);
	return portNumber;
}

bool Server::Initialize() {
	// while this should never be called multiple times, can't hurt to make sure the old socket gets cleaned
	if (IsInitialized()) {
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
	}

	struct addrinfo hints = { .ai_family = AF_INET, .ai_socktype = SOCK_STREAM, .ai_protocol = IPPROTO_TCP };
	listenSocket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);

	if (listenSocket == INVALID_SOCKET) {
		return false;
	}

	struct addrinfo* addressInfo;
	if (getaddrinfo(ipAddress.c_str(), std::to_string(portNumber).c_str(), &hints, &addressInfo)) {
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
		return false;
	}

	if (bind(listenSocket, addressInfo->ai_addr, addressInfo->ai_addrlen) == SOCKET_ERROR) {
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
		return false;
	}

	freeaddrinfo(addressInfo);
	addressInfo = nullptr;
	return true;
}

bool Server::IsInitialized() {
	return this->listenSocket != INVALID_SOCKET;
}

bool Server::Start() {
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		return false;
	}

	SOCKET clientSocket = INVALID_SOCKET;
	while (!IsStopRequested()) {
		using Type = Message::Type;

		clientSocket = accept(listenSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET) {
			continue;
		}

		while (!IsStopRequested()) {
			std::optional<Message> messageOpt = Message::TryReceive(clientSocket);
			if (!messageOpt) {
				break;
			}

			const Message message = std::move(*messageOpt);

			// Invoke message handler if set
			if (messageHandler) {
				try {
					bool handlerResult = (*messageHandler)(clientSocket, message);

					// handler can return false if they want to end the conversation
					if (!handlerResult) {
						break;
					}
				}
				catch (...) {
					// the callback threw an exception, this is not a good thing!
					break;
				}

			}

			if (message.type == Type::GOODBYE) {
				break;
			}
		}


		shutdown(clientSocket, SD_BOTH);
		closesocket(clientSocket);
	}
}

void Server::Stop() {
	closesocket(listenSocket); // kills any pending network operations
	listenSocket = INVALID_SOCKET; // prevents the main loop from continuing
}

bool Server::IsStopRequested() {
	return listenSocket == INVALID_SOCKET;
}

void Server::SetMessageHandler(std::function<bool(const SOCKET, const Message)> handler) {
	messageHandler = handler;
}

Server::~Server() {
	if (listenSocket != INVALID_SOCKET) {
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
	}
}