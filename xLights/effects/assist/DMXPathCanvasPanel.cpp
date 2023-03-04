#include "DMXPathCanvasPanel.h"

#include <wx/dcbuffer.h>
#include <wx/graphics.h>


BEGIN_EVENT_TABLE(DMXPathCanvasPanel, wxPanel)
    EVT_PAINT(DMXPathCanvasPanel::OnSketchPaint)

END_EVENT_TABLE()


DMXPathCanvasPanel::DMXPathCanvasPanel(wxWindow* parent, wxWindowID id /*=wxID_ANY*/, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*=wxDefaultSize*/) :
    wxPanel(parent, id, pos, size, wxNO_BORDER | wxWANTS_CHARS)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void DMXPathCanvasPanel::OnSketchPaint(wxPaintEvent& /*event*/)
{
}

