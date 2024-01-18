#include "window.hpp"

#include <format>

#include "../preferenceseditor/preferenceseditor.hpp"

using namespace StudentSync::Networking;

namespace StudentSync::Client {

	void Window::OnShowPreferences(wxCommandEvent& event) {
		PreferencesEditor editor{ PreferencesManager::GetInstance().GetPreferences(), this };
		editor.ShowModal();
	}

	void Window::OnAbout(wxCommandEvent& event) {
		TCPSocket::SocketInfo localConnection = client->GetClientInfo();
		TCPSocket::SocketInfo remoteConnection = client->GetRemoteInfo();

		std::string message = "";
		message += std::format("Local connection: {}:{}\n", localConnection.Address, localConnection.Port);
		message += std::format("Remote connection: {}:{}\n", remoteConnection.Address, remoteConnection.Port);
		message += "wxWidgets: " wxVERSION_NUM_DOT_STRING;

		wxMessageBox(message);
	}

	void Window::OnExit(wxCommandEvent& event) {
		Close(true);
	}

	void Window::OnClose(wxCloseEvent& event) {
		client->Stop();

		if (clientThread && clientThread->joinable()) {
			clientThread->join();
		}

		Destroy();
	}

	void Window::OnClientPushLog(wxThreadEvent& event) {
		wxString message = event.GetPayload<wxString>();
		this->GetStatusBar()->SetLabel(message);
	}

	void Window::OnRegistrationFailed(wxThreadEvent& event) {
		wxLogFatalError("Failed to register with remote server.");
		this->Close();
	}
}