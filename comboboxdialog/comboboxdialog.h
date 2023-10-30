#pragma once

#include <wx/wx.h>

/// <summary>
/// Displays a modal which asks the user whether they want the program to start
/// as a client or a server.
/// 
/// You MUST use ShowModal() to display this dialog. Using Show() will cause all kinds of issues,
/// including not blocking to wait for input. ShowModal() will return the result the user chose,
/// which will be an int index into the options vector representing the choice. The only exception is
/// ComboBoxDialog::SELECTION_CANCELED, which indicates the user did not make a choice.
/// 
/// Please also be aware that you are responsible for destroying any instances you create.
/// </summary>
class ComboBoxDialog : public wxDialog {
public:
	const static int SELECTION_CANCELED = -1;

	ComboBoxDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("ComboBoxDialog"), const wxString& prompt = wxT("Select an option:"), int defaultOptionIndex = 0, std::vector<wxString> options = {});
	~ComboBoxDialog();

private:
	wxStaticText* message;
	wxComboBox* comboBox;
	wxStdDialogButtonSizer* dialogButtons;
	wxButton* dialogButtonsOK;
	wxButton* dialogButtonsCancel;

	void OnOkClicked(wxCommandEvent& event);
	void OnCancelClicked(wxCommandEvent& event);
	void OnClosed(wxCloseEvent& event);
};

