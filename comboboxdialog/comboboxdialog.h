#pragma once

#include <wx/wx.h>

template <class T>
class ComboBoxDialog : public wxDialog {
public:
	ComboBoxDialog(
		wxWindow* parent = nullptr, wxWindowID id = wxID_ANY,
		const wxString& title = wxT("StudentSync"), const wxString& prompt = wxT("Select an option:"),
		wxString defaultOption = wxEmptyString, std::unordered_map<wxString, T> options = {},
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE);

	~ComboBoxDialog();

	T GetValue();

private:
	wxStaticText* message;
	wxComboBox* comboBox;
	wxStdDialogButtonSizer* dialogButtons;
	wxButton* dialogButtonsOK;
	wxButton* dialogButtonsCancel;

	std::unordered_map<wxString, T> values;

	void OnOkClicked(wxCommandEvent& event);
	void OnCancelClicked(wxCommandEvent& event);
	void OnClosed(wxCloseEvent& event);
};

