#include "clientwindow.h"
#include "clientwindow.events.cpp"

#include <wx/numdlg.h>

ClientWindow::ClientWindow(wxString title, std::string_view serverHostname, int serverPort)
	: wxFrame(NULL, wxID_ANY, title), client{ Client{} }
{
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_String, "Send A &String...\tCtrl-S", "Send a String to the Server");
	menuFile->Append(ID_Number, "Send A &Number...\tCtrl-N", "Send a Number to the Server");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu* menuStream = new wxMenu;
	menuStream->Append(ID_StartStream, "&Begin Streaming\tCtrl-B", "Start streaming your screen to the Server");
	menuStream->Append(ID_NextFrame, "Send &Frame\tCtrl-F", "Send another video frame to the Server");
	menuStream->Append(ID_EndStream, "&End Streaming\tCtrl-E", "Stop streaming your screen to the Server");

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuStream, "&Stream");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	Bind(wxEVT_MENU, &ClientWindow::OnString, this, ID_String);
	Bind(wxEVT_MENU, &ClientWindow::OnNumber, this, ID_Number);

	Bind(wxEVT_MENU, &ClientWindow::OnStartStream, this, ID_StartStream);
	Bind(wxEVT_MENU, &ClientWindow::OnSendNextFrame, this, ID_NextFrame);
	Bind(wxEVT_MENU, &ClientWindow::OnEndStream, this, ID_EndStream);

	Bind(wxEVT_MENU, &ClientWindow::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &ClientWindow::OnExit, this, wxID_EXIT);

	if (!client.Connect(serverHostname, serverPort)) {
		wxLogFatalError("Failed to connect to server");
	}
}