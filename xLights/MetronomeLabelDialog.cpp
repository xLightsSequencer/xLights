/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "MetronomeLabelDialog.h"

//(*InternalHeaders(MetronomeLabelDialog)
#include <wx/button.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MetronomeLabelDialog)
const long MetronomeLabelDialog::ID_STATICTEXT_DIALOGTEXT = wxNewId();
const long MetronomeLabelDialog::ID_STATICTEXT_TIMING = wxNewId();
const long MetronomeLabelDialog::ID_SPINCTRL_TIMING = wxNewId();
const long MetronomeLabelDialog::ID_STATICTEXT_REPEATING = wxNewId();
const long MetronomeLabelDialog::ID_SPINCTRL_REPEATING = wxNewId();
//*)

BEGIN_EVENT_TABLE(MetronomeLabelDialog,wxDialog)
	//(*EventTable(MetronomeLabelDialog)
	//*)
END_EVENT_TABLE()

MetronomeLabelDialog::MetronomeLabelDialog(int base_timing, wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(MetronomeLabelDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxGridSizer* GridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	StaticTextDialogText = new wxStaticText(this, ID_STATICTEXT_DIALOGTEXT, _("Enter Metronome Timing and Repeating Tag"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DIALOGTEXT"));
	FlexGridSizer1->Add(StaticTextDialogText, 1, wxALL|wxEXPAND, 5);
	GridSizer1 = new wxGridSizer(0, 2, 0, 0);
	StaticTextTiming = new wxStaticText(this, ID_STATICTEXT_TIMING, _("Metronome Timing(ms)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_TIMING"));
	GridSizer1->Add(StaticTextTiming, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlTiming = new wxSpinCtrl(this, ID_SPINCTRL_TIMING, _T("50"), wxDefaultPosition, wxDefaultSize, 0, 1, 60000, 50, _T("ID_SPINCTRL_TIMING"));
	SpinCtrlTiming->SetValue(_T("50"));
	GridSizer1->Add(SpinCtrlTiming, 1, wxALL|wxEXPAND, 5);
	StaticTextRepeating = new wxStaticText(this, ID_STATICTEXT_REPEATING, _("Repeating Tag Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_REPEATING"));
	GridSizer1->Add(StaticTextRepeating, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlRepeating = new wxSpinCtrl(this, ID_SPINCTRL_REPEATING, _T("4"), wxDefaultPosition, wxDefaultSize, 0, 0, 10000, 4, _T("ID_SPINCTRL_REPEATING"));
	SpinCtrlRepeating->SetValue(_T("4"));
	GridSizer1->Add(SpinCtrlRepeating, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(GridSizer1, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
	SpinCtrlTiming->SetRange(base_timing, SpinCtrlTiming->GetMax());
	SpinCtrlTiming->SetValue( 10 * base_timing);

	SetEscapeId(wxID_CANCEL);
}

MetronomeLabelDialog::~MetronomeLabelDialog()
{
	//(*Destroy(MetronomeLabelDialog)
	//*)
}

