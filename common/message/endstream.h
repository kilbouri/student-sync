#pragma once
#include <optional>
#include "../networkmessage/networkmessage.h"

namespace StudentSync::Common::Messages {
	struct EndStream {
		static std::optional<EndStream> FromNetworkMessage(const NetworkMessage& netMessage) noexcept {
			if (netMessage.tag != NetworkMessage::Tag::EndStream) {
				return std::nullopt;
			}

			return EndStream{};
		}

		NetworkMessage ToNetworkMessage() const noexcept {
			return NetworkMessage(NetworkMessage::Tag::EndStream);
		}
	};
}