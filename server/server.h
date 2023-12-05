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

	std::optional<std::string> GetHostname();
	std::optional<int> GetPort();

	void SetClientConnectedHandler(std::function<void(TCPSocket& client)> handler);
	void SetMessageReceivedHandler(std::function<bool(TCPSocket& client, const NetworkMessage message)> handler);
	void SetClientDisconnectedHandler(std::function<void(TCPSocket& client)> handler);

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
/// to use compared to MultiConnectServer, so it may be useful
/// for testing new behaviour.
/// </summary>
class SingleConnectServer : public Server {
public:
	SingleConnectServer();

	// Inherited via Server
	void Start() override;
	void Stop(bool now) override;
	bool IsStopRequested() override;

private:
	TCPSocket currentClient;
};

class MultiConnectServer : public Server {
};