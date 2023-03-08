#pragma once

//(*Headers(DMXPathAssistPanel)
#include <wx/panel.h>
#include <wx/sizer.h>
//*)

#include "DMXPathAssistPanel.h"  
#include "DMXPathCanvasPanel.h"  

#include <functional>
#include <string>

#include <wx/image.h>
#include <wx/panel.h>



class wxButton;
class wxListBox;

class DMXPathAssistPanel : public wxPanel
{
public:

    DMXPathAssistPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

    // Assist panels seem to be created/destroyed quite frequently when switching between effects
    virtual ~DMXPathAssistPanel();
    
    //(*Declarations(DMXPathAssistPanel)
    //*)


	//(*Identifiers(DMXPathAssistPanel)
	//*)

    void SetEffectDef(Effect* effect)
    {
        mEffect = effect;
        if (nullptr != m_dmxPathCanvasPanel) {
            m_dmxPathCanvasPanel->SetEffectDef(effect);
            m_dmxPathCanvasPanel->Refresh();
        }
        
    }

	private:

    DMXPathCanvasPanel* m_dmxPathCanvasPanel = nullptr;
    Effect* mEffect{nullptr};

	//(*Handlers(DMXPathAssistPanel)

	//*)
    DECLARE_EVENT_TABLE()
};