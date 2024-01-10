#include <functional>
#include <thread>
#include <queue>
#include <optional>
#include <format>

#include "serverwindow.h"
#include "serverwindow.thread.cpp"
#include "serverwindow.events.cpp"

ServerWindow::ServerWindow(wxString title, std::string& hostname, int port)
	: wxFrame(NULL, wxID_ANY, title), server{ nullptr }
{
	this->SetSize(this->FromDIP(wxSize{ 500, 400 }));

	statusBar = new wxStatusBar(this);
	statusBar->SetFieldsCount(2);

	this->SetConnectedClientsCounter(0);
	this->SetLastLogMessage("All quiet...");
	this->SetStatusBar(statusBar);

	// GUI Building
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Details, "Server &Details...\tCtrl+D", "Server Connection Details");
	menuFile->Append(ID_ShowPreferences, "Preferences...\tCtrl+,");

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");

	SetMenuBar(menuBar);

	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

	splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
	splitter->SetMinimumPaneSize(std::max(1, this->GetCharWidth() * 12));

	sidebar = new wxScrolledWindow(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
	sidebar->SetScrollRate(5, 5);
	sidebar->SetBackgroundColour(wxColour(238, 238, 238));

	wxBoxSizer* sidebarItemsSizer = new wxBoxSizer(wxVERTICAL);

	// add sidebar items here...
	wxStaticText* placeholder = new wxStaticText(sidebar, wxID_ANY, "Clients will appear here in the future.");
	placeholder->SetMinSize(wxSize{ 5 * this->GetCharWidth(), placeholder->GetMinHeight() });
	sidebarItemsSizer->Add(placeholder, 1, wxEXPAND);

	sidebar->SetSizer(sidebarItemsSizer);
	sidebar->Layout();

	mainContentPanel = new wxPanel(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_NONE);

	wxBoxSizer* mainContentSizer = new wxBoxSizer(wxVERTICAL);

	streamView = new VideoFrameBitmap(mainContentPanel, wxID_ANY);
	streamView->SetScaleMode(VideoFrameBitmap::ScaleMode::Scale_AspectFit);
	streamView->SetBackgroundColour(wxColor{ 24, 24, 24 });

	mainContentSizer->Add(streamView, 1, wxEXPAND);

	mainContentPanel->SetSizer(mainContentSizer);
	mainContentPanel->Layout();

	constexpr double defaultSidebarProportion = 0.33;
	const int defaultSidebarPosition = wxRound(defaultSidebarProportion * this->GetClientSize().GetWidth());
	splitter->SplitVertically(sidebar, mainContentPanel, defaultSidebarPosition);

	rootSizer->Add(splitter, 1, wxEXPAND);

	this->SetDoubleBuffered(true);
	this->SetSizer(rootSizer);
	this->Layout();
	this->Centre(wxBOTH);

	// Window event bindings
	Bind(wxEVT_MENU, &ServerWindow::OnDetails, this, ID_Details);
	Bind(wxEVT_MENU, &ServerWindow::OnShowPreferences, this, ID_ShowPreferences);
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
	// todo: we are recommended to use std::jthread instead!

	// create a thread for the server to run on
	if (CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR) {
		return false; // failed to create thread
	}

	// We have a thread, lets make sure we have a valid Server instance for it to use
	auto handler = std::bind(&ServerWindow::ConnectionHandler, this, std::placeholders::_1);
	server = std::make_unique<Server>(hostname, port, handler);

	return GetThread()->Run() == wxTHREAD_NO_ERROR;
}

void ServerWindow::SetConnectedClientsCounter(int numClients) {
	const bool plural = numClients != 1;
	this->statusBar->SetStatusText(std::format(
		"{} client{} connected",
		numClients,
		numClients == 1 ? "" : "s"
	), 0);
}

void ServerWindow::SetLastLogMessage(std::string lastMessage) {
	this->statusBar->SetStatusText(lastMessage, 1);
}