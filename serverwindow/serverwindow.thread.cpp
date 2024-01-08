// This file defines the background thread for ServerWindow. It is #included in serverwindow.cpp
#pragma once
#include "serverwindow.h"
#include "../common/messages/stringmessage.h"
#include "../common/messages/number64message.h"
#include "../common/messages/streamframemessage.h"

wxDEFINE_EVENT(SERVER_EVT_PUSH_LOG, wxThreadEvent);
wxDEFINE_EVENT(SERVER_EVT_CLIENT_STARTING_STREAM, wxThreadEvent);
wxDEFINE_EVENT(SERVER_EVT_CLIENT_STREAM_FRAME_RECEIVED, wxThreadEvent);
wxDEFINE_EVENT(SERVER_EVT_CLIENT_ENDING_STREAM, wxThreadEvent);

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
	{
		using namespace std::placeholders;
		server->SetConnectionHandler(std::bind(&ServerWindow::ConnectionHandler, this, _1));
	}

	server->Start();
	return 0;
}

Task<void> ServerWindow::ConnectionHandler(std::shared_ptr<Server::ConnectionContext> ctx) {
	this->OnClientConnect();

	while (true) {

		// I had a brain-wave... we can make this return a Task<NetworkMessage?>. The server can populate the value before resuming the coroutne!
		std::optional<NetworkMessage> message = co_await ctx->Recieve();

		if (!message) {
			break;
		}

		this->OnServerMessageReceived(*message);
	}

	this->OnClientDisconnect();
	co_return;
}

void ServerWindow::OnClientConnect() {
	PUSH_LOG_MESSAGE("Client connected");
}

bool ServerWindow::OnServerMessageReceived(NetworkMessage receivedMessage) {
	using Tag = NetworkMessage::Tag;

	PUSH_LOG_MESSAGE(CreateLogMessage(receivedMessage));

#define SERVER_MESSAGE_HANDLER(type, handlerFunc) case type: return handlerFunc(receivedMessage)
	switch (receivedMessage.tag) {
		SERVER_MESSAGE_HANDLER(Tag::String, NoOpMessageHandler);
		SERVER_MESSAGE_HANDLER(Tag::Number64, NoOpMessageHandler);
		SERVER_MESSAGE_HANDLER(Tag::StartStream, StartVideoStreamMessageHandler);
		SERVER_MESSAGE_HANDLER(Tag::StreamFrame, StreamFrameMessageHandler);
		SERVER_MESSAGE_HANDLER(Tag::StopStream, EndVideoStreamMessageHandler);
		default: return false;
	}
#undef SERVER_MESSAGE_HANDLER
}

void ServerWindow::OnClientDisconnect() {
	PUSH_LOG_MESSAGE("Client disconnected");
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
