#include "server.h"

// included by server.cpp

#pragma region ConnectionContext
Task<void> MultiConnectServer::ConnectionContext::Send(NetworkMessage message) {
	co_return;
}

Task<std::optional<NetworkMessage>> MultiConnectServer::ConnectionContext::Recieve() {
	co_return std::nullopt;
}
#pragma endregion

#pragma region Server
MultiConnectServer::MultiConnectServer()
	: Server()
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