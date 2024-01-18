#pragma once

#include "socket.hpp"

namespace StudentSync::Networking {
	/// <summary>
	/// Represents a TCP platform-specific socket
	/// </summary>
	struct TCPSocket : public Socket {
		struct SocketInfo {
			std::string Address;
			int Port;
		};

		TCPSocket();

		static constexpr int MaxConnectionQueueLength = SOMAXCONN;
		static const TCPSocket InvalidSocket() { return TCPSocket(INVALID_SOCKET); }

		/// <summary>
		/// Connects to a remote host at the provided hostname and port
		/// </summary>
		bool Connect(std::string_view hostname, int portNumber);

		/// <summary>
		/// Binds the socket to the hostname and port
		/// </summary>
		bool Bind(std::string_view hostname, int portNumber);
		bool Listen(int queueLength);
		std::optional<TCPSocket> Accept();

		std::optional<SocketInfo> GetBoundSocketInfo() const;
		std::optional<SocketInfo> GetPeerSocketInfo() const;

	private:
		TCPSocket(SOCKET existing);
	};
}