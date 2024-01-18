#pragma once
#include "../tlvmessage/tlvmessage.hpp"

namespace StudentSync::Networking::Message {
	struct Hello {
		std::string username;

		static std::optional<Hello> FromTLVMessage(const TLVMessage& netMessage) noexcept {
			if (netMessage.tag != TLVMessage::Tag::Hello) {
				return std::nullopt;
			}

			std::string uname{ reinterpret_cast<const char*>(netMessage.data.data()), netMessage.data.size() };
			return Hello{
				.username = uname
			};
		}

		TLVMessage ToTLVMessage() const noexcept {
			TLVMessage::Value value{ username.begin(), username.end() };
			return TLVMessage(TLVMessage::Tag::Hello, value);
		}
	};
}