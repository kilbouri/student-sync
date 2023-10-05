#include "message.h"

int recvBytesSafe(SOCKET socket, char* dataBuffer, int numBytes);
int sendBytesSafe(SOCKET socket, char* dataBuffer, int numBytes);

Message::Message(Type type, int length, std::vector<char> data)
	: type{ type }, length{ length }, data{ data } {}

Message::Message(int64_t value)
	: type{ Type::NUMBER_64 }, length{ sizeof(value) }
{
	this->data = std::vector<char>(sizeof(value));

	int64_t networkValue = htonll(value);
	std::memcpy(this->data.data(), &networkValue, sizeof(value));
}

Message::Message(std::string_view value)
	: type{ Type::STRING }, length{ (int) value.length() }
{
	// assumes value is NON-UNICODE! Network byte order MAY mismatch and screw up
	// Unicode strings.
	this->data = std::vector<char>(value.data(), value.data() + value.length());
}

Message Message::Goodbye() {
	return Message(Type::GOODBYE, 0, std::vector<char>(0));
}

std::optional<Message> Message::TryReceive(SOCKET socket) {
	int32_t type;
	int length;

	char typeData[sizeof(type)] = { 0 };
	char lengthData[sizeof(length)] = { 0 };

	if (recvBytesSafe(socket, typeData, sizeof(type))) {
		return std::nullopt;
	}

	if (recvBytesSafe(socket, lengthData, sizeof(length))) {
		return std::nullopt;
	}

	// these will be in network byte order
	std::memcpy(&type, typeData, sizeof(type));
	std::memcpy(&length, lengthData, sizeof(length));

	// now they are in host order (which is probably the same but better to be safe)
	type = ntohl(type);
	length = ntohl(length);

	std::vector<char> data(length);

	// the length check here prevents us from trying to receive data when
	// there is 0 bytes of data to be received (such as in the case of GOODBYE)
	if (length != 0 && recvBytesSafe(socket, data.data(), length)) {
		return std::nullopt;
	}

	return Message((Type)type, length, data);
}

int Message::Send(SOCKET socket) {
	int32_t networkTag = htonl(this->type);
	int networkLength = htonl(this->length);

	int dataSize = sizeof(networkTag) + sizeof(networkLength) + this->length;
	std::vector<char> networkValue(dataSize);

	char* tagStart = networkValue.data();
	char* lengthStart = tagStart + sizeof(networkTag);
	char* dataStart = lengthStart + sizeof(networkLength);

	std::memcpy(tagStart, &networkTag, sizeof(networkTag));
	std::memcpy(lengthStart, &networkLength, sizeof(networkLength));
	std::memcpy(dataStart, this->data.data(), this->length);

	if (sendBytesSafe(socket, networkValue.data(), dataSize)) {
		return 1;
	}

	return 0;
}

// Returns 0 if all numBytes bytes were received, otherwise 1
int recvBytesSafe(SOCKET socket, char* dataBuffer, int numBytes) {
	int bytesRead = 0;
	do {
		int read = recv(socket, dataBuffer + bytesRead, numBytes - bytesRead, 0);
		if (read <= 0) {
			return 1;
		}

		bytesRead += read;
	} while (bytesRead < numBytes);

	return 0;
}

// Returns 0 if all numBytes were sent, otherwise 1
int sendBytesSafe(SOCKET socket, char* dataBuffer, int numBytes) {
	int bytesSent = 0;
	do {
		int sent = send(socket, dataBuffer + bytesSent, numBytes - bytesSent, 0);
		if (sent == SOCKET_ERROR || sent < 0) {
			return 1;
		}

		bytesSent += sent;
	} while (bytesSent < numBytes);

	return 0;
}