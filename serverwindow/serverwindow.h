#pragma once

#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <wx/mstream.h>
#include <wx/splitter.h>

#include "../server/server.h"
#include "../common/socket/socket.h"
#include "../common/videostreamwindow/videostreamwindow.h"
#include "../common/message/message.h"


#define Events(x) \
	x(SERVER_EVT_PUSH_LOG)						/* a server log is being pushed from server to window */ \
	x(SERVER_EVT_CLIENT_CONNECT)  				/* a client has connected to the server */ \
	x(SERVER_EVT_CLIENT_REGISTERED)  			/* a client has completed registration with the server */ \
	x(SERVER_EVT_CLIENT_DISCONNECT)  			/* a client has disconnected from the server */ \
	x(SERVER_EVT_CLIENT_STREAM_FRAME_RECEIVED)	/* a client has sent a stream frame */

#define DeclareEvent(evtName) wxDECLARE_EVENT(evtName, wxThreadEvent);
Events(DeclareEvent)

class ServerWindow : public wxFrame, public wxThreadHelper {
public:
	struct ClientInfo {
		unsigned long identifier;
		std::string username;
	};

	enum {
		ID_Details,
		ID_ShowPreferences
	};

	ServerWindow(wxString title, std::string& hostname, int port);

protected:
	// Server data
	std::unique_ptr<Server> server;

	// This mutex must only ever be acquired by:
	// 1. The window (main) thread, when the program is shutting down, or
	// 2. By the server thread, at any time.
	//
	// This means that if the server thread cannot immediately acquire the lock,
	// the program is shutting down and therefore the connection should be
	// immediately closed.
	std::mutex shutdownLock;
	std::vector<std::future<void>> connectionFutures;

	// Main Thread ONLY elements
	std::unordered_map<unsigned long, ClientInfo> clients;

	// Window elements
	wxSplitterWindow* splitter;
	wxScrolledWindow* sidebar;
	wxStaticText* sidebarText; // this is a bodge, we should make this a box sizer of specialized components
	wxPanel* mainContentPanel;
	VideoFrameBitmap* streamView;
	wxStatusBar* statusBar;

	void ConnectionHandler(Server::Connection& connection);

	// Window events (defined in serverwindow.events.cpp)
	void OnShowPreferences(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);

	// Server-sent events (defined in serverwindow.events.cpp)
	void OnClientConnected(wxThreadEvent& event);
	void OnClientDisconnected(wxThreadEvent& event);
	void OnClientRegistered(wxThreadEvent& event);
	void OnServerPushLog(wxThreadEvent& event);
	void OnClientStartStream(wxThreadEvent& event);
	void OnClientStreamFrameReceived(wxThreadEvent& event);
	void OnClientEndStream(wxThreadEvent& event);

	// Server event handlers (defined in serverwindow.thread.cpp)
	void ClientConnected(Server::Connection& connection);
	void ClientRegistered(Server::Connection& connection, HelloMessage& hello);
	void ClientDisconnected(Server::Connection& connection);
	bool MessageReceived(NetworkMessage& message);

	// Message type handlers (defined in serverwindow.thread.cpp)
	// todo: can these be inlined?
	bool NoOpMessageHandler(NetworkMessage& message);
	bool StreamFrameMessageHandler(NetworkMessage& message);

	// Helpers I guess
	void SetConnectedClientsCounter(int numClients);
	void SetLastLogMessage(std::string lastMessage);
	void RefreshClientList(); // todo: I'd like to make the client list its own component

	// Server Thread elements (defined in serverwindow.thread.cpp)
	bool StartServerThread(std::string& hostname, int port);
	void* Entry() override; // Inherited via wxThreadHelper

};