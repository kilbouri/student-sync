#include "serverwindow.h"

ServerWindow::ServerWindow(wxString title, std::string& hostname, int port)
	: wxFrame(NULL, wxID_ANY, title), server{ Server(hostname, port) }
{
	wxMenu* menuFile = new wxMenu;

	menuFile->Append(ID_Details, "&Server details...\tCtrl+D", "Server Connection Details");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	Bind(wxEVT_MENU, &ServerWindow::OnDetails, this, ID_Details);
	Bind(wxEVT_MENU, &ServerWindow::OnExit, this, wxID_EXIT);

	if (server.Initialize() != 0) {
		wxLogFatalError("Failed to create and initialize server");
	}
}

void ServerWindow::OnExit(wxCommandEvent& event) {
	Close(true);
}

void ServerWindow::OnDetails(wxCommandEvent& event) {
	wxString message;

	if (!server.IsInitialized()) {
		message = "Server has not been created";
	}
	else {
		// todo: port may be zero, we should really be getting this info from the stored Server instance (not the parameters to the window)
		int port = server.GetPortNumber();
		std::string hostname = server.GetExternalAddress();

		message = "Hostname: " + hostname + "\nPort: " + std::to_string(port);
	}

	wxMessageBox(message, "Server Details");
}
