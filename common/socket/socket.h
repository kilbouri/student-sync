#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>

#include "../../win32includes.h"

/// <summary>
/// This serves as a thin wrapper around a platform-specific socket.
/// </summary>
class Socket {
public:
	enum class Direction {
		Send,
		Receive,
		Both
	};

	enum class IOResult {
		Success,
		ConnectionClosed,
		Error
	};

	bool IsValid() const;
	bool Shutdown(Direction direction);
	bool Close();

	int WriteBytes(const byte* buffer, int nBytes);
	int ReadBytes(byte* buffer, int nBytes);

	IOResult WriteAllBytes(const byte* buffer, size_t nBytes);
	IOResult ReadAllBytes(byte* buffer, size_t nBytes);

	/// <summary>
	/// HACK: this provides direct access to the platform-specific
	/// socket.
	/// </summary>
	/// <returns>The underlying socket</returns>
	const SOCKET GetDescriptor() const;

protected:
	Socket(SOCKET s);
	std::shared_ptr<SOCKET> underlyingSocket;
};

/// <summary>
/// Represents a TCP platform-specific socket
/// </summary>
class TCPSocket : public Socket {
public:
	struct SocketInfo;

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

struct TCPSocket::SocketInfo {
	std::string Address;
	int Port;
};