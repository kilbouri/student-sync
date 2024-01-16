#pragma once
#include "../networkmessage/networkmessage.h"

namespace StudentSync::Common::Messages {
	struct Hello {
		std::string username;

		static std::optional<Hello> FromNetworkMessage(const NetworkMessage& netMessage) noexcept {
			if (netMessage.tag != NetworkMessage::Tag::Hello) {
				return std::nullopt;
			}

			std::string uname{ reinterpret_cast<const char*>(netMessage.data.data()), netMessage.data.size() };
			return Hello{
				.username = uname
			};
		}

		NetworkMessage ToNetworkMessage() const noexcept {
			NetworkMessage::Value value{ username.begin(), username.end() };
			return NetworkMessage(NetworkMessage::Tag::Hello, value);
		}
	};
}