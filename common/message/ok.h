#pragma once
#include "../networkmessage/networkmessage.h"

struct OkMessage {
	static std::optional<OkMessage> FromNetworkMessage(const NetworkMessage& netMessage) noexcept {
		if (netMessage.tag != NetworkMessage::Tag::Ok) {
			return std::nullopt;
		}

		return OkMessage{};
	}

	NetworkMessage ToNetworkMessage() const noexcept {
		return NetworkMessage(NetworkMessage::Tag::Ok);
	}
};