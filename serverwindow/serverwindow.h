#pragma once

#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <wx/mstream.h>
#include <wx/splitter.h>

#include "../server/server.h"
#include "../common/socket/socket.h"
#include "../common/videostreamwindow/videostreamwindow.h"
#include "../common/task/task.h"

wxDECLARE_EVENT(SERVER_EVT_PUSH_LOG, wxThreadEvent);
wxDECLARE_EVENT(SERVER_EVT_CLIENT_STARTING_STREAM, wxThreadEvent);
wxDECLARE_EVENT(SERVER_EVT_CLIENT_STREAM_FRAME_RECEIVED, wxThreadEvent);
wxDECLARE_EVENT(SERVER_EVT_CLIENT_ENDING_STREAM, wxThreadEvent);

class ServerWindow : public wxFrame, public wxThreadHelper {
public:
	enum {
		ID_Details,
		ID_ShowPreferences
	};

	ServerWindow(wxString title, std::string& hostname, int port);

protected:
	// Server data
	std::unique_ptr<Server> server;

	// Window elements
	wxSplitterWindow* splitter;
	wxScrolledWindow* sidebar;
	wxPanel* mainContentPanel;
	VideoFrameBitmap* streamView;
	wxStatusBar* statusBar;

	void ConnectionHandler(Server::Connection& connection);

	// Window events (defined in serverwindow.events.cpp)
	void OnClose(wxCloseEvent& event);
	void OnDetails(wxCommandEvent& event);
	void OnShowPreferences(wxCommandEvent& event);
	void OnServerPushLog(wxThreadEvent& event);
	void OnClientStartStream(wxThreadEvent& event);
	void OnClientStreamFrameReceived(wxThreadEvent& event);
	void OnClientEndStream(wxThreadEvent& event);

	void SetConnectedClientsCounter(int numClients);
	void SetLastLogMessage(std::string lastMessage);

	// Server Thread elements (defined in serverwindow.thread.cpp)
	bool StartServerThread(std::string& hostname, int port);
	void* Entry() override; // Inherited via wxThreadHelper

	void OnClientConnect();
	bool OnServerMessageReceived(NetworkMessage message);
	void OnClientDisconnect();

	bool NoOpMessageHandler(NetworkMessage& message);
	bool StartVideoStreamMessageHandler(NetworkMessage& message);
	bool StreamFrameMessageHandler(NetworkMessage& message);
	bool EndVideoStreamMessageHandler(NetworkMessage& message);
};