#pragma once

#include <optional>

#include <wx/wx.h>
#include <wx/generic/statbmpg.h>

class VideoFrameBitmap : public wxStaticBitmapBase {
public:
	VideoFrameBitmap(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = wxASCII_STR(wxStaticBitmapNameStr));

	virtual void SetBitmap(const wxBitmapBundle& bitmap) wxOVERRIDE;
	virtual void ClearBitmap();

	virtual ScaleMode GetScaleMode() const wxOVERRIDE;
	virtual void SetScaleMode(ScaleMode scaleMode) wxOVERRIDE;

	virtual wxSize DoGetBestClientSize() const wxOVERRIDE;

private:
	ScaleMode scaleMode;
	bool hasBitmap;

	bool Create(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = wxASCII_STR(wxStaticBitmapNameStr));

	void OnPaint(wxPaintEvent& event);
};
