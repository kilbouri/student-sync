#include "client.hpp"

#include <fstream>
#include <optional>
#include <string>

using namespace StudentSync::Networking;

namespace StudentSync::Client {
	Client::Client(std::string& hostname, int port, std::function<void(Connection)> handler)
		: socket(TCPSocket{})
		, handler{ handler }
	{
		if (!socket.Connect(hostname, port)) {
			throw "Failed to connect to server at " + hostname + ":" + std::to_string(port);
		}
	}

	void Client::Run() {
		// wow much complicated!

		this->handler(Connection{
			.socket = this->socket
		});
	}

	bool Client::Stop() {
		return socket.Close();
	}

	TCPSocket::SocketInfo Client::GetClientInfo() {
		auto info = socket.GetBoundSocketInfo();
		if (!info) {
			throw "Failed to get local socket info";
		}

		return *info;
	}

	TCPSocket::SocketInfo Client::GetRemoteInfo() {
		auto info = socket.GetPeerSocketInfo();
		if (!info) {
			throw "Failed to get remote socket info";
		}

		return *info;
	}

	Client::~Client() {
		Stop();
	}
}