#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <optional>

#include "../../win32includes.h"
#include "../../common/socket/socket.h"

int64_t htonll_signed(int64_t value);
int64_t ntohll_signed(int64_t value);

// Represents a Type-Length-Value encoded message that
// may be transmitted over a socket.
class Message {
public:
	// An enumeration of Types that may be sent
	enum Type : uint8_t {
		String = 0, // An ASCII string. No null terminator is included.
		Number64 = 1, // A 64-bit signed integer.

		StartVideoStream = 127, // indicates a video stream is starting. Contains the first frame.
		VideoFramePNG = 128, // contains a subsequent video frame
		EndVideoStream = 129, // indicates the video stream is ending. Has no payload.

		Goodbye = 255 // A special type, indicating the conversation is over.
	};

	typedef size_t Length;
	typedef std::vector<byte> Value;
	typedef std::underlying_type_t<Message::Type> RawType;

	static constexpr std::optional<Message::Type> ToMessageType(RawType t);

	const Type type;
	const Value data;

	Message(Type, Value = Value(0));
	Message(std::string_view);
	Message(int64_t);

	// Tries to receive a message from the specified socket.
	// Returns std::nullopt if an error occurs, or the message is
	// malformed, otherwise the received message.
	static std::optional<Message> TryReceive(Socket& socket);

	// Sends the message on the specified socket.
	bool Send(Socket& socket);

};