#include "comboboxdialog.h";

ComboBoxDialog::ComboBoxDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxString& prompt, int defaultOptionIndex, std::vector<wxString> options)
	: wxDialog(parent, id, title)
{
	wxBoxSizer* contentBox = new wxBoxSizer(wxVERTICAL);

	message = new wxStaticText(this, wxID_ANY, prompt, wxDefaultPosition, wxDefaultSize, 0);
	message->Wrap(-1);

	comboBox = new wxComboBox(this, wxID_ANY, options[defaultOptionIndex], wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	comboBox->Insert(options, 0);
	comboBox->SetStringSelection(options[defaultOptionIndex]);

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
	this->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(ComboBoxDialog::OnClosed));
	dialogButtonsCancel->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ComboBoxDialog::OnCancelClicked), NULL, this);
	dialogButtonsOK->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ComboBoxDialog::OnOkClicked), NULL, this);
}

ComboBoxDialog::~ComboBoxDialog()
{
	// Disconnect Events
	this->Disconnect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(ComboBoxDialog::OnClosed));
	dialogButtonsCancel->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ComboBoxDialog::OnCancelClicked), NULL, this);
	dialogButtonsOK->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ComboBoxDialog::OnOkClicked), NULL, this);
}

void ComboBoxDialog::OnOkClicked(wxCommandEvent& event) {
	EndDialog(comboBox->GetSelection());
}

void ComboBoxDialog::OnCancelClicked(wxCommandEvent& event) {
	EndDialog(ComboBoxDialog::SELECTION_CANCELED);
}

void ComboBoxDialog::OnClosed(wxCloseEvent& event) {
	EndDialog(ComboBoxDialog::SELECTION_CANCELED);
}
