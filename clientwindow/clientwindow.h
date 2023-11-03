#pragma once

#include <wx/wx.h>

#include "../client/client.h"

class ClientWindow : public wxFrame
{
public:
	ClientWindow(wxString title, std::string_view serverHostname, int serverPort);

private:
	Client client;

	bool picture;
	wxSize newSize;
	wxBitmap originalBitmap;
	wxBitmap alternativeBitmap;
	wxBitmap currentBitmap;
	wxButton* button;
	wxStaticBitmap* imageDisplay;
	std::vector<wxBitmap> imageStream;
	wxBitmap ResizeBitmap(const wxBitmap& bitmap, const wxSize& newSize);


	void UpdateImage(const wxSize& newSize = wxDefaultSize);
	void OnSize(wxSizeEvent& event);
	void OnButtonClick(wxCommandEvent& event);
	void OnString(wxCommandEvent& event);
	void OnNumber(wxCommandEvent& event);
	void OnPNG(wxCommandEvent& event);
	void OnJPG(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
};

enum {
	ID_String,
	ID_Number,
	ID_PNG,
	ID_JPG,
};