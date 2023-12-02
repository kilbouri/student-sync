#pragma once

#include <wx/wx.h>
#include "videoframebitmap.h"

class VideoStreamWindow : public wxFrame {
public:
	VideoStreamWindow(wxFrame* parent, wxString title, wxBitmap firstFrame);
	void SetFrame(wxBitmap nextFrame);
	void StreamEnded();

protected:
	VideoFrameBitmap* videoFrame;
	void OnResizeMonkeyPatch(wxSizeEvent&);
};