#pragma once
#include "../tlvmessage/tlvmessage.hpp"

namespace StudentSync::Networking::Message {
	struct Ok {
		static std::optional<Ok> FromTLVMessage(const TLVMessage& netMessage) noexcept {
			if (netMessage.tag != TLVMessage::Tag::Ok) {
				return std::nullopt;
			}

			return Ok{};
		}

		TLVMessage ToTLVMessage() const noexcept {
			return TLVMessage(TLVMessage::Tag::Ok);
		}
	};
}