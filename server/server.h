#pragma once

#include <iostream>
#include <functional>
#include <vector>

#include "../common/networkmessage/networkmessage.h"
#include "../common/task/task.h"
#include "../win32includes.h"
#include <future>

/// <summary>
/// Base class for all Server implementations. Provides basic server operations and facilities to script
/// server-client conversation.
/// </summary>
struct Server {
	using Job = std::suspend_always;

	/// <summary>
	/// A context object which provides methods for connection handlers to communicate with the server
	/// in a safe, extendable manner.
	/// </summary>
	struct ConnectionContext {
		/// <summary>
		/// Tells the server that the handler would like to send the provided message.
		/// </summary>
		virtual Server::Job Send(NetworkMessage message) = 0;

		/// <summary>
		/// Tells the server that the handler would like to receive another message.
		/// </summary>
		Server::Job NextMessage();
		virtual Server::Job DoReceive() = 0;

		std::optional<NetworkMessage> GetLatestMessage();

	protected:
		std::optional<NetworkMessage> latestMessage;
	};

	bool BindAndListen(std::string& ipAddress, int portNumber);

	virtual void Start() = 0;
	virtual void Stop(bool now = false) = 0;
	virtual bool IsStopRequested() = 0;
	virtual int GetConnectionCount() = 0;

	std::optional<std::string> GetHostname();
	std::optional<int> GetPort();

	using ConnectionHandlerFunc = std::function<Task<void>(ConnectionContext&)>;
	void SetConnectionHandler(ConnectionHandlerFunc handler);

	~Server();

protected:
	TCPSocket listenSocket;
	std::optional<ConnectionHandlerFunc> connectionHandler;
};

/// <summary>
/// A simple server which accepts only one connection
/// at a time This isn't very useful, but it is much simpler
/// compared to MultiConnectServer, so it may be useful
/// for testing new behaviour.
/// </summary>
struct SingleConnectServer : public Server {
	struct ConnectionContext : Server::ConnectionContext {
		Server::Job Send(NetworkMessage message) override;
		Server::Job DoReceive() override;

		ConnectionContext(SingleConnectServer* server, TCPSocket socket);

	private:
		TCPSocket clientSocket;
		SingleConnectServer* server;
	};

	SingleConnectServer();

	// Inherited via Server
	void Start() override;
	void Stop(bool now) override;
	bool IsStopRequested() override;
	int GetConnectionCount() override;

private:
	std::optional<ConnectionContext> currentConnection;
	std::optional<TCPSocket> currentClientSocket;
};

#if 0
class MultiConnectServer : public Server {
public:
	MultiConnectServer(); // todo: optional connection limit

	// Inherited via Server
	void Start() override;
	void Stop(bool now) override;
	bool IsStopRequested() override;
	int GetConnectionCount() override;

private:
	std::vector<ConnectionContext> connections;


	std::vector<TCPSocket> currentClients;
	std::unordered_map<SOCKET, std::vector<byte>> clientReadBuffers;

	/// <summary>
	/// Does all cleanup required to close/drop a connection, including
	/// releasing any buffers, closing the connection, and removing the
	/// connection from the current client list
	/// </summary>
	std::vector<TCPSocket>::iterator EndConnection(std::vector<TCPSocket>::iterator& client);
};

#endif