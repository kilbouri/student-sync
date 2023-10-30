#pragma once

#include <wx/wx.h>

template <class T>
class ComboBoxDialog : public wxDialog {
private:
	wxStaticText* message;
	wxComboBox* comboBox;
	wxStdDialogButtonSizer* dialogButtons;
	wxButton* dialogButtonsOK;
	wxButton* dialogButtonsCancel;

	std::unordered_map<wxString, T> values;

	void OnOkClicked(wxCommandEvent& event) {
		EndDialog(wxID_OK);
	}

	void OnCancelClicked(wxCommandEvent& event) {
		EndDialog(wxID_CANCEL);
	}

	void OnClosed(wxCloseEvent& event) {
		EndDialog(wxID_CLOSE);
	}

public:
	ComboBoxDialog(
		wxWindow* parent = nullptr, wxWindowID id = wxID_ANY,
		const wxString& title = wxT("ComboBoxDialog"), const wxString& prompt = wxT("Select an option:"),
		wxString defaultOption = wxEmptyString, std::unordered_map<wxString, T> options = {},
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE) : wxDialog(parent, id, title, pos, size, parent == nullptr ? style | wxDIALOG_NO_PARENT : style), values{ options }
	{
		wxBoxSizer* contentBox = new wxBoxSizer(wxVERTICAL);

		message = new wxStaticText(this, wxID_ANY, prompt, wxDefaultPosition, wxDefaultSize, 0);
		message->Wrap(-1);

		comboBox = new wxComboBox(this, wxID_ANY, defaultOption, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);

		int index = 0;
		for (std::pair<wxString, T> thing : options) {
			comboBox->Insert(thing.first, index++);
		}

		comboBox->SetStringSelection(defaultOption);

		dialogButtonsOK = new wxButton(this, wxID_OK);
		dialogButtonsCancel = new wxButton(this, wxID_CANCEL);

		dialogButtons = new wxStdDialogButtonSizer();
		dialogButtons->AddButton(dialogButtonsOK);
		dialogButtons->AddButton(dialogButtonsCancel);
		dialogButtons->Realize();

		contentBox->Add(message, 0, wxALL, 5);
		contentBox->Add(comboBox, 0, wxALL | wxEXPAND, 5);
		contentBox->Add(dialogButtons, 1, wxALIGN_RIGHT | wxBOTTOM, 5);
		contentBox->Fit(this);

		this->SetSizer(contentBox);
		this->SetSizeHints(wxDefaultSize, wxDefaultSize);
		this->Center(wxBOTH);

		// Connect Events
		this->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(ComboBoxDialog<T>::OnClosed));
		dialogButtonsCancel->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ComboBoxDialog<T>::OnCancelClicked), NULL, this);
		dialogButtonsOK->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ComboBoxDialog<T>::OnOkClicked), NULL, this);
	}

	~ComboBoxDialog()
	{
		// Disconnect Events
		this->Disconnect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(ComboBoxDialog<T>::OnClosed));
		dialogButtonsCancel->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ComboBoxDialog<T>::OnCancelClicked), NULL, this);
		dialogButtonsOK->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ComboBoxDialog<T>::OnOkClicked), NULL, this);
	}

	T GetValue() {
		return values.at(comboBox->GetStringSelection());
	}
};

