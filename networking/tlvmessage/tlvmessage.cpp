#include "tlvmessage.hpp"

//int64_t htonll_signed(int64_t value) {
//	return static_cast<int64_t>(htonll(static_cast<uint64_t>(value)));
//}
//
//int64_t ntohll_signed(int64_t value) {
//	return static_cast<int64_t>(ntohll(static_cast<uint64_t>(value)));
//}

namespace StudentSync::Networking {
	TLVMessage::TLVMessage(Tag dataType, Value data)
		: tag{ dataType }
		, data{ data }
	{}

	std::optional<TLVMessage> TLVMessage::TryReceive(Socket& socket) {
		using IOResult = Socket::IOResult;

		// ---------------------- TAG -------------------------

		TagType rawTag = 0;
		if (socket.ReadAllBytes(&rawTag, sizeof(rawTag)) != IOResult::Success) {
			return std::nullopt;
		}

		if (!TLVMessage::IsValidTag(rawTag)) { return std::nullopt; }
		Tag tag = static_cast<Tag>(rawTag);

		// ---------------------- LENGTH ----------------------

		Length length;
		if (socket.ReadAllBytes(reinterpret_cast<uint8_t*>(&length), sizeof(length)) != IOResult::Success) {
			return std::nullopt;
		}

		length = ntohll(length);

		// ---------------------- VALUE -----------------------

		if (length == 0) {
			return TLVMessage(tag);
		}

		// there is data to be read from the socket to populate the buffer
		Value value(length, 0);
		if (socket.ReadAllBytes(value.data(), length) != IOResult::Success) {
			return std::nullopt;
		}

		return TLVMessage(tag, value);
	}

	std::optional<std::tuple<TLVMessage, size_t>> TLVMessage::TryFromBuffer(const std::vector<uint8_t>& buffer) {
		// Tag and length aren't optional, so the message must always be at least the size of them
		constexpr size_t minSize = sizeof(TagType) + sizeof(Length);
		if (buffer.size() < minSize) {
			return std::nullopt;
		}

		const uint8_t* tagStart = buffer.data();
		const uint8_t* lengthStart = tagStart + sizeof(TagType);
		const uint8_t* valueStart = lengthStart + sizeof(Length); // Careful, this is out of bounds if length is 0!

		// ---------------------- LENGTH ----------------------

		// Since we have random access to the uint8_ts, we can check the length field first to
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

		if (!TLVMessage::IsValidTag(rawTag)) {
			return std::nullopt;
		}

		Tag tag = static_cast<Tag>(rawTag);

		// ---------------------- VALUE -----------------------

		if (length == 0) {
			return std::make_tuple(TLVMessage(tag), minSize);
		}

		// there is data to be read from the socket to populate the buffer
		Value value(length);
		std::memcpy(value.data(), valueStart, length);

		return std::make_tuple(TLVMessage(tag, value), minSize + length);
	}

	bool TLVMessage::Send(Socket& socket) {
		TagType rawTag = static_cast<TagType>(this->tag);
		if (!TLVMessage::IsValidTag(rawTag)) { return false; } // just in case :)

		Length length = htonll(this->data.size());
		std::vector<uint8_t> networkData(sizeof(rawTag) + sizeof(length) + this->data.size());

		// We eat the copy cost in order to create a single buffer. This allows us to give the
		// socket the opportunity to be as efficient as possible by providing all the data at once.
		uint8_t* tagStart = networkData.data();
		uint8_t* lengthStart = tagStart + sizeof(rawTag);
		uint8_t* dataStart = lengthStart + sizeof(length);

		std::memcpy(tagStart, &tag, sizeof(tag));
		std::memcpy(lengthStart, &length, sizeof(length));
		std::memcpy(dataStart, data.data(), data.size());

		return socket.WriteAllBytes(networkData.data(), networkData.size()) == Socket::IOResult::Success;
	}
}