#pragma once

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/image.h>

#include "DMXPathAssistPanel.h"  
#include "DMXPathCanvasPanel.h"  

#include "UtilClasses.h"

#include <string>
#include <memory>

class wxButton;
class wxListBox;

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
            m_dmxPathCanvasPanel->SetSettingDef(settings);
            m_dmxPathCanvasPanel->Refresh();
        }
        Refresh();
    }

private:
    std::unique_ptr<DMXPathCanvasPanel> m_dmxPathCanvasPanel{ nullptr };
    //SettingsMap mSettings;

    DECLARE_EVENT_TABLE()
};