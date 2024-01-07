#include "clientwindow.h"
#include "clientwindow.events.cpp"

#include <wx/numdlg.h>

ClientWindow::ClientWindow(wxString title, std::string_view serverHostname, int serverPort)
	: wxFrame(NULL, wxID_ANY, title), client{ Client{} }
{
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_String, "Send A &String...\tCtrl-S", "Send a String to the Server");
	menuFile->Append(ID_Number, "Send A &Number...\tCtrl-N", "Send a Number to the Server");
	menuFile->Append(ID_ShowPreferences, "Preferences...\tCtrl-,", "Edit client preferences");
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

	wxFrame::Bind(wxEVT_MENU, &ClientWindow::OnString, this, ID_String);
	wxFrame::Bind(wxEVT_MENU, &ClientWindow::OnNumber, this, ID_Number);

	wxFrame::Bind(wxEVT_MENU, &ClientWindow::OnStartStream, this, ID_StartStream);
	wxFrame::Bind(wxEVT_MENU, &ClientWindow::OnSendNextFrame, this, ID_NextFrame);
	wxFrame::Bind(wxEVT_MENU, &ClientWindow::OnEndStream, this, ID_EndStream);

	wxFrame::Bind(wxEVT_MENU, &ClientWindow::OnAbout, this, wxID_ABOUT);
	wxFrame::Bind(wxEVT_MENU, &ClientWindow::OnExit, this, wxID_EXIT);
	wxFrame::Bind(wxEVT_MENU, &ClientWindow::OnShowPreferences, this, ID_ShowPreferences);

	if (!client.Connect(serverHostname, serverPort)) {
		wxLogFatalError("Failed to connect to server");
	}
}