#pragma once

#include <iostream>

#include "../win32includes.h"
#include "../common/displaycapturer/displaycapturer.h"


class Client {
public:
	Client(std::string_view serverHostname, int serverPort);

	bool SendString(const std::string& str);
	bool SendNumber(int64_t number);
	bool SendScreenshot(const DisplayCapturer::Format format);

	bool Initialize();
	bool Connect();

	~Client();
private:
	std::string hostname;
	std::string port;

	struct addrinfo hints;
	struct addrinfo* serverAddress;
	SOCKET serverSocket;
};