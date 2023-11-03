#include "clientwindow.h"

#include <wx/numdlg.h>

ClientWindow::ClientWindow(wxString title, std::string_view serverHostname, int serverPort)
	: wxFrame(NULL, wxID_ANY, title), client{ Client{} }
{

	// Simulate receiving a stream of images

	wxImage::AddHandler(new wxPNGHandler());
	picture = true;

	originalBitmap = wxBitmap(wxImage("../../../MNM.png", wxBITMAP_TYPE_PNG));
	alternativeBitmap = wxBitmap(wxImage("../../../Sans_Shady.png", wxBITMAP_TYPE_PNG));

	imageStream.push_back(originalBitmap);
	imageStream.push_back(alternativeBitmap);

	button = new wxButton(this, wxID_ANY, "Toggle Image", wxPoint(100, 100), wxDefaultSize);
	button->Bind(wxEVT_BUTTON, &ClientWindow::OnButtonClick, this);

	imageDisplay = new wxStaticBitmap(this, wxID_ANY, originalBitmap, wxDefaultPosition, wxDefaultSize);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	mainSizer->Add(button, 0, wxALIGN_CENTER | wxALL, 10);
	mainSizer->Add(imageDisplay, 1, wxEXPAND | wxALL, 10);

	SetSizer(mainSizer);

	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_String, "&Send A String...\tCtrl-S", "Send a String to the Server");
	menuFile->Append(ID_Number, "&Send A Number...\tCtrl-C", "Send a Number to the Server");
	menuFile->AppendSeparator();
	menuFile->Append(ID_JPG, "&Send A Screenshot in JPG format...\tCtrl-J", "Send a JPG to the Server");
	menuFile->Append(ID_PNG, "&Send A Screenshot in PNG format...\tCtrl-P", "Send a PNG to the Server");

	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	Bind(wxEVT_MENU, &ClientWindow::OnString, this, ID_String);
	Bind(wxEVT_MENU, &ClientWindow::OnNumber, this, ID_Number);
	Bind(wxEVT_MENU, &ClientWindow::OnJPG, this, ID_JPG);
	Bind(wxEVT_MENU, &ClientWindow::OnPNG, this, ID_PNG);

	Bind(wxEVT_MENU, &ClientWindow::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &ClientWindow::OnExit, this, wxID_EXIT);

	Bind(wxEVT_SIZE, &ClientWindow::OnSize, this);

	if (!client.Connect(serverHostname, serverPort)) {
		wxLogFatalError("Failed to connect to server");
	}
}
void ClientWindow::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void ClientWindow::OnAbout(wxCommandEvent& event)
{
	wxMessageBox("You're in the client software poggers!",
		"About this Software", wxOK | wxICON_INFORMATION);
}

void ClientWindow::OnString(wxCommandEvent& event) {
	wxTextEntryDialog stringDialog(this, "Enter a string:", "StudentSync - Client");
	if (stringDialog.ShowModal() != wxID_OK) {
		return;
	}

	std::string string = stringDialog.GetValue().ToStdString();
	if (!client.SendString(string)) {
		wxMessageBox("Failed to send '" + string + "' to the server", "StudentSync - Client", wxOK | wxICON_WARNING);
	}
	else {
		wxMessageBox("Sent '" + string + "' to the server", "StudentSync - Client", wxOK | wxICON_INFORMATION);
	}
}

void ClientWindow::OnNumber(wxCommandEvent& event) {
	wxTextEntryDialog stringDialog(this, "Enter a number:", "StudentSync - Client");
	if (stringDialog.ShowModal() != wxID_OK) {
		return;
	}

	int64_t number = 0;
	if (!stringDialog.GetValue().ToLongLong(&number)) {
		wxMessageBox("Invalid number entered");
		return;
	}

	if (!client.SendNumber(number)) {
		wxMessageBox("Failed to send '" + std::to_string(number) + "' to the server", "StudentSync - Client", wxOK | wxICON_WARNING);
	}
	else {
		wxMessageBox("Sent '" + std::to_string(number) + "' to the server", "StudentSync - Client", wxOK | wxICON_INFORMATION);
	}
}

void ClientWindow::OnJPG(wxCommandEvent& event) {
	if (!client.SendScreenshot(DisplayCapturer::Format::JPG)) {
		wxMessageBox("Failed to send JPG screenshot to the server", "StudentSync - Client", wxOK | wxICON_WARNING);
	}
	else {
		wxMessageBox("Sent JPG screenshot to the server", "StudentSync - Client", wxOK | wxICON_INFORMATION);
	}
}

void ClientWindow::OnPNG(wxCommandEvent& event) {
	if (!client.SendScreenshot(DisplayCapturer::Format::PNG)) {
		wxMessageBox("Failed to send PNG screenshot to the server", "StudentSync - Client", wxOK | wxICON_WARNING);
	}
	else {
		wxMessageBox("Sent PNG screenshot to the server", "StudentSync - Client", wxOK | wxICON_INFORMATION);
	}
}

void ClientWindow::OnButtonClick(wxCommandEvent& event) {
	picture = !picture;
	newSize = GetClientSize();
	UpdateImage(newSize);
}

void ClientWindow::OnSize(wxSizeEvent& event) {
	newSize = GetClientSize();
	UpdateImage(newSize);
}

void ClientWindow::UpdateImage(const wxSize& newSize) {
	originalBitmap = imageStream.at(0);
	alternativeBitmap = imageStream.at(1);


	if (picture) {
		originalBitmap = ResizeBitmap(originalBitmap, newSize);
		currentBitmap = originalBitmap;
	}
	else {
		alternativeBitmap = ResizeBitmap(alternativeBitmap, newSize);
		currentBitmap = alternativeBitmap;
	}

	imageDisplay->SetBitmap(currentBitmap);

}

wxBitmap ClientWindow::ResizeBitmap(const wxBitmap& bitmap, const wxSize& newSize) {
	wxImage image = bitmap.ConvertToImage();
	image.Rescale(newSize.GetWidth(), newSize.GetHeight(), wxIMAGE_QUALITY_HIGH);
	return wxBitmap(image);
}