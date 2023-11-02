#include "message.h"

constexpr std::optional<Message::Type> Message::ToMessageType(int32_t t) {
	using Type = Message::Type;

	switch (t) {
		case Type::String: return Type::String;
		case Type::Number64: return Type::Number64;
		case Type::ImagePNG: return Type::ImagePNG;
		case Type::ImageJPG: return Type::ImageJPG;
		case Type::Goodbye: return Type::Goodbye;
		default: return std::nullopt;
	}
}

Message::Message(Type dataType, std::vector<byte> data) : type{ dataType }, data{ data } { }
Message::Message(std::string_view value) : Message(Type::String, std::vector<byte>(value.data(), value.data() + value.length())) {}
Message::Message(int64_t value) : Message(Type::Number64, ([value]() {
	// Well, C++, you forced my hand. I wanted this data vector to be const, but in doing so I (rightly) can't 
	// memcpy to it. So you get to have this IIFE hack.
	int64_t networkValue = htonll(value);

	std::vector<byte> newData(sizeof(value));
	std::memcpy(newData.data(), &networkValue, sizeof(value));
	return newData;
})()) {}

std::optional<Message> Message::TryReceive(Socket& socket) {

	// Reading & parsing the Type segment
	int32_t rawType;
	byte typeData[sizeof(rawType)] = { 0 };
	if (!socket.ReadAllBytes(typeData, sizeof(rawType))) {
		int lastError = GetLastError();
		return std::nullopt;
	}

	std::memcpy(&rawType, typeData, sizeof(rawType));
	rawType = ntohl(rawType);

	std::optional<Type> messageType = ToMessageType(rawType);
	if (!messageType) {
		int lastError = GetLastError();
		return std::nullopt;
	}

	// Reading & parsing the Length segment
	int length;
	byte lengthData[sizeof(length)] = { 0 };
	if (!socket.ReadAllBytes(lengthData, sizeof(length))) {
		int lastError = GetLastError();
		return std::nullopt;
	}

	std::memcpy(&length, lengthData, sizeof(length));
	length = ntohl(length);

	// while a count constructor exists, it is more performant to call reserve(). The
	// count constructor will default-initialize each element, which is pointless here
	// as we are about to memcpy into it.
	std::vector<byte> data(length);

	if (length == 0) {
		return Message(*messageType, data);
	}

	// there is data to be read from the socket to populate the buffer
	if (!socket.ReadAllBytes(data.data(), length)) {
		int lastError = GetLastError();
		return std::nullopt;
	}

	return Message(*messageType, data);
}

bool Message::Send(Socket& socket) {
	int32_t tag = htonl(this->type);
	int length = htonl(this->data.size());

	// We eat the copy cost in order to create a single buffer. This allows us to give the
	// socket the opportunity to be as efficient as possible by providing all the data at once.
	int messageByteCount = sizeof(tag) + sizeof(length) + data.size();
	std::vector<byte> networkData(messageByteCount);

	byte* tagStart = networkData.data();
	byte* lengthStart = tagStart + sizeof(tag);
	byte* dataStart = lengthStart + sizeof(length);

	std::memcpy(tagStart, &tag, sizeof(tag));
	std::memcpy(lengthStart, &length, sizeof(length));
	std::memcpy(dataStart, data.data(), data.size());

	return socket.WriteAllBytes(networkData.data(), messageByteCount);
}