#pragma once

#include <wx/wx.h>

#include "../client/client.h"

class ClientWindow : public wxFrame
{
public:
	ClientWindow(wxString title, std::string_view serverHostname, int serverPort);

private:
	Client client;

	void OnString(wxCommandEvent& event);
	void OnNumber(wxCommandEvent& event);
	void OnPNG(wxCommandEvent& event);
	void OnJPG(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
};

enum {
	ID_String,
	ID_Number,
	ID_PNG,
	ID_JPG,
};