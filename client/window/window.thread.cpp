// included in clientwindow.cpp

#include "window.hpp"

#include <thread>
#include <format>

#include "../preferencesmanager/preferencesmanager.hpp"
#include "../../networking/message/message.hpp"

using namespace StudentSync::Networking;

namespace StudentSync::Client {
	#define PUSH_LOG_MESSAGE(message)									\
	{																	\
		wxThreadEvent* event = new wxThreadEvent(CLIENT_EVT_PUSH_LOG);	\
		event->SetPayload(wxString(message));							\
		wxQueueEvent(this, event);										\
	}

	void Window::ThreadEntry() {
		this->client->Run();
	}

	void Window::ConnectionHandler(Client::Connection connection) {
		std::string username = PreferencesManager::GetInstance().GetPreferences().displayName;

		if (!Message::Hello{ username }.ToNetworkMessage().Send(connection.socket)) {
			PUSH_LOG_MESSAGE("Registration failed (failed to send Hello)");
			return wxQueueEvent(this, new wxThreadEvent(CLIENT_EVT_REGISTRATION_FAILED));
		}

		auto okReply = Message::TryReceive<Message::Ok>(connection.socket);
		if (!okReply) {
			PUSH_LOG_MESSAGE("Registration failed (reply was not Ok)");
			return wxQueueEvent(this, new wxThreadEvent(CLIENT_EVT_REGISTRATION_FAILED));
		}

		while (connection.socket.IsValid()) {
			auto message = TLVMessage::TryReceive(connection.socket);
			if (!message) {
				PUSH_LOG_MESSAGE("Failed to recieve message");
				continue;
			}

			MessageReceived(connection, *message);
		}
	}

	void Window::MessageReceived(Client::Connection const& connection, TLVMessage& message) {
		PUSH_LOG_MESSAGE(std::format(
			"Received {} message ({} bytes)",
			TLVMessage::TagName(message.tag),
			message.data.size())
		);
	}
}