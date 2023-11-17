#pragma once

#include <wx/wx.h>
#include <wx/generic/statbmpg.h>

class VideoStreamWindow : public wxFrame {
public:
	VideoStreamWindow(wxFrame* parent, wxString title, wxBitmap firstFrame);
	void SetFrame(wxBitmap nextFrame);
	void StreamEnded();

protected:
	wxGenericStaticBitmap* videoFrame;
	void OnResizeMonkeyPatch(wxSizeEvent&);
};