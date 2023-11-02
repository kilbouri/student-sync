#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <optional>

#include "../../win32includes.h"
#include "../../common/socket/socket.h"

// Represents a Type-Length-Value encoded message that
// may be transmitted over a socket.
class Message {
public:
	// An enumeration of Types that may be sent
	enum Type : int32_t {
		String = 0, // An ASCII string. No null terminator is included.
		Number64 = 1, // A 64-bit signed integer.
		ImageJPG = 2, // a JPEG-encoded image
		ImagePNG = 3, // a PNG-encoded image
		Goodbye = ~0 // A special type, indicating the conversation is over.
	};

	static constexpr std::optional<Message::Type> ToMessageType(int32_t t);

	const Type type;
	const std::vector<byte> data;

	Message(Type, std::vector<byte> = std::vector<byte>(0));
	Message(std::string_view);
	Message(int64_t);

	// Tries to receive a message from the specified socket.
	// Returns std::nullopt if an error occurs, or the message is
	// malformed, otherwise the received message.
	static std::optional<Message> TryReceive(Socket& socket);

	// Sends the message on the specified socket.
	bool Send(Socket& socket);
};