#pragma once

#include "../shared.h";

class Server {
public:
	Server(int portNumber);

	int Initialize();
	bool sendMessage(SOCKET socket, const std::string& message);
	std::string receiveMessage(SOCKET socket);
	int Start();

	~Server();
private:
	std::string port;

	struct addrinfo hints;
	SOCKET listenSocket;
};