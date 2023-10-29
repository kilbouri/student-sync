#pragma once

#include <wx/wx.h>

class StartupWindow : public wxFrame
{
public:
	StartupWindow();
private:
	void OnClient(wxCommandEvent& event);
	void OnServer(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
};

enum
{
	ID_Client,
	ID_Server,
};
