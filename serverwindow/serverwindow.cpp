#include <functional>
#include <thread>
#include <queue>

#include "serverwindow.h"
#include "serverwindow.events.cpp"

ServerWindow::ServerWindow(wxString title, std::string& hostname, int port)
	: wxFrame(NULL, wxID_ANY, title), server{ nullptr }, serverThread{}
{
	// GUI Building
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Details, "Server &Details...\tCtrl+D", "Server Connection Details");

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");

	SetMenuBar(menuBar);

	// Log panel
	wxBoxSizer* contentSizer = new wxBoxSizer(wxVERTICAL);
	logContainer = new wxBoxSizer(wxVERTICAL);

	logScroller = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
	logScroller->SetSizer(logContainer);

	logContainer->Fit(logScroller);
	contentSizer->Add(logScroller, 1, wxEXPAND | wxALL, 5);

	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetSizer(contentSizer);
	this->Centre(wxBOTH);

	// Window event bindings
	Bind(wxEVT_CLOSE_WINDOW, &ServerWindow::OnClose, this);
	Bind(wxEVT_MENU, &ServerWindow::OnDetails, this, ID_Details);

	// Server Startup
	server = new Server();
	if (server->BindAndListen(hostname, port) == false) {
		wxLogFatalError("Failed to initialize server");
	}

	// set server callbacks
	{
		using namespace std::placeholders;
		server->SetMessageHandler(std::bind(&ServerWindow::OnServerMessageReceived, this, _1, _2));
	}
	// start server on other thread
	auto threadtask = std::bind_front(&Server::Start, server);
	serverThread = std::jthread(threadtask);
}

void ServerWindow::AppendLog(wxString string) {
	wxStaticText* message = new wxStaticText(logScroller, wxID_ANY, string);
	logContainer->Add(message);
}

ServerWindow::~ServerWindow() {
	if (server != nullptr) {
		delete server;
	}
}