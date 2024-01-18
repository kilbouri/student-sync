#include "eventdispatcher.hpp"

using namespace StudentSync::Networking;

namespace StudentSync::Server {
	EventDispatcher::EventDispatcher(Window* window)
		: window{ window }
	{}

	void EventDispatcher::SessionStarted(Session const& session) {
		wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_CONNECT);
		event->SetPayload(session.identifier);
		wxQueueEvent(window, event);
	}

	void EventDispatcher::SessionEnded(Session const& session) {
		wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_DISCONNECT);
		event->SetPayload(session.identifier);
		wxQueueEvent(window, event);
	}

	void EventDispatcher::ClientRegistered(Session const& session, Message::Hello const& message) {
		wxThreadEvent* event = new wxThreadEvent(SERVER_EVT_CLIENT_REGISTERED);
		Window::ClientInfo payload{
			.identifier = session.identifier,
			.username = message.username
		};

		event->SetPayload(payload);
		wxQueueEvent(window, event);
	}
}