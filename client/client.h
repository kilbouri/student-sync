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

	// Updated methods for handling video stream requests and responses
	bool RequestVideoStream();

	bool StartVideoStream();
	bool SendVideoFrame();
	bool EndVideoStream();

	~Client();
private:
	TCPSocket socket;
};