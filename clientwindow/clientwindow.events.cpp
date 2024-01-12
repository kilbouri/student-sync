#include "clientwindow.h"
#include "../clientpreferenceseditor/clientpreferenceseditor.h"

#include <format>

void ClientWindow::OnShowPreferences(wxCommandEvent& event) {
	ClientPreferencesEditor editor{ ClientPreferencesManager::GetInstance().GetPreferences(), this };
	editor.ShowModal();
}

void ClientWindow::OnAbout(wxCommandEvent& event) {
	TCPSocket::SocketInfo localConnection = client->GetClientInfo();
	TCPSocket::SocketInfo remoteConnection = client->GetRemoteInfo();

	std::string message = "";
	message += std::format("Local connection: {}:{}\n", localConnection.Address, localConnection.Port);
	message += std::format("Remote connection: {}:{}\n", remoteConnection.Address, remoteConnection.Port);
	message += "wxWidgets: " wxVERSION_NUM_DOT_STRING;

	wxMessageBox(message);
}

void ClientWindow::OnExit(wxCommandEvent& event) {
	Close(true);
}

void ClientWindow::OnClose(wxCloseEvent& event) {
	client->Stop();

	if (clientThread && clientThread->joinable()) {
		clientThread->join();
	}

	Destroy();	
}

void ClientWindow::OnClientPushLog(wxThreadEvent& event) {
	wxString message = event.GetPayload<wxString>();
	this->GetStatusBar()->SetLabel(message);
}

void ClientWindow::OnRegistrationFailed(wxThreadEvent& event) {
	wxLogFatalError("Failed to register with remote server.");
	this->Close();
}