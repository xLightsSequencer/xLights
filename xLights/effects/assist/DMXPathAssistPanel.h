#pragma once

#include <wx/panel.h>

#include "DMXPathCanvasPanel.h"  

class DMXPathAssistPanel : public wxPanel
{
public:

    DMXPathAssistPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

    // Assist panels seem to be created/destroyed quite frequently when switching between effects
    virtual ~DMXPathAssistPanel();
    
    void SetSettingDef(SettingsMap settings)
    {
        //mSettings = settings;
        if (nullptr != m_dmxPathCanvasPanel) {
            if (!m_dmxPathCanvasPanel->IsBeingDeleted()) {
                m_dmxPathCanvasPanel->SetSettingDef(settings);
                m_dmxPathCanvasPanel->Refresh();
            }
        }
        Refresh();
    }

private:
    DMXPathCanvasPanel* m_dmxPathCanvasPanel = nullptr;
    //SettingsMap mSettings;

    DECLARE_EVENT_TABLE()
};