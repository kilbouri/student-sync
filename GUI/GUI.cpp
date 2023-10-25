// For compilers that support precompilation, includes "wx/wx.h".

#include "GUI.h"
#include "../client/client.h"
#include "../libs/wxWidgets/interface/wx/numdlg.h"

std::string serverAddress;
long serverPort;

bool MyApp::OnInit() //initiates the frame: frame, 
{
    MyFrame* frame = new MyFrame();
    frame->Show(true);// MAKE SURE TO ALWAYS DO THIS OR YOU WILL BE GASLIT
    return true;
}

MyFrame::MyFrame() : wxFrame(NULL, wxID_ANY, "NOTSPYWARE")
{
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Client, "&I am a Client!...\tCtrl-C",
        "Opening a client window");

    menuFile->Append(ID_Server, "&I am a Server!...\tCtrl-S",
        "Opening a server window");

    menuFile->AppendSeparator();

    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Not a keylogger either!");

    Bind(wxEVT_MENU, &MyFrame::OnClient, this, ID_Client);
    Bind(wxEVT_MENU, &MyFrame::OnServer, this, ID_Server);

    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);//About and exit already have their own ID thats built-in, hence wxID
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}

void MyFrame::OnExit(wxCommandEvent& event) { Close(true); }

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is NOT a Virus or Spyware!",
        "About this Spyware", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnClient(wxCommandEvent& event)//opens up the client window when selected, hopefully
{
    serverAddress = wxGetTextFromUser("Enter your Server's Address:", "Server Address", "");
    long serverPort = wxGetNumberFromUser("Enter the Server's Port:", "Server Port", "Enter Number", 50, 0, LONG_MAX);

    clientFrame* cFrame = new clientFrame();
    cFrame->Show(true);// always do this still.
    Close(true);
}

void MyFrame::OnServer(wxCommandEvent& event)
{
    serverFrame* sFrame = new serverFrame();
    sFrame->Show(true);
    Close(true);
}

clientFrame::clientFrame() : wxFrame(NULL, wxID_ANY, "ClientWindow")
{

    wxMenu* menuFile = new wxMenu;

    menuFile->Append(ID_String, "&Send A String...\tCtrl-S",
        "Send a String to the Server");

    menuFile->Append(ID_Number, "&Send A Number...\tCtrl-C",
        "Send a Number to the Server");

    menuFile->AppendSeparator();

    menuFile->Append(ID_JPG, "&Send A Screenshot in JPG format...\tCtrl-J",
        "Send a JPG to the Server");

    menuFile->Append(ID_PNG, "&Send A Screenshot in PNG format...\tCtrl-P",
        "Send a PNG to the Server");

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

    Bind(wxEVT_MENU, &clientFrame::OnString, this, ID_String);
    Bind(wxEVT_MENU, &clientFrame::OnNumber, this, ID_Number);
    Bind(wxEVT_MENU, &clientFrame::OnJPG, this, ID_JPG);
    Bind(wxEVT_MENU, &clientFrame::OnPNG, this, ID_PNG);

    Bind(wxEVT_MENU, &clientFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &clientFrame::OnExit, this, wxID_EXIT);

}
void clientFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void clientFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("You're in the client software poggers!",
        "About this Software", wxOK | wxICON_INFORMATION);
}

Client client(serverAddress, serverPort);

void clientFrame::OnString(wxCommandEvent& event) {

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

void clientFrame::OnNumber(wxCommandEvent& event) {

    long result = wxGetNumberFromUser("Enter a number:", "Number Entry", "Enter Number", 50, 0, LONG_MAX);
    if (result != -1) {
        client.SendNumber(result);
        wxMessageBox("Sending '" + wxString::Format("%ld", result) + "' to the server", "Number Entry Result", wxOK | wxICON_INFORMATION);
    }
    else {
        wxMessageBox("You canceled the number entry.", "Number Entry Result", wxOK | wxICON_INFORMATION);
    }
}

void clientFrame::OnJPG(wxCommandEvent& event) {

    client.SendScreenshot(DisplayCapturer::Format::JPG);
    wxLogMessage("Sending JPG...");
}

void clientFrame::OnPNG(wxCommandEvent& event) {

    client.SendScreenshot(DisplayCapturer::Format::PNG);
    wxLogMessage("Sending PNG...");
}

serverFrame::serverFrame() : wxFrame(NULL, wxID_ANY, "ServerWindow")
{
    wxMenu* menuFile = new wxMenu;

    menuFile->Append(ID_Details, "&Server details...\tCtrl-D",
        "Server Connection Details");

    menuFile->AppendSeparator();

    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Server!");

    Bind(wxEVT_MENU, &serverFrame::OnDetails, this, ID_Details);
    Bind(wxEVT_MENU, &serverFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &serverFrame::OnExit, this, wxID_EXIT);

}
void serverFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void serverFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("You're in the Server software poggers!", "About this Software", wxOK | wxICON_INFORMATION);
}

void serverFrame::OnDetails(wxCommandEvent& event)//Either in about or in here I want to put the server connection details, ip and port for example.
{
    wxMessageBox("Deez Nutz", "About this Software", wxOK | wxICON_INFORMATION);
}
