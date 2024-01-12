// This file defines the background thread for ServerWindow. It is #included in serverwindow.cpp
#include "serverwindow.h"

#define PUSH_LOG_MESSAGE(message)								\
{																\
wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_PUSH_LOG);	\
event->SetPayload(wxString(message));							\
wxQueueEvent(this, event);										\
}

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
		// most of the time we will acquire the lock. We can tell the compiler that we expect not to fall into this branch.
		[[unlikely]]

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

		ClientConnected(connection);

		// todo: distinguish between force closing due to protocol violation and closing due to disconnect
		auto firstMessage = NetworkMessage::TryReceive(connection.socket);
		if (!firstMessage) {
			connection.Terminate();
			return ClientDisconnected(connection);
		}

		auto helloMessage = HelloMessage::FromNetworkMessage(std::move(*firstMessage));
		if (!helloMessage) {
			connection.Terminate();
			return ClientDisconnected(connection);
		}

		if (!OkMessage{}.ToNetworkMessage().Send(connection.socket)) {
			connection.Terminate();
			return ClientDisconnected(connection);
		}

		ClientRegistered(connection, *helloMessage);

		while (connection.socket.IsValid()) {
			auto message = NetworkMessage::TryReceive(connection.socket);
			if (!message) {
				PUSH_LOG_MESSAGE("Failed to receive message");
			}
			else {
				MessageReceived(*message);
			}
		}

		return ClientDisconnected(connection);
	}));
}

void ServerWindow::ClientConnected(Server::Connection& connection) {
	PUSH_LOG_MESSAGE(std::format("Client connected (id: {})", connection.identifier));

	wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_CONNECT);
	event->SetPayload(ClientInfo {.identifier = connection.identifier, .username = "Connecting..."});

	wxQueueEvent(this, event);
}

void ServerWindow::ClientRegistered(Server::Connection& connection, HelloMessage& hello) {
	PUSH_LOG_MESSAGE(std::format("Client registered (id: {}, username: '{}')", connection.identifier, hello.username));

	wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_REGISTERED);
	event->SetPayload(ClientInfo {.identifier = connection.identifier, .username = hello.username});

	wxQueueEvent(this, event);
}

void ServerWindow::ClientDisconnected(Server::Connection& connection) {
	PUSH_LOG_MESSAGE(std::format("Client disconnected (id: {})", connection.identifier));

	wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_DISCONNECT);

	// the username is not required at this time (if it is, then we would need to lock the clients collection and search it.. yikes)
	event->SetPayload(connection.identifier);

	wxQueueEvent(this, event);
}

bool ServerWindow::MessageReceived(NetworkMessage& receivedMessage) {
	PUSH_LOG_MESSAGE(std::format("Received {} message ({} bytes)", NetworkMessage::TagName(receivedMessage.tag), receivedMessage.data.size()));

	using Tag = NetworkMessage::Tag;
	switch (receivedMessage.tag) {
		case Tag::StreamFrame:	return StreamFrameMessageHandler(receivedMessage);
		default: return false;
	}
}

bool ServerWindow::NoOpMessageHandler(NetworkMessage& message) {
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

// TODO: could this be handled on another thread then moved over the thread boundary?
// Would sure improve window performance...
wxBitmap BitmapFromByteVector(std::vector<byte> data) {
	wxMemoryInputStream imageDataStream(data.data(), data.size());

	wxImage image;
	image.LoadFile(imageDataStream);

	return wxBitmap(image);
}
