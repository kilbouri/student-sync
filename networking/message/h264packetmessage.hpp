#pragma once

#include <iterator>

#include "../tlvmessage/tlvmessage.hpp"
#include "../../ffmpegincludes.h"

namespace StudentSync::Networking::Message {
	struct H264Packet {

	public:
		std::vector<uint8_t> imageData;

		int frameWidth;
		int frameHeight;

	private:
		static constexpr size_t MIN_BYTES_SIZE = sizeof(frameWidth) + sizeof(frameHeight);

	public:
		static std::optional<H264Packet> FromTLVMessage(const TLVMessage& netMessage) noexcept {
			if (netMessage.tag != TLVMessage::Tag::H264Packet) {
				return std::nullopt;
			}

			if (netMessage.data.size() < MIN_BYTES_SIZE) {
				return std::nullopt;
			}

			const int* frameDimPtr = reinterpret_cast<const int*>(netMessage.data.data());

			int frameWidth = *frameDimPtr;
			int frameHeight = *(frameDimPtr + 1);

			const void* imageDataStart = frameDimPtr + 2;

			std::vector<uint8_t> data{};
			std::move(
				netMessage.data.begin() + sizeof(frameWidth) + sizeof(frameHeight),
				netMessage.data.end(),
				std::back_inserter(data)
			);


			return H264Packet{
				.imageData = data,
				.frameWidth = static_cast<int>(ntohl(frameWidth)),
				.frameHeight = static_cast<int>(ntohl(frameHeight)),
			};
		}

		TLVMessage ToTLVMessage() const noexcept {

			std::vector<uint8_t> networkValue;
			networkValue.reserve(MIN_BYTES_SIZE + imageData.size());

			// copy in width and height
			int netWidth = static_cast<int>(htonl(frameWidth));
			int netHeight = static_cast<int>(htonl(frameHeight));

			const uint8_t* wPtr = reinterpret_cast<const uint8_t*>(&netWidth);
			const uint8_t* hPtr = reinterpret_cast<const uint8_t*>(&netHeight);

			std::copy(
				wPtr,
				wPtr + sizeof(netWidth),
				std::back_inserter(networkValue)
			);

			std::copy(
				hPtr,
				hPtr + sizeof(netHeight),
				std::back_inserter(networkValue)
			);

			// copy image data
			std::copy(
				imageData.begin(),
				imageData.end(),
				std::back_inserter(networkValue)
			);

			return TLVMessage(TLVMessage::Tag::H264Packet, networkValue);
		}
	};
}