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
	Server(std::string& ipAddress, int portNumber);

	std::string GetExternalAddress();
	int GetPortNumber();

	bool Initialize();
	bool IsInitialized();

	bool Start();

	void Stop();
	bool IsStopRequested();

	void SetMessageHandler(std::function<bool(const SOCKET, const Message)> handler);

	~Server();

private:
	const std::string ipAddress;
	int portNumber;

	volatile SOCKET listenSocket;
	std::optional<std::function<bool(const SOCKET, const Message)>> messageHandler;

};