#include "message.h"

constexpr int64_t htonll_signed(int64_t value) {
	return static_cast<int64_t>(htonll(static_cast<uint64_t>(value)));
}

constexpr int64_t ntohll_signed(int64_t value) {
	return static_cast<int64_t>(ntohll(static_cast<uint64_t>(value)));
}

constexpr std::optional<Message::Type> Message::ToMessageType(RawType t) {
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

Message::Message(Type dataType, Value data) : type{ dataType }, data{ data } {}
Message::Message(std::string_view value) : Message(Type::String, Value(value.data(), value.data() + value.length())) {}
Message::Message(int64_t value) : Message(Type::Number64, ([value]() {
	// Well, C++, you forced my hand. I wanted this data vector to be const, but in doing so I (rightly) can't 
	// memcpy to it. So you get to have this IIFE hack.
	int64_t networkValue = htonll_signed(value);

	Value newData(sizeof(networkValue));
	std::memcpy(newData.data(), &networkValue, sizeof(networkValue));

	return newData;
})()) {}

std::optional<Message> Message::TryReceive(Socket& socket) {

	// Reading & parsing the Type segment
	RawType rawType;
	byte typeData[sizeof(rawType)] = { 0 };
	if (!socket.ReadAllBytes(typeData, sizeof(rawType))) {
		int lastError = GetLastError();
		return std::nullopt;
	}

	// no byte order conversion required for a single-byte type
	std::memcpy(&rawType, typeData, sizeof(rawType));

	std::optional<Type> messageType = ToMessageType(rawType);
	if (!messageType.has_value()) {
		int lastError = GetLastError();
		return std::nullopt;
	}

	// Reading & parsing the Length segment
	Length length;
	byte lengthData[sizeof(length)] = { 0 };
	if (!socket.ReadAllBytes(lengthData, sizeof(length))) {
		int lastError = GetLastError();
		return std::nullopt;
	}

	std::memcpy(&length, lengthData, sizeof(length));
	length = ntohl(length);

	Value data(length);
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
	RawType tag = this->type; // no byte order conversion required
	Length length = htonl(this->data.size());

	// We eat the copy cost in order to create a single buffer. This allows us to give the
	// socket the opportunity to be as efficient as possible by providing all the data at once.
	int messageByteCount = sizeof(tag) + sizeof(length) + data.size();
	Value networkData(messageByteCount);

	byte* tagStart = networkData.data();
	byte* lengthStart = tagStart + sizeof(tag);
	byte* dataStart = lengthStart + sizeof(length);

	std::memcpy(tagStart, &tag, sizeof(tag));
	std::memcpy(lengthStart, &length, sizeof(length));
	std::memcpy(dataStart, data.data(), data.size());

	return socket.WriteAllBytes(networkData.data(), messageByteCount);
}