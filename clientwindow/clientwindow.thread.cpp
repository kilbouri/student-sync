// included in clientwindow.cpp

#include "clientwindow.h"
#include <thread>
#include "../clientpreferencesmanager/clientpreferencesmanager.h"
#include "../common/messages/hellomessage.h"
#include "../common/messages/okmessage.h"

#define PUSH_LOG_MESSAGE(message)								\
{																\
wxThreadEvent* event = new wxThreadEvent(CLIENT_EVT_PUSH_LOG);	\
event->SetPayload(wxString(message));							\
wxQueueEvent(this, event);										\
}

void ClientWindow::ThreadEntry() {
	this->client->Run();
}

void ClientWindow::ConnectionHandler(Client::Connection connection) {
	std::string username = ClientPreferencesManager::GetInstance().GetPreferences().displayName;

	if (!HelloMessage{ username }.ToNetworkMessage().Send(connection.socket)) {
		PUSH_LOG_MESSAGE("Registration failed (failed to send Hello)");
		return wxQueueEvent(this, new wxThreadEvent(CLIENT_EVT_REGISTRATION_FAILED));
	}

	auto reply = NetworkMessage::TryReceive(connection.socket);
	if (!reply || reply->tag != NetworkMessage::Tag::Ok) {
		PUSH_LOG_MESSAGE("Registration failed (reply was not Ok)");
		return wxQueueEvent(this, new wxThreadEvent(CLIENT_EVT_REGISTRATION_FAILED));
	}

	while (connection.socket.IsValid()) {
		auto message = NetworkMessage::TryReceive(connection.socket);
		if (!message) {
			PUSH_LOG_MESSAGE("Failed to recieve message");
			continue;
		}

		MessageReceived(connection, *message);
	}
}

void ClientWindow::MessageReceived(Client::Connection const& connection, NetworkMessage& message) {
	PUSH_LOG_MESSAGE(std::format("Received {} message ({} bytes)", NetworkMessage::TagName(message.tag), message.data.size()));
}