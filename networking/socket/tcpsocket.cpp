#include "tcpsocket.hpp"

namespace StudentSync::Networking {
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

		SOCKET socket = accept(*underlyingSocket, NULL, NULL);
		if (socket == INVALID_SOCKET) {
			return std::nullopt;
		}

		return std::optional<TCPSocket>{ std::in_place, TCPSocket(socket) };
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

	static inline std::optional<std::string> GetAddress(struct sockaddr_in);
	static std::optional<TCPSocket::SocketInfo> GetSocketInfo(SOCKET socket, int(*fetchFunc)(SOCKET, sockaddr*, int*)) {
		sockaddr_in socketInfo = { 0 };
		int socketInfoSize = sizeof(socketInfo);

		if (fetchFunc(socket, reinterpret_cast<sockaddr*>(&socketInfo), &socketInfoSize) == SOCKET_ERROR) {
			return std::nullopt;
		}

		return TCPSocket::SocketInfo{
			.Address = GetAddress(socketInfo).value_or("Unknown"),
			.Port = ntohs(socketInfo.sin_port)
		};
	}

	static inline std::optional<std::string> GetAddress(struct sockaddr_in socketInfo) {
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

	std::optional<TCPSocket::SocketInfo> TCPSocket::GetBoundSocketInfo() const {
		return GetSocketInfo(this->GetDescriptor(), &getsockname);
	}

	std::optional<TCPSocket::SocketInfo> TCPSocket::GetPeerSocketInfo() const {
		return GetSocketInfo(this->GetDescriptor(), &getpeername);
	}
}