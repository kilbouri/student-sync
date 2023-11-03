#include "socket.h"
#include "../../win32includes.h"

// Constructor for creation from a platform socket
Socket::Socket(SOCKET s) : underlyingSocket{ new SOCKET(s) }, referenceCount{ new size_t(1) } {}

// Copy constructor
Socket::Socket(const Socket& other) :
	underlyingSocket{ other.underlyingSocket },
	referenceCount{ other.referenceCount }
{
	(*referenceCount)++;
}

// Copy assignment operator
Socket& Socket::operator=(const Socket& other) {
	if (this == &other) {
		return *this;
	}

	if (underlyingSocket && referenceCount) {
		(*referenceCount)--;

		if (*referenceCount == 0) {
			Close();
			delete underlyingSocket;
			delete referenceCount;
		}
	}

	// Copy data from the other object
	underlyingSocket = other.underlyingSocket;
	referenceCount = other.referenceCount;
	(*referenceCount)++;

	return *this;
}

// Move constructor
Socket::Socket(Socket&& other) noexcept :
	underlyingSocket{ std::exchange(other.underlyingSocket, nullptr) },
	referenceCount{ std::exchange(other.referenceCount, nullptr) }
{}

Socket& Socket::operator=(Socket&& other) noexcept {
	if (this == &other) {
		return *this;
	}

	if (underlyingSocket && referenceCount) {
		(*referenceCount)--;

		if (*referenceCount == 0) {
			Close();
			delete underlyingSocket;
			delete referenceCount;
		}
	}

	// Move data from the other object
	underlyingSocket = std::exchange(other.underlyingSocket, nullptr);
	referenceCount = std::exchange(other.referenceCount, nullptr);

	return *this;
}

Socket::~Socket() {
	// if either of these are unset, that pretty much means this object was moved out of
	if (underlyingSocket && referenceCount) {
		*referenceCount -= 1;

		if (*referenceCount == 0) {
			Close();

			delete underlyingSocket;
			delete referenceCount;
		}
	}
}

bool Socket::Shutdown(SocketDirection direction) {
	if (!IsValid()) {
		return false;
	}

	int how;
	switch (direction) {
		case SocketDirection::Both: how = SD_BOTH; break;
		case SocketDirection::Send: how = SD_SEND; break;
		case SocketDirection::Receive: how = SD_RECEIVE; break;
		default: return false;
	}

	return shutdown(*underlyingSocket, how) != SOCKET_ERROR;
}

bool Socket::Close() {
	// closing an invalid socket is vacuous.
	if (*underlyingSocket == INVALID_SOCKET) {
		return true;
	}

	int rc = closesocket(*underlyingSocket);
	if (rc != SOCKET_ERROR) {
		*underlyingSocket = INVALID_SOCKET;
	}

	return rc != SOCKET_ERROR;
}

std::optional<size_t> Socket::WriteBytes(const byte* buffer, size_t nBytes) {
	if (!IsValid()) {
		return std::nullopt;
	}

	// just because we accept a size_t, does not mean we can write that many bytes.
	// We take the smaller of nBytes and the max value for an int.
	int bytesToWrite = static_cast<int>(std::min(nBytes, static_cast<size_t>(std::numeric_limits<int>::max())));
	int rc = send(*underlyingSocket, reinterpret_cast<const char*>(buffer), bytesToWrite, 0);
	if (rc == SOCKET_ERROR) {
		return std::nullopt;
	}

	return rc;
}

std::optional<size_t> Socket::ReadBytes(byte* buffer, size_t nBytes) {
	if (!IsValid()) {
		return std::nullopt;
	}

	// just because we accept a size_t, does not mean we can read that many bytes.
	// We take the smaller of nBytes and the max value for an int.
	int bytesToRead = static_cast<int>(std::min(nBytes, static_cast<size_t>(std::numeric_limits<int>::max())));
	int rc = recv(*underlyingSocket, reinterpret_cast<char*>(buffer), bytesToRead, 0);
	if (rc == SOCKET_ERROR) {
		return std::nullopt;
	}

	return rc;
}

bool Socket::WriteAllBytes(const byte* buffer, size_t nBytes) {
	size_t bytesSent = 0;
	do {
		std::optional<int> writeResult = WriteBytes(buffer + bytesSent, nBytes - bytesSent);
		if (!writeResult) {
			int errorCode = GetLastError();
			return false;
		}

		bytesSent += *writeResult;
	} while (bytesSent < nBytes);

	return true;
}

bool Socket::ReadAllBytes(byte* buffer, size_t nBytes) {
	size_t bytesRead = 0;
	do {
		std::optional<int> readResult = ReadBytes(buffer + bytesRead, nBytes - bytesRead);
		if (!readResult) {
			return false;
		}

		bytesRead += *readResult;
	} while (bytesRead < nBytes);

	return true;
}

bool Socket::IsValid() {
	return *underlyingSocket != INVALID_SOCKET;
}

TCPSocket::TCPSocket() : Socket(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) {}
TCPSocket::TCPSocket(SOCKET existing) : Socket(existing) {}

bool TCPSocket::Bind(std::string_view hostname, int portNumber) {
	if (!this->IsValid()) {
		return false;
	}

	struct addrinfo* addressOptions = nullptr;
	struct addrinfo hints = { .ai_family = AF_INET, .ai_socktype = SOCK_STREAM, .ai_protocol = IPPROTO_TCP };

	std::string address = std::string(hostname);
	std::string port = std::to_string(portNumber);

	if (getaddrinfo(address.c_str(), port.c_str(), &hints, &addressOptions) != 0) {
		return false;
	}

	struct addrinfo* foundAddress = nullptr;
	for (struct addrinfo* current = addressOptions; current != nullptr; current = current->ai_next) {
		if (current->ai_family == hints.ai_family && current->ai_socktype == hints.ai_socktype && current->ai_protocol == hints.ai_protocol) {
			foundAddress = current;
			break;
		}
	}

	// didn't find an address, either because we got no results (addressOptions == nullptr),
	// or because none of the results matched the underlying socket details.
	if (foundAddress == nullptr) {
		if (addressOptions != nullptr) {
			freeaddrinfo(addressOptions);
			addressOptions = nullptr;
		}

		return false;
	}

	// try to bind to the found address
	int bindResult = bind(*underlyingSocket, addressOptions->ai_addr, static_cast<int>(addressOptions->ai_addrlen));

	freeaddrinfo(addressOptions);
	addressOptions = nullptr;

	return bindResult != SOCKET_ERROR;
}

bool TCPSocket::Listen(int queueLength) {
	if (!IsValid()) {
		return false;
	}

	return listen(*underlyingSocket, queueLength) != SOCKET_ERROR;
}

std::optional<TCPSocket> TCPSocket::Accept() {
	if (!IsValid()) {
		return std::nullopt;
	}

	SOCKET clientSocket = accept(*underlyingSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET) {
		return std::nullopt;
	}

	return std::optional<TCPSocket>{ std::in_place, TCPSocket(clientSocket) };
}

bool TCPSocket::Connect(std::string_view hostname, int portNumber) {
	if (!IsValid()) {
		return false;
	}

	std::string host = std::string(hostname);
	std::string port = std::to_string(portNumber);

	struct addrinfo* addressOptions = nullptr;
	struct addrinfo hints = { .ai_family = AF_INET, .ai_socktype = SOCK_STREAM, .ai_protocol = IPPROTO_TCP };

	if (getaddrinfo(host.c_str(), port.c_str(), &hints, &addressOptions) != 0) {
		return false;
	}

	struct addrinfo* foundAddress = nullptr;
	for (struct addrinfo* current = addressOptions; current != nullptr; current = current->ai_next) {
		if (current->ai_family == hints.ai_family && current->ai_socktype == hints.ai_socktype && current->ai_protocol == hints.ai_protocol) {
			foundAddress = current;
			break;
		}
	}

	// didn't find an address, either because we got no results (addressOptions == nullptr),
	// or because none of the results matched the underlying socket details.
	if (foundAddress == nullptr) {
		if (addressOptions != nullptr) {
			freeaddrinfo(addressOptions);
			addressOptions = nullptr;
		}

		return false;
	}

	int connectResult = connect(*underlyingSocket, foundAddress->ai_addr, static_cast<int>(foundAddress->ai_addrlen));

	freeaddrinfo(addressOptions);
	addressOptions = nullptr;

	return connectResult != SOCKET_ERROR;
}

std::optional<std::string> TCPSocket::GetBoundAddress() {
	// we're gonna try this...
	sockaddr_in socketInfo = { 0 };
	int socketInfoSize = sizeof(socketInfo);

	if (getsockname(*underlyingSocket, reinterpret_cast<sockaddr*>(&socketInfo), &socketInfoSize) == SOCKET_ERROR) {
		return std::nullopt;
	}

	char addressBuffer[INET_ADDRSTRLEN] = { 0 };
	if (inet_ntop(AF_INET, &(socketInfo.sin_addr), addressBuffer, sizeof(addressBuffer)) == nullptr) {
		return std::nullopt;
	}

	std::string addressString = std::string(addressBuffer);
	size_t portStartIndex = addressString.rfind(':');

	// port number got included, we don't want it here
	if (portStartIndex != std::string::npos) {
		// remove port number
		while (addressString.back() != ':') {
			addressString.pop_back();
		}

		// remove the ':'
		addressString.pop_back();
	}

	return addressString;
}

std::optional<int> TCPSocket::GetBoundPort() {
	struct sockaddr_in socketInfo = { 0 };
	int socketInfoLength = sizeof(socketInfo);

	if (getsockname(*underlyingSocket, (struct sockaddr*)&socketInfo, &socketInfoLength) != 0) {
		return std::nullopt;
	}

	return ntohs(socketInfo.sin_port);
}