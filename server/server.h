#pragma once

#include <iostream>
#include <vector>

#include "../win32includes.h"

class Server {
public:
	Server(int portNumber);

	int Initialize();
	int Start();

	~Server();
private:
	std::string port;

	struct addrinfo hints;
	SOCKET listenSocket;
};