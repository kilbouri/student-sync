#include <cstdint>
#include <vector>
#include <memory>
#include <optional>

#include "../../shared.h"

// An enumeration of "tags" for use in the Message struct
enum MessageType: int32_t {
	STRING = 0,
	NUMBER_64 = 1,
	GOODBYE = ~0
};

// A container for TLV-encoded messages
struct Message
{
	MessageType type;
	int64_t dataLength;
	std::vector<char> data;
};

Message messageFrom(int64_t value);
Message messageFrom(std::string_view value);
Message goodbyeMessage();

// Tries to receive a message on the provided socket.
// Returns std::nullopt if any error occurs in the process.
std::optional<Message> receiveMessage(SOCKET socket);

// Tries to send a message. Returns 0 if it succeeds,
// otherwise 1.
int sendMessage(SOCKET socket, Message toSend);
