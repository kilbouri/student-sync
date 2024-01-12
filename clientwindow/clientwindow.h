#pragma once

#include <wx/wx.h>
#include <thread>

#include "../client/client.h"
#include "../common/networkmessage/networkmessage.h"

#define Events(x) \
	x(CLIENT_EVT_PUSH_LOG) /* a log message has been pushed from the client to the window */ \
	x(CLIENT_EVT_REGISTRATION_FAILED) /* the client failed to register with the server */

#define DeclareEvent(event) wxDECLARE_EVENT(event, wxThreadEvent);
Events(DeclareEvent)

class ClientWindow : public wxFrame {
public:
	enum {
		ID_ShowPreferences,
	};

	ClientWindow(wxString title, std::string& serverHostname, int serverPort);

private:
	// this pointer is owned by the window before the client thread is started,
	// at which point ownership now belongs to the client thread.
	//! Once the client thread is running, the window thread MUST NOT use the pointer
	std::unique_ptr<Client> client;
	std::optional<std::jthread> clientThread;

	// ClientWindow events (defined in clientwindow.events.cpp)
	void OnShowPreferences(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);

	// Client events (defined in clientwindow.events.cpp)
	void OnClientPushLog(wxThreadEvent& event);
	void OnRegistrationFailed(wxThreadEvent& event);

	// Client elements (defined in clientwindow.thread.cpp)
	void ThreadEntry();
	void ConnectionHandler(Client::Connection connection);
	void MessageReceived(Client::Connection const& connection, NetworkMessage& message);
};