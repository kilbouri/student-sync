#pragma once

#include "../shared.h"

class Client {
public:
	Client(std::string_view serverHostname, int serverPort);

	int Initialize();
	bool SendMessage(SOCKET socket, const std::string& message);
	bool SendMessage(SOCKET socket, int64_t message);
	std::string ReceiveMessage(SOCKET socket);
	int Connect();

	~Client();
private:
	std::string hostname;
	std::string port;

	struct addrinfo hints;
	struct addrinfo* serverAddress;
	SOCKET connectSocket;
};