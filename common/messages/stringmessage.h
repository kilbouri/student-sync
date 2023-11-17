#pragma once
#include "../networkmessage/networkmessage.h"

class StringMessage {
public:
	std::string string;

	StringMessage(std::string_view str) : string{ std::string(str) } {}

	static std::optional<StringMessage> FromNetworkMessage(const NetworkMessage& netMessage) noexcept {
		if (netMessage.tag != NetworkMessage::Tag::String) {
			return std::nullopt;
		}

		return StringMessage(std::string(netMessage.data.begin(), netMessage.data.end()));
	}

	NetworkMessage ToNetworkMessage() noexcept {
		std::vector<byte> data(string.size());
		std::memcpy(data.data(), string.data(), string.size());

		return NetworkMessage(NetworkMessage::Tag::String, data);
	}
};