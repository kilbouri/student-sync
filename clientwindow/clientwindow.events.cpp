#include "clientwindow.h"
#include "../clientpreferenceseditor/clientpreferenceseditor.h"

void ClientWindow::OnExit(wxCommandEvent& event) {
	Close(true);
}

void ClientWindow::OnAbout(wxCommandEvent& event) {
	wxMessageBox("You're in the client software poggers!", "About this Software", wxOK | wxICON_INFORMATION);
}

void ClientWindow::OnShowPreferences(wxCommandEvent& event)
{
	ClientPreferencesEditor editor{ ClientPreferencesManager::GetInstance().GetPreferences(), this };
	editor.ShowModal();
}

// wxTimer::Notify
void ClientWindow::Notify() {
	client.SendVideoFrame();
}

void ClientWindow::OnStartStream(wxCommandEvent& event) {
	if (!client.StartVideoStream()) {
		wxMessageBox("Failed to start stream", "StudentSync - Client", wxICON_WARNING | wxOK);
		return;
	}

	constexpr int targetFrameRate = 2;
	wxTimer::Start(1000 / targetFrameRate);
}

void ClientWindow::OnSendNextFrame(wxCommandEvent& event) {
	if (!client.SendVideoFrame()) {
		wxMessageBox("Failed to send frame", "StudentSync - Client", wxICON_WARNING | wxOK);
	}
}

void ClientWindow::OnEndStream(wxCommandEvent& event) {
	wxTimer::Stop();
	if (!client.EndVideoStream()) {
		wxMessageBox("Failed to end stream", "StudentSync - Client", wxICON_WARNING | wxOK);
	}
}