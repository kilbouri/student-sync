#pragma once

#include <wx/wx.h>

#include "../client/client.h"

class ClientWindow : public wxFrame, public wxTimer
{
public:
	ClientWindow(wxString title, std::string_view serverHostname, int serverPort);

private:
	Client client;

	void OnString(wxCommandEvent& event);
	void OnNumber(wxCommandEvent& event);

	void OnStartStream(wxCommandEvent& event);
	void OnSendNextFrame(wxCommandEvent& event);
	void OnEndStream(wxCommandEvent& event);

	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnShowPreferences(wxCommandEvent& event);

	virtual void Notify();
};

enum {
	ID_String,
	ID_Number,
	ID_StartStream,
	ID_NextFrame,
	ID_EndStream,
	ID_ShowPreferences,
};