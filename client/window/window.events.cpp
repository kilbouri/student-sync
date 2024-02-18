#include "window.hpp"

#include <format>
#include <wx/mstream.h>

#include "../../common/timer/timer.hpp"
#include "../../common/screenresolution/screenresolution.hpp"
#include "../../common/ffmpeg/encoders/h264Encoder.hpp"
#include "../../common/gdiplusutil/gdiplusutil.hpp"
#include "../preferenceseditor/preferenceseditor.hpp"

using namespace StudentSync::Networking;
using namespace StudentSync::Common;

namespace StudentSync::Client {
	void RecordClip(int seconds, int fps, std::string_view path) {
		auto currentResolution = ScreenResolution::GetCurrentDisplayResolution();
		FFmpeg::Encoders::H264Encoder encoder{ (int)currentResolution.width, (int)currentResolution.height, fps };

		for (int i = 0; i < fps * seconds; ++i) {
			std::vector<uint8_t> frame{};

			wxMemoryInputStream memoryInStream{ frame.data(), frame.size() };
			wxImage image{ memoryInStream, wxBitmapType::wxBITMAP_TYPE_ANY };

			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));
		}
	}

	void Window::OnTestFFmpegEncode(wxCommandEvent& event) {
		wxFileDialog saveLocationPrompt{
			this, "Save Video",
			wxEmptyString, wxEmptyString,
			"mp4 files (*.mp4)|*.mp4", wxFD_SAVE | wxFD_OVERWRITE_PROMPT
		};

		if (saveLocationPrompt.ShowModal() != wxID_OK) {
			return;
		}

		// this is fucking stupid
		std::string path{ saveLocationPrompt.GetPath().c_str() };
		std::thread job = std::thread{ &RecordClip, 10, 30, path };
	}

	void Window::OnShowPreferences(wxCommandEvent& event) {
		PreferencesEditor editor{ PreferencesManager::GetInstance().GetPreferences(), this };
		editor.ShowModal();
	}

	void Window::OnAbout(wxCommandEvent& event) {
		TCPSocket::SocketInfo localConnection = client->GetClientInfo();
		TCPSocket::SocketInfo remoteConnection = client->GetRemoteInfo();

		std::string message = "";
		message += std::format("Local connection: {}:{}\n", localConnection.Address, localConnection.Port);
		message += std::format("Remote connection: {}:{}\n", remoteConnection.Address, remoteConnection.Port);
		message += "wxWidgets: " wxVERSION_NUM_DOT_STRING;

		wxMessageBox(message);
	}

	void Window::OnExit(wxCommandEvent& event) {
		Close(true);
	}

	void Window::OnClose(wxCloseEvent& event) {
		client->Stop();

		if (clientThread && clientThread->joinable()) {
			clientThread->join();
		}

		Destroy();
	}

	void Window::OnClientPushLog(wxThreadEvent& event) {
		wxString message = event.GetPayload<wxString>();
		this->GetStatusBar()->SetLabel(message);
	}

	void Window::OnRegistrationFailed(wxThreadEvent& event) {
		wxLogFatalError("Failed to register with remote server.");
		this->Close();
	}
}