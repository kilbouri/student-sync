// this file provides event callbacks for ServerWindow. It is #included in serverwindow.cpp
#include "serverwindow.h"

void ServerWindow::OnClose(wxCloseEvent& event) {
	server->Stop();
	serverThread.join();

	Destroy();
}

void ServerWindow::OnDetails(wxCommandEvent& event) {
	auto hostResult = server->GetHostname();
	auto portResult = server->GetPort();

	std::string hostname = hostResult.value_or("Unknown");
	std::string port = portResult.has_value() ? std::to_string(*portResult) : "Unknown";

	wxMessageBox("Hostname: " + hostname + "\nPort: " + port, "Server Details");
}

bool ServerWindow::OnServerMessageReceived(TCPSocket& clientSocket, Message receivedMessage) {
	std::string message = "No data";
	switch (receivedMessage.type) {
		case Message::Type::String: {
			std::string receivedString = std::string(reinterpret_cast<const char*>(receivedMessage.data.data()), receivedMessage.data.size());
			message = "String: '" + receivedString + "'";
			break;
		}
		case Message::Type::Number64: {
			int64_t number;
			std::memcpy(&number, receivedMessage.data.data(), std::min(receivedMessage.data.size(), sizeof(number)));
			message = "Number64: " + std::to_string(ntohll(number));
			break;
		}

		default: message = "Unhandled message type: " + std::to_string(receivedMessage.type);
	}

	AppendLog(wxString(message));
	return true;
}