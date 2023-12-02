#include "videoframebitmap.h"

#include <wx/dcbuffer.h>
#if wxUSE_GRAPHICS_CONTEXT
#include "wx/graphics.h"
#include "wx/scopedptr.h"
#else
#include "wx/image.h"
#include "wx/math.h"
#endif

bool VideoFrameBitmap::Create(wxWindow* parent, wxWindowID id, const wxBitmapBundle& bitmap, const wxPoint& pos, const wxSize& size, long style, const wxString& name) {
	if (!wxControl::Create(parent, id, pos, size, style, wxDefaultValidator, name)) {
		return false;
	}

	// Don't call SetBitmap() here, as it changes the size and refreshes the
	// window unnecessarily, when we don't need either of these side effects
	// here.
	m_bitmapBundle = bitmap;
	SetInitialSize(size);

	// This is recommended by wxAutoBufferingPaintDC used in OnPaint.
	// Prevents the screen from being cleared before we draw ourselves.
	this->SetBackgroundStyle(wxBG_STYLE_PAINT);

	Bind(wxEVT_PAINT, &VideoFrameBitmap::OnPaint, this);
	return true;
}

void VideoFrameBitmap::OnPaint(wxPaintEvent& WXUNUSED(event)) {
	wxAutoBufferedPaintDC dc(this);

	if (!m_bitmapBundle.IsOk()) {
		return;
	}

	const wxSize drawSize = GetClientSize();
	if (!drawSize.x || !drawSize.y) {
		return;
	}

	// Paint background first
	// TODO: we don't always need to do this. Perhaps a clear flag + SetBitmap check?
	dc.Clear();

	// Paint scaled bitmap
	wxBitmap bitmap = GetBitmap();
	const wxSize bmpSize = bitmap.GetSize();

	wxDouble w = 0;
	wxDouble h = 0;

	switch (m_scaleMode) {
		case Scale_None: {
			dc.DrawBitmap(bitmap, 0, 0, true);
			return;
		}
		case Scale_Fill:
			w = drawSize.x;
			h = drawSize.y;
			break;
		case Scale_AspectFill:
		case Scale_AspectFit: {
			wxDouble scaleFactor;
			wxDouble scaleX = (wxDouble)drawSize.x / (wxDouble)bmpSize.x;
			wxDouble scaleY = (wxDouble)drawSize.y / (wxDouble)bmpSize.y;
			if ((m_scaleMode == Scale_AspectFit && scaleY < scaleX) ||
				 (m_scaleMode == Scale_AspectFill && scaleY > scaleX))
				scaleFactor = scaleY;
			else
				scaleFactor = scaleX;

			w = bmpSize.x * scaleFactor;
			h = bmpSize.y * scaleFactor;

			break;
		}
		default:
			wxFAIL_MSG("Unknown scale mode");
	}

	wxDouble x = (drawSize.x - w) / 2;
	wxDouble y = (drawSize.y - h) / 2;

#if wxUSE_GRAPHICS_CONTEXT
	wxScopedPtr<wxGraphicsContext> const gc(wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(dc));
	gc->DrawBitmap(bitmap, x, y, w, h);
#else
	wxBitmap::Rescale(bitmap, wxSize(wxRound(w), wxRound(h)));
	dc.DrawBitmap(bitmap, wxRound(x), wxRound(y), true);
#endif
}
