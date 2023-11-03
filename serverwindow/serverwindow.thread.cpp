#include "serverwindow.h"

wxDEFINE_EVENT(EVT_SERVER_PUSH_LOG, wxThreadEvent);


// Despite the name of this function, it is defining the entry for the server thread.
void* ServerWindow::Entry() {
	// yea I know we're supposed to check TestDestroy() periodically...
	// but in this case, we simply assume that if the thread is
	// expected to terminate, Server::Stop() will be called on the
	// server this thread is running.

	// set server callbacks
	{
		using namespace std::placeholders;
		server->SetMessageReceivedHandler(std::bind(&ServerWindow::OnServerMessageReceived, this, _1, _2));
	}

	// Hit that go button!
	server->Start();
	return 0;
}

bool ServerWindow::OnServerMessageReceived(TCPSocket& clientSocket, Message receivedMessage) {
	wxString message = "No data";
	switch (receivedMessage.type) {
		case Message::Type::String: {
			std::string receivedString = std::string(reinterpret_cast<const char*>(receivedMessage.data.data()), receivedMessage.data.size());
			message = "String: '" + receivedString + "'";
			break;
		}
		case Message::Type::Number64: {
			int64_t number = 0;
			std::memcpy(&number, receivedMessage.data.data(), std::min(receivedMessage.data.size(), sizeof(number)));
			message = "Number64: " + std::to_string(ntohll_signed(number));
			break;
		}

		default: message = "Unhandled message type: " + std::to_string(receivedMessage.type);
	}

	wxThreadEvent* event = new wxThreadEvent(EVT_SERVER_PUSH_LOG);
	event->SetPayload(message.Clone());

	wxQueueEvent(this, event);
	return true;
}