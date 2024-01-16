#pragma once
#include "../networkmessage/networkmessage.h"

namespace StudentSync::Common::Messages {
	struct Ok {
		static std::optional<Ok> FromNetworkMessage(const NetworkMessage& netMessage) noexcept {
			if (netMessage.tag != NetworkMessage::Tag::Ok) {
				return std::nullopt;
			}

			return Ok{};
		}

		NetworkMessage ToNetworkMessage() const noexcept {
			return NetworkMessage(NetworkMessage::Tag::Ok);
		}
	};
}