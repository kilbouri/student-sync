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
	/// <summary>
	/// A context object which provides methods for connection handlers to communicate with the server
	/// in a safe, extendable manner.
	/// </summary>
	struct ConnectionContext {
		/// <summary>
		/// Tells the server that the handler would like to send the provided message.
		/// </summary>
		virtual Task<void> Send(NetworkMessage message) = 0;

		/// <summary>
		/// Tells the server that the handler would like to receive another message.
		/// </summary>
		virtual Task<std::optional<NetworkMessage>> Recieve() = 0;

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

	using ConnectionHandlerFunc = std::function<Task<void>(std::shared_ptr<ConnectionContext>)>;
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
		Task<void> Send(NetworkMessage message) override;
		Task<std::optional<NetworkMessage>> Recieve() override;

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
	std::optional<std::shared_ptr<ConnectionContext>> currentConnection;
	std::optional<TCPSocket> currentClientSocket;
};

struct MultiConnectServer : public Server {
	struct ConnectionContext : Server::ConnectionContext {
		Task<void> Send(NetworkMessage message) override;
		Task<std::optional<NetworkMessage>> Recieve() override;
	};

	 // todo: optional connection limit
	MultiConnectServer();

	// Inherited via Server
	void Start() override;
	void Stop(bool now) override;
	bool IsStopRequested() override;
	int GetConnectionCount() override;

private:
	// maps a socket to the context for the connection on that socket
	std::unordered_map<TCPSocket, ConnectionContext> connections;
};