#include <functional>
#include <thread>
#include <queue>
#include <optional>

#include "serverwindow.h"
#include "serverwindow.thread.cpp"
#include "serverwindow.events.cpp"

ServerWindow::ServerWindow(wxString title, std::string& hostname, int port)
	: wxFrame(NULL, wxID_ANY, title), server{ }
{
	// GUI Building
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Details, "Server &Details...\tCtrl+D", "Server Connection Details");

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");

	SetMenuBar(menuBar);

	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

	splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
	splitter->SetSashGravity(0.33);
	splitter->SetMinimumPaneSize(std::max(1, this->GetCharWidth() * 12));

	sidebar = new wxScrolledWindow(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
	sidebar->SetScrollRate(5, 5);
	sidebar->SetBackgroundColour(wxColour(255, 255, 255));

	wxBoxSizer* sidebarItemsSizer = new wxBoxSizer(wxVERTICAL);

	// add sidebar items here...

	sidebar->SetSizerAndFit(sidebarItemsSizer);
	sidebar->Layout();

	mainContentPanel = new wxPanel(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_NONE);

	wxBoxSizer* mainContentSizer = new wxBoxSizer(wxVERTICAL);

	streamView = new VideoFrameBitmap(mainContentPanel, wxID_ANY);
	streamView->SetScaleMode(VideoFrameBitmap::ScaleMode::Scale_AspectFit);
	streamView->SetBackgroundColour(wxColor{ 24, 24, 24 });

	mainContentSizer->Add(streamView, 1, wxEXPAND);

	mainContentPanel->SetSizerAndFit(mainContentSizer);
	mainContentPanel->Layout();

	splitter->SplitVertically(sidebar, mainContentPanel);
	rootSizer->Add(splitter, 1, wxEXPAND);


	this->SetSizerAndFit(rootSizer);
	this->Layout();
	this->Centre(wxBOTH);

	// Window event bindings
	Bind(wxEVT_MENU, &ServerWindow::OnDetails, this, ID_Details);
	Bind(wxEVT_CLOSE_WINDOW, &ServerWindow::OnClose, this);

	Bind(SERVER_EVT_PUSH_LOG, &ServerWindow::OnServerPushLog, this);
	Bind(SERVER_EVT_CLIENT_STARTING_STREAM, &ServerWindow::OnClientStartStream, this);
	Bind(SERVER_EVT_CLIENT_STREAM_FRAME_RECEIVED, &ServerWindow::OnClientStreamFrameReceived, this);
	Bind(SERVER_EVT_CLIENT_ENDING_STREAM, &ServerWindow::OnClientEndStream, this);

	// Start server
	if (!StartServerThread(hostname, port)) {
		wxLogFatalError("Failed to start server");
	}
}

bool ServerWindow::StartServerThread(std::string& hostname, int port) {
	// create a thread for the server to run on
	if (CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR) {
		return false; // failed to create thread
	}

	// We have a thread, lets make sure we have a valid Server instance for it to use
	server = std::make_unique<Server>();

	if (!server->BindAndListen(hostname, port)) {
		return false; // can't bind and listen, maybe already taken?
	}

	// All preconditions satisfied, SMASH THAT GO BUTTON!
	if (GetThread()->Run() != wxTHREAD_NO_ERROR) {
		return false; // button smashed too hard, thread failed to start
	}

	return true;
}