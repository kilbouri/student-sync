#pragma once

#include <iostream>
#include <functional>
#include <vector>
#include <future>

#include "../win32includes.h"
#include "../common/networkmessage/networkmessage.h"
#include "../common/task/task.h"
#include "../serversession/serversession.h"

namespace StudentSync::Server {
	struct Server {

		Server(std::string& hostname, int port);

		void Run();
		void Stop();

		int GetConnectionCount() const;
		TCPSocket::SocketInfo GetServerInfo() const;

		~Server();
	private:
		TCPSocket serverSocket;
		std::vector<std::unique_ptr<Session>> sessions;
	};
}