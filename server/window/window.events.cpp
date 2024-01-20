#include "window.hpp"

#include <format>

#include "../preferencesmanager/preferencesmanager.hpp"
#include "../preferenceseditor/preferenceseditor.hpp"

using namespace StudentSync::Networking;

//! All methods defined in this file MUST execute solely on the MAIN GUI THREAD.

namespace StudentSync::Server {
	void Window::OnShowPreferences(wxCommandEvent& event) {
		PreferencesEditor editor{ PreferencesManager::GetInstance().GetPreferences(), this };
		editor.ShowModal();
	}

	void Window::OnAbout(wxCommandEvent& event) {
		TCPSocket::SocketInfo localConnection = server->GetServerInfo();

		std::string message = "";
		message += "Hostname: " + localConnection.Address + "\n";
		message += "Port: " + std::to_string(localConnection.Port) + "\n";
		message += "wxWidgets: " wxVERSION_NUM_DOT_STRING;

		wxMessageBox(message);
	}

	void Window::OnExit(wxCommandEvent& event) {
		this->Close();
	}

	void Window::OnClose(wxCloseEvent& event) {
		// we MUST wait for the thread to stop. The thread assumes this object
		// has not been destroyed while it is running.

		wxThread* thread = GetThread();
		if (thread && thread->IsRunning()) {
			// Stopping the server will block until all sessions have terminated.
			this->LogInfo("Stopping server...");
			server->Stop();
			this->LogInfo("Server stopped, waiting for thread to join...");
			thread->Wait();
		}

		Destroy();
	}

	void Window::OnServerPushLog(wxThreadEvent& event) {
		std::string payload = event.GetPayload<std::string>();
		this->LogInfo(payload);
	}

	void Window::OnClientConnected(wxThreadEvent& event) {
		unsigned long identifier = event.GetPayload<unsigned long>();

		ClientInfo newClient{
			.identifier = identifier,
			.username = "Unknown",
		};

		const auto foundSession = server->GetSession(identifier);
		if (foundSession) {
			TCPSocket::SocketInfo clientInfo = (*foundSession)->GetPeerInfo();
			newClient.username = std::format("{}:{}", clientInfo.Address, clientInfo.Port);
		}

		this->clients[identifier] = newClient;

		this->LogInfo(std::format("{} connected (id: {})", newClient.username, newClient.identifier));
		this->RefreshClientList();
		this->RefreshConnectionCount();
	}

	void Window::OnClientRegistered(wxThreadEvent& event) {
		ClientInfo client = event.GetPayload<ClientInfo>();

		this->clients[client.identifier] = client;

		const auto foundSession = server->GetSession(client.identifier);
		std::string ipAddress = "Unknown";

		if (foundSession) {
			TCPSocket::SocketInfo clientInfo = (*foundSession)->GetPeerInfo();
			ipAddress = std::format("{}:{}", clientInfo.Address, clientInfo.Port);
		}

		this->LogInfo(std::format("{} (id: {}) registered as {}", ipAddress, client.identifier, client.username));
		this->RefreshClientList();
	}

	void Window::OnClientDisconnected(wxThreadEvent& event) {
		unsigned long identifier = event.GetPayload<unsigned long>();

		ClientInfo client = ClientInfo{ clients.at(identifier) };
		this->clients.erase(identifier);

		this->LogInfo(std::format("{} disconnected (id: {})", client.username, client.identifier));
		this->RefreshClientList();
		this->RefreshConnectionCount();
	}

	void Window::OnClientStreamFrameReceived(wxThreadEvent& event) {
		wxBitmap nextFrame = event.GetPayload<wxBitmap>();
		this->streamView->SetBitmap(nextFrame);
	}

	void Window::OnClientClicked(wxCommandEvent& event, unsigned long sessionId) {
		ClientInfo const& info = clients.at(sessionId);
		auto const& foundSession = server->GetSession(sessionId);

		if (!foundSession) {
			return;
		}

		auto const& session = *foundSession;
		bool nowStreaming = session->ToggleStreaming();

		this->LogInfo(std::format(
			"{} (id: {}) is {} streaming",
			info.username,
			info.identifier,
			nowStreaming ? "now" : "no longer"
		));
	}
}