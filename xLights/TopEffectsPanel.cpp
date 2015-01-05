#include "TopEffectsPanel.h"
#include <wx/event.h>

//(*InternalHeaders(TopEffectsPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(TopEffectsPanel)
const long TopEffectsPanel::ID_NOTEBOOK1 = wxNewId();
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
	EffectsNotebook = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	FlexGridSizer1->Add(EffectsNotebook, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
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
