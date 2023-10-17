#include "server.h"

#include <string>
#include <optional>
#include <fstream>

#include "../common/message/message.h"

void HandleConnection(SOCKET server, SOCKET client);

/**
 * Creates a server that will listen on the specified interface and port
 */
Server::Server(std::string& ipAddress, int portNumber)
	: ipAddress{ std::string(ipAddress) }, portNumber{ portNumber }, listenSocket{ INVALID_SOCKET }
{}

std::string Server::GetExternalAddress() {
	return std::string(ipAddress);
}

int Server::GetPortNumber() {
	if (listenSocket == INVALID_SOCKET) {
		// This shouldn't happen unless Initialize() wasn't called first.
		// Which makes this a pretty exceptional circumstance.
		throw "GetPortNumber called while Server has invalid socket";
	}

	// if port is non-zero, and we are successfully bound, then the port is the same
	if (portNumber != 0) {
		return portNumber;
	}

	// if the port is zero, and we are successfully bound, then the port is unknown and must be retrieved
	
	struct sockaddr_in socketInfo = {};
	int socketInfoLength = sizeof(socketInfo);

	if (getsockname(listenSocket, (struct sockaddr*)&socketInfo, &socketInfoLength) != 0) {
		throw "Failed to get server socket info";
	}

	portNumber = ntohs(socketInfo.sin_port);
	return portNumber;
}

/**
 * Prepares the server to start listening for connections.
 */
int Server::Initialize() {
	// while this should never be called multiple times, can't hurt to make sure the old socket gets cleaned
	if (listenSocket != INVALID_SOCKET) {
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
	}

	struct addrinfo hints = { .ai_family = AF_INET, .ai_socktype = SOCK_STREAM, .ai_protocol = IPPROTO_TCP };
	listenSocket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);

	if (listenSocket == INVALID_SOCKET) {
		return 1;
	}

	struct addrinfo* addressInfo;
	if (getaddrinfo(ipAddress.c_str(), std::to_string(portNumber).c_str(), &hints, &addressInfo)) {
		return 1;
	}

	if (bind(listenSocket, addressInfo->ai_addr, addressInfo->ai_addrlen) == SOCKET_ERROR) {
		return 1;
	}

	freeaddrinfo(addressInfo);
	addressInfo = nullptr;
	return 0;
}

int Server::Start() {
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		return 1;
	}

	SOCKET clientSocket = INVALID_SOCKET;
	while (true) {
		clientSocket = accept(listenSocket, NULL, NULL);

		if (clientSocket == INVALID_SOCKET) {
			continue;
		}

		HandleConnection(listenSocket, clientSocket);
	}
}

void HandleConnection(SOCKET server, SOCKET client) {
	shutdown(client, SD_SEND);
	using Type = Message::Type;

	// Receive all messages regardless of type, echo to screen. 
	// Breaks when GOODBYE is received, or socket is closed
	while (true) {
		std::optional<Message> messageOpt = Message::TryReceive(client);
		if (!messageOpt) {
			std::cout << "Malformed/no message received\n";
			break;
		}

		Message message = std::move(*messageOpt);
		if (message.type == Type::GOODBYE) {
			// goodbye message received, client is hecking off to god knows where else
			std::cout << "Received GOODBYE\n";
			break;
		}

		switch (message.type) {
			case Type::NUMBER_64:
				int64_t receivedNumber;
				memcpy(&receivedNumber, message.data.data(), sizeof(int64_t));

				// convert from network to host byte order
				receivedNumber = ntohll(receivedNumber);
				std::cout << "Received NUMBER_64: " << receivedNumber << "\n";
				break;

			case Type::STRING: {
				std::string receivedString(message.data.data(), message.length);
				std::cout << "Received STRING: " << receivedString << "\n";
				break;
			}

			case Type::IMAGE_PNG:
			case Type::IMAGE_JPG: {
				std::wstring extension = (message.type == Type::IMAGE_PNG) ? L".png" : L".jpg";
				std::cout << "Received IMAGE_" << ((message.type == Type::IMAGE_PNG) ? "PNG" : "JPG") << "\n";

				LPWSTR picturesFolderPath = nullptr;
				if (!SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Pictures, 0, nullptr, &picturesFolderPath))) {
					std::cerr << "Failed to obtain user Pictures folder\n";
					break;
				}

				std::wstring picturesPath(picturesFolderPath);
				picturesPath = picturesPath + L"\\receivedImage" + extension;

				std::ofstream outFile(picturesPath, std::ios::binary);
				if (!outFile.is_open()) {
					std::wcerr << "Failed to open " << picturesPath << "\n";
				}
				else {
					outFile.write(message.data.data(), message.data.size());
					std::wcout << "Wrote image to '" << picturesPath << "'\n";
				}

				CoTaskMemFree(picturesFolderPath);
				break;
			}

			default:
				std::cout << "Invalid message type received " << message.type << "\n";
				break;
		}
	}

	std::cout << "Connection closed\n";
	shutdown(client, SD_RECEIVE);
	closesocket(client);
}

Server::~Server() {
	std::cout << "Destroying server!\n";

	// clean up listen socket
	if (listenSocket != INVALID_SOCKET) {
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
	}
}