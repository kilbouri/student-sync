#pragma once

#include <wx/wx.h>
#include <thread>

#include "../server/server.h"

class ServerWindow : public wxFrame {
public:
	ServerWindow(wxString title, std::string& hostname, int port);
	~ServerWindow();
private:
	Server* server;
	std::jthread serverThread;

	// Window events
	void OnClose(wxCloseEvent& event);
	void OnDetails(wxCommandEvent& event);

	// Server events
	bool OnServerMessageReceived(SOCKET clientSocket, Message receivedMessage);
};

enum {
	ID_Details,
};