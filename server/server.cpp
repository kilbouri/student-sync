#include "server.h"

#include "../common/networkmessage/networkmessage.h"
#include "../common/generator/generator.h"
#include "../common/task/task.h"

#include <string>
#include <optional>
#include <fstream>
#include <chrono>
#include <iostream>
#include <thread>
#include <wx/wx.h>
#include <coroutine>

#include "server.singleconnect.cpp"
#include "server.multiconnect.cpp"

// The rationale for using coroutines in the server, instead of (for example) dedicated threads,
// is that the server must be capable of handling hundreds of concurrent connections, AND that most
// of those concurrent connections will be inactive most of the time. That means creating a thread
// for every connection would rapidly waste threads.
//
// It also has a nice side effect of making the connection handlers very, very nice to write for the
// consuming code.

#pragma region Server
bool Server::BindAndListen(std::string& ipAddress, int portNumber) {
	return listenSocket.Bind(ipAddress, portNumber) && listenSocket.Listen(TCPSocket::MaxConnectionQueueLength);
}

std::optional<std::string> Server::GetHostname() {
	return listenSocket.GetBoundAddress();
}

std::optional<int> Server::GetPort() {
	return listenSocket.GetBoundPort();
}

void Server::SetConnectionHandler(ConnectionHandlerFunc handler) {
	this->connectionHandlerFunc = handler;
}

void Server::Run() {
	if (!connectionHandlerFunc) {
		throw "ConnectionHandler not set before calling Server::Run()!";
	}

	DoRun();
}

Server::~Server() {
	listenSocket.Close();
}
#pragma endregion