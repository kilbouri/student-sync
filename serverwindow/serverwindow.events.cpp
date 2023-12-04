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
	statusBar->SetStatusText(message);
}

void ServerWindow::OnClientStartStream(wxThreadEvent& event) {
	wxBitmap firstFrame = BitmapFromByteVector(event.GetPayload<std::vector<byte>>());

	// TODO: we will eventually not be receiving a bitmap on stream start
	// TODO: StreamInitialize is a better name for this. idk where to put that but this is where I thought it. -IK
	this->streamView->SetBitmap(firstFrame);
}

void ServerWindow::OnClientStreamFrameReceived(wxThreadEvent& event) {
	wxBitmap nextFrame = BitmapFromByteVector(event.GetPayload<std::vector<byte>>());
	this->streamView->SetBitmap(nextFrame);
}

void ServerWindow::OnClientEndStream(wxThreadEvent& event) {
	this->streamView->ClearBitmap();
}

// TODO: could this be handled on another thread then moved over the thread boundary?
// Would sure improve window performance...
wxBitmap BitmapFromByteVector(std::vector<byte> data) {
	wxMemoryInputStream imageDataStream(data.data(), data.size());

	wxImage image;
	image.LoadFile(imageDataStream);

	return wxBitmap(image);
}