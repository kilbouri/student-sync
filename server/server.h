#pragma once

#include <iostream>
#include <functional>
#include <vector>

#include "../common/networkmessage/networkmessage.h"
#include "../win32includes.h"

class Server {
public:
	bool BindAndListen(std::string& ipAddress, int portNumber);

	virtual void Start() = 0;
	virtual void Stop(bool now = false) = 0;
	virtual bool IsStopRequested() = 0;
	virtual int GetConnectionCount() = 0;

	std::optional<std::string> GetHostname();
	std::optional<int> GetPort();

	void SetClientConnectedHandler(std::function<void(TCPSocket& client)> handler);
	void SetMessageReceivedHandler(std::function<bool(TCPSocket& client, const NetworkMessage message)> handler);
	void SetClientDisconnectedHandler(std::function<void(TCPSocket& client)> handler);

	void InvokeClientConnectedHandler(TCPSocket& client) noexcept;
	std::optional<bool> InvokeMessageReceivedHandler(TCPSocket& client, const NetworkMessage message) noexcept;
	void InvokeClientDisconnectedHandler(TCPSocket& client) noexcept;

	~Server();

protected:
	TCPSocket listenSocket;

	std::optional<std::function<void(TCPSocket&)>> connectHandler;
	std::optional<std::function<bool(TCPSocket&, const NetworkMessage)>> messageHandler;
	std::optional<std::function<void(TCPSocket&)>> disconnectHandler;
};

/// <summary>
/// A simple server which accepts only one connection
/// at a time This isn't very useful, but it is much simpler
/// compared to MultiConnectServer, so it may be useful
/// for testing new behaviour.
/// </summary>
class SingleConnectServer : public Server {
public:
	SingleConnectServer();

	// Inherited via Server
	void Start() override;
	void Stop(bool now) override;
	bool IsStopRequested() override;
	int GetConnectionCount() override;

private:
	TCPSocket currentClient;
};

class MultiConnectServer : public Server {
public:
	MultiConnectServer(); // todo: optional connection limit

	// Inherited via Server
	void Start() override;
	void Stop(bool now) override;
	bool IsStopRequested() override;
	int GetConnectionCount() override;

private:
	std::vector<TCPSocket> currentClients;
	std::unordered_map<SOCKET, std::vector<byte>> clientReadBuffers;

	/// <summary>
	/// Does all cleanup required to close/drop a connection, including
	/// releasing any buffers, closing the connection, and removing the
	/// connection from the current client list
	/// </summary>
	std::vector<TCPSocket>::iterator EndConnection(std::vector<TCPSocket>::iterator& client);
};