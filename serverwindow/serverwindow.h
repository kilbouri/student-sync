#pragma once

#include <wx/wx.h>
#include <thread>

#include "../server/server.h"
#include "../common/socket/socket.h"

class ServerWindow : public wxFrame {
public:
	ServerWindow(wxString title, std::string& hostname, int port);
	~ServerWindow();
private:
	Server* server;
	std::jthread serverThread;

	// Window events (defined in serverwindow.events.cpp)
	void OnClose(wxCloseEvent& event);
	void OnDetails(wxCommandEvent& event);

	// Server events (defined in serverwindow.events.cpp)
	bool OnServerMessageReceived(TCPSocket& clientSocket, Message receivedMessage);
};

enum {
	ID_Details,
};