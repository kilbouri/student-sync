#pragma once
#include "../tlvmessage/tlvmessage.hpp"

namespace StudentSync::Networking::Message {
	struct Ok {
		static std::optional<Ok> FromNetworkMessage(const TLVMessage& netMessage) noexcept {
			if (netMessage.tag != TLVMessage::Tag::Ok) {
				return std::nullopt;
			}

			return Ok{};
		}

		TLVMessage ToNetworkMessage() const noexcept {
			return TLVMessage(TLVMessage::Tag::Ok);
		}
	};
}