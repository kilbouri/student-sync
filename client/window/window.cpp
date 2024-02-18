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
		menuFile->Append(Id::TestFFmpeg, "Test FFmpeg", "Record a 5-second video of the desktop");
		menuFile->Append(Id::ShowPreferences, "Preferences...\tCtrl-,", "Edit client preferences");
		menuFile->AppendSeparator();
		menuFile->Append(wxID_ABOUT);
		menuFile->Append(wxID_EXIT);

		wxMenuBar* menuBar = new wxMenuBar;
		menuBar->Append(menuFile, "File");
		SetMenuBar(menuBar);

		statusBar = new wxStatusBar(this);
		SetStatusBar(statusBar);

		Bind(wxEVT_MENU, &Window::OnTestFFmpegEncode, this, Id::TestFFmpeg);
		Bind(wxEVT_MENU, &Window::OnShowPreferences, this, Id::ShowPreferences);
		Bind(wxEVT_MENU, &Window::OnAbout, this, wxID_ABOUT);
		Bind(wxEVT_MENU, &Window::OnExit, this, wxID_EXIT);
		Bind(wxEVT_CLOSE_WINDOW, &Window::OnClose, this);

		// Client event bindings
		Bind(CLIENT_EVT_REGISTRATION_FAILED, &Window::OnRegistrationFailed, this);
		Bind(CLIENT_EVT_PUSH_LOG, &Window::OnClientPushLog, this);

		// create a Client instance
		using namespace std::placeholders;
		client = std::make_unique<Client>(hostname, port, std::bind(&Window::ConnectionHandler, this, _1));

		// start the thread
		clientThread = std::jthread(std::bind(&Window::ThreadEntry, this));
	}
}