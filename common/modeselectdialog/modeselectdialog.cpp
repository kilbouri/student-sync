#include "modeselectdialog.hpp"

namespace StudentSync::Common {
	ModeSelectDialog::ModeSelectDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
		: wxDialog(parent, id, title, pos, size, parent == nullptr ? style | wxDIALOG_NO_PARENT : style)
		, choice{ Result::NoChoice }
	{
		wxBoxSizer* contentBox = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer* buttonBox = new wxBoxSizer(wxHORIZONTAL);

		instructions = new wxStaticText(this, wxID_ANY, wxT("Would you like to be a client or server?"), wxDefaultPosition, wxDefaultSize, wxALL);
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
		this->Bind(wxEVT_CLOSE_WINDOW, &ModeSelectDialog::OnClosed, this);
		clientButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ModeSelectDialog::OnClientClicked, this);
		serverButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ModeSelectDialog::OnServerClicked, this);
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
}