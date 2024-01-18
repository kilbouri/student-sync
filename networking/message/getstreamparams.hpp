#pragma once
#include "../tlvmessage/tlvmessage.hpp"

namespace StudentSync::Networking::Message {
	struct GetStreamParams {
		static std::optional<GetStreamParams> FromNetworkMessage(const TLVMessage& netMessage) noexcept {
			if (netMessage.tag != TLVMessage::Tag::GetStreamParams) {
				return std::nullopt;
			}

			return GetStreamParams{};
		}

		TLVMessage ToNetworkMessage() const noexcept {
			return TLVMessage(TLVMessage::Tag::GetStreamParams);
		}
	};
}