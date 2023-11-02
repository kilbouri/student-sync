#pragma once

#include <iostream>
#include <functional>
#include <vector>

#include "../common/message/message.h"
#include "../win32includes.h"

// todo: use threads to handle each connection so we can have concurrent connections
// todo: create a mechanism to run some callback when a connection is created/deleted/etc

class Server {
public:
	Server();

	bool BindAndListen(std::string& ipAddress, int portNumber);
	void Start();
	void Stop();
	bool IsStopRequested();

	std::optional<std::string> GetHostname();
	std::optional<int> GetPort();

	void SetMessageHandler(std::function<bool(TCPSocket& client, const Message message)> handler);

	~Server();

private:
	TCPSocket listenSocket;
	TCPSocket currentClient;

	std::optional<std::function<bool(TCPSocket&, const Message)>> messageHandler;
};