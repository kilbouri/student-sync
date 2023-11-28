#pragma once

#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <wx/mstream.h>

#include "../server/server.h"
#include "../common/socket/socket.h"
#include "../videostreamwindow/videostreamwindow.h"

wxDECLARE_EVENT(SERVER_EVT_PUSH_LOG, wxThreadEvent);
wxDECLARE_EVENT(SERVER_EVT_CLIENT_STARTING_STREAM, wxThreadEvent);
wxDECLARE_EVENT(SERVER_EVT_CLIENT_STREAM_FRAME_RECEIVED, wxThreadEvent);
wxDECLARE_EVENT(SERVER_EVT_CLIENT_ENDING_STREAM, wxThreadEvent);

class ServerWindow : public wxFrame, public wxThreadHelper {
public:
	enum {
		ID_Details
	};

	ServerWindow(wxString title, std::string& hostname, int port);
protected:
	std::unique_ptr<Server> server;
	VideoStreamWindow* streamWindow;

	// Window elements
	wxScrolledWindow* logScroller;
	wxBoxSizer* logContainer;

	// Window events (defined in serverwindow.events.cpp)
	void OnClose(wxCloseEvent& event);
	void OnDetails(wxCommandEvent& event);
	void OnServerPushLog(wxThreadEvent& event);
	void OnClientStartStream(wxThreadEvent& event);
	void OnClientStreamFrameReceived(wxThreadEvent& event);
	void OnClientEndStream(wxThreadEvent& event);

	// Server Thread elements (defined in serverwindow.thread.cpp)
	bool StartServerThread(std::string& hostname, int port);
	void* Entry() override; // Inherited via wxThreadHelper

	void OnClientConnect(TCPSocket& socket);
	bool OnServerMessageReceived(TCPSocket& socket, NetworkMessage message);
	void OnClientDisconnect(TCPSocket& socket);

	bool NoOpMessageHandler(TCPSocket& client, NetworkMessage& message);
	bool StartVideoStreamMessageHandler(TCPSocket& client, NetworkMessage& message);
	bool StreamFrameMessageHandler(TCPSocket& client, NetworkMessage& message);
	bool EndVideoStreamMessageHandler(TCPSocket& client, NetworkMessage& message);
};