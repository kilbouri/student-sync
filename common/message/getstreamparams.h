#pragma once
#include "../networkmessage/networkmessage.h"

namespace StudentSync::Common::Messages {
	struct GetStreamParams {
		static std::optional<GetStreamParams> FromNetworkMessage(const NetworkMessage& netMessage) noexcept {
			if (netMessage.tag != NetworkMessage::Tag::GetStreamParams) {
				return std::nullopt;
			}

			return GetStreamParams{};
		}

		NetworkMessage ToNetworkMessage() const noexcept {
			return NetworkMessage(NetworkMessage::Tag::GetStreamParams);
		}
	};
}