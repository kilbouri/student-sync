#include "clientwindow.h"

#include <wx/numdlg.h>

ClientWindow::ClientWindow(wxString title, std::string_view serverHostname, int serverPort)
	: wxFrame(NULL, wxID_ANY, title), client{ Client(serverHostname, serverPort) }
{
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

	CreateStatusBar();
	SetStatusText("Client!");

	Bind(wxEVT_MENU, &ClientWindow::OnString, this, ID_String);
	Bind(wxEVT_MENU, &ClientWindow::OnNumber, this, ID_Number);
	Bind(wxEVT_MENU, &ClientWindow::OnJPG, this, ID_JPG);
	Bind(wxEVT_MENU, &ClientWindow::OnPNG, this, ID_PNG);

	Bind(wxEVT_MENU, &ClientWindow::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &ClientWindow::OnExit, this, wxID_EXIT);

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

	wxString result = wxGetTextFromUser("Enter your text:", "Text Entry", "");
	if (!result.empty()) {
		std::string strToSend = result.ToStdString();
		client.SendString(strToSend);
		wxMessageBox("Sending '" + result + "' to the server", "Text Entry Result", wxOK | wxICON_INFORMATION);
	}
	else {
		wxMessageBox("You canceled the text entry.", "Text Entry Result", wxOK | wxICON_INFORMATION);
	}
}

void ClientWindow::OnNumber(wxCommandEvent& event) {

	long result = wxGetNumberFromUser("Enter a number:", "Number Entry", "Enter Number", 50, 0, LONG_MAX);
	if (result != -1) {
		client.SendNumber(result);
		wxMessageBox("Sending '" + wxString::Format("%ld", result) + "' to the server", "Number Entry Result", wxOK | wxICON_INFORMATION);
	}
	else {
		wxMessageBox("You canceled the number entry.", "Number Entry Result", wxOK | wxICON_INFORMATION);
	}
}

void ClientWindow::OnJPG(wxCommandEvent& event) {

	client.SendScreenshot(DisplayCapturer::Format::JPG);
	wxLogMessage("Sending JPG...");
}

void ClientWindow::OnPNG(wxCommandEvent& event) {

	client.SendScreenshot(DisplayCapturer::Format::PNG);
	wxLogMessage("Sending PNG...");
}