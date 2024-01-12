#include "clientwindow.h"
#include "clientwindow.events.cpp"
#include "clientwindow.thread.cpp"

#include <wx/numdlg.h>

#define DefineEvent(evtName) wxDEFINE_EVENT(evtName, wxThreadEvent);
Events(DefineEvent)

ClientWindow::ClientWindow(wxString title, std::string& hostname, int port)
	: wxFrame(NULL, wxID_ANY, title)
	, client{ nullptr }
	, clientThread{ std::nullopt }
{
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_ShowPreferences, "Preferences...\tCtrl-,", "Edit client preferences");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_ABOUT);
	menuFile->Append(wxID_EXIT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "File");

	SetMenuBar(menuBar);

	wxStatusBar* statusBar = new wxStatusBar(this);
	statusBar->SetLabel("All quiet...");
	SetStatusBar(statusBar);

	wxFrame::Bind(wxEVT_MENU, &ClientWindow::OnShowPreferences, this, ID_ShowPreferences);
	wxFrame::Bind(wxEVT_MENU, &ClientWindow::OnAbout, this, wxID_ABOUT);
	wxFrame::Bind(wxEVT_MENU, &ClientWindow::OnExit, this, wxID_EXIT);
	wxFrame::Bind(wxEVT_CLOSE_WINDOW, &ClientWindow::OnClose, this);

	// Client event bindings
	wxFrame::Bind(CLIENT_EVT_REGISTRATION_FAILED, &ClientWindow::OnRegistrationFailed, this);

	// create a Client instance
	{
		using namespace std::placeholders;
		client = std::make_unique<Client>(hostname, port, std::bind(&ClientWindow::ConnectionHandler, this, _1));
	}

	// start the thread
	clientThread = std::jthread(std::bind(&ClientWindow::ThreadEntry, this));
}