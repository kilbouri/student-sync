#include <functional>
#include <thread>
#include <queue>
#include <optional>
#include <format>
#include <ranges>
#include <algorithm>

#include "serverwindow.h"
#include "serverwindow.thread.cpp"
#include "serverwindow.events.cpp"
#include "serverwindow.eventdispatcher.cpp"

namespace StudentSync::Server {
	#define DefineEvent(evtName) wxDEFINE_EVENT(evtName, wxThreadEvent);
	Events(DefineEvent);

	ServerWindow::ServerWindow(wxString title, std::string& hostname, int port)
		: wxFrame(NULL, wxID_ANY, title), server{ nullptr }, clients{}
	{
		statusBar = new wxStatusBar(this);
		statusBar->SetFieldsCount(2);

		this->SetLastLogMessage("All quiet...");
		this->SetStatusBar(statusBar);

		// GUI Building
		wxMenu* menuFile = new wxMenu;
		menuFile->Append(ID_ShowPreferences, "Preferences...\tCtrl-,", "Edit server preferences");
		menuFile->AppendSeparator();
		menuFile->Append(wxID_ABOUT);
		menuFile->Append(wxID_EXIT);

		wxMenuBar* menuBar = new wxMenuBar;
		menuBar->Append(menuFile, "File");
		SetMenuBar(menuBar);

		wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

		splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
		splitter->SetMinimumPaneSize(this->GetCharWidth() * 16 + 1);

		sidebar = new wxScrolledWindow(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
		sidebar->SetScrollRate(5, 5);
		sidebar->SetBackgroundColour(wxColour(238, 238, 238));

		sidebarItems = new wxBoxSizer(wxVERTICAL);
		sidebarItems->Add(new wxStaticText(sidebar, wxID_ANY, "No Clients"), 1, wxEXPAND);

		sidebar->SetSizer(sidebarItems);
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

		this->SetMinClientSize(this->FromDIP(wxSize{ 250, 200 }));
		this->SetSize(this->FromDIP(wxSize{ 500, 400 }));

		this->SetDoubleBuffered(true);
		this->SetSizer(rootSizer);
		this->Layout();
		this->Centre(wxBOTH);

		// Window event bindings
		Bind(wxEVT_MENU, &ServerWindow::OnShowPreferences, this, ID_ShowPreferences);
		Bind(wxEVT_MENU, &ServerWindow::OnAbout, this, wxID_ABOUT);
		Bind(wxEVT_MENU, &ServerWindow::OnExit, this, wxID_EXIT);
		Bind(wxEVT_CLOSE_WINDOW, &ServerWindow::OnClose, this);

		// Server event bindings
		Bind(SERVER_EVT_CLIENT_CONNECT, &ServerWindow::OnClientConnected, this);
		Bind(SERVER_EVT_CLIENT_DISCONNECT, &ServerWindow::OnClientDisconnected, this);
		Bind(SERVER_EVT_CLIENT_REGISTERED, &ServerWindow::OnClientRegistered, this);
		Bind(SERVER_EVT_CLIENT_STREAM_FRAME_RECEIVED, &ServerWindow::OnClientStreamFrameReceived, this);

		this->RefreshClientList();
		this->RefreshConnectionCount();

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
		auto dispatcher = std::make_shared<EventDispatcher>(this);
		server = std::make_unique<Server>(hostname, port, dispatcher);
		return GetThread()->Run() == wxTHREAD_NO_ERROR;
	}

	void ServerWindow::SetLastLogMessage(std::string lastMessage) {
		this->statusBar->SetStatusText(lastMessage, 1);
	}

	void ServerWindow::RefreshClientList() {
		// todo: is there a better way to sort this stuff?
		std::vector<ClientInfo> sortedClients;
		sortedClients.reserve(this->clients.size());

		for (auto const& [_, client] : clients) { sortedClients.push_back(client); }
		std::sort(sortedClients.begin(), sortedClients.end(), [](ClientInfo const& first, ClientInfo const& second) -> bool {
			return (first.username == second.username)
				? first.identifier < second.identifier
				: first.username < second.username;
		});

		this->Freeze();
		sidebarItems->Clear(true);

		for (auto const& [identifier, username] : sortedClients) {
			std::string label = std::format("{} (id: {}) \n", username, identifier);
			wxButton* clickyMcClickerson = new wxButton(sidebar, wxID_ANY, label);
			sidebarItems->Add(clickyMcClickerson, 0, wxEXPAND, 0);
		}

		this->Thaw();
	}

	void ServerWindow::RefreshConnectionCount() {
		const size_t numClients = this->clients.size();
		const bool plural = numClients != 1;
		this->statusBar->SetStatusText(std::format(
			"{} client{} connected",
			numClients,
			numClients == 1 ? "" : "s"
		), 0);
	}
}