#pragma once
#include "../tlvmessage/tlvmessage.hpp"

#include "../../common/screenresolution/screenresolution.hpp"

namespace StudentSync::Networking::Message {
	struct InitializeStream {
		long frameRate;
		Common::ScreenResolution resolution;

		static std::optional<InitializeStream> FromTLVMessage(const TLVMessage& netMessage) noexcept {
			if (netMessage.tag != TLVMessage::Tag::InitializeStream) {
				return std::nullopt;
			}

			// the internal structure of this message is the same as StreamParams
			TLVMessage copy{ netMessage };
			copy.tag = TLVMessage::Tag::StreamParams;

			auto streamParams = StreamParams::FromTLVMessage(netMessage);
			if (!streamParams) {
				return std::nullopt;
			}

			return InitializeStream{
				.frameRate = streamParams->frameRate,
				.resolution = streamParams->resolution
			};
		}

		TLVMessage ToTLVMessage() const noexcept {
			// In a realm where magic intertwines,
			// A wizard with skills of unique designs.
			// StreamParams, plain and true,
			// Transformed by spells into something new.
			//
			// With a flick of the wand, a mystical dance,
			// The message shifted, as if in a trance.
			// InitializeStream, born in the night,
			// Magic's embrace, weaving code just right.
			TLVMessage message = StreamParams{ .frameRate = frameRate, .resolution = resolution }.ToTLVMessage();
			message.tag = TLVMessage::Tag::InitializeStream;
			return message;
		}
	};
}