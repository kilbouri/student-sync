// this file provides event callbacks for ServerWindow. It is #included in serverwindow.cpp
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

	std::string hostname = hostResult.value_or("Unknown");
	std::string port = portResult.has_value() ? std::to_string(*portResult) : "Unknown";

	wxMessageBox("Hostname: " + hostname + "\nPort: " + port, "Server Details");
}

void ServerWindow::OnServerPushLog(wxThreadEvent& event) {
	wxString message = event.GetPayload<wxString>(); // eheh, I love this type system... not... WHY CAN I NOT DEFINE A TYPE FOR THE PAYLOAD OF THE EVENT WTF

	logContainer->Add(new wxStaticText(logScroller, wxID_ANY, message));
	logContainer->Layout();
}