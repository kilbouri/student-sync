#pragma once

#include <optional>

#include "../tlvmessage/tlvmessage.hpp"

namespace StudentSync::Networking::Message {
	struct EndStream {
		static std::optional<EndStream> FromTLVMessage(const TLVMessage& netMessage) noexcept {
			if (netMessage.tag != TLVMessage::Tag::EndStream) {
				return std::nullopt;
			}

			return EndStream{};
		}

		TLVMessage ToTLVMessage() const noexcept {
			return TLVMessage(TLVMessage::Tag::EndStream);
		}
	};
}