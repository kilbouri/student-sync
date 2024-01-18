#include "window.hpp"
#include "window.events.cpp"
#include "window.thread.cpp"

#include <wx/numdlg.h>

namespace StudentSync::Client {
	#define DefineEvent(evtName) wxDEFINE_EVENT(evtName, wxThreadEvent);
	Events(DefineEvent);

	Window::Window(wxString title, std::string& hostname, int port)
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

		wxFrame::Bind(wxEVT_MENU, &Window::OnShowPreferences, this, ID_ShowPreferences);
		wxFrame::Bind(wxEVT_MENU, &Window::OnAbout, this, wxID_ABOUT);
		wxFrame::Bind(wxEVT_MENU, &Window::OnExit, this, wxID_EXIT);
		wxFrame::Bind(wxEVT_CLOSE_WINDOW, &Window::OnClose, this);

		// Client event bindings
		wxFrame::Bind(CLIENT_EVT_REGISTRATION_FAILED, &Window::OnRegistrationFailed, this);

		// create a Client instance
		{
			using namespace std::placeholders;
			client = std::make_unique<Client>(hostname, port, std::bind(&Window::ConnectionHandler, this, _1));
		}

		// start the thread
		clientThread = std::jthread(std::bind(&Window::ThreadEntry, this));
	}
}