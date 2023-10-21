#pragma once

#include <wx/wx.h>

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
    MyFrame();
private:
    void OnClient(wxCommandEvent& event);
    void OnServer(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
};

class clientFrame : public wxFrame
{
public:
    clientFrame();
private:
    void OnString(wxCommandEvent& event);
    void OnNumber(wxCommandEvent& event);
    void OnPNG(wxCommandEvent& event);
    void OnJPG(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
};

class serverFrame : public wxFrame
{
public:
    serverFrame();
private:
    void OnDetails(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
};

enum
{
    ID_Client = 1,
    ID_Server = 2,
    ID_String = 3,
    ID_Number = 4,
    ID_PNG = 5,
    ID_JPG = 6,
    ID_Details = 7
};

wxIMPLEMENT_APP(MyApp);

