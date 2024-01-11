#pragma once
#include "../networkmessage/networkmessage.h"
#include "../displaycapturer/displaycapturer.h"

// The internal structure of one of these messages is:
// [format (sizeof Format)] [image data (variable)]
// At some point we should add in extra info like resolution but what the hell why now

class StreamFrameMessage {
public:

	DisplayCapturer::Format format;
	std::vector<byte> imageData;

	StreamFrameMessage(DisplayCapturer::Format format, std::vector<byte> imageData) : format{ format }, imageData{ imageData } {}

	static std::optional<StreamFrameMessage> FromDisplay(DisplayCapturer::Format format) {
		std::optional<std::vector<byte>> imageData = DisplayCapturer::CaptureScreen(format);
		if (!imageData) {
			return std::nullopt;
		}

		return StreamFrameMessage(format, std::move(*imageData));
	}

	static std::optional<StreamFrameMessage> FromNetworkMessage(const NetworkMessage& netMessage) noexcept {
		if (netMessage.tag != NetworkMessage::Tag::StreamFrame || netMessage.data.size() < sizeof(DisplayCapturer::Format)) {
			return std::nullopt;
		}

		// ------ read and process the image format

		DisplayCapturer::Format format; // 8 bits, no endianness conversion needed
		std::memcpy(&format, netMessage.data.data(), sizeof(DisplayCapturer::Format));

		switch (format) {
			case DisplayCapturer::Format::BMP:
			case DisplayCapturer::Format::GIF:
			case DisplayCapturer::Format::JPG:
			case DisplayCapturer::Format::TIF:
			case DisplayCapturer::Format::PNG:
				break;
			default: return std::nullopt;
		}

		// ------ format ok, rest of message is image data
		std::vector<byte> imageData(netMessage.data.begin() + sizeof(DisplayCapturer::Format), netMessage.data.end());
		return StreamFrameMessage(format, imageData);
	}

	NetworkMessage ToNetworkMessage() const noexcept {
		size_t imageDataSize = imageData.size() * sizeof(decltype(imageData)::value_type);
		size_t formatSize = sizeof(format); // 8 bits, no endianness conversion needed

		std::vector<byte> payload(imageDataSize + formatSize);
		byte* formatStart = payload.data();
		byte* imageDataStart = formatStart + formatSize;

		std::memcpy(formatStart, &format, formatSize);
		std::memcpy(imageDataStart, imageData.data(), imageDataSize);

		return NetworkMessage(NetworkMessage::Tag::StreamFrame, payload);
	}
};