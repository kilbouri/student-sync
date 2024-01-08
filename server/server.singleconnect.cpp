#include "server.h"

// included by server.cpp

#pragma region ConnectionContext
SingleConnectServer::ConnectionContext::ConnectionContext(SingleConnectServer* server, TCPSocket socket)
	: server{ server }, clientSocket{ socket } {}

Task<void> SingleConnectServer::ConnectionContext::Send(NetworkMessage message) {
	// in this server, we don't have to do any fanciness to queue the message until
	// a send opening or anything like that
	message.Send(this->clientSocket);
	co_return;
}

Task<std::optional<NetworkMessage>> SingleConnectServer::ConnectionContext::Recieve() {
	// in this server, we don't have to do any fanciness to queue a read until data is present
	co_return NetworkMessage::TryReceive(this->clientSocket);
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

		// if we don't have a connection handler, that means the server will do nothing
		// meaningful and should therefore immediately end the connection.
		if (!connectionHandler) {
			acceptResult->Close();
			continue;
		}

		currentClientSocket = acceptResult;
		currentConnection = std::make_shared<ConnectionContext>(this, *acceptResult);

		// We basically just synchronously wait for the handler coroutine to
		// be done (or the server be requested to stop).
		auto handler = (*connectionHandler)(*currentConnection);
		while (!handler.Done() && !IsStopRequested()) {
			handler.Resume();
		}

		currentClientSocket = std::nullopt;
		currentConnection = std::nullopt;
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