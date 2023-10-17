#pragma once

#include <iostream>
#include <vector>

#include "../win32includes.h"

class Server {
public:
	Server(std::string& ipAddress, int portNumber);

	std::string GetExternalAddress();
	int GetPortNumber();

	int Initialize();
	int Start();

	~Server();
private:
	const std::string ipAddress;
	int portNumber;
	SOCKET listenSocket;
};