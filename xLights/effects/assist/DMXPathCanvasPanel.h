#pragma once

#include <wx/geometry.h>
#include <wx/panel.h>

#include <memory>
#include <vector>
#include "../../sequencer/Effect.h"


class wxBitmap;



class DMXPathCanvasPanel : public wxPanel
{
public:
    DMXPathCanvasPanel(Effect* effect, wxWindow* parent, wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~DMXPathCanvasPanel() = default;

    bool AcceptsFocus() const override
    {
        return true;
    }

    void SetEffectDef(Effect* effect)
    {
        mEffect = effect;
    }

private:

    DECLARE_EVENT_TABLE()
    void OnSketchPaint(wxPaintEvent& event);
    wxPoint NormalizedToUI(const wxPoint& pt) const;
    Effect* mEffect{nullptr};

};
