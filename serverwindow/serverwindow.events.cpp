// this file provides event callbacks for ServerWindow. It is #included in serverwindow.cpp
#include "serverwindow.h"

#include "../serverpreferencesmanager/serverpreferencesmanager.h"
#include "../serverpreferenceseditor/serverpreferenceseditor.h"

namespace StudentSync::Server {
	#pragma region Window Events
	void ServerWindow::OnShowPreferences(wxCommandEvent& event) {
		ServerPreferencesEditor editor{ ServerPreferencesManager::GetInstance().GetPreferences(), this };
		editor.ShowModal();
	}

	void ServerWindow::OnAbout(wxCommandEvent& event) {
		TCPSocket::SocketInfo localConnection = server->GetServerInfo();

		std::string message = "";
		message += "Hostname: " + localConnection.Address + "\n";
		message += "Port: " + std::to_string(localConnection.Port) + "\n";
		message += "wxWidgets: " wxVERSION_NUM_DOT_STRING;

		wxMessageBox(message);
	}

	void ServerWindow::OnExit(wxCommandEvent& event) {
		this->Close();
	}

	void ServerWindow::OnClose(wxCloseEvent& event) {
		// we MUST wait for the thread to stop. The thread assumes this object
		// has not been destroyed while it is running.

		wxThread* thread = GetThread();
		if (thread && thread->IsRunning()) {
			// Stopping the server will block until all sessions have terminated.
			server->Stop();
			thread->Wait();
		}

		Destroy();
	}
	#pragma endregion

	#pragma region Server-sent Events
	void ServerWindow::OnClientConnected(wxThreadEvent& event) {
		ClientInfo client = event.GetPayload<ClientInfo>();
		this->clients[client.identifier] = client;
		this->RefreshClientList();
	}

	void ServerWindow::OnClientDisconnected(wxThreadEvent& event) {
		unsigned long identifier = event.GetPayload<unsigned long>();
		this->clients.erase(identifier);
		this->RefreshClientList();
	}

	void ServerWindow::OnClientRegistered(wxThreadEvent& event) {
		ClientInfo client = event.GetPayload<ClientInfo>();
		this->clients[client.identifier] = client;
		this->RefreshClientList();
	}

	void ServerWindow::OnServerPushLog(wxThreadEvent& event) {
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
	#pragma endregion
}