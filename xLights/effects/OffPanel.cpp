#include "OffPanel.h"

//(*InternalHeaders(OffPanel)
#include <wx/checkbox.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

//(*IdInit(OffPanel)
const long OffPanel::ID_CHECKBOX_Off_Transparent = wxNewId();
//*)

BEGIN_EVENT_TABLE(OffPanel,wxPanel)
	//(*EventTable(OffPanel)
	//*)
END_EVENT_TABLE()

OffPanel::OffPanel(wxWindow* parent)
{
	//(*Initialize(OffPanel)
	wxFlexGridSizer* FlexGridSizer77;
	wxStaticText* StaticText1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer77 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer77->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("This Effect simply turns every pixel off on this model.\n\nIf set to be transparent it does not change any pixels."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer77->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Transparent = new BulkEditCheckBox(this, ID_CHECKBOX_Off_Transparent, _("Transparent"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Off_Transparent"));
	CheckBox_Transparent->SetValue(false);
	FlexGridSizer77->Add(CheckBox_Transparent, 1, wxALL|wxEXPAND, 5);
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

