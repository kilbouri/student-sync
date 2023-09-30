#pragma once

#include "../shared.h";
#include <iostream>

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