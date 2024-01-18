#include "window.hpp"

#include "../preferencesmanager/preferencesmanager.hpp"
#include "../preferenceseditor/preferenceseditor.hpp"

using namespace StudentSync::Networking;
namespace StudentSync::Server {
	void Window::OnShowPreferences(wxCommandEvent& event) {
		PreferencesEditor editor{ PreferencesManager::GetInstance().GetPreferences(), this };
		editor.ShowModal();
	}

	void Window::OnAbout(wxCommandEvent& event) {
		TCPSocket::SocketInfo localConnection = server->GetServerInfo();

		std::string message = "";
		message += "Hostname: " + localConnection.Address + "\n";
		message += "Port: " + std::to_string(localConnection.Port) + "\n";
		message += "wxWidgets: " wxVERSION_NUM_DOT_STRING;

		wxMessageBox(message);
	}

	void Window::OnExit(wxCommandEvent& event) {
		this->Close();
	}

	void Window::OnClose(wxCloseEvent& event) {
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

	void Window::OnClientConnected(wxThreadEvent& event) {
		unsigned long identifier = event.GetPayload<unsigned long>();
		this->clients[identifier] = ClientInfo{
			.identifier = identifier,
			.username = "Connecting...",
		};

		this->RefreshClientList();
		this->RefreshConnectionCount();
	}

	void Window::OnClientDisconnected(wxThreadEvent& event) {
		unsigned long identifier = event.GetPayload<unsigned long>();
		this->clients.erase(identifier);
		this->RefreshClientList();
		this->RefreshConnectionCount();
	}

	void Window::OnClientRegistered(wxThreadEvent& event) {
		ClientInfo client = event.GetPayload<ClientInfo>();
		this->clients[client.identifier] = client;
		this->RefreshClientList();
	}

	void Window::OnClientStartStream(wxThreadEvent& event) {
		this->streamView->ClearBitmap();
	}

	void Window::OnClientStreamFrameReceived(wxThreadEvent& event) {
		wxBitmap nextFrame = event.GetPayload<wxBitmap>();
		this->streamView->SetBitmap(nextFrame);
	}

	void Window::OnClientEndStream(wxThreadEvent& event) {
		this->streamView->ClearBitmap();
	}
}