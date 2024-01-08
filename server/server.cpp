#include "server.h"

#include "../common/networkmessage/networkmessage.h"
#include "../common/generator/generator.h"
#include "../common/task/task.h"

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

void Server::SetConnectionHandler(ConnectionHandlerFunc handler) {
	this->connectionHandler = handler;
}

Server::~Server() {
	listenSocket.Close();
}
#pragma endregion

#pragma region SingleConnectServer
SingleConnectServer::SingleConnectServer()
	: Server{},
	currentConnection{ }
{}

Task<void> SingleConnectServer::Start() {
	while (!IsStopRequested()) {
		std::optional<TCPSocket> acceptResult = listenSocket.Accept();
		if (!acceptResult) {
			continue;
		}

		// if we don't have a connection handler, we should just kill the connection
		if (!connectionHandler) {
			acceptResult->Close();
			continue;
		}

		// Fire of the connection handler and await its completion
		currentConnection = ConnectionContext{ this, *acceptResult };
		co_await(*connectionHandler)(*currentConnection);

		// Ensure the client gets closed if the handler did not close it
		if (currentConnection) {
			currentConnection->Terminate();
		}
	}
}

void SingleConnectServer::Stop(bool now) {
	// prevent new connections
	listenSocket.Close();

	// kill current connection, if there is any
	if (now && currentConnection) {
		currentConnection->Terminate();
	}
}

bool SingleConnectServer::IsStopRequested() {
	// if the socket is invalid, the server has been stopped
	return !listenSocket.IsValid();
}

int SingleConnectServer::GetConnectionCount() {
	return currentConnection.has_value() ? 1 : 0;
}
#pragma endregion

#pragma region SingleConnectServer::ConnectionContext
SingleConnectServer::ConnectionContext::ConnectionContext(SingleConnectServer* server, TCPSocket socket)
	: server{ server }, clientSocket{ socket } {}

void SingleConnectServer::ConnectionContext::Terminate() {
	this->clientSocket.Close();
	this->server->currentConnection = std::nullopt;
}

Task<void> SingleConnectServer::ConnectionContext::Send(NetworkMessage message) {
	message.Send(this->clientSocket);
	co_return;
}

Task<NetworkMessage> SingleConnectServer::ConnectionContext::Receive() {
	while (true) {
		std::optional<NetworkMessage> reply = NetworkMessage::TryReceive(this->clientSocket);
		if (reply) {
			co_return reply.value();
		}
	}
}
bool SingleConnectServer::ConnectionContext::ConnectionIsAlive() {
	return this->clientSocket.IsValid();
}
#pragma endregion

#if 0
#pragma region MultiConnectServer
MultiConnectServer::MultiConnectServer()
	: Server(), currentClients{}, clientReadBuffers{}, connections{}
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
#endif