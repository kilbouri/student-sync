#pragma once

#include <string>
#include <vector>
#include <optional>

#include "../../win32includes.h"

/// <summary>
/// This serves as a thin wrapper around a platform-specific socket.
/// </summary>
class Socket {
public:
	enum SocketDirection {
		Send,
		Receive,
		Both
	};

	bool IsValid();
	bool Shutdown(SocketDirection direction);
	bool Close();

	std::optional<int> WriteBytes(const byte* buffer, int nBytes);
	std::optional<int> ReadBytes(byte* buffer, int nBytes);

	bool WriteAllBytes(const byte* buffer, int nBytes);
	bool ReadAllBytes(byte* buffer, int nBytes);

	Socket(const Socket& other); // Copy constructor
	Socket(Socket&& other) noexcept; // Move constructor

	Socket& operator=(const Socket& other); // Copy assignment
	Socket& operator=(Socket&& other) noexcept; // Move assignment

	~Socket();

protected:
	Socket(SOCKET s);
	SOCKET* underlyingSocket;

private:
	// we're gonna basically do ARC on the underlying socket
	size_t* referenceCount;
};

/// <summary>
/// Represents a TCP platform-specific socket
/// </summary>
class TCPSocket : public Socket {
public:
	TCPSocket();

	static constexpr int MaxConnectionQueueLength = SOMAXCONN;
	static const TCPSocket InvalidSocket() { return TCPSocket(INVALID_SOCKET); }

	bool Connect(std::string_view hostname, int portNumber);

	bool Bind(std::string_view hostname, int portNumber);
	bool Listen(int queueLength);
	std::optional<TCPSocket> Accept();

	std::optional<std::string> GetBoundAddress();
	std::optional<int> GetBoundPort();

private:
	TCPSocket(SOCKET existing);
};