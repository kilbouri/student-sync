#pragma once
#include "../networkmessage/networkmessage.h"

class Number64Message {
public:
	int64_t number;

	Number64Message(int64_t number) : number{ number } {}

	static std::optional<Number64Message> FromNetworkMessage(const NetworkMessage& netMessage) noexcept {
		if (netMessage.tag != NetworkMessage::Tag::Number64) {
			return std::nullopt;
		}

		int64_t networkNumber = 0;

		if (netMessage.data.size() < sizeof(networkNumber)) {
			return std::nullopt;
		}

		std::memcpy(&networkNumber, netMessage.data.data(), sizeof(networkNumber));
		return Number64Message(ntohll_signed(networkNumber));
	}

	NetworkMessage ToNetworkMessage() noexcept {
		std::vector<byte> data(sizeof(number));

		int64_t networkNumber = htonll_signed(number);
		std::memcpy(data.data(), &networkNumber, sizeof(networkNumber));

		return NetworkMessage(NetworkMessage::Tag::Number64, data);
	}
};