#pragma once

#include <iostream>

#include "../win32includes.h"
#include "../common/displaycapturer/displaycapturer.h"


class Client {
public:
	Client(std::string_view serverHostname, int serverPort);

	void SendString(const std::string& str);
	void SendNumber(int64_t number);
	void SendScreenshot(const DisplayCapturer::Format format);

	int Initialize();
	int Connect();

	~Client();
private:
	std::string hostname;
	std::string port;

	struct addrinfo hints;
	struct addrinfo* serverAddress;
	SOCKET serverSocket;
};