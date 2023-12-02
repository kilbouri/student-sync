#pragma once

#include <wx/wx.h>
#include <wx/generic/statbmpg.h>

class WXDLLIMPEXP_CORE VideoFrameBitmap : public wxStaticBitmapBase
{
public:
	VideoFrameBitmap(wxWindow* parent,
						  wxWindowID id,
						  const wxBitmapBundle& bitmap,
						  const wxPoint& pos = wxDefaultPosition,
						  const wxSize& size = wxDefaultSize,
						  long style = 0,
						  const wxString& name = wxASCII_STR(wxStaticBitmapNameStr))
	{
		Create(parent, id, bitmap, pos, size, style, name);
	}

	bool Create(wxWindow* parent,
				wxWindowID id,
				const wxBitmapBundle& bitmap,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = 0,
				const wxString& name = wxASCII_STR(wxStaticBitmapNameStr));

	virtual void SetBitmap(const wxBitmapBundle& bitmap) wxOVERRIDE
	{
		m_bitmapBundle = bitmap;
		InvalidateBestSize();
		SetSize(GetBestSize());
		Refresh(false);
	}

	virtual void SetScaleMode(ScaleMode scaleMode) wxOVERRIDE
	{
		m_scaleMode = scaleMode;
		Refresh(false);
	}

	virtual ScaleMode GetScaleMode() const wxOVERRIDE { return m_scaleMode; }

private:
	ScaleMode m_scaleMode;
	void OnPaint(wxPaintEvent& event);
};
