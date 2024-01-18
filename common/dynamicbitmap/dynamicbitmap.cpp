#include "dynamicbitmap.hpp"

#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/scopedptr.h>

namespace StudentSync::Common {
	DynamicBitmap::DynamicBitmap(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
		: hasBitmap{ false }
		, scaleMode{ ScaleMode::Scale_None }
	{
		Create(parent, id, pos, size, style, name);
	}

	bool DynamicBitmap::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) {
		// otherwise you'll get partial repaints which take fucking ages to debug (-IK, 4 hours debugging)
		style |= wxFULL_REPAINT_ON_RESIZE;

		if (!wxControl::Create(parent, id, pos, size, style, wxDefaultValidator, name)) {
			return false;
		}

		// Don't call SetBitmap() here, as it changes the size and refreshes the
		// window unnecessarily, when we don't need either of these side effects
		// here.
		m_bitmapBundle = wxNullBitmap;
		SetInitialSize(size);

		// This is recommended by wxAutoBufferingPaintDC used in OnPaint.
		// Prevents the screen from being cleared before we draw ourselves.
		this->SetBackgroundStyle(wxBG_STYLE_PAINT);

		Bind(wxEVT_PAINT, &DynamicBitmap::OnPaint, this);
		return true;
	}

	wxSize DynamicBitmap::DoGetBestClientSize() const {
		if (this->hasBitmap && this->GetBitmap().IsOk()) {
			return this->GetBitmap().GetSize();
		}

		return wxDefaultSize;
	}

	void DynamicBitmap::OnPaint(wxPaintEvent& event) {
		wxAutoBufferedPaintDC dc(this);
		wxBitmap bitmap = GetBitmap();

		// skip repaint if the bitmap we have is bad
		if (hasBitmap && !bitmap.IsOk()) {
			return;
		}

		dc.Clear();

		// if we don't have a bitmap, all we need to do is clear
		if (!hasBitmap) {
			return;
		}

		wxSize clientSize = this->GetClientSize();
		if (!clientSize.x || !clientSize.y) {
			return;
		}

		const wxSize bitmapSize = bitmap.GetSize();
		if (!bitmapSize.x || !bitmapSize.y) {
			return;
		}

		wxDouble drawWidth = 0;
		wxDouble drawHeight = 0;

		switch (scaleMode) {
			// No rescaling
			case Scale_None:
				drawWidth = bitmapSize.GetWidth();
				drawHeight = bitmapSize.GetHeight();
				break;

			// Non-aspect-preserving stretch to fill client area
			case Scale_Fill:
				drawWidth = clientSize.x;
				drawHeight = clientSize.y;
				break;

			// Aspect-preserving fill/fit
			case Scale_AspectFill:
			case Scale_AspectFit: {
				double xFactor = (double)clientSize.x / bitmapSize.x; /* UK television is great */
				double yFactor = (double)clientSize.y / bitmapSize.y;

				double fitFactor = (xFactor > yFactor) ? yFactor : xFactor; // fit takes the smaller factor
				double fillFactor = (xFactor > yFactor) ? xFactor : yFactor; // fill takes the larger factor

				double factor = (scaleMode == Scale_AspectFit) ? fitFactor : fillFactor;

				drawWidth = factor * bitmapSize.GetWidth();
				drawHeight = factor * bitmapSize.GetHeight();
				break;
			}

			default:
				wxFAIL_MSG("Unknown scale mode");
		}

		wxDouble drawX = (clientSize.x - drawWidth) / 2;
		wxDouble drawY = (clientSize.y - drawHeight) / 2;

		if (scaleMode == ScaleMode::Scale_None) {
			drawX = 0;
			drawY = 0;
		}

		// Use a graphics context to draw the bitmap if possible.
		if constexpr (wxUSE_GRAPHICS_CONTEXT) {
			wxScopedPtr<wxGraphicsContext> gc(wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(dc));
			gc->DrawBitmap(bitmap, drawX, drawY, drawWidth, drawHeight);
		}
		else {
			wxBitmap::Rescale(bitmap, wxSize(wxRound(drawWidth), wxRound(drawHeight)));
			dc.DrawBitmap(bitmap, wxRound(drawX), wxRound(drawY), true);
		}
	}

	void DynamicBitmap::SetBitmap(const wxBitmapBundle& bitmap) {
		this->m_bitmapBundle = bitmap;
		this->hasBitmap = true;

		InvalidateBestSize();
		SetSize(GetBestSize());

		// this is such a hack but it does work...
		this->GetContainingSizer()->Layout();

		// We need to redraw
		this->Refresh();
	}

	void DynamicBitmap::ClearBitmap() {
		this->hasBitmap = false;
		this->m_bitmapBundle = wxNullBitmap;

		this->GetContainingSizer()->Layout();
		this->Refresh();
	}

	wxStaticBitmapBase::ScaleMode DynamicBitmap::GetScaleMode() const {
		return scaleMode;
	}

	void DynamicBitmap::SetScaleMode(wxStaticBitmapBase::ScaleMode mode) {
		this->scaleMode = mode;
		this->Refresh();
	}
}