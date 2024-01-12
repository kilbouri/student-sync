#pragma once

#include <iostream>
#include <functional>

#include "../win32includes.h"
#include "../common/socket/socket.h"
#include "../common/displaycapturer/displaycapturer.h"


class Client {
public:
	struct Connection;
	using ConnectionHandler = std::function<void(Connection)>;

	Client(std::string& hostname, int port, ConnectionHandler handler);

	void Run();
	bool Stop();

	TCPSocket::SocketInfo GetClientInfo();
	TCPSocket::SocketInfo GetRemoteInfo();

	~Client();
private:
	TCPSocket socket;
	ConnectionHandler handler;
};

struct Client::Connection {
	TCPSocket socket;
};