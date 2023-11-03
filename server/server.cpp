#include "server.h"
#include "../common/message/message.h"

#include <string>
#include <optional>
#include <fstream>
#include <chrono>
#include <iostream>
#include <thread>
#include <wx/wx.h>

Server::Server() :
	listenSocket{ TCPSocket() },
	messageHandler{ std::nullopt },
	currentClient{ TCPSocket::InvalidSocket() }
{}

bool Server::BindAndListen(std::string& ipAddress, int portNumber) {
	return listenSocket.Bind(ipAddress, portNumber) && listenSocket.Listen(TCPSocket::MaxConnectionQueueLength);
}

void Server::Start() {
	while (!IsStopRequested()) {
		using Type = Message::Type;

		std::optional<TCPSocket> acceptResult = listenSocket.Accept();
		if (!acceptResult) {
			continue;
		}

		currentClient = *acceptResult;

		if (connectHandler) {
			try {
				(*connectHandler)(currentClient);
			}
			catch (...) {
				// threw exception, not good!
			}
		}

		while (!IsStopRequested()) {
			std::optional<Message> messageOpt = Message::TryReceive(currentClient);
			if (!messageOpt) {
				break;
			}

			const Message message = std::move(*messageOpt);

			// Invoke message handler if set
			if (messageHandler) {
				try {
					bool handlerResult = (*messageHandler)(currentClient, message);

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

			if (message.type == Type::Goodbye) {
				break;
			}
		}

		if (disconnectHandler) {
			try {
				(*disconnectHandler)(currentClient);
			}
			catch (...) {
				// threw exception, not good!
			}
		}

		currentClient.Close();
		currentClient = TCPSocket::InvalidSocket();
	}
}

void Server::Stop(bool now) {
	// prevent new connections
	listenSocket.Close();

	if (now) {
		// kill current connection, if there is any
		currentClient.Close();
	}
}

bool Server::IsStopRequested() {
	// if the socket is invalid, the server has been stopped
	return !listenSocket.IsValid();
}

std::optional<std::string> Server::GetHostname() {
	return listenSocket.GetBoundAddress();
}

std::optional<int> Server::GetPort() {
	return listenSocket.GetBoundPort();
}

void Server::SetClientConnectedHandler(std::function<void(TCPSocket& client)> handler) {
	connectHandler = handler;
}

void Server::SetMessageReceivedHandler(std::function<bool(TCPSocket& client, const Message message)> handler) {
	messageHandler = handler;
}

void Server::SetClientDisconnectedHandler(std::function<void(TCPSocket& client)> handler) {
	disconnectHandler = handler;
}

Server::~Server() {
	Stop();
}