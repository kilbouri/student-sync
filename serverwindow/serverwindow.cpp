#include "serverwindow.h"

std::string serverAddress;
long serverPort;

ServerWindow::ServerWindow(std::string_view hostname, int port) : wxFrame(NULL, wxID_ANY, "ServerWindow") {
	wxMenu* menuFile = new wxMenu;

	menuFile->Append(ID_Details, "&Server details...\tCtrl-D", "Server Connection Details");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Server!");

	Bind(wxEVT_MENU, &ServerWindow::OnDetails, this, ID_Details);
	Bind(wxEVT_MENU, &ServerWindow::OnExit, this, wxID_EXIT);

}
void ServerWindow::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void ServerWindow::OnDetails(wxCommandEvent& event)
{
	// todo: put server details here (port, hostname, etc)
	wxMessageBox("Deez Nutz", "About this Software", wxOK | wxICON_INFORMATION);
}
