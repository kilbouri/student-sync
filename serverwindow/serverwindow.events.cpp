// this file provides event callbacks for ServerWindow. It is #included in serverwindow.cpp
#pragma once
#include "serverwindow.h"

wxBitmap BitmapFromByteVector(std::vector<byte> data);

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
	// eheh, I love this type system... not... WHY CAN I NOT FORCE A TYPE FOR THE PAYLOAD OF THE EVENT WTF
	wxString message = event.GetPayload<wxString>();

	logContainer->Add(new wxStaticText(logScroller, wxID_ANY, message));
	logContainer->Layout();
}

void ServerWindow::OnClientStartStream(wxThreadEvent& event) {
	// todo: use a critical section to guard access to streamWindow
	wxBitmap firstFrame = BitmapFromByteVector(event.GetPayload<std::vector<byte>>());
	streamWindow = new VideoStreamWindow(this, "StudentSync - Remote Screen", firstFrame);
	streamWindow->Show();
}

void ServerWindow::OnClientStreamFrameReceived(wxThreadEvent& event) {
	// todo: use a critical section to guard access to streamWindow
	if (!streamWindow) {
		return;
	}

	wxBitmap nextFrame = BitmapFromByteVector(event.GetPayload<std::vector<byte>>());
	streamWindow->SetFrame(nextFrame);
}

void ServerWindow::OnClientEndStream(wxThreadEvent& event) {
	// todo: use a critical section to guard access to streamWindow
	if (streamWindow) {
		streamWindow->StreamEnded();
	}
}


wxBitmap BitmapFromByteVector(std::vector<byte> data) {
	wxMemoryInputStream imageDataStream(data.data(), data.size());

	wxImage image;
	image.LoadFile(imageDataStream, wxBITMAP_TYPE_PNG);

	return wxBitmap(image);
}