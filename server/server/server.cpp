#include "server.hpp"

#include <string>
#include <optional>
#include <fstream>
#include <chrono>
#include <iostream>
#include <thread>
#include <wx/wx.h>
#include <coroutine>

using namespace StudentSync::Networking;

namespace StudentSync::Server {
	Server::Server(std::string& hostname, int port, std::shared_ptr<Session::EventDispatcher> eventDispatcher)
		: serverSocket{ TCPSocket{} }
		, dispatcher{ eventDispatcher }
	{
		if (!serverSocket.Bind(hostname, port)) {
			throw "Server failed to bind to " + hostname + ":" + std::to_string(port);
		}
	}

	void Server::Run() {
		unsigned long sessionId = 0;

		if (!serverSocket.Listen(TCPSocket::MaxConnectionQueueLength)) {
			throw "Server failed to listen";
		}

		fd_set readSet = {};
		while (serverSocket.IsValid()) {
			std::optional<TCPSocket> client = serverSocket.Accept();
			if (!client) {
				continue;
			}

			sessions.emplace_back(std::make_shared<Session>(
				sessionId,
				std::move(*client),
				dispatcher
			));
		}
	}

	void Server::Stop() {
		serverSocket.Close();

		for (auto& session : sessions) {
			session->Terminate();
		}

		for (auto& session : sessions) {
			session->Join();
		}

		// don't forget to clear the session list :)
		sessions.clear();
	}

	std::optional<std::shared_ptr<Session>> Server::GetSession(unsigned long sessionId) const {
		for (auto const& session : sessions) {
			if (session->identifier == sessionId) {
				return session;
			}
		}

		return std::nullopt;
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