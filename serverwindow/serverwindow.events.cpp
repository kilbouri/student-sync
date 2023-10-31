// this file provides event callbacks for ServerWindow. It is #included in serverwindow.cpp
#include "serverwindow.h"

void ServerWindow::OnClose(wxCloseEvent& event) {
	server->Stop();
	serverThread.join();

	Destroy();
}

void ServerWindow::OnDetails(wxCommandEvent& event) {
	wxString message;

	if (!server->IsInitialized()) {
		message = "Server has not been created";
	}
	else {
		int port = server->GetPortNumber();
		std::string hostname = server->GetExternalAddress();

		message = "Hostname: " + hostname + "\nPort: " + std::to_string(port);
	}

	wxMessageBox(message, "Server Details");
}

bool ServerWindow::OnServerMessageReceived(SOCKET clientSocket, Message receivedMessage) {
	wxLogInfo("Received message of type " + wxString(std::to_string(receivedMessage.type)));
	return false;
}