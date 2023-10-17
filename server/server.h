#pragma once

#include <iostream>
#include <vector>

#include "../win32includes.h"

class Server {
public:
	Server(int portNumber, bool netAttach = false);

	int Initialize();
	int Start();

	~Server();
private:
	const std::string port;
	const std::string ipAddress;
	SOCKET listenSocket;
};