#include "comboboxdialog.h";

template <class T>
ComboBoxDialog<T>::ComboBoxDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxString& prompt, wxString defaultOption, std::unordered_map<wxString, T> options, const wxPoint& pos, const wxSize& size, long style)
	: wxDialog(parent, id, title, pos, size, parent == nullptr ? style | wxDIALOG_NO_PARENT : style),
	values{ options }
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

template <class T>
ComboBoxDialog<T>::~ComboBoxDialog()
{
	// Disconnect Events
	this->Disconnect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(ComboBoxDialog<T>::OnClosed));
	dialogButtonsCancel->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ComboBoxDialog<T>::OnCancelClicked), NULL, this);
	dialogButtonsOK->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ComboBoxDialog<T>::OnOkClicked), NULL, this);
}

template <class T>
T ComboBoxDialog<T>::GetValue() {
	return values.at(comboBox->GetStringSelection());
}

template <class T>
void ComboBoxDialog<T>::OnOkClicked(wxCommandEvent& event) {
	EndDialog(wxID_OK);
}

template <class T>
void ComboBoxDialog<T>::OnCancelClicked(wxCommandEvent& event) {
	EndDialog(wxID_CANCEL);
}

template <class T>
void ComboBoxDialog<T>::OnClosed(wxCloseEvent& event) {
	EndDialog(wxID_CLOSE);
}
