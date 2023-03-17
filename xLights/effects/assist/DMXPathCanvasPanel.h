#pragma once

#include <wx/geometry.h>
#include <wx/panel.h>

#include "UtilClasses.h"

#include <memory>
#include <vector>

class wxBitmap;

class DMXPathCanvasPanel : public wxPanel
{
public:
    DMXPathCanvasPanel(wxWindow* parent, wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    
        virtual ~DMXPathCanvasPanel(){};
    

    bool AcceptsFocus() const override
    {
        return true;
    }

    void SetSettingDef(SettingsMap settings)
    {
        mSettings = settings;
    }

private:
    SettingsMap mSettings;

    void OnSketchPaint(wxPaintEvent& event);
    wxPoint NormalizedToUI(const wxPoint& pt) const;

    DECLARE_EVENT_TABLE()
};
