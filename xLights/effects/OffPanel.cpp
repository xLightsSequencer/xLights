/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "OffPanel.h"

//(*InternalHeaders(OffPanel)
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

//(*IdInit(OffPanel)
const long OffPanel::ID_STATICTEXT_Off_Text = wxNewId();
const long OffPanel::ID_CHOICE_Off_Style = wxNewId();
//*)

BEGIN_EVENT_TABLE(OffPanel,wxPanel)
	//(*EventTable(OffPanel)
	//*)
END_EVENT_TABLE()

OffPanel::OffPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(OffPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer77;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer77 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer77->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Off_Text, _("This Effect simply turns every pixel off on this model.\n\nIf set to be transparent it does not change any pixels."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Off_Text"));
	FlexGridSizer77->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Style:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	OffStyleChoice = new wxChoice(this, ID_CHOICE_Off_Style, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Off_Style"));
	OffStyleChoice->SetSelection( OffStyleChoice->Append(_("Black")) );
	OffStyleChoice->Append(_("Transparent"));
	OffStyleChoice->Append(_("Black -> Transparent"));
	OffStyleChoice->Append(_("Transparent -> Black"));
	FlexGridSizer1->Add(OffStyleChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer77->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	SetSizer(FlexGridSizer77);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&OffPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&OffPanel::OnValidateWindow, 0, this);
    
    SetName("ID_PANEL_OFF");

	ValidateWindow();
}

OffPanel::~OffPanel()
{
	//(*Destroy(OffPanel)
	//*)
}

void OffPanel::ValidateWindow()
{
}
