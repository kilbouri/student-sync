#include "startupwindow.h"

#include "../client/client.h"
#include "../clientwindow/clientwindow.h"
#include "../serverwindow/serverwindow.h"

#include <wx/wx.h>
#include <wx/numdlg.h>

StartupWindow::StartupWindow() : wxFrame(NULL, wxID_ANY, "NOTSPYWARE")
{
	// todo: using a menu is a bit odd for this. Nicer to have a couple buttons, then swap the content pane
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Client, "&Run Client...\tCtrl-Shift-C");
	menuFile->Append(ID_Server, "&Run Server...\tCtrl-Shift-S");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Not a keylogger either!");

	Bind(wxEVT_MENU, &StartupWindow::OnClient, this, ID_Client);
	Bind(wxEVT_MENU, &StartupWindow::OnServer, this, ID_Server);

	Bind(wxEVT_MENU, &StartupWindow::OnAbout, this, wxID_ABOUT);//About and exit already have their own ID thats built-in, hence wxID
	Bind(wxEVT_MENU, &StartupWindow::OnExit, this, wxID_EXIT);
}

void StartupWindow::OnExit(wxCommandEvent& event) { Close(true); }

void StartupWindow::OnAbout(wxCommandEvent& event)
{
	wxMessageBox("This is NOT a Virus or Spyware!",
		"About this Spyware", wxOK | wxICON_INFORMATION);
}

void StartupWindow::OnClient(wxCommandEvent& event)//opens up the client window when selected, hopefully
{
	wxString serverAddress = wxGetTextFromUser("Enter your Server's Address:", "Server Address", "");
	long serverPort = wxGetNumberFromUser("Enter the Server's Port:", "Server Port", "Enter Number", 50, 0, LONG_MAX);

	ClientWindow* cFrame = new ClientWindow(serverAddress.ToStdString(), serverPort);
	cFrame->Show(true);

	Close(true);
}

void StartupWindow::OnServer(wxCommandEvent& event)
{
	ServerWindow* sFrame = new ServerWindow();
	sFrame->Show(true);
	Close(true);
}
