/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "FacesPanel.h"
#include "EffectPanelUtils.h"

//(*InternalHeaders(FacesPanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(FacesPanel)
const long FacesPanel::IDD_RADIOBUTTON_Faces_Phoneme = wxNewId();
const long FacesPanel::ID_CHOICE_Faces_Phoneme = wxNewId();
const long FacesPanel::IDD_RADIOBUTTON_Faces_TimingTrack = wxNewId();
const long FacesPanel::ID_CHOICE_Faces_TimingTrack = wxNewId();
const long FacesPanel::ID_STATICTEXT15 = wxNewId();
const long FacesPanel::ID_CHOICE_Faces_FaceDefinition = wxNewId();
const long FacesPanel::ID_STATICTEXT_Faces_Eyes = wxNewId();
const long FacesPanel::ID_CHOICE_Faces_Eyes = wxNewId();
const long FacesPanel::ID_STATICTEXT_EYEBLINKFREQUENCY = wxNewId();
const long FacesPanel::ID_CHOICE_Faces_EyeBlinkFrequency = wxNewId();
const long FacesPanel::ID_CHECKBOX_Faces_Outline = wxNewId();
const long FacesPanel::ID_CHECKBOX_Faces_SuppressShimmer = wxNewId();
const long FacesPanel::ID_STATICTEXT1 = wxNewId();
const long FacesPanel::ID_CHOICE_Faces_UseState = wxNewId();
const long FacesPanel::ID_CHECKBOX_Faces_SuppressWhenNotSinging = wxNewId();
const long FacesPanel::ID_STATICTEXT_Faces_Lead_Frames = wxNewId();
const long FacesPanel::ID_SPINCTRL_Faces_LeadFrames = wxNewId();
const long FacesPanel::ID_CHECKBOX_Faces_Fade = wxNewId();
const long FacesPanel::ID_CHECKBOX_Faces_TransparentBlack = wxNewId();
const long FacesPanel::IDD_SLIDER_Faces_TransparentBlack = wxNewId();
const long FacesPanel::ID_TEXTCTRL_Faces_TransparentBlack = wxNewId();
//*)

BEGIN_EVENT_TABLE(FacesPanel,wxPanel)
	//(*EventTable(FacesPanel)
	//*)
END_EVENT_TABLE()

FacesPanel::FacesPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(FacesPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer47;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer97;
	wxFlexGridSizer* FlexGridSizer98;
	wxStaticBoxSizer* StaticBoxSizer2;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer47 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer47->AddGrowableCol(0);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Mouth Movements"));
	FlexGridSizer97 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer97->AddGrowableCol(1);
	RadioButton1 = new wxRadioButton(this, IDD_RADIOBUTTON_Faces_Phoneme, _("Phoneme"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_RADIOBUTTON_Faces_Phoneme"));
	RadioButton1->SetValue(true);
	FlexGridSizer97->Add(RadioButton1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Faces_Phoneme = new BulkEditChoice(this, ID_CHOICE_Faces_Phoneme, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Faces_Phoneme"));
	Choice_Faces_Phoneme->SetSelection( Choice_Faces_Phoneme->Append(_("AI")) );
	Choice_Faces_Phoneme->Append(_("E"));
	Choice_Faces_Phoneme->Append(_("FV"));
	Choice_Faces_Phoneme->Append(_("L"));
	Choice_Faces_Phoneme->Append(_("MBP"));
	Choice_Faces_Phoneme->Append(_("O"));
	Choice_Faces_Phoneme->Append(_("U"));
	Choice_Faces_Phoneme->Append(_("WQ"));
	Choice_Faces_Phoneme->Append(_("etc"));
	Choice_Faces_Phoneme->Append(_("rest"));
	Choice_Faces_Phoneme->Append(_("(off)"));
	FlexGridSizer97->Add(Choice_Faces_Phoneme, 1, wxALL|wxEXPAND, 5);
	RadioButton2 = new wxRadioButton(this, IDD_RADIOBUTTON_Faces_TimingTrack, _("Timing Track"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_RADIOBUTTON_Faces_TimingTrack"));
	FlexGridSizer97->Add(RadioButton2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Faces_TimingTrack = new BulkEditChoice(this, ID_CHOICE_Faces_TimingTrack, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Faces_TimingTrack"));
	Choice_Faces_TimingTrack->Disable();
	FlexGridSizer97->Add(Choice_Faces_TimingTrack, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer2->Add(FlexGridSizer97, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer47->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer98 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer98->AddGrowableCol(1);
	StaticText14 = new wxStaticText(this, ID_STATICTEXT15, _("Face Definition"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
	FlexGridSizer98->Add(StaticText14, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Face_FaceDefinitonChoice = new BulkEditFaceChoice(this, ID_CHOICE_Faces_FaceDefinition, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Faces_FaceDefinition"));
	FlexGridSizer98->Add(Face_FaceDefinitonChoice, 1, wxALL|wxEXPAND, 5);
	StaticText71 = new wxStaticText(this, ID_STATICTEXT_Faces_Eyes, _("Eyes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Faces_Eyes"));
	FlexGridSizer98->Add(StaticText71, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Faces_Eyes = new BulkEditChoice(this, ID_CHOICE_Faces_Eyes, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Faces_Eyes"));
	Choice_Faces_Eyes->Append(_("Open"));
	Choice_Faces_Eyes->Append(_("Closed"));
	Choice_Faces_Eyes->SetSelection( Choice_Faces_Eyes->Append(_("Auto")) );
	Choice_Faces_Eyes->Append(_("(off)"));
	FlexGridSizer98->Add(Choice_Faces_Eyes, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Faces_EyeBlinkFrequency = new wxStaticText(this, ID_STATICTEXT_EYEBLINKFREQUENCY, _("Eye Blink Frequency"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_EYEBLINKFREQUENCY"));
	FlexGridSizer98->Add(StaticText_Faces_EyeBlinkFrequency, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Faces_EyeBlinkFrequency = new BulkEditChoice(this, ID_CHOICE_Faces_EyeBlinkFrequency, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Faces_EyeBlinkFrequency"));
	Choice_Faces_EyeBlinkFrequency->Append(_("Slowest"));
	Choice_Faces_EyeBlinkFrequency->Append(_("Slow"));
	Choice_Faces_EyeBlinkFrequency->SetSelection( Choice_Faces_EyeBlinkFrequency->Append(_("Normal")) );
	Choice_Faces_EyeBlinkFrequency->Append(_("Fast"));
	Choice_Faces_EyeBlinkFrequency->Append(_("Fastest"));
	FlexGridSizer98->Add(Choice_Faces_EyeBlinkFrequency, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer98->Add(-1,-1,1, wxALL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Faces_Outline = new BulkEditCheckBox(this, ID_CHECKBOX_Faces_Outline, _("Show outline"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Faces_Outline"));
	CheckBox_Faces_Outline->SetValue(false);
	FlexGridSizer98->Add(CheckBox_Faces_Outline, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer98->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_SuppressShimmer = new BulkEditCheckBox(this, ID_CHECKBOX_Faces_SuppressShimmer, _("Suppress shimmer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Faces_SuppressShimmer"));
	CheckBox_SuppressShimmer->SetValue(false);
	FlexGridSizer98->Add(CheckBox_SuppressShimmer, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT1, _("Use State as outline"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer98->Add(StaticText2, 1, wxALL|wxEXPAND, 5);
	Choice1 = new BulkEditStateChoice(this, ID_CHOICE_Faces_UseState, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Faces_UseState"));
	FlexGridSizer98->Add(Choice1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer98->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	CheckBox_SuppressWhenNotSinging = new BulkEditCheckBox(this, ID_CHECKBOX_Faces_SuppressWhenNotSinging, _("Suppress when not singing"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Faces_SuppressWhenNotSinging"));
	CheckBox_SuppressWhenNotSinging->SetValue(false);
	FlexGridSizer1->Add(CheckBox_SuppressWhenNotSinging, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Faces_Lead_Frames, _("Lead In/Out Frames"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Faces_Lead_Frames"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
	SpinCtrl_LeadFrames = new BulkEditSpinCtrl(this, ID_SPINCTRL_Faces_LeadFrames, _T("0"), wxDefaultPosition, wxSize(100,-1), 0, 0, 1000, 0, _T("ID_SPINCTRL_Faces_LeadFrames"));
	SpinCtrl_LeadFrames->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_LeadFrames, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fade = new BulkEditCheckBox(this, ID_CHECKBOX_Faces_Fade, _("Fade during lead in/out"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Faces_Fade"));
	CheckBox_Fade->SetValue(false);
	FlexGridSizer1->Add(CheckBox_Fade, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer98->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer47->Add(FlexGridSizer98, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer7->AddGrowableCol(1);
	CheckBox_TransparentBlack = new BulkEditCheckBox(this, ID_CHECKBOX_Faces_TransparentBlack, _("Transparent Black"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Faces_TransparentBlack"));
	CheckBox_TransparentBlack->SetValue(false);
	FlexGridSizer7->Add(CheckBox_TransparentBlack, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Faces_TransparentBlack = new BulkEditSlider(this, IDD_SLIDER_Faces_TransparentBlack, 0, 0, 300, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Faces_TransparentBlack"));
	FlexGridSizer7->Add(Slider_Faces_TransparentBlack, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Faces_TransparentBlack = new BulkEditTextCtrl(this, ID_TEXTCTRL_Faces_TransparentBlack, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(40,-1)), wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL_Faces_TransparentBlack"));
	FlexGridSizer7->Add(TextCtrl_Faces_TransparentBlack, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer47->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer47);
	FlexGridSizer47->Fit(this);
	FlexGridSizer47->SetSizeHints(this);

	Connect(IDD_RADIOBUTTON_Faces_Phoneme,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&FacesPanel::OnMouthMovementTypeSelected);
	Connect(IDD_RADIOBUTTON_Faces_TimingTrack,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&FacesPanel::OnMouthMovementTypeSelected);
	Connect(ID_CHOICE_Faces_Eyes,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&FacesPanel::OnChoice_Faces_EyesSelect);
	Connect(ID_CHOICE_Faces_EyeBlinkFrequency,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&FacesPanel::OnChoice_Faces_EyeBlinkFrequencySelect);
	Connect(ID_CHECKBOX_Faces_Outline,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FacesPanel::OnCheckBox_Faces_OutlineClick);
	Connect(ID_CHECKBOX_Faces_SuppressWhenNotSinging,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FacesPanel::OnCheckBox_SuppressWhenNotSingingClick);
	Connect(ID_CHECKBOX_Faces_Fade,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FacesPanel::OnCheckBox_FadeClick);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&FacesPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&FacesPanel::OnValidateWindow, 0, this);

    SetName("ID_PANEL_FACES");

	ValidateWindow();
}

FacesPanel::~FacesPanel()
{
	//(*Destroy(FacesPanel)
	//*)
}

void FacesPanel::ValidateWindow()
{
	if (RadioButton1->GetValue()) {
		Choice_Faces_Phoneme->Enable();
		Choice_Faces_TimingTrack->Disable();
	}
	else {
		Choice_Faces_Phoneme->Disable();
		Choice_Faces_TimingTrack->Enable();
	}

	if (RadioButton1->GetValue()) {
		CheckBox_SuppressWhenNotSinging->Disable();
		SpinCtrl_LeadFrames->Disable();
		CheckBox_Fade->Disable();
	}
	else {
		CheckBox_SuppressWhenNotSinging->Enable();
		if (CheckBox_SuppressWhenNotSinging->IsChecked()) {
			CheckBox_Fade->Enable();
			SpinCtrl_LeadFrames->Enable();
		}
		else {
			SpinCtrl_LeadFrames->Disable();
			CheckBox_Fade->Disable();
		}
	}

	if (CheckBox_Faces_Outline->IsChecked()) {
        Choice1->Enable();
	}
	else {
        Choice1->Enable(false);
    }

    wxString type = Choice_Faces_Eyes->GetStringSelection();
    if (type == "Auto")
    {
        Choice_Faces_EyeBlinkFrequency->Enable();
    }
    else
    {
        Choice_Faces_EyeBlinkFrequency->Disable();
    }
}

void FacesPanel::OnMouthMovementTypeSelected(wxCommandEvent& event)
{
	ValidateWindow();
}

void FacesPanel::OnCheckBox_SuppressWhenNotSingingClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void FacesPanel::OnCheckBox_FadeClick(wxCommandEvent& event)
{
	ValidateWindow();
}
void FacesPanel::OnCheckBox_Faces_OutlineClick(wxCommandEvent& event)
{
	ValidateWindow();
}

void FacesPanel::OnChoice_Faces_EyeBlinkFrequencySelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void FacesPanel::OnChoice_Faces_EyesSelect(wxCommandEvent& event)
{
    ValidateWindow();
}
