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

	bool IsValid();
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
	const SOCKET GetUnderlyingSocket() const;

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

	std::optional<std::string> GetPeerAddress();
	std::optional<int> GetPeerPort();

private:
	TCPSocket(SOCKET existing);
};