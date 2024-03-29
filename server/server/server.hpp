#pragma once

#include <iostream>
#include <functional>
#include <vector>
#include <future>

#include "../session/session.hpp"
#include "../../networking/socket/tcpsocket.hpp"

namespace StudentSync::Server {
	struct Server {

		Server(std::string& hostname, int port, std::shared_ptr<Session::EventDispatcher> eventDispatcher);

		void Run();
		void Stop();

		std::optional<std::shared_ptr<Session>> GetSession(unsigned long sessionId) const;
		Networking::TCPSocket::SocketInfo GetServerInfo() const;

		~Server();
	private:
		Networking::TCPSocket serverSocket;

		std::shared_ptr<Session::EventDispatcher> dispatcher;
		std::vector<std::shared_ptr<Session>> sessions;
	};
}