#include "modeselectdialog.h"

ModeSelectDialog::ModeSelectDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxDialog(parent, id, title, pos, size, style), choice{ NoChoice }
{
	wxBoxSizer* contentBox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* buttonBox = new wxBoxSizer(wxHORIZONTAL);

	instructions = new wxStaticText(this, wxID_ANY, wxT("Would you like to be a <b>client</b> or <b>server</b>?"), wxDefaultPosition, wxDefaultSize, wxALL);
	instructions->SetLabelMarkup(wxT("Would you like to be a <b>client</b> or <b>server</b>?"));
	instructions->Wrap(-1);

	serverButton = new wxButton(this, wxID_ANY, wxT("Server"), wxDefaultPosition, wxDefaultSize, 0);
	clientButton = new wxButton(this, wxID_ANY, wxT("Client"), wxDefaultPosition, wxDefaultSize, 0);
	clientButton->SetDefault();

	buttonBox->Add(clientButton, 0, wxALL, 5);
	buttonBox->Add(serverButton, 0, wxALL, 5);

	contentBox->Add(instructions, 0, wxALL, 5);
	contentBox->Add(buttonBox, 1, wxALIGN_RIGHT, 5);
	contentBox->Fit(this);

	this->SetSizer(contentBox);
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->Center(wxBOTH);

	// Connect Events
	this->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(ModeSelectDialog::OnClosed));
	clientButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ModeSelectDialog::OnClientClicked), NULL, this);
	serverButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ModeSelectDialog::OnServerClicked), NULL, this);
}

ModeSelectDialog::~ModeSelectDialog() {
	this->Disconnect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(ModeSelectDialog::OnClosed));
	clientButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ModeSelectDialog::OnClientClicked), NULL, this);
	serverButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ModeSelectDialog::OnServerClicked), NULL, this);
}

ModeSelectDialog::Result ModeSelectDialog::GetValue() {
	return choice;
}

void ModeSelectDialog::OnClientClicked(wxCommandEvent& event) {
	choice = Result::Client;
	EndModal(wxID_OK);
}

void ModeSelectDialog::OnServerClicked(wxCommandEvent& event) {
	choice = Result::Server;
	EndModal(wxID_OK);
}

void ModeSelectDialog::OnClosed(wxCloseEvent& event) {
	choice = Result::NoChoice;
	EndModal(wxID_CLOSE);
}