// this file provides event callbacks for ServerWindow. It is #included in serverwindow.cpp
#pragma once
#include "serverwindow.h"

void ServerWindow::OnClose(wxCloseEvent& event) {
	// we MUST wait for the thread to stop. The thread assumes this object
	// has not been destroyed while it is running.
	wxThread* thread = GetThread();
	if (thread && thread->IsRunning()) {
		server->Stop(true); // force stop server immediately
		thread->Wait(); // wait for thread to end
	}

	Destroy();
}

void ServerWindow::OnDetails(wxCommandEvent& event) {
	auto hostResult = server.get()->GetHostname();
	auto portResult = server.get()->GetPort();

	std::string message = "";

	std::string hostname = hostResult.value_or("Unknown");
	message += "Hostname: " + hostname + "\n";

	std::string port = portResult.has_value() ? std::to_string(*portResult) : "Unknown";
	message += "Port: " + port + "\n";

	message += "wxWidgets: " wxVERSION_NUM_DOT_STRING;

	wxMessageBox(message);
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