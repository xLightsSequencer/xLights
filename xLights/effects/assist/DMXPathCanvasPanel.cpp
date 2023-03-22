#include "DMXPathCanvasPanel.h"

#include <wx/dcbuffer.h>
#include <wx/graphics.h>

#include "../DMXPathEffect.h"
#include "../../UtilClasses.h"

#include <wx/gdicmn.h>


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
    // hmm... not sure why mSettings can seemingly be emptied out during execution of this method
    auto settings = mSettings;
    if (settings.empty())
        return;
 
    std::string type_Str = settings["CHOICE_DMXPath_Type"];
    auto width = settings.GetInt("SLIDER_DMXPath_Width", 30);
    auto height = settings.GetInt("SLIDER_DMXPath_Height", 30);
    auto x_offset = settings.GetInt("SLIDER_DMXPath_X_Off", 0);
    auto y_offset = settings.GetInt("SLIDER_DMXPath_Y_Off", 0);

    auto rotation = settings.GetInt("SLIDER_DMXPath_Rotation", 0);
    auto pathType = DMXPathEffect::DecodeType(type_Str);
    
    wxAutoBufferedPaintDC pdc(this);
    wxSize sz(GetSize());

    pdc.SetPen(*wxWHITE_PEN);
    pdc.SetBrush(*wxWHITE_BRUSH);
    pdc.DrawRectangle(wxPoint(0, 0), sz);

    pdc.DrawText("Tilt+", NormalizedToUI(wxPoint(-180, 180)));
    pdc.DrawText("Tilt-", NormalizedToUI(wxPoint(-180, -110)));

    pdc.DrawText("Pan+", NormalizedToUI(wxPoint(130, -140)));
    pdc.DrawText("Pan-", NormalizedToUI(wxPoint(-180, -140)));

    pdc.SetPen(*wxBLUE_PEN);
    pdc.SetBrush(*wxBLUE_BRUSH);

    for(int i =0;i<100;++i) {
         auto [x, y] = DMXPathEffect::RenderPath(pathType, (double)i/100.0, height, width, x_offset, y_offset, rotation);
         wxPoint pt(x,y);
         pdc.DrawCircle(NormalizedToUI(pt) , 1);
    }
}

wxPoint DMXPathCanvasPanel::NormalizedToUI(const wxPoint& pt) const
{   
    wxSize sz(GetSize());

    double x = (double)(pt.x + 180.0)/360.0 * sz.GetWidth();
    double y = (double)(pt.y + 180.0)/360.0 * sz.GetHeight();
    return wxPoint( x, (sz.GetHeight() - y));
}

