#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <optional>

#include "../../win32includes.h"
#include "../socket/socket.hpp"

namespace StudentSync::Networking {

	// Hello: initial message sent by the client upon connecting
	// Ok: the server has registered the client
	// GetStreamParams: the server would like to know the client's stream preferences
	// StreamParams: the client's stream preferences
	// InitializeStream: the server wants the client to start streaming with the provided preferences
	// StreamFrame: a single frame of the stream
	// EndStream: the client should stop sending frames
	#define TagValues(x)	\
		x(Hello)			\
		x(Ok)				\
		x(GetStreamParams)	\
		x(StreamParams)		\
		x(InitializeStream)	\
		x(StreamFrame)      \
		x(EndStream)

	// Represents a Type-Length-Value encoded message that
	// may be transmitted over a socket.
	struct TLVMessage {
		#define CreateEnum(name) name,
		enum class Tag : uint8_t {
			TagValues(CreateEnum)
		};
		#undef CreateEnum

		// HERE BE DRAGONS!
		// If you change these to a type of different width, you NEED to change TryReceive, TryFromBuffer,
		// and Send to use appropriate byte order conversions!
		using TagType = std::underlying_type_t<TLVMessage::Tag>;
		using Length = size_t;
		using Value = std::vector<uint8_t>;

		static constexpr bool IsValidTag(TagType);
		static constexpr std::string TagName(Tag);

		Tag tag;
		Value data;

		TLVMessage(Tag, Value = Value(0));

		/// <summary>
		/// Tries to receive a message from the specified socket.
		/// Returns std::nullopt if an error occurs, or the message is </summary>
		/// malformed, otherwise the received message. <param name="socket"></param>
		static std::optional<TLVMessage> TryReceive(Socket& socket);

		/// <summary>
		/// Tries to read a message from an existing byte buffer.
		/// Returns std::nullopt if a message cannot be constructed, otherwise
		/// returns a tuple containing the TLVMessage, and the number of bytes
		/// consumed in the process.
		/// </summary>
		/// <param name="buffer">The buffer to try to create a message from</param>
		/// <returns>std::nullopt if the buffer does not contain a message, otherwise
		/// a tuple containing the message and the number of bytes consumed.</returns>
		static std::optional<std::tuple<TLVMessage, size_t>> TryFromBuffer(const std::vector<uint8_t>& buffer);

		/// <summary>
		/// Sends the message on the specified socket.
		/// </summary>
		bool Send(Socket& socket);
	};

	constexpr bool TLVMessage::IsValidTag(TagType tag) {
		#define CreateSwitch(name) case static_cast<TagType>(TLVMessage::Tag::name): return true;
		switch (tag) {
			TagValues(CreateSwitch)
			default: return false;
		}
		#undef CreateSwitch
	}

	constexpr std::string TLVMessage::TagName(Tag tag) {
		#define CreateSwitch(name) case TLVMessage::Tag::name: return #name;
		switch (tag) {
			TagValues(CreateSwitch)
			default: return "Invalid Tag";
		}
		#undef CreateSwitch
	}
}