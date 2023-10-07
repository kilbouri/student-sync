#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <optional>

#include "../../win32includes.h"

// Represents a Type-Length-Value encoded message that
// may be transmitted over a socket.
class Message {
public:

	// An enumeration of Types that may be sent
	enum Type : int32_t {
		STRING = 0, // An ASCII string. No null terminator is included.
		NUMBER_64 = 1, // A 64-bit signed integer.
		IMAGE_BMP = 2, // a bitmap image, uncompressed
		GOODBYE = ~0 // A special type, indicating the conversation is over.
	};

	const Type type;
	const int length;
	std::vector<char> data;

	Message(std::string_view);
	Message(int64_t);
	Message(Type, std::vector<char>);

	static Message Goodbye();

	// Tries to receive a message from the specified socket.
	// Returns std::nullopt if an error occurs, or the message is
	// malformed, otherwise the received message.
	static std::optional<Message> TryReceive(SOCKET socket);

	// Sends the message on the specified socket.
	// Returns 1 if an error occurs, otherwise 0.
	int Send(SOCKET socket);
};