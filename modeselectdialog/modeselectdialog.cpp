#include "modeselectdialog.h"

ModeSelectDialog::ModeSelectDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxDialog(parent, id, title, pos, size, style)
{
	wxBoxSizer* contentBox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* buttonBox = new wxBoxSizer(wxHORIZONTAL);

	instructions = new wxStaticText(this, wxID_ANY, wxT("Would you like to be a <b>client</b> or <b>server</b>?"), wxDefaultPosition, wxDefaultSize, 0);
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
	this->Centre(wxBOTH);

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

void ModeSelectDialog::OnClientClicked(wxCommandEvent& event) {
	EndModal(Result::Client);
}

void ModeSelectDialog::OnServerClicked(wxCommandEvent& event) {
	EndModal(Result::Server);
}

void ModeSelectDialog::OnClosed(wxCloseEvent& event) {
	EndModal(Result::NoChoice);
}

//void ModeSelectDialog::OnClientClicked(wxCommandEvent& event) {
//	wxString serverAddress = wxGetTextFromUser("Enter your Server's Address:", "Server Address", "");
//	long serverPort = wxGetNumberFromUser("Enter the Server's Port:", "Server Port", "Enter Number", 50, 0, LONG_MAX);
//
//	ClientWindow* cFrame = new ClientWindow(serverAddress.ToStdString(), serverPort);
//	cFrame->Show(true);
//
//	Close(true);
//}
//
//void ModeSelectDialog::OnServerClicked(wxCommandEvent& event) {
//	ServerWindow* sFrame = new ServerWindow();
//	sFrame->Show(true);
//	Close(true);
//}
