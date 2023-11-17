#pragma once
#include "../networkmessage/networkmessage.h"

class HelloMessage {
public:
	static std::optional<HelloMessage> FromNetworkMessage(const NetworkMessage& netMessage) noexcept {
		if (netMessage.tag != NetworkMessage::Tag::Hello) {
			return std::nullopt;
		}

		return HelloMessage();
	}

	NetworkMessage ToNetworkMessage() noexcept {
		return NetworkMessage(NetworkMessage::Tag::Hello);
	}
};