#include "serverwindow.h"

wxDEFINE_EVENT(EVT_SERVER_PUSH_LOG, wxThreadEvent);
wxDEFINE_EVENT(EVT_SERVER_RECEIVE_IMAGE_JPG, wxThreadEvent);
wxDEFINE_EVENT(EVT_SERVER_RECEIVE_IMAGE_PNG, wxThreadEvent);

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
	wxThreadEvent* event = new wxThreadEvent(EVT_SERVER_PUSH_LOG);

	std::optional<std::string> hostResult = socket.GetBoundAddress();
	std::optional<int> portResult = socket.GetBoundPort();

	std::string hostname = hostResult.value_or("<unknown host>");
	std::string port = portResult.has_value() ? std::to_string(*portResult) : "<unknown port>";

	event->SetPayload(wxString(hostname + ":" + port + " connected"));
	wxQueueEvent(this, event);
}

bool ServerWindow::OnServerMessageReceived(TCPSocket& clientSocket, Message receivedMessage) {
	wxThreadEvent* event = new wxThreadEvent(EVT_SERVER_PUSH_LOG);
	event->SetPayload(CreateLogMessage(receivedMessage));

	wxQueueEvent(this, event);
	return true;
}

void ServerWindow::OnClientDisconnect(TCPSocket& socket) {
	wxThreadEvent* event = new wxThreadEvent(EVT_SERVER_PUSH_LOG);

	std::optional<std::string> hostResult = socket.GetBoundAddress();
	std::optional<int> portResult = socket.GetBoundPort();

	std::string hostname = hostResult.value_or("<unknown host>");
	std::string port = portResult.has_value() ? std::to_string(*portResult) : "<unknown port>";

	event->SetPayload(wxString(hostname + ":" + port + " disconnected"));
	wxQueueEvent(this, event);
}

wxString CreateLogMessage(Message& receivedMessage) {
	switch (receivedMessage.type) {
		case Message::Type::String: {
			std::string receivedString = std::string(reinterpret_cast<const char*>(receivedMessage.data.data()), receivedMessage.data.size());
			return "Received String: '" + receivedString + "'";
		}
		case Message::Type::Number64: {
			int64_t number = 0;
			std::memcpy(&number, receivedMessage.data.data(), std::min(receivedMessage.data.size(), sizeof(number)));
			return "Number64: " + std::to_string(ntohll_signed(number));
		}
		case Message::Type::ImagePNG: return "Received ImagePNG";
		case Message::Type::ImageJPG: return "Received ImageJPG";
		case Message::Type::Goodbye: return "Received Goodbye";
		default: return "Unhandled message type: " + std::to_string(receivedMessage.type);
	}
}
