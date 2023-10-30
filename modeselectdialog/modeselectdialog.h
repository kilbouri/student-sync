#pragma once

#include <wx/wx.h>
#include <wx/spinctrl.h>

/// <summary>
/// Displays a modal which asks the user whether they want the program to start
/// as a client or a server.
/// 
/// You MUST use ShowModal() to display this dialog. Using Show() will cause all kinds of issues,
/// including not blocking to wait for input. ShowModal() will return the result the user chose,
/// which will be an int as enumerated in ModeSelectDialog::Result.
/// 
/// Please also be aware that you are responsible for destroying any instances you create.
/// </summary>
class ModeSelectDialog : public wxDialog {
private:
	wxStaticText* instructions;
	wxButton* clientButton;
	wxButton* serverButton;

	void OnClientClicked(wxCommandEvent& event);
	void OnServerClicked(wxCommandEvent& event);
	void OnClosed(wxCloseEvent& event);

public:
	enum Result {
		NoChoice,
		Server,
		Client
	};

	ModeSelectDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("StudentSync"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE | wxDIALOG_NO_PARENT);
	~ModeSelectDialog();
};