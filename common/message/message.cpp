#include "message.h"

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

Message messageFrom(int64_t value) {
	std::vector<char> dataBuffer(sizeof(value));
	int64_t networkValue = htonll(value);

	std::memcpy(dataBuffer.data(), &networkValue, sizeof(networkValue));

	return Message{
		.type = MessageType::NUMBER_64,
		.dataLength = sizeof(value),
		.data = dataBuffer,
	};
}

Message messageFrom(std::string_view value) {
	// assumes value is NON-UNICODE! Network byte order MAY mismatch and screw up
	// Unicode strings.
	std::vector<char> dataBuffer(value.data(), value.data() + value.length());

	return Message{
		.type = MessageType::STRING,
		.dataLength = (int64_t)value.length(), // casting from unsigned to signed... VERY unlikely to have 2^63 character strings...
		.data = dataBuffer,
	};
}

Message goodbyeMessage() {
	return Message{
		.type = MessageType::GOODBYE,
		.dataLength = 0,
		.data = std::vector<char>(0)
	};
}

std::optional<Message> receiveMessage(SOCKET socket) {
	int32_t type;
	int64_t length;

	char typeData[sizeof(int32_t)] = { 0 };
	char lengthData[sizeof(int64_t)] = { 0 };

	if (recvBytesSafe(socket, typeData, sizeof(int32_t))) {
		return std::nullopt;
	}

	if (recvBytesSafe(socket, lengthData, sizeof(int64_t))) {
		return std::nullopt;
	}

	// these will be in network byte order
	std::memcpy(&type, typeData, sizeof(type));
	std::memcpy(&length, lengthData, sizeof(length));

	// now they are in host order (which is probably the same but better to be safe)
	type = ntohl(type);
	length = ntohll(length);

	std::vector<char> data(length);

	// the length check here prevents us from trying to receive data when
	// there is 0 bytes of data to be received (such as in the case of GOODBYE)
	if (length != 0 && recvBytesSafe(socket, data.data(), length)) {
		return std::nullopt;
	}

	return Message{
		.type = (MessageType)type,
		.dataLength = length,
		.data = data
	};
}

int sendMessage(SOCKET socket, Message toSend) {
	int32_t networkTag = htonl(toSend.type);
	int64_t networkLength = htonll(toSend.dataLength);

	int64_t dataSize = sizeof(networkTag) + sizeof(networkLength) + toSend.dataLength;
	std::vector<char> networkValue(dataSize);

	char* tagStart = networkValue.data();
	char* lengthStart = tagStart + sizeof(networkTag);
	char* dataStart = lengthStart + sizeof(networkLength);

	std::memcpy(tagStart, &networkTag, sizeof(networkTag));
	std::memcpy(lengthStart, &networkLength, sizeof(networkLength));
	std::memcpy(dataStart, toSend.data.data(), toSend.dataLength);

	if (sendBytesSafe(socket, networkValue.data(), dataSize)) {
		return 1;
	}

	return 0;
}