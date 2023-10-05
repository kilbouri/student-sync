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

Message messageFrom(int64_t value)
{
	std::cout << "Creating message for integer " << value << "\n";

	std::vector<char> dataBuffer(sizeof(value));
	int64_t networkValue = htonll(value);

	memcpy(dataBuffer.data(), &networkValue, sizeof(networkValue));

	return Message {
		.type = MessageType::NUMBER_64,
		.dataLength = sizeof(value),
		.data = dataBuffer,
	};
}

Message messageFrom(std::string_view value)
{
	std::cout << "Creating message for string " << value << " (length " << value.length() << ")\n";

	// assumes value is NON-UNICODE! Network byte order MAY mismatch and screw up
	// Unicode strings.
	std::vector<char> dataBuffer(value.data(), value.data() + value.length());

	return Message{
		.type = MessageType::STRING,
		.dataLength = sizeof(value),
		.data = dataBuffer,
	};
}

Message goodbyeMessage()
{
	std::cout << "Creating goodbye message\n";

	return Message{
		.type = MessageType::GOODBYE,
		.dataLength = 0,
		.data = std::vector<char>(0)
	};
}

std::optional<Message> receiveMessage(SOCKET socket) {
	std::cout << "Begin receiving message\n";

	int32_t type;
	int64_t length;

	char typeData[sizeof(int32_t)] = { 0 };
	char lengthData[sizeof(int64_t)] = { 0 };

	if (recvBytesSafe(socket, typeData, sizeof(int32_t))) {
		std::cout << "Failed to recieve message type\n";
		return std::nullopt;
	}

	if (recvBytesSafe(socket, lengthData, sizeof(int64_t))) {
		std::cout << "Failed to recieve message length\n";
		return std::nullopt;
	}

	// these will be in network byte order
	std::memcpy(&type, typeData, sizeof(type));
	std::memcpy(&length, lengthData, sizeof(length));

	// now they are in host order (which is probably the same but better to be safe)
	type = ntohl(type);
	length = ntohll(length);

	std::vector<char> data(length); // this is guaranteed to be contiguous memory
	if (recvBytesSafe(socket, data.data(), length)) {
		std::cout << "Failed to recieve message data\n";
		return std::nullopt;
	}

	return Message{
		.type = (MessageType)type,
		.dataLength = length,
		.data = data
	};
}

int sendMessage(SOCKET socket, Message toSend) {
	std::cout << "Begin sending message\n";

	int32_t networkType = htonl(toSend.type);
	int64_t networkLength = htonll(toSend.dataLength);

	int64_t dataSize = sizeof(networkType) + sizeof(networkLength) + toSend.dataLength;
	std::vector<char> dataBuffer(dataSize);

	memcpy(dataBuffer.data(), &networkType, sizeof(networkType));
	memcpy(dataBuffer.data() + sizeof(networkType), &networkLength, sizeof(networkLength));
	memcpy(dataBuffer.data() + sizeof(networkType) + sizeof(networkLength), toSend.data.data(), toSend.dataLength);

	if (sendBytesSafe(socket, dataBuffer.data(), dataSize)) {
		return 1;
	}

	return 0;
}