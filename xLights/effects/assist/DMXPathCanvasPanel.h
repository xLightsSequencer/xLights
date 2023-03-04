#pragma once

#include <wx/geometry.h>
#include <wx/panel.h>

#include <memory>
#include <vector>


class wxBitmap;



class DMXPathCanvasPanel : public wxPanel
{
public:
    DMXPathCanvasPanel( wxWindow* parent, wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~DMXPathCanvasPanel() = default;

    bool AcceptsFocus() const override
    {
        return true;
    }

private:


    DECLARE_EVENT_TABLE()
    void OnSketchPaint(wxPaintEvent& event);


};
