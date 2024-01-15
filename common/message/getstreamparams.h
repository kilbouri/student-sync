#pragma once
#include "../networkmessage/networkmessage.h"

struct GetStreamParamsMessage {
	static std::optional<GetStreamParamsMessage> FromNetworkMessage(const NetworkMessage& netMessage) noexcept {
		if (netMessage.tag != NetworkMessage::Tag::GetStreamParams) {
			return std::nullopt;
		}

		return GetStreamParamsMessage{};
	}

	NetworkMessage ToNetworkMessage() const noexcept {
		return NetworkMessage(NetworkMessage::Tag::GetStreamParams);
	}
};