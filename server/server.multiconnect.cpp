#include "server.h"

// included by server.cpp

#pragma region Connection
void MultiConnectServer::Connection::Terminate() {
	this->socket.Close();
	this->future.wait();
}

bool MultiConnectServer::Connection::IsClosed() const {
	return !this->socket.IsValid();
}
MultiConnectServer::Connection::Connection(TCPSocket socket, ConnectionHandlerFunc& handlerFunc)
	: socket{ socket }, future{ Connection::CreateFuture(this->socket, handlerFunc) }
{}

std::future<void> MultiConnectServer::Connection::CreateFuture(TCPSocket socket, ConnectionHandlerFunc& handler) {

	return std::async(std::launch::async, [socket, handler]() -> void {
		TCPSocket constFix{ socket };

		auto ctx = std::make_shared<ConnectionContext>(socket);
		auto task = handler(ctx);

		while (!task.Done()) {
			task.Resume();
		}

		// ensure the socket gets closed
		constFix.Close();
	});
}
#pragma endregion

#pragma region ConnectionContext
MultiConnectServer::ConnectionContext::ConnectionContext(TCPSocket socket)
	: clientSocket{ socket } {}

Task<void> MultiConnectServer::ConnectionContext::Send(NetworkMessage message) {
	message.Send(this->clientSocket);
	co_return;
}

Task<std::optional<NetworkMessage>> MultiConnectServer::ConnectionContext::Recieve() {
	co_return NetworkMessage::TryReceive(this->clientSocket);
}
#pragma endregion

#pragma region Server
MultiConnectServer::MultiConnectServer()
	: Server()
{}

void MultiConnectServer::DoRun() {
	ConnectionHandlerFunc connectionHandler = *connectionHandlerFunc;
	while (!IsStopRequested()) {
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(listenSocket.GetDescriptor(), &readSet);

		// 60 second timeout
		timeval timeout{
			.tv_sec = 60,
			.tv_usec = 0
		};

		// clean up any exited connections (none on first iteration)
		for (auto iterator = connections.begin(); iterator != connections.end();) {
			if (iterator->IsClosed()) {
				iterator = connections.erase(iterator);
				continue;
			}

			iterator++;
		}

		// todo: check return code here
		int selectCode = select(0, &readSet, nullptr, nullptr, &timeout);
		if (selectCode == SOCKET_ERROR) {
			continue;
		}

		// if we got here because of an incoming connection, accept it
		if (!FD_ISSET(listenSocket.GetDescriptor(), &readSet)) {
			continue;
		}

		std::optional<TCPSocket> clientSocket = listenSocket.Accept();
		if (!clientSocket) {

			continue;
		}

		connections.emplace_back(Connection{ *clientSocket, connectionHandler });
	}
}

void MultiConnectServer::Stop(bool now) {
	listenSocket.Close();

	if (now) {
		for (auto& connection : connections) {
			connection.Terminate();
		}
	}
}

bool MultiConnectServer::IsStopRequested() {
	return !listenSocket.IsValid();
}

int MultiConnectServer::GetConnectionCount() {
	return connections.size();
}
#pragma endregion