#include "clientwindow.h"

void ClientWindow::OnExit(wxCommandEvent& event) {
	Close(true);
}

void ClientWindow::OnAbout(wxCommandEvent& event) {
	wxMessageBox("You're in the client software poggers!", "About this Software", wxOK | wxICON_INFORMATION);
}

// wxTimer::Notify
void ClientWindow::Notify() {
	client.SendVideoFrame();
}

void ClientWindow::OnString(wxCommandEvent& event) {
	wxTextEntryDialog stringDialog(this, "Enter a string:", "StudentSync - Client");
	if (stringDialog.ShowModal() != wxID_OK) {
		return;
	}

	std::string string = stringDialog.GetValue().ToStdString();
	if (!client.SendString(string)) {
		wxMessageBox("Failed to send '" + string + "' to the server", "StudentSync - Client", wxOK | wxICON_WARNING);
	}
	else {
		wxMessageBox("Sent '" + string + "' to the server", "StudentSync - Client", wxOK | wxICON_INFORMATION);
	}
}

void ClientWindow::OnNumber(wxCommandEvent& event) {
	wxTextEntryDialog stringDialog(this, "Enter a number:", "StudentSync - Client");
	if (stringDialog.ShowModal() != wxID_OK) {
		return;
	}

	int64_t number = 0;
	if (!stringDialog.GetValue().ToLongLong(&number)) {
		wxMessageBox("Invalid number entered");
		return;
	}

	if (!client.SendNumber(number)) {
		wxMessageBox("Failed to send '" + std::to_string(number) + "' to the server", "StudentSync - Client", wxOK | wxICON_WARNING);
	}
	else {
		wxMessageBox("Sent '" + std::to_string(number) + "' to the server", "StudentSync - Client", wxOK | wxICON_INFORMATION);
	}
}

void ClientWindow::OnStartStream(wxCommandEvent& event) {
	if (!client.StartVideoStream()) {
		wxMessageBox("Failed to start stream", "StudentSync - Client", wxICON_WARNING | wxOK);
		return;
	}

	constexpr int targetFrameRate = 5;
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