#pragma once
#include "../networkmessage/networkmessage.h"

struct HelloMessage {
	std::string username;

	static std::optional<HelloMessage> FromNetworkMessage(const NetworkMessage& netMessage) noexcept {
		if (netMessage.tag != NetworkMessage::Tag::Hello) {
			return std::nullopt;
		}

		std::string uname{ reinterpret_cast<const char*>(netMessage.data.data()), netMessage.data.size() };
		return HelloMessage{
			.username = uname
		};
	}

	NetworkMessage ToNetworkMessage() const noexcept {
		return NetworkMessage(NetworkMessage::Tag::Hello);
	}
};