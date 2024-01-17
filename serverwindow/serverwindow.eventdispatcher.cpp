#include "serverwindow.h"

using namespace StudentSync::Common;

namespace StudentSync::Server {
	ServerWindow::EventDispatcher::EventDispatcher(ServerWindow* window)
		: window{ window }
	{}

	void ServerWindow::EventDispatcher::SessionStarted(Session const& session) {
		wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_CONNECT);
		event->SetPayload(session.identifier);
		wxQueueEvent(window, event);
	}

	void ServerWindow::EventDispatcher::SessionEnded(Session const& session) {
		wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_DISCONNECT);
		event->SetPayload(session.identifier);
		wxQueueEvent(window, event);
	}

	void ServerWindow::EventDispatcher::ClientRegistered(Session const& session, Messages::Hello const& message) {
		wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_REGISTERED);
		ClientInfo payload{
			.identifier = session.identifier,
			.username = message.username
		};

		event->SetPayload(payload);
		wxQueueEvent(window, event);
	}
}