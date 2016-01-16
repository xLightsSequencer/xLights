#include "EffectAssist.h"
#include "xLightsMain.h"
#include "effects/EffectManager.h"
#include "xlGridCanvasEmpty.h"
#include "../xLightsXmlFile.h"

//(*InternalHeaders(EffectAssist)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EffectAssist)
//*)

BEGIN_EVENT_TABLE(EffectAssist,wxPanel)
	//(*EventTable(EffectAssist)
	//*)
END_EVENT_TABLE()

EffectAssist::EffectAssist(wxWindow* parent, wxWindowID id)
: mAssistPanel(nullptr)
{
	//(*Initialize(EffectAssist)
	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxWANTS_CHARS, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(wxEVT_SIZE,(wxObjectEventFunction)&EffectAssist::OnResize);
	//*)

    mSize.SetHeight(0);
    mSize.SetWidth(0);
}

EffectAssist::~EffectAssist()
{
	//(*Destroy(EffectAssist)
	//*)
}

void EffectAssist::ForceRefresh()
{
    if( mAssistPanel != nullptr )
    {
        mAssistPanel->Refresh();
        mAssistPanel->Update();
    }
}

void EffectAssist::SetPanel(AssistPanel* panel_)
{
    wxSize s = GetSize();
    AssistPanel* old_panel = mAssistPanel;
    mAssistPanel = panel_;

    if( old_panel != nullptr )
    {
        FlexGridSizer1->Detach(old_panel);
        delete old_panel;
    }

    if( mAssistPanel != nullptr )
    {
        FlexGridSizer1->Add(mAssistPanel, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
        AdjustClientSizes(s);
    }
}

void EffectAssist::OnResize(wxSizeEvent& event)
{
    wxSize s = GetClientSize();
    AdjustClientSizes(s);
    Layout();
}

void EffectAssist::AdjustClientSizes(wxSize s)
{
    mSize = s;
    if( mAssistPanel != nullptr )
    {
        s.SetWidth(s.GetWidth()-15);
        s.SetHeight(s.GetHeight()-15);
        mAssistPanel->AdjustSize(s);
    }
}
