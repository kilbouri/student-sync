#pragma once

#include <iostream>

#include "../win32includes.h"
#include "../common/socket/socket.h"
#include "../common/displaycapturer/displaycapturer.h"


class Client {
public:
	Client();

	bool Connect(std::string_view hostname, int portNumber);
	bool Disconnect();

	bool SendString(const std::string& str);
	bool SendNumber(int64_t number);

	// Updated methods for handling video stream requests and responses
	bool RequestVideoStream();
	bool HandleVideoStreamResponse();

	bool StartVideoStream();
	bool SendVideoFrame();
	bool EndVideoStream();

	~Client();
private:
	TCPSocket socket;
};