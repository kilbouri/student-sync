#pragma once

#include <wx/wx.h>
#include <wx/scrolwin.h>

#include "../server/server.h"
#include "../common/socket/socket.h"

wxDECLARE_EVENT(EVT_SERVER_PUSH_LOG, wxThreadEvent);

class ServerWindow : public wxFrame, public wxThreadHelper {
public:
	enum {
		ID_Details
	};

	ServerWindow(wxString title, std::string& hostname, int port);
protected:
	std::unique_ptr<Server> server;

	// Window elements
	wxScrolledWindow* logScroller;
	wxBoxSizer* logContainer;

	// Window events (defined in serverwindow.events.cpp)
	void OnClose(wxCloseEvent& event);
	void OnDetails(wxCommandEvent& event);
	void OnServerPushLog(wxThreadEvent& event);

	// Server Thread elements (defined in serverwindow.thread.cpp)
	bool StartServerThread(std::string& hostname, int port);
	void* Entry() override; // Inherited via wxThreadHelper
	bool OnServerMessageReceived(TCPSocket& socket, Message message);
};