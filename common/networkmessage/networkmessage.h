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
class NetworkMessage {
public:

#define TagValues(x) \
		x(Hello)         \
		x(Ok)            \
		x(StartStream)   \
		x(StreamFrame)   \
		x(StopStream)    \
		x(String)        \
		x(Number64)
#define CreateEnum(name) name,

	enum class Tag : uint8_t { TagValues(CreateEnum) };

	// HERE BE DRAGONS!
	// If you change these to a type of different width, you NEED to change TryReceive, TryFromBuffer,
	// and Send to use appropriate byte order conversions!
	typedef std::underlying_type_t<NetworkMessage::Tag> TagType;
	typedef size_t Length;
	typedef std::vector<byte> Value;

	static constexpr bool IsValidTag(TagType);

	const Tag tag;
	const Value data;

	NetworkMessage(Tag, Value = Value(0));

	/// <summary>
	/// Tries to receive a message from the specified socket.
	/// Returns std::nullopt if an error occurs, or the message is </summary>
	/// malformed, otherwise the received message. <param name="socket"></param>
	static std::optional<NetworkMessage> TryReceive(Socket& socket);

	/// <summary>
	/// Tries to read a message from an existing byte buffer.
	/// Returns std::nullopt if a message cannot be constructed, otherwise
	/// returns a tuple containing the NetworkMessage, and the number of bytes
	/// consumed in the process.
	/// </summary>
	/// <param name="buffer">The buffer to try to create a message from</param>
	/// <returns>std::nullopt if the buffer does not contain a message, otherwise
	/// a tuple containing the message and the number of bytes consumed.</returns>
	static std::optional<std::tuple<NetworkMessage, size_t>> TryFromBuffer(const std::vector<byte>& buffer);

	/// <summary>
	/// Sends the message on the specified socket.
	/// </summary>
	bool Send(Socket& socket);
};