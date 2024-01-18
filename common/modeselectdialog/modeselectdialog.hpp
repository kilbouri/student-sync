#pragma once

#include <wx/wx.h>

namespace StudentSync::Common {
	/// <summary>
	/// Displays a modal which asks the user whether they want the program to start
	/// as a client or a server. The ModeSelectDialog::Result enum enumerates all
	/// possible results from this dialog.
	/// 
	/// It is safe to call GetValue() at any point in the dialog's lifecycle, however
	/// the only time it contains a meaningful value is after ShowModal() has returned
	/// wxID_OK.
	/// 
	/// Please be aware that you are responsible for destroying any instances you create.
	/// </summary>
	class ModeSelectDialog : public wxDialog {
	public:
		enum class Result {
			NoChoice,
			Server,
			Client
		};

		ModeSelectDialog(wxWindow* parent = nullptr, wxWindowID id = wxID_ANY, const wxString& title = wxT("StudentSync"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);

		ModeSelectDialog::Result GetValue();

	private:
		wxStaticText* instructions;
		wxButton* clientButton;
		wxButton* serverButton;

		ModeSelectDialog::Result choice;

		void OnClientClicked(wxCommandEvent& event);
		void OnServerClicked(wxCommandEvent& event);
		void OnClosed(wxCloseEvent& event);
	};
}