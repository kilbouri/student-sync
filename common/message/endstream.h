#pragma once
#include <optional>
#include "../networkmessage/networkmessage.h"

struct EndStreamMessage {
	static std::optional<EndStreamMessage> FromNetworkMessage(const NetworkMessage& netMessage) noexcept {
		if (netMessage.tag != NetworkMessage::Tag::EndStream) {
			return std::nullopt;
		}

		return EndStreamMessage{};
	}

	NetworkMessage ToNetworkMessage() const noexcept {
		return NetworkMessage(NetworkMessage::Tag::EndStream);
	}
};