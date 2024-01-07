#include "server.h"
#include "../common/networkmessage/networkmessage.h"
#include "../common/generator/generator.h"
#include <string>
#include <optional>
#include <fstream>
#include <chrono>
#include <iostream>
#include <thread>
#include <wx/wx.h>
#include <coroutine>

// The rationale for using coroutines in the server, instead of (for example) dedicated threads,
// is that the server must be capable of handling hundreds of concurrent connections, AND that most
// of those concurrent connections will be inactive most of the time. That means creating a thread
// for every connection would rapidly waste threads.
//
// It also has a nice side effect of making the connection handlers very, very nice to write for the
// consuming code.

namespace ServerActions {
	struct Action {
	protected:
		Action() {};
	};

	struct AwaitMessage : public Action {
		AwaitMessage() {}
	};

	struct Send : public Action {
		const NetworkMessage message;
		Send(const NetworkMessage& message) : message{ message } {}
	};
}

Generator<ServerActions::Action> HandleConnection() {
	co_yield ServerActions::Send(NetworkMessage{ NetworkMessage::Tag::Hello });
	co_yield ServerActions::AwaitMessage{};
}

#pragma region Server
bool Server::BindAndListen(std::string& ipAddress, int portNumber) {
	return listenSocket.Bind(ipAddress, portNumber) && listenSocket.Listen(TCPSocket::MaxConnectionQueueLength);
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

void Server::SetMessageReceivedHandler(std::function<bool(TCPSocket& client, const NetworkMessage message)> handler) {
	messageHandler = handler;
}

void Server::SetClientDisconnectedHandler(std::function<void(TCPSocket& client)> handler) {
	disconnectHandler = handler;
}

void Server::InvokeClientConnectedHandler(TCPSocket& client) noexcept {
	if (!connectHandler) {
		return;
	}

	try {
		(*connectHandler)(client);
	}
	catch (...) { /* threw exception, not good! */ }
}

std::optional<bool> Server::InvokeMessageReceivedHandler(TCPSocket& client, const NetworkMessage message) noexcept {
	if (!messageHandler) {
		return std::nullopt;
	}

	try {
		return (*messageHandler)(client, message);
	}
	catch (...) {
		return std::nullopt;
	}
}

void Server::InvokeClientDisconnectedHandler(TCPSocket& client) noexcept {
	if (!disconnectHandler) {
		return;
	}

	try {
		(*disconnectHandler)(client);
	}
	catch (...) { /* threw exception, not good! */ }
}

Server::~Server() {
	listenSocket.Close();
}
#pragma endregion

#pragma region SingleConnectServer
SingleConnectServer::SingleConnectServer()
	: Server{},
	currentClient{ TCPSocket::InvalidSocket() }
{}

void SingleConnectServer::Start() {
	while (!IsStopRequested()) {
		std::optional<TCPSocket> acceptResult = listenSocket.Accept();
		if (!acceptResult) {
			continue;
		}

		currentClient = *acceptResult;
		InvokeClientConnectedHandler(currentClient);

		while (!IsStopRequested()) {
			std::optional<NetworkMessage> messageOpt = NetworkMessage::TryReceive(currentClient);
			if (!messageOpt) {
				break;
			}

			const NetworkMessage message = std::move(*messageOpt);
			InvokeMessageReceivedHandler(currentClient, message);
		}

		InvokeClientDisconnectedHandler(currentClient);

		currentClient.Close();
		currentClient = TCPSocket::InvalidSocket();
	}
}

void SingleConnectServer::Stop(bool now) {
	// prevent new connections
	listenSocket.Close();

	if (now) {
		// kill current connection, if there is any
		currentClient.Close();
	}
}

bool SingleConnectServer::IsStopRequested() {
	// if the socket is invalid, the server has been stopped
	return !listenSocket.IsValid();
}

int SingleConnectServer::GetConnectionCount() {
	return currentClient.IsValid() ? 1 : 0;
}
#pragma endregion

#pragma region MultiConnectServer
MultiConnectServer::MultiConnectServer()
	: Server(), currentClients{}, clientReadBuffers{}
{}

// TODO: servers shouldn't allow event handlers to interact with a client socket
// directly, especially in the MultiConnectServer's case. Its fine for now
// because IK is feeling overconfident, but its not a great architecture.
// Better would be to provide event handlers a mechanism to enqueue messages
// to send, then use a writeSet to write when actually able
void MultiConnectServer::Start() {
	while (!IsStopRequested()) {

		// readSet is a set of sockets that we wish to wait for
		// readability on (ie. incoming connection or data ready to read).
		fd_set readSet = {};
		FD_ZERO(&readSet);
		FD_SET(listenSocket.GetUnderlyingSocket(), &readSet);

		for (auto& clientSocket : currentClients) {
			FD_SET(clientSocket.GetUnderlyingSocket(), &readSet);
		}

		// this will block until at least one socket in readSet is readable
		int selectResult = select(0, &readSet, nullptr, nullptr, nullptr);
		if (selectResult == SOCKET_ERROR) {
			continue; // we should really handle this better...
		}

		// is there an incoming connection?
		if (FD_ISSET(listenSocket.GetUnderlyingSocket(), &readSet)) {
			std::optional<TCPSocket> newClient = listenSocket.Accept();
			if (newClient) {
				currentClients.push_back(*newClient);
				InvokeClientConnectedHandler(*newClient);
			}
		}

		// are there any pending reads?
		for (auto clientIterator = currentClients.begin(); clientIterator != currentClients.end();) {
			SOCKET clientRawSocket = (*clientIterator).GetUnderlyingSocket();
			if (!FD_ISSET(clientRawSocket, &readSet)) { // socket not ready
				clientIterator++;
				continue;
			}

			unsigned long pendingBytes = 0;
			if (ioctlsocket(clientRawSocket, FIONREAD, &pendingBytes) == SOCKET_ERROR) {
				// Drop the connection if we can't know how many bytes are pending
				clientIterator = EndConnection(clientIterator);
				continue;
			}

			// get buffer and its current size
			std::vector<byte>& buffer = clientReadBuffers[clientRawSocket];
			size_t originalSize = buffer.size();

			// create room for all buffered + pending bytes
			buffer.resize(buffer.size() + pendingBytes);

			int readResult = clientIterator->ReadBytes(buffer.data() + originalSize, pendingBytes);
			if (readResult == SOCKET_ERROR) {
				// todo: a connection drop handler
				clientIterator = EndConnection(clientIterator);
				continue;
			}

			if (readResult == 0) {
				// client disconnected
				InvokeClientDisconnectedHandler(*clientIterator);
				clientIterator = EndConnection(clientIterator);
				continue;
			}

			// resize down to the actual size of data we have (may be different, but probably won't be)
			buffer.resize(originalSize + readResult);

			std::optional<std::tuple<NetworkMessage, int>> maybeMessage = NetworkMessage::TryFromBuffer(buffer);
			if (maybeMessage) {
				auto& [message, consumedBytes] = *maybeMessage;
				buffer.erase(buffer.begin(), buffer.begin() + consumedBytes);

				InvokeMessageReceivedHandler(*clientIterator, message);
			}

			clientIterator++;
		}
	}
}

void MultiConnectServer::Stop(bool now) {
	listenSocket.Close();

	if (now) {
		// Kill all connected clients during immediate shutdown
		for (auto iterator = currentClients.begin(); iterator != currentClients.end();) {
			// DO NOT move the iterator forward!
			iterator = EndConnection(iterator);
		}
	}
}

bool MultiConnectServer::IsStopRequested() {
	return !listenSocket.IsValid();
}

int MultiConnectServer::GetConnectionCount() {
	return currentClients.size();
}

std::vector<TCPSocket>::iterator MultiConnectServer::EndConnection(std::vector<TCPSocket>::iterator& client) {
	// erase any buffers
	clientReadBuffers.erase(client->GetUnderlyingSocket());

	// close the connection
	client->Close();
	return currentClients.erase(client); // do this last, or `client` will be invalid!
}
#pragma endregion