// this file provides event callbacks for ServerWindow. It is #included in serverwindow.cpp
#pragma once
#include "serverwindow.h"
#include "../serverpreferencesmanager/serverpreferencesmanager.h"
#include "../serverpreferenceseditor/serverpreferenceseditor.h"

void ServerWindow::OnClose(wxCloseEvent& event) {
	// we MUST wait for the thread to stop. The thread assumes this object
	// has not been destroyed while it is running.
	wxThread* thread = GetThread();
	if (thread && thread->IsRunning()) {
		std::unique_lock<std::mutex> lock(shutdownLock);

		server->Stop(true); // force stop server immediately

		// wait for all features to finish
		for (auto& connection : connectionFutures) {
			connection.wait();
		}

		// wait for all connections to be finished
		thread->Wait(); // wait for thread to end
	}

	Destroy();
}

void ServerWindow::OnDetails(wxCommandEvent& event) {
	TCPSocket::SocketInfo localConnection = server->GetServerInfo();

	std::string message = "";
	message += "Hostname: " + localConnection.Address + "\n";
	message += "Port: " + std::to_string(localConnection.Port) + "\n";
	message += "wxWidgets: " wxVERSION_NUM_DOT_STRING;

	wxMessageBox(message);
}

void ServerWindow::OnShowPreferences(wxCommandEvent& event) {
	ServerPreferencesEditor editor{ ServerPreferencesManager::GetInstance().GetPreferences(), this };
	editor.ShowModal();
}

void ServerWindow::OnServerPushLog(wxThreadEvent& event) {
	// eheh, I love this type system... not... WHY CAN I NOT FORCE A TYPE FOR THE PAYLOAD OF THE EVENT WTF
	wxString message = event.GetPayload<wxString>();
	this->SetLastLogMessage(message.ToStdString());

	if (this->server) {
		this->SetConnectedClientsCounter(this->server->GetConnectionCount());
	}
}

void ServerWindow::OnClientStartStream(wxThreadEvent& event) {
	this->streamView->ClearBitmap();
}

void ServerWindow::OnClientStreamFrameReceived(wxThreadEvent& event) {
	wxBitmap nextFrame = event.GetPayload<wxBitmap>();
	this->streamView->SetBitmap(nextFrame);
}

void ServerWindow::OnClientEndStream(wxThreadEvent& event) {
	this->streamView->ClearBitmap();
}