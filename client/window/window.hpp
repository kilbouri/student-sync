#pragma once

#include <wx/wx.h>
#include <thread>

#include "../client/client.hpp"
#include "../../networking/tlvmessage/tlvmessage.hpp"

namespace StudentSync::Client {
	#define Events(x) \
		x(CLIENT_EVT_PUSH_LOG) /* a log message has been pushed from the client to the window */ \
		x(CLIENT_EVT_REGISTRATION_FAILED) /* the client failed to register with the server */

	#define DeclareEvent(event) wxDECLARE_EVENT(event, wxThreadEvent);
	Events(DeclareEvent);

	struct Window : public wxFrame {
		enum {
			ID_ShowPreferences,
		};

		Window(wxString title, std::string& serverHostname, int serverPort);

	private:
		// Window elements
		wxStatusBar* statusBar;

		// this pointer is owned by the window before the client thread is started,
		// at which point ownership now belongs to the client thread.
		//! Once the client thread is running, the window thread MUST NOT use the pointer
		std::unique_ptr<Client> client;
		std::optional<std::jthread> clientThread;

		// Window events (defined in clientwindow.events.cpp)
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
		void MessageReceived(Client::Connection const& connection, Networking::TLVMessage& message);
	};
}