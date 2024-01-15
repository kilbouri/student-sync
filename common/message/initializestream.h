#pragma once
#include "../networkmessage/networkmessage.h"

struct InitializeStreamMessage {
	long frameRate;
	ScreenResolution resolution;

	static std::optional<InitializeStreamMessage> FromNetworkMessage(const NetworkMessage& netMessage) noexcept {
		if (netMessage.tag != NetworkMessage::Tag::InitializeStream) {
			return std::nullopt;
		}

		// the internal structure of this message is the same as StreamParams
		NetworkMessage copy{ netMessage };
		copy.tag = NetworkMessage::Tag::StreamParams;

		auto streamParams = StreamParamsMessage::FromNetworkMessage(netMessage);
		if (!streamParams) {
			return std::nullopt;
		}

		return InitializeStreamMessage{
			.frameRate = streamParams->frameRate,
			.resolution = streamParams->resolution
		};
	}

	NetworkMessage ToNetworkMessage() const noexcept {
		// In a realm where magic intertwines,
		// A wizard with skills of unique designs.
		// StreamParamsMessage, plain and true,
		// Transformed by spells into something new.
		// 
		// With a flick of the wand, a mystical dance,
		// The message shifted, as if in a trance.
		// InitializeStreamMessage, born in the night,
		// Magic's embrace, weaving code just right.
		NetworkMessage message = StreamParamsMessage{ .frameRate = frameRate, .resolution = resolution }.ToNetworkMessage();
		message.tag = NetworkMessage::Tag::InitializeStream;
		return message;
	}
};