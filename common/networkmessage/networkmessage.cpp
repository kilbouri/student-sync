#include "networkmessage.h"

int64_t htonll_signed(int64_t value) {
	return static_cast<int64_t>(htonll(static_cast<uint64_t>(value)));
}

int64_t ntohll_signed(int64_t value) {
	return static_cast<int64_t>(ntohll(static_cast<uint64_t>(value)));
}

constexpr bool NetworkMessage::IsValidTag(TagType tag) {
#define CreateSwitch(name) case static_cast<TagType>(NetworkMessage::Tag::name): return true;
	switch (tag) {
		TagValues(CreateSwitch)
		default: return false;
	}
#undef CreateSwitch
}

NetworkMessage::NetworkMessage(Tag dataType, Value data) : tag{ dataType }, data{ data } {}

std::optional<NetworkMessage> NetworkMessage::TryReceive(Socket& socket) {
	using IOResult = Socket::IOResult;

	// ---------------------- TAG -------------------------

	TagType rawTag = 0;
	if (socket.ReadAllBytes(&rawTag, sizeof(rawTag)) != IOResult::Success) {
		return std::nullopt;
	}

	if (!NetworkMessage::IsValidTag(rawTag)) { return std::nullopt; }
	Tag tag = static_cast<Tag>(rawTag);

	// ---------------------- LENGTH ----------------------

	Length length;
	if (socket.ReadAllBytes(reinterpret_cast<byte*>(&length), sizeof(length)) != IOResult::Success) {
		return std::nullopt;
	}

	length = ntohll(length);

	// ---------------------- VALUE -----------------------

	if (length == 0) {
		return NetworkMessage(tag);
	}

	// there is data to be read from the socket to populate the buffer
	Value value(length);
	if (socket.ReadAllBytes(value.data(), length) != IOResult::Success) {
		return std::nullopt;
	}

	return NetworkMessage(tag, value);
}

std::optional<std::tuple<NetworkMessage, size_t>> NetworkMessage::TryFromBuffer(const std::vector<byte>& buffer) {
	// Tag and length aren't optional, so the message must always be at least the size of them
	constexpr size_t minSize = sizeof(TagType) + sizeof(Length);
	if (buffer.size() < minSize) {
		return std::nullopt;
	}

	const byte* tagStart = buffer.data();
	const byte* lengthStart = tagStart + sizeof(TagType);
	const byte* valueStart = lengthStart + sizeof(Length); // Careful, this is out of bounds if length is 0!

	// ---------------------- LENGTH ----------------------

	// Since we have random access to the bytes, we can check the length field first to
	// short circuit if the buffer is too small.
	Length length;
	std::memcpy(&length, lengthStart, sizeof(Length));
	length = ntohll(length);

	if (buffer.size() < (minSize + length)) {
		return std::nullopt;
	}

	// ---------------------- TAG -------------------------

	TagType rawTag;
	std::memcpy(&rawTag, tagStart, sizeof(TagType));

	if (!NetworkMessage::IsValidTag(rawTag)) { return std::nullopt; }
	Tag tag = static_cast<Tag>(rawTag);

	// ---------------------- VALUE -----------------------

	if (length == 0) {
		return std::make_tuple(NetworkMessage(tag), minSize);
	}

	// there is data to be read from the socket to populate the buffer
	Value value(length);
	std::memcpy(value.data(), valueStart, length);

	return std::make_tuple(NetworkMessage(tag, value), minSize + length);
}

bool NetworkMessage::Send(Socket& socket) {
	TagType rawTag = static_cast<TagType>(this->tag);
	if (!NetworkMessage::IsValidTag(rawTag)) { return false; } // just in case :)

	Length length = htonll(this->data.size());
	std::vector<byte> networkData(sizeof(rawTag) + sizeof(length) + this->data.size());

	// We eat the copy cost in order to create a single buffer. This allows us to give the
	// socket the opportunity to be as efficient as possible by providing all the data at once.
	byte* tagStart = networkData.data();
	byte* lengthStart = tagStart + sizeof(rawTag);
	byte* dataStart = lengthStart + sizeof(length);

	std::memcpy(tagStart, &tag, sizeof(tag));
	std::memcpy(lengthStart, &length, sizeof(length));
	std::memcpy(dataStart, data.data(), data.size());

	return socket.WriteAllBytes(networkData.data(), networkData.size()) == Socket::IOResult::Success;
}