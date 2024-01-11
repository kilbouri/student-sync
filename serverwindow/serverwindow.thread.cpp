// This file defines the background thread for ServerWindow. It is #included in serverwindow.cpp
#include "serverwindow.h"

#include "../common/messages/stringmessage.h"
#include "../common/messages/number64message.h"
#include "../common/messages/streamframemessage.h"

#define PUSH_LOG_MESSAGE(message)								\
{																\
wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_PUSH_LOG);	\
event->SetPayload(wxString(message));							\
wxQueueEvent(this, event);										\
}

wxString CreateLogMessage(NetworkMessage& receivedMessage);
wxBitmap BitmapFromByteVector(std::vector<byte> data);

// Despite the name of this function, it is defining the entry for the server thread.
void* ServerWindow::Entry() {
	// yea I know we're supposed to check TestDestroy() periodically...
	// but in this case, we simply assume that if the thread is
	// expected to terminate, Server::Stop() will be called on the
	// server this thread is running.

	// set server callbacks
	PUSH_LOG_MESSAGE("Server starting...");
	server->Run();
	return 0;
}

void ServerWindow::ConnectionHandler(Server::Connection& con) {
	std::unique_lock<std::mutex> lock(shutdownLock, std::try_to_lock);
	if (!lock.owns_lock()) {
		// we did not acquire the lock. This means that we MUST NOT create a future (we are in the process of shutting down)
		con.Terminate();
		return;
	}

	// erase any completed futures (this isn't as fast as it could be, but who cares)
	std::erase_if(connectionFutures, [](std::future<void> const& future) -> bool {
		using namespace std::chrono_literals;
		return future.wait_for(0s) == std::future_status::ready;
	});

	connectionFutures.emplace_back(std::async(std::launch::async, [this, &con]() -> void {
		// create a local copy of the connection data
		Server::Connection connection(con);

		OnClientConnect(connection);

		while (connection.socket.IsValid()) {
			auto message = NetworkMessage::TryReceive(connection.socket);
			if (!message) {
				PUSH_LOG_MESSAGE("Failed to receive message");
			}
			else {
				OnServerMessageReceived(*message);
			}
		}

		OnClientDisconnect(connection);
	}));
}

void ServerWindow::OnClientConnect(Server::Connection& connection) {
	PUSH_LOG_MESSAGE(std::format("Client connected (user: {}, id: {})", connection.username, connection.identifier));

	wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_ADD_CLIENT);
}

void ServerWindow::OnClientDisconnect(Server::Connection& connection) {
	PUSH_LOG_MESSAGE(std::format("Client disconnected (user: {}, id: {})", connection.username, connection.identifier));
}

bool ServerWindow::OnServerMessageReceived(NetworkMessage receivedMessage) {
	using Tag = NetworkMessage::Tag;

	PUSH_LOG_MESSAGE(CreateLogMessage(receivedMessage));

	switch (receivedMessage.tag) {
		case Tag::String:		return NoOpMessageHandler(receivedMessage);
		case Tag::Number64:		return NoOpMessageHandler(receivedMessage);
		case Tag::StartStream:	return StartVideoStreamMessageHandler(receivedMessage);
		case Tag::StreamFrame:	return StreamFrameMessageHandler(receivedMessage);
		case Tag::StopStream:	return EndVideoStreamMessageHandler(receivedMessage);
		default: return false;
	}
}

bool ServerWindow::NoOpMessageHandler(NetworkMessage& message) {
	return true;
}

bool ServerWindow::StartVideoStreamMessageHandler(NetworkMessage& message) {
	wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_STARTING_STREAM);
	wxQueueEvent(this, event);
	return true;
}

bool ServerWindow::StreamFrameMessageHandler(NetworkMessage& message) {
	std::optional<StreamFrameMessage> streamFrameMessage = StreamFrameMessage::FromNetworkMessage(message);
	if (!streamFrameMessage) {
		return true;
	}

	wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_STREAM_FRAME_RECEIVED);

	wxBitmap imagePayload = BitmapFromByteVector(streamFrameMessage->imageData);
	event->SetPayload(imagePayload);

	wxQueueEvent(this, event);
	return true;
}

bool ServerWindow::EndVideoStreamMessageHandler(NetworkMessage& message) {
	wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_ENDING_STREAM);
	wxQueueEvent(this, event);
	return true;
}

wxString CreateLogMessage(NetworkMessage& receivedMessage) {
	using Tag = NetworkMessage::Tag;

	switch (receivedMessage.tag) {
		case Tag::String: {
			std::optional<StringMessage> stringMessage = StringMessage::FromNetworkMessage(receivedMessage);
			if (!stringMessage) {
				return "Received malformed String message";
			}

			return "Received String: '" + (*stringMessage).string + "'";
		}

		case Tag::Number64: {
			std::optional<Number64Message> n64Message = Number64Message::FromNetworkMessage(receivedMessage);
			if (!n64Message) {
				return "Received malformed Number64 message";
			}

			return "Received Number64: " + std::to_string((*n64Message).number);
		}

		case Tag::StartStream: return "Received StartVideoStream";
		case Tag::StopStream: return "Received EndVideoStream";
		case Tag::StreamFrame: return "Received StreamFrame";
		default: return "Unhandled message type: " + std::to_string(static_cast<NetworkMessage::TagType>(receivedMessage.tag));
	}
}

// TODO: could this be handled on another thread then moved over the thread boundary?
// Would sure improve window performance...
wxBitmap BitmapFromByteVector(std::vector<byte> data) {
	wxMemoryInputStream imageDataStream(data.data(), data.size());

	wxImage image;
	image.LoadFile(imageDataStream);

	return wxBitmap(image);
}
