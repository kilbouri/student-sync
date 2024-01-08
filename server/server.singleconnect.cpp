#include "server.h"

// included by server.cpp

#pragma region ConnectionContext
SingleConnectServer::ConnectionContext::ConnectionContext(SingleConnectServer* server, TCPSocket socket)
	: server{ server }, clientSocket{ socket } {}

Server::Job SingleConnectServer::ConnectionContext::Send(NetworkMessage message) {
	// in this server, we don't have to do any fanciness to queue the message until
	// a send opening or anything like that
	message.Send(this->clientSocket);
	return {};
}

Server::Job SingleConnectServer::ConnectionContext::DoReceive() {
	// in this server, we don't have to do any fanciness to wait for a read opening
	this->latestMessage = NetworkMessage::TryReceive(this->clientSocket);
	return {};
}
#pragma endregion

#pragma region Server
SingleConnectServer::SingleConnectServer()
	: Server{}, currentConnection{ std::nullopt }, currentClientSocket{ TCPSocket::InvalidSocket() }
{}

void SingleConnectServer::Start() {
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

		// Fire off the connection handler and await its completion
		currentClientSocket = acceptResult;
		currentConnection = ConnectionContext{ this, *acceptResult };

		auto handler = (*connectionHandler)(*currentConnection);
		while (!handler.Done() && !IsStopRequested()) {
			handler.Resume();
		}

		// Ensure the client gets closed if the handler did not close it
		acceptResult->Close();
	}
}

void SingleConnectServer::Stop(bool now) {
	// prevent new connections
	listenSocket.Close();

	// kill current connection, if there is any
	if (now && currentConnection) {
		currentClientSocket->Close();
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