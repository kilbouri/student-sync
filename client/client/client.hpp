#pragma once

#include <iostream>
#include <functional>

#include "../../networking/socket/tcpsocket.hpp"

namespace StudentSync::Client {
	struct Client {
		struct Connection {
			Networking::TCPSocket socket;
		};

		Client(std::string& hostname, int port, std::function<void(Connection)> handler);

		void Run();
		bool Stop();

		Networking::TCPSocket::SocketInfo GetClientInfo();
		Networking::TCPSocket::SocketInfo GetRemoteInfo();

		~Client();
	private:
		bool connected;
		Networking::TCPSocket socket;
		std::function<void(Connection)> handler;
	};
}