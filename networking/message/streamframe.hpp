#pragma once

#include "../tlvmessage/tlvmessage.hpp"
#include "../../common/gdiplusutil/gdiplusutil.hpp"

// The internal structure of one of these messages is:
// [format (sizeof Format)] [image data (variable)]
// At some point we should add in extra info like resolution but what the hell why now
namespace StudentSync::Networking::Message {
	struct StreamFrame {
		using DisplayCapturer = Common::GDIPlusUtil;

		DisplayCapturer::Encoding format;
		std::vector<uint8_t> imageData;

		StreamFrame(DisplayCapturer::Encoding format, std::vector<uint8_t> imageData) : format{ format }, imageData{ imageData } {}

		static std::optional<StreamFrame> FromDisplay(DisplayCapturer::Encoding format) {
			//std::optional<std::vector<uint8_t>> imageData = DisplayCapturer::CaptureScreen();
			std::optional<std::vector<uint8_t>> imageData = std::vector<uint8_t>{};
			if (!imageData) {
				return std::nullopt;
			}

			return StreamFrame(format, std::move(*imageData));
		}

		static std::optional<StreamFrame> FromTLVMessage(const TLVMessage& netMessage) noexcept {
			if (netMessage.tag != TLVMessage::Tag::StreamFrame || netMessage.data.size() < sizeof(DisplayCapturer::Encoding)) {
				return std::nullopt;
			}

			// ------ read and process the image format

			DisplayCapturer::Encoding format; // 8 bits, no endianness conversion needed
			std::memcpy(&format, netMessage.data.data(), sizeof(DisplayCapturer::Encoding));

			switch (format) {
				case DisplayCapturer::Encoding::BMP:
				case DisplayCapturer::Encoding::GIF:
				case DisplayCapturer::Encoding::JPG:
				case DisplayCapturer::Encoding::TIF:
				case DisplayCapturer::Encoding::PNG:
					break;
				default: return std::nullopt;
			}

			// ------ format ok, rest of message is image data
			std::vector<uint8_t> imageData(netMessage.data.begin() + sizeof(DisplayCapturer::Encoding), netMessage.data.end());
			return StreamFrame(format, imageData);
		}

		TLVMessage ToTLVMessage() const noexcept {
			size_t imageDataSize = imageData.size() * sizeof(decltype(imageData)::value_type);
			size_t formatSize = sizeof(format); // 8 bits, no endianness conversion needed

			std::vector<uint8_t> payload(imageDataSize + formatSize);
			uint8_t* formatStart = payload.data();
			uint8_t* imageDataStart = formatStart + formatSize;

			std::memcpy(formatStart, &format, formatSize);
			std::memcpy(imageDataStart, imageData.data(), imageDataSize);

			return TLVMessage(TLVMessage::Tag::StreamFrame, payload);
		}
	};
}