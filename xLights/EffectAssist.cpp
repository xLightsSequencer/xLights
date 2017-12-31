#include "EffectAssist.h"
#include "xLightsMain.h"
#include "effects/EffectManager.h"
#include "effects/assist/xlGridCanvasEmpty.h"
#include "xLightsXmlFile.h"

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
: mAssistPanel(nullptr), defaultAssistPanel(nullptr)
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
        mAssistPanel->RefreshEffect();
    }
}

AssistPanel *EffectAssist::GetDefaultAssistPanel() {
    if (defaultAssistPanel == nullptr) {
        defaultAssistPanel = new AssistPanel(this);
        xlGridCanvas* grid = new xlGridCanvasEmpty(defaultAssistPanel->GetCanvasParent(), wxNewId(), wxDefaultPosition,
                                                   wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("EmptyGrid"));
        defaultAssistPanel->SetGridCanvas(grid);
        defaultAssistPanel->Hide();
    }
    return defaultAssistPanel;
}

void EffectAssist::SetPanel(AssistPanel* panel_)
{
    wxSize s = GetSize();
    AssistPanel* old_panel = mAssistPanel;
    mAssistPanel = panel_;

    if( old_panel != nullptr)
    {
        FlexGridSizer1->Detach(old_panel);
        if (old_panel != defaultAssistPanel) {
            delete old_panel;
        } else {
            defaultAssistPanel->Hide();
        }
    }
    if( mAssistPanel != nullptr )
    {
        FlexGridSizer1->Add(mAssistPanel, 1, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 2);
        if (mAssistPanel == defaultAssistPanel) {
            defaultAssistPanel->Show();
        }
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
