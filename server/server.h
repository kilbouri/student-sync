#pragma once

#include <iostream>
#include <functional>
#include <vector>
#include <future>

#include "../common/networkmessage/networkmessage.h"
#include "../common/task/task.h"
#include "../win32includes.h"

struct Server {
	struct Connection;
	using ConnectionHandler = std::function<void(Connection&)>;

	/// <summary>
	/// Creates a new server which will be bound to hostname:port. The provided handler
	/// will receive references to connections each time a new connection is established.
	/// The handler MUST assume that any Connection references become invalid when the
	/// server that created them is either stopped or destroyed. The Connection reference
	/// also becomes invalid the moment they call Connection::Terminate.
	/// </summary>
	Server(std::string& hostname, int port, ConnectionHandler handler);

	void Run();
	void Stop(bool force = false);

	bool IsRunning() const;
	int GetConnectionCount() const;
	TCPSocket::SocketInfo GetServerInfo() const;

	~Server();
private:
	ConnectionHandler connectionHandler;
	TCPSocket serverSocket;
	std::vector<Connection> connections;
};

struct Server::Connection {
	// This reference is always safe. When the server is stopped or destroyed,
	// it will also destroy all of its Connections. Thus if a Connection exists,
	// then the server also still exists.
	TCPSocket socket = TCPSocket::InvalidSocket();
	unsigned long identifier;
	std::string username;

	void Terminate();
};