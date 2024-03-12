#pragma once
#include "../tlvmessage/tlvmessage.hpp"

namespace StudentSync::Networking::Message {
	struct H264Packet {
		std::vector<uint8_t> imageData;

		static std::optional<H264Packet> FromTLVMessage(const TLVMessage& netMessage) noexcept {
			if (netMessage.tag != TLVMessage::Tag::H264Packet) {
				return std::nullopt;
			}

			return H264Packet{
				.imageData = netMessage.data
			};
		}

		TLVMessage ToTLVMessage() const noexcept {
			return TLVMessage(TLVMessage::Tag::H264Packet, imageData);
		}
	};
}