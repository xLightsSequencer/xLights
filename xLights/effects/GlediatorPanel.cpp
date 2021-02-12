/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "GlediatorPanel.h"
#include "EffectPanelUtils.h"
#include "wx/filedlg.h"

//(*InternalHeaders(GlediatorPanel)
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

//(*IdInit(GlediatorPanel)
const long GlediatorPanel::ID_STATICTEXT2 = wxNewId();
const long GlediatorPanel::ID_FILEPICKERCTRL_Glediator_Filename = wxNewId();
const long GlediatorPanel::ID_STATICTEXT_Glediator_DurationTreatment = wxNewId();
const long GlediatorPanel::ID_CHOICE_Glediator_DurationTreatment = wxNewId();
//*)

BEGIN_EVENT_TABLE(GlediatorPanel,wxPanel)
	//(*EventTable(GlediatorPanel)
	//*)
END_EVENT_TABLE()

GlediatorPanel::GlediatorPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(GlediatorPanel)
	wxFlexGridSizer* FlexGridSizer53;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer53 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Glediator File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer53->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_Glediator_Filename = new BulkEditFilePickerCtrl(this, ID_FILEPICKERCTRL_Glediator_Filename, wxEmptyString, _("Select a glediator file"), _T("Glediator Files (*.gled)|*.gled|Jinx! Glediator files (*.out)|*.out|CSV files (*.csv)|*.csv"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL_Glediator_Filename"));
	FlexGridSizer53->Add(FilePickerCtrl_Glediator_Filename, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Glediator_DurationTreatment, _("Duration Treatment:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Glediator_DurationTreatment"));
	FlexGridSizer53->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Glediator_DurationTreatment = new BulkEditChoice(this, ID_CHOICE_Glediator_DurationTreatment, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Glediator_DurationTreatment"));
	Choice_Glediator_DurationTreatment->SetSelection( Choice_Glediator_DurationTreatment->Append(_("Normal")) );
	Choice_Glediator_DurationTreatment->Append(_("Loop"));
	Choice_Glediator_DurationTreatment->Append(_("Slow/Accelerate"));
	FlexGridSizer53->Add(Choice_Glediator_DurationTreatment, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer53);
	FlexGridSizer53->Fit(this);
	FlexGridSizer53->SetSizeHints(this);
	//*)
	
	Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&GlediatorPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&GlediatorPanel::OnValidateWindow, 0, this);

    SetName("ID_PANEL_GLEDIATOR");

    ValidateWindow();
}

GlediatorPanel::~GlediatorPanel()
{
	//(*Destroy(GlediatorPanel)
	//*)
}

void GlediatorPanel::ValidateWindow()
{
}
