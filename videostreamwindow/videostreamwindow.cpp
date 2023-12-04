#include "videostreamwindow.h"
#include <wx/generic/statbmpg.h>

VideoStreamWindow::VideoStreamWindow(wxFrame* parent, wxString title, wxBitmap firstFrame)
	: wxFrame(parent, wxID_ANY, title)
{
	videoFrame = new VideoFrameBitmap(this, wxID_ANY);
	videoFrame->SetBackgroundColour(wxColor(18, 18, 18));
	videoFrame->SetScaleMode(VideoFrameBitmap::ScaleMode::Scale_AspectFit);

	wxBoxSizer* contentSizer = new wxBoxSizer(wxVERTICAL);
	contentSizer->Add(videoFrame, wxEXPAND);

	this->SetSizer(contentSizer);
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->Layout();
	this->Centre(wxBOTH);

	Bind(wxEVT_SIZE, &VideoStreamWindow::OnResizeMonkeyPatch, this);
}

void VideoStreamWindow::SetFrame(wxBitmap nextFrame) {
	videoFrame->SetBitmap(nextFrame);
	Refresh(false);
	Layout();
}

void VideoStreamWindow::StreamEnded() {
	Close(true);
}

void VideoStreamWindow::OnResizeMonkeyPatch(wxSizeEvent& event) {
	// this is a moneky-patch that aims to fix the videoFrame not redrawing properly when the window resizes.
	// It basically looks like its not treating the previous window area as dirty, when it really should be
	Refresh(false);
	Layout();
}
