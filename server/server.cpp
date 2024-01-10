#include "server.h"

#include <string>
#include <optional>
#include <fstream>
#include <chrono>
#include <iostream>
#include <thread>
#include <wx/wx.h>
#include <coroutine>

Server::Server(std::string& hostname, int port, ConnectionHandler handler)
	: serverSocket{ TCPSocket{} }, connections{}, connectionHandler{ handler }
{
	if (!serverSocket.Bind(hostname, port)) {
		throw "Server failed to bind to " + hostname + ":" + std::to_string(port);
	}
}

void Server::Run() {
	static unsigned long identifier = 0;

	if (!serverSocket.Listen(TCPSocket::MaxConnectionQueueLength)) {
		throw "Server failed to listen";
	}

	fd_set readSet = {};
	while (serverSocket.IsValid()) {
		// clean up closed connections
		for (auto iter = connections.begin(); iter != connections.end();) {
			if (iter->socket.IsValid()) {
				++iter;
			}
			else {
				iter = connections.erase(iter);
			}
		}

		FD_ZERO(&readSet);
		FD_SET(serverSocket.GetDescriptor(), &readSet);

		timeval timeout{
			.tv_sec = 30,
			.tv_usec = 0
		};

		int selectResult = select(0, &readSet, nullptr, nullptr, &timeout);
		if (selectResult == 0 || selectResult == SOCKET_ERROR) {
			// error or timeout
			continue;
		}

		auto client = serverSocket.Accept();
		if (!client) {
			continue;
		}

		Connection& connection = connections.emplace_back<Connection>({
			.socket = *client,
			.identifier = ++identifier,
			.username = "Placeholder",
		});

		connectionHandler(connection);
	}
}

void Server::Stop(bool force) {
	serverSocket.Close();
	if (!force) {
		return;
	}

	// close connections
	for (auto& connection : connections) {
		connection.Terminate();
	}

	// clear connection list
	connections.clear();
}

bool Server::IsRunning() const {
	return serverSocket.IsValid();
}

int Server::GetConnectionCount() const {
	return connections.size();
}

TCPSocket::SocketInfo Server::GetServerInfo() const {
	return serverSocket.GetBoundSocketInfo().value_or(TCPSocket::SocketInfo{
			.Address = "Unknown",
			.Port = 0
	});
}

Server::~Server() {
	Stop(true);
}

void Server::Connection::Terminate() {
	socket.Close();
}