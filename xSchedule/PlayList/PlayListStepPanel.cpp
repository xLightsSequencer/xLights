/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListStepPanel.h"

#include "PlayListDialog.h"
#include "PlayListStep.h"

//(*InternalHeaders(PlayListStepPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListStepPanel)
const wxWindowID PlayListStepPanel::ID_STATICTEXT1 = wxNewId();
const wxWindowID PlayListStepPanel::ID_TEXTCTRL1 = wxNewId();
const wxWindowID PlayListStepPanel::ID_CHECKBOX1 = wxNewId();
const wxWindowID PlayListStepPanel::ID_CHECKBOX2 = wxNewId();
const wxWindowID PlayListStepPanel::ID_STATICTEXT2 = wxNewId();
const wxWindowID PlayListStepPanel::ID_CHECKBOX3 = wxNewId();
const wxWindowID PlayListStepPanel::ID_STATICTEXT3 = wxNewId();
const wxWindowID PlayListStepPanel::ID_CHECKBOX4 = wxNewId();
const wxWindowID PlayListStepPanel::ID_CHECKBOX5 = wxNewId();
const wxWindowID PlayListStepPanel::ID_STATICTEXT4 = wxNewId();
const wxWindowID PlayListStepPanel::ID_TIMEPICKERCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListStepPanel, wxPanel)
//(*EventTable(PlayListStepPanel)
//*)
END_EVENT_TABLE()

PlayListStepPanel::PlayListStepPanel(wxWindow* parent, PlayListStep* step, wxWindowID id, const wxPoint& pos, const wxSize& size) {
    _step = step;

    //(*Initialize(PlayListStepPanel)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;

    Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer1->AddGrowableCol(1);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_PlayListStepName = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer1->Add(TextCtrl_PlayListStepName, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_ExcludeFromRandom = new wxCheckBox(this, ID_CHECKBOX1, _("Exclude from shuffle"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_ExcludeFromRandom->SetValue(false);
    FlexGridSizer1->Add(CheckBox_ExcludeFromRandom, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_EveryStep = new wxCheckBox(this, ID_CHECKBOX2, _("Run everything in this step in every step"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    CheckBox_EveryStep->SetValue(false);
    FlexGridSizer1->Add(CheckBox_EveryStep, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer2->AddGrowableCol(1);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_ExcludeFirstStep = new wxCheckBox(this, ID_CHECKBOX3, _("Exclude First Step If Only Once"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
    CheckBox_ExcludeFirstStep->SetValue(false);
    FlexGridSizer2->Add(CheckBox_ExcludeFirstStep, 1, wxALL|wxEXPAND, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_ExcludeLastStep = new wxCheckBox(this, ID_CHECKBOX4, _("Exclude Last Step If Only Once"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
    CheckBox_ExcludeLastStep->SetValue(false);
    FlexGridSizer2->Add(CheckBox_ExcludeLastStep, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_TimeCodeOverride = new wxCheckBox(this, ID_CHECKBOX5, _("Time Code Override"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
    CheckBox_TimeCodeOverride->SetValue(false);
    FlexGridSizer1->Add(CheckBox_TimeCodeOverride, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_BaseTime = new wxStaticText(this, ID_STATICTEXT4, _("Base Time:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer1->Add(StaticText_BaseTime, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TimeCodeBaseTime = new wxTimePickerCtrl(this, ID_TIMEPICKERCTRL1, wxDateTime::Now(), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TIMEPICKERCTRL1"));
    FlexGridSizer1->Add(TimeCodeBaseTime, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_TEXTCTRL1, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&PlayListStepPanel::OnTextCtrl_PlayListStepNameText);
    Connect(ID_CHECKBOX2, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&PlayListStepPanel::OnCheckBox_EveryStepClick);
    Connect(ID_CHECKBOX5, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&PlayListStepPanel::OnCheckBox_TimeCodeOverrideClick);
    //*)

    wxTextValidator tv(wxFILTER_EXCLUDE_CHAR_LIST);
    tv.SetCharExcludes("\"'`");
    TextCtrl_PlayListStepName->SetValidator(tv);

    TextCtrl_PlayListStepName->SetValue(step->GetRawName());
    CheckBox_ExcludeFromRandom->SetValue(step->GetExcludeFromRandom());
    CheckBox_EveryStep->SetValue(step->GetEveryStep());
    CheckBox_ExcludeFirstStep->SetValue(step->GetEveryStepExcludeFirst());
    CheckBox_ExcludeLastStep->SetValue(step->GetEveryStepExcludeLast());

    int bt = step->GetBaseTimeCodeTime();

    if (bt < 0) {
        CheckBox_TimeCodeOverride->SetValue(false);
        TimeCodeBaseTime->SetValue(wxDateTime(0, 0, 0));
    } else {
        CheckBox_TimeCodeOverride->SetValue(true);
        TimeCodeBaseTime->SetValue(wxDateTime(bt / (60 * 60), (bt % (60 * 60)) / 60, bt % 60));
    }

    ValidateWindow();
}

PlayListStepPanel::~PlayListStepPanel() {
    //(*Destroy(PlayListStepPanel)
    //*)
    _step->SetName(TextCtrl_PlayListStepName->GetValue().ToStdString());
    _step->SetExcludeFromRandom(CheckBox_ExcludeFromRandom->GetValue());
    _step->SetEveryStep(CheckBox_EveryStep->GetValue());
    _step->SetEveryStepExcludeFirst(CheckBox_ExcludeFirstStep->GetValue());
    _step->SetEveryStepExcludeLast(CheckBox_ExcludeLastStep->GetValue());
    if (CheckBox_TimeCodeOverride->GetValue()) {
        wxDateTime bt = TimeCodeBaseTime->GetValue();
        _step->SetBaseTimeCodeTime(bt.GetHour() * 60 * 60 + bt.GetMinute() * 60 + bt.GetSecond());
    } else {
        _step->SetBaseTimeCodeTime(-1);
    }
}

void PlayListStepPanel::OnTextCtrl_PlayListStepNameText(wxCommandEvent& event) {
    _step->SetName(TextCtrl_PlayListStepName->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

void PlayListStepPanel::OnCheckBox_EveryStepClick(wxCommandEvent& event) {
    ValidateWindow();
}

void PlayListStepPanel::ValidateWindow() {
    if (CheckBox_EveryStep->GetValue()) {
        CheckBox_ExcludeFirstStep->Enable();
        CheckBox_ExcludeLastStep->Enable();
    } else {
        CheckBox_ExcludeFirstStep->Enable(false);
        CheckBox_ExcludeLastStep->Enable(false);
    }

    if (CheckBox_TimeCodeOverride->GetValue()) {
        StaticText_BaseTime->Enable(true);
        TimeCodeBaseTime->Enable(true);
    } else {
        StaticText_BaseTime->Enable(false);
        TimeCodeBaseTime->Enable(false);
    }
}
void PlayListStepPanel::OnCheckBox_ExcludeFirstStepClick(wxCommandEvent& event) {
    ValidateWindow();
}

void PlayListStepPanel::OnCheckBox_TimeCodeOverrideClick(wxCommandEvent& event) {
    ValidateWindow();
}
