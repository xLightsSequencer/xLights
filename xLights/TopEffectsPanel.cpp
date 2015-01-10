#include "TopEffectsPanel.h"
#include <wx/event.h>

//(*InternalHeaders(TopEffectsPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(TopEffectsPanel)
//*)

BEGIN_EVENT_TABLE(TopEffectsPanel,wxPanel)
	//(*EventTable(TopEffectsPanel)
	//*)
END_EVENT_TABLE()

TopEffectsPanel::TopEffectsPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(TopEffectsPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(wxEVT_SIZE,(wxObjectEventFunction)&TopEffectsPanel::OnResize);
	//*)
}

TopEffectsPanel::~TopEffectsPanel()
{
	//(*Destroy(TopEffectsPanel)
	//*)
}


void TopEffectsPanel::OnResize(wxSizeEvent& event)
{
    wxCommandEvent eventWindowResized(EVT_WINDOW_RESIZED);
    wxPostEvent(GetParent(), eventWindowResized);
    Refresh();
}
