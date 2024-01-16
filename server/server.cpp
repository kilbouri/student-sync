#include "server.h"

#include <string>
#include <optional>
#include <fstream>
#include <chrono>
#include <iostream>
#include <thread>
#include <wx/wx.h>
#include <coroutine>

namespace StudentSync::Server {
	Server::Server(std::string& hostname, int port) : serverSocket{ TCPSocket{} }
	{
		if (!serverSocket.Bind(hostname, port)) {
			throw "Server failed to bind to " + hostname + ":" + std::to_string(port);
		}
	}

	void Server::Run() {
		if (!serverSocket.Listen(TCPSocket::MaxConnectionQueueLength)) {
			throw "Server failed to listen";
		}

		fd_set readSet = {};
		while (serverSocket.IsValid()) {
			std::optional<TCPSocket> client = serverSocket.Accept();
			if (!client) {
				continue;
			}

			sessions.emplace_back(std::make_unique<Session>(std::move(*client)));
		}
	}

	void Server::Stop() {
		serverSocket.Close();

		for (auto& session : sessions) {
			session->SetState(Session::State::Terminated);
		}

		for (auto& session : sessions) {
			session->Join();
		}

		// don't forget to clear the session list :)
		sessions.clear();
	}

	int Server::GetConnectionCount() const {
		return sessions.size();
	}

	TCPSocket::SocketInfo Server::GetServerInfo() const {
		return serverSocket
			.GetBoundSocketInfo()
			.value_or(TCPSocket::SocketInfo{ .Address = "Unknown", .Port = 0 });
	}

	Server::~Server() {
		Stop();
	}
}