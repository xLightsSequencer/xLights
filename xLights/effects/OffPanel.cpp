#include "OffPanel.h"

//(*InternalHeaders(OffPanel)
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/intl.h>
#include <wx/stattext.h>
//*)

//(*IdInit(OffPanel)
//*)

BEGIN_EVENT_TABLE(OffPanel,wxPanel)
	//(*EventTable(OffPanel)
	//*)
END_EVENT_TABLE()

OffPanel::OffPanel(wxWindow* parent)
{
	//(*Initialize(OffPanel)
	wxStaticText* StaticText1;
	wxFlexGridSizer* FlexGridSizer77;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer77 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer77->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("This Effect simply turns every pixel off on this model."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer77->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer77);
	FlexGridSizer77->Fit(this);
	FlexGridSizer77->SetSizeHints(this);
	//*)
    
    SetName("ID_PANEL_OFF");
}

OffPanel::~OffPanel()
{
	//(*Destroy(OffPanel)
	//*)
}

