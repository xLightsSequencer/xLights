//(*InternalHeaders(DMXPathAssistPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)


#include "DMXPathAssistPanel.h"


#include "UtilFunctions.h"
#include <xLightsMain.h>
#include "../../ExternalHooks.h"

//(*IdInit(DMXPathAssistPanel)
//*)


BEGIN_EVENT_TABLE(DMXPathAssistPanel, wxPanel)
	//(*EventTable(DMXPathAssistPanel)
	//*)
END_EVENT_TABLE()

DMXPathAssistPanel::DMXPathAssistPanel(wxWindow* parent, wxWindowID id /*wxID_ANY*/, const wxPoint& pos /*wxDefaultPosition*/, const wxSize& size /*wxDefaultSize*/) :
    wxPanel(parent, id, pos, size)
{
   //(*Initialize(DMXPathAssistPanel)
   wxFlexGridSizer* FlexGridSizerMain;

   FlexGridSizerMain = new wxFlexGridSizer(1, 1, 0, 0);
   SetSizer(FlexGridSizerMain);
   FlexGridSizerMain->Fit(this);
   FlexGridSizerMain->SetSizeHints(this);
   //*)

    m_dmxPathCanvasPanel = new DMXPathCanvasPanel(this, wxID_ANY, wxDefaultPosition, wxSize(400, 300));
    FlexGridSizerMain->Add(m_dmxPathCanvasPanel, 1, wxALL | wxEXPAND);
   
}

DMXPathAssistPanel::~DMXPathAssistPanel()
{
	//(*Destroy(DMXPathAssistPanel)
	//*)
}
