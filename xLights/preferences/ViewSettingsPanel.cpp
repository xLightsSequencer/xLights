/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ViewSettingsPanel.h"

//(*InternalHeaders(ViewSettingsPanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

#include <wx/preferences.h>
#include "../xLightsMain.h"
#include "../graphics/xlGraphicsBase.h"


//(*IdInit(ViewSettingsPanel)
const long ViewSettingsPanel::ID_CHOICE3 = wxNewId();
const long ViewSettingsPanel::ID_CHOICE1 = wxNewId();
const long ViewSettingsPanel::ID_CHOICE2 = wxNewId();
const long ViewSettingsPanel::ID_CHOICE4 = wxNewId();
const long ViewSettingsPanel::ID_CHOICE5 = wxNewId();
const long ViewSettingsPanel::ID_CHECKBOX1 = wxNewId();
const long ViewSettingsPanel::ID_CHECKBOX2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ViewSettingsPanel,wxPanel)
	//(*EventTable(ViewSettingsPanel)
	//*)
END_EVENT_TABLE()

ViewSettingsPanel::ViewSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id,const wxPoint& pos,const wxSize& size) : frame(f)
{
	//(*Initialize(ViewSettingsPanel)
	wxFlexGridSizer* FlexGridSizer2;
	wxGridBagSizer* GridBagSizer1;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;
	wxStaticText* StaticText4;
	wxStaticText* StaticText5;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Tool Icon Size"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText1, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ToolIconSizeChoice = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	ToolIconSizeChoice->Append(_("Small"));
	ToolIconSizeChoice->SetSelection( ToolIconSizeChoice->Append(_("Medium")) );
	ToolIconSizeChoice->Append(_("Large"));
	ToolIconSizeChoice->Append(_("Extra Large"));
	GridBagSizer1->Add(ToolIconSizeChoice, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("OpenGL Settings"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Version"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	OpenGLVersionChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	OpenGLVersionChoice->SetSelection( OpenGLVersionChoice->Append(_("Auto Detect")) );
	OpenGLVersionChoice->Append(_("1.x/2.x"));
	OpenGLVersionChoice->Append(_("3.x"));
	FlexGridSizer2->Add(OpenGLVersionChoice, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, wxID_ANY, _("Render Order"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	OpenGLRenderOrderChoice = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	OpenGLRenderOrderChoice->SetSelection( OpenGLRenderOrderChoice->Append(_("1")) );
	OpenGLRenderOrderChoice->Append(_("2"));
	OpenGLRenderOrderChoice->Append(_("3"));
	OpenGLRenderOrderChoice->Append(_("4"));
	OpenGLRenderOrderChoice->Append(_("5"));
	OpenGLRenderOrderChoice->Append(_("6"));
	FlexGridSizer2->Add(OpenGLRenderOrderChoice, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	GridBagSizer1->Add(StaticBoxSizer1, wxGBPosition(1, 0), wxGBSpan(1, 2), wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, wxID_ANY, _("Model Handle Size"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText4, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ModelHandleSizeChoice = new wxChoice(this, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
	ModelHandleSizeChoice->SetSelection( ModelHandleSizeChoice->Append(_("Normal")) );
	ModelHandleSizeChoice->Append(_("Large"));
	ModelHandleSizeChoice->Append(_("Extra Large"));
	GridBagSizer1->Add(ModelHandleSizeChoice, wxGBPosition(2, 1), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, wxID_ANY, _("Effect Assist Window"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText5, wxGBPosition(3, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	EffectAssistChoice = new wxChoice(this, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
	EffectAssistChoice->Append(_("Always On"));
	EffectAssistChoice->Append(_("Always Off"));
	EffectAssistChoice->SetSelection( EffectAssistChoice->Append(_("Auto Toggle")) );
	GridBagSizer1->Add(EffectAssistChoice, wxGBPosition(3, 1), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PlayControlsCheckBox = new wxCheckBox(this, ID_CHECKBOX1, _("Show Play Controls on Preview"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	PlayControlsCheckBox->SetValue(true);
	GridBagSizer1->Add(PlayControlsCheckBox, wxGBPosition(4, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	HousePreviewCheckBox = new wxCheckBox(this, ID_CHECKBOX2, _("Auto Show House Preview"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	HousePreviewCheckBox->SetValue(true);
	GridBagSizer1->Add(HousePreviewCheckBox, wxGBPosition(5, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(GridBagSizer1);
	GridBagSizer1->Fit(this);
	GridBagSizer1->SetSizeHints(this);

	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ViewSettingsPanel::OnToolIconSizeChoiceSelect);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ViewSettingsPanel::OnOpenGLVersionChoiceSelect);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ViewSettingsPanel::OnOpenGLRenderOrderChoiceSelect);
	Connect(ID_CHOICE4,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ViewSettingsPanel::OnModelHandleSizeChoiceSelect);
	Connect(ID_CHOICE5,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ViewSettingsPanel::OnEffectAssistChoiceSelect);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ViewSettingsPanel::OnPlayControlsCheckBoxClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ViewSettingsPanel::OnHousePreviewCheckBoxClick);
	//*)
    
#ifdef XL_DRAWING_WITH_METAL
    StaticBoxSizer1->Show(false);
#endif
    #ifdef _MSC_VER
    MSWDisableComposited();
    #endif
}

ViewSettingsPanel::~ViewSettingsPanel()
{
	//(*Destroy(ViewSettingsPanel)
	//*)
}

bool ViewSettingsPanel::TransferDataToWindow() {
    HousePreviewCheckBox->SetValue(frame->AutoShowHousePreview());
    PlayControlsCheckBox->SetValue(frame->PlayControlsOnPreview());
    int i = frame->EffectAssistMode();
    if (i >= 3) {
        i = 0;
    }
    EffectAssistChoice->SetSelection(i);
    ModelHandleSizeChoice->SetSelection(frame->ModelHandleSize());
    int glv = frame->OpenGLVersion();
    if (glv == 99) {
        glv = 0;
    }
    if (glv == 2) {
        glv = 1;
    }
    OpenGLVersionChoice->SetSelection(glv);
    OpenGLRenderOrderChoice->SetSelection(frame->OpenGLRenderOrder());
    int ts = frame->ToolIconSize();
    switch (ts) {
        case 48:
            ToolIconSizeChoice->SetSelection(3);
            break;
        case 32:
            ToolIconSizeChoice->SetSelection(2);
            break;
        case 24:
            ToolIconSizeChoice->SetSelection(1);
            break;
        case 16:
        default:
            ToolIconSizeChoice->SetSelection(0);
            break;
    }

    return true;
}
bool ViewSettingsPanel::TransferDataFromWindow() {
    frame->SetOpenGLVersion(OpenGLVersionChoice->GetSelection());
    frame->SetOpenGLRenderOrder(OpenGLRenderOrderChoice->GetSelection());
    frame->SetModelHandleSize(ModelHandleSizeChoice->GetSelection());
    frame->SetEffectAssistMode(EffectAssistChoice->GetSelection());
    frame->SetPlayControlsOnPreview(PlayControlsCheckBox->IsChecked());
    frame->SetAutoShowHousePreview(HousePreviewCheckBox->IsChecked());
    switch (ToolIconSizeChoice->GetSelection()) {
        case 3:
            frame->SetToolIconSize(48);
            break;
        case 2:
            frame->SetToolIconSize(32);
            break;
        case 1:
            frame->SetToolIconSize(24);
            break;
        case 0:
        default:
            frame->SetToolIconSize(16);
            break;
    }
    return true;
}

void ViewSettingsPanel::OnToolIconSizeChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void ViewSettingsPanel::OnHousePreviewCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void ViewSettingsPanel::OnPlayControlsCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void ViewSettingsPanel::OnEffectAssistChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void ViewSettingsPanel::OnModelHandleSizeChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void ViewSettingsPanel::OnOpenGLRenderOrderChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void ViewSettingsPanel::OnOpenGLVersionChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
