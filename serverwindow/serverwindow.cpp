#include <functional>
#include <thread>

#include "serverwindow.h"
#include "serverwindow.events.cpp"


ServerWindow::ServerWindow(wxString title, std::string& hostname, int port)
	: wxFrame(NULL, wxID_ANY, title), server{ nullptr }
{
	// GUI Building
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Details, "Server &Details...\tCtrl+D", "Server Connection Details");

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");

	SetMenuBar(menuBar);

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

ServerWindow::~ServerWindow() {
	if (server != nullptr) {
		delete server;
	}
}
