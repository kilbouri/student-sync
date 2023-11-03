// This file defines the background thread for ServerWindow. It is #included in serverwindow.cpp
#pragma once
#include "serverwindow.h"

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

wxString CreateLogMessage(Message& receivedMessage);

// Despite the name of this function, it is defining the entry for the server thread.
void* ServerWindow::Entry() {
	// yea I know we're supposed to check TestDestroy() periodically...
	// but in this case, we simply assume that if the thread is
	// expected to terminate, Server::Stop() will be called on the
	// server this thread is running.

	// set server callbacks
	{
		using namespace std::placeholders;
		server->SetClientConnectedHandler(std::bind(&ServerWindow::OnClientConnect, this, _1));
		server->SetMessageReceivedHandler(std::bind(&ServerWindow::OnServerMessageReceived, this, _1, _2));
		server->SetClientDisconnectedHandler(std::bind(&ServerWindow::OnClientDisconnect, this, _1));
	}

	// Hit that go button!
	server->Start();
	return 0;
}

void ServerWindow::OnClientConnect(TCPSocket& socket) {
	std::optional<std::string> hostResult = socket.GetBoundAddress();
	std::optional<int> portResult = socket.GetBoundPort();

	std::string hostname = hostResult.value_or("<unknown host>");
	std::string port = portResult.has_value() ? std::to_string(*portResult) : "<unknown port>";

	PUSH_LOG_MESSAGE(hostname + ":" + port + " connected");
}

bool ServerWindow::OnServerMessageReceived(TCPSocket& clientSocket, Message receivedMessage) {
	using Type = Message::Type;

	PUSH_LOG_MESSAGE(CreateLogMessage(receivedMessage));

#define SERVER_MESSAGE_HANDLER(type, handlerFunc) case type: return handlerFunc(clientSocket, receivedMessage)
	switch (receivedMessage.type) {
		SERVER_MESSAGE_HANDLER(Type::String, DefaultMessageHandler);
		SERVER_MESSAGE_HANDLER(Type::Number64, DefaultMessageHandler);
		SERVER_MESSAGE_HANDLER(Type::StartVideoStream, StartVideoStreamMessageHandler);
		SERVER_MESSAGE_HANDLER(Type::VideoFramePNG, VideoFramePNGMessageHandler);
		SERVER_MESSAGE_HANDLER(Type::EndVideoStream, EndVideoStreamMessageHandler);
		default: return false;
	}
#undef SERVER_MESSAGE_HANDLER
}

void ServerWindow::OnClientDisconnect(TCPSocket& socket) {
	std::optional<std::string> hostResult = socket.GetBoundAddress();
	std::optional<int> portResult = socket.GetBoundPort();

	std::string hostname = hostResult.value_or("<unknown host>");
	std::string port = portResult.has_value() ? std::to_string(*portResult) : "<unknown port>";

	PUSH_LOG_MESSAGE(hostname + ":" + port + " disconnected");
}

wxString CreateLogMessage(Message& receivedMessage) {
	using Type = Message::Type;

	switch (receivedMessage.type) {
		case Type::String: {
			std::string receivedString = std::string(reinterpret_cast<const char*>(receivedMessage.data.data()), receivedMessage.data.size());
			return "Received String: '" + receivedString + "'";
		}
		case Type::Number64: {
			int64_t number = 0;
			std::memcpy(&number, receivedMessage.data.data(), std::min(receivedMessage.data.size(), sizeof(number)));
			return "Received Number64: " + std::to_string(ntohll_signed(number));
		}

		case Type::StartVideoStream: return "Received StartVideoStream";
		case Type::EndVideoStream: return "Received EndVideoStream";
		case Type::VideoFramePNG: return "Received VideoFramePNG";
		case Type::Goodbye: return "Received Goodbye";

		default: return "Unhandled message type: " + std::to_string(receivedMessage.type);
	}
}

bool ServerWindow::DefaultMessageHandler(TCPSocket& client, Message& message) {
	return true;
}

bool ServerWindow::StartVideoStreamMessageHandler(TCPSocket& client, Message& message) {
	wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_STARTING_STREAM);
	event->SetPayload(message.data);

	wxQueueEvent(this, event);
	return true;
}

bool ServerWindow::VideoFramePNGMessageHandler(TCPSocket& client, Message& message) {
	wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_STREAM_FRAME_RECEIVED);
	event->SetPayload(message.data);

	wxQueueEvent(this, event);
	return true;
}

bool ServerWindow::EndVideoStreamMessageHandler(TCPSocket& client, Message& message) {
	wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_ENDING_STREAM);
	wxQueueEvent(this, event);
	return true;
}
