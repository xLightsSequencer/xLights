#include "MovingHeadPanel.h"
#include "MovingHeadEffect.h"
#include "Model.h"
#include "DmxMovingHeadAdv.h"
#include "../xLightsMain.h"
#include "../sequencer/MainSequencer.h"
#include "../sequencer/Effect.h"
#include "../sequencer/Element.h"
#include "../models/ModelGroup.h"
#include "xLightsApp.h"

//(*InternalHeaders(MovingHeadPanel)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MovingHeadPanel)
const long MovingHeadPanel::ID_STATICTEXT_Fixtures = wxNewId();
const long MovingHeadPanel::IDD_CHECKBOX_MH1 = wxNewId();
const long MovingHeadPanel::IDD_CHECKBOX_MH2 = wxNewId();
const long MovingHeadPanel::IDD_CHECKBOX_MH3 = wxNewId();
const long MovingHeadPanel::IDD_CHECKBOX_MH4 = wxNewId();
const long MovingHeadPanel::IDD_CHECKBOX_MH5 = wxNewId();
const long MovingHeadPanel::IDD_CHECKBOX_MH6 = wxNewId();
const long MovingHeadPanel::IDD_CHECKBOX_MH7 = wxNewId();
const long MovingHeadPanel::IDD_CHECKBOX_MH8 = wxNewId();
const long MovingHeadPanel::ID_BUTTON_All = wxNewId();
const long MovingHeadPanel::ID_BUTTON_None = wxNewId();
const long MovingHeadPanel::ID_BUTTON_Evens = wxNewId();
const long MovingHeadPanel::ID_BUTTON_Odds = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_Pan = wxNewId();
const long MovingHeadPanel::ID_SLIDER_Pan = wxNewId();
const long MovingHeadPanel::IDD_TEXTCTRL_Pan = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_Tilt = wxNewId();
const long MovingHeadPanel::ID_SLIDER_Tilt = wxNewId();
const long MovingHeadPanel::IDD_TEXTCTRL_Tilt = wxNewId();
const long MovingHeadPanel::ID_PANEL_Position = wxNewId();
const long MovingHeadPanel::ID_PANEL_Fan = wxNewId();
const long MovingHeadPanel::ID_PANEL_Movement = wxNewId();
const long MovingHeadPanel::ID_PANEL_Control = wxNewId();
const long MovingHeadPanel::ID_NOTEBOOK1 = wxNewId();
const long MovingHeadPanel::ID_BUTTON_Apply = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH1 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH1 = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH2 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH2 = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH3 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH3 = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH4 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH4 = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH5 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH5 = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH6 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH6 = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH7 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH7 = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH8 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH8 = wxNewId();
//*)

BEGIN_EVENT_TABLE(MovingHeadPanel,wxPanel)
	//(*EventTable(MovingHeadPanel)
	//*)
END_EVENT_TABLE()

MovingHeadPanel::MovingHeadPanel(wxWindow* parent) : xlEffectPanel(parent)
{
    //(*Initialize(MovingHeadPanel)
    BulkEditTextCtrlF1* TextCtrl_Pan;
    BulkEditTextCtrlF1* TextCtrl_Tilt;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizerControl;
    wxFlexGridSizer* FlexGridSizerFan;
    wxFlexGridSizer* FlexGridSizerFixtures;
    wxFlexGridSizer* FlexGridSizerFixturesLabel;
    wxFlexGridSizer* FlexGridSizerFixturesSelection;
    wxFlexGridSizer* FlexGridSizerMovement;
    wxFlexGridSizer* FlexGridSizerPosiitonPan;
    wxFlexGridSizer* FlexGridSizerPosition;
    wxFlexGridSizer* FlexGridSizerPositionTilt;
    wxFlexGridSizer* FlexGridSizer_Positions;
    wxTextCtrl* TextCtrl_MH1;
    wxTextCtrl* TextCtrl_MH2;
    wxTextCtrl* TextCtrl_MH3;
    wxTextCtrl* TextCtrl_MH4;
    wxTextCtrl* TextCtrl_MH5;
    wxTextCtrl* TextCtrl_MH6;
    wxTextCtrl* TextCtrl_MH7;
    wxTextCtrl* TextCtrl_MH8;

    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
    FlexGridSizer_Main = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer_Main->AddGrowableCol(0);
    FlexGridSizerFixtures = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerFixturesLabel = new wxFlexGridSizer(0, 3, 0, 0);
    StaticTextFixtures = new wxStaticText(this, ID_STATICTEXT_Fixtures, _("Fixtures"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fixtures"));
    wxFont StaticTextFixturesFont(12,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,_T("Arial"),wxFONTENCODING_DEFAULT);
    StaticTextFixtures->SetFont(StaticTextFixturesFont);
    FlexGridSizerFixturesLabel->Add(StaticTextFixtures, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerFixtures->Add(FlexGridSizerFixturesLabel, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizerFixturesSelection = new wxFlexGridSizer(0, 8, 0, 0);
    CheckBox_MH1 = new wxCheckBox(this, IDD_CHECKBOX_MH1, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_CHECKBOX_MH1"));
    CheckBox_MH1->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MH2 = new wxCheckBox(this, IDD_CHECKBOX_MH2, _("2"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_CHECKBOX_MH2"));
    CheckBox_MH2->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MH3 = new wxCheckBox(this, IDD_CHECKBOX_MH3, _("3"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_CHECKBOX_MH3"));
    CheckBox_MH3->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MH4 = new wxCheckBox(this, IDD_CHECKBOX_MH4, _("4"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_CHECKBOX_MH4"));
    CheckBox_MH4->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MH5 = new wxCheckBox(this, IDD_CHECKBOX_MH5, _("5"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_CHECKBOX_MH5"));
    CheckBox_MH5->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MH6 = new wxCheckBox(this, IDD_CHECKBOX_MH6, _("6"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_CHECKBOX_MH6"));
    CheckBox_MH6->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MH7 = new wxCheckBox(this, IDD_CHECKBOX_MH7, _("7"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_CHECKBOX_MH7"));
    CheckBox_MH7->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MH8 = new wxCheckBox(this, IDD_CHECKBOX_MH8, _("8"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_CHECKBOX_MH8"));
    CheckBox_MH8->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerFixtures->Add(FlexGridSizerFixturesSelection, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
    Button_All = new wxButton(this, ID_BUTTON_All, _("All"), wxDefaultPosition, wxSize(50,23), 0, wxDefaultValidator, _T("ID_BUTTON_All"));
    FlexGridSizer2->Add(Button_All, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_None = new wxButton(this, ID_BUTTON_None, _("None"), wxDefaultPosition, wxSize(50,23), 0, wxDefaultValidator, _T("ID_BUTTON_None"));
    FlexGridSizer2->Add(Button_None, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Evens = new wxButton(this, ID_BUTTON_Evens, _("Evens"), wxDefaultPosition, wxSize(50,23), 0, wxDefaultValidator, _T("ID_BUTTON_Evens"));
    FlexGridSizer2->Add(Button_Evens, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Odds = new wxButton(this, ID_BUTTON_Odds, _("Odds"), wxDefaultPosition, wxSize(50,23), 0, wxDefaultValidator, _T("ID_BUTTON_Odds"));
    FlexGridSizer2->Add(Button_Odds, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerFixtures->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer_Main->Add(FlexGridSizerFixtures, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 5);
    Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
    PanelPosition = new wxPanel(Notebook1, ID_PANEL_Position, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Position"));
    FlexGridSizerPosition = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerPosition->AddGrowableCol(0);
    FlexGridSizerPosiitonPan = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizerPosiitonPan->AddGrowableCol(1);
    Label_Pan = new wxStaticText(PanelPosition, ID_STATICTEXT_Pan, _("Pan (deg):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Pan"));
    FlexGridSizerPosiitonPan->Add(Label_Pan, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Pan = new BulkEditSliderF1(PanelPosition, ID_SLIDER_Pan, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Pan"));
    FlexGridSizerPosiitonPan->Add(Slider_Pan, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Pan = new BulkEditTextCtrlF1(PanelPosition, IDD_TEXTCTRL_Pan, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelPosition,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_Pan"));
    FlexGridSizerPosiitonPan->Add(TextCtrl_Pan, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerPosition->Add(FlexGridSizerPosiitonPan, 1, wxALL|wxEXPAND, 5);
    FlexGridSizerPositionTilt = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizerPositionTilt->AddGrowableCol(1);
    Label_Tilt = new wxStaticText(PanelPosition, ID_STATICTEXT_Tilt, _("Tilt (deg):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Tilt"));
    FlexGridSizerPositionTilt->Add(Label_Tilt, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Tilt = new BulkEditSliderF1(PanelPosition, ID_SLIDER_Tilt, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Tilt"));
    FlexGridSizerPositionTilt->Add(Slider_Tilt, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Tilt = new BulkEditTextCtrlF1(PanelPosition, IDD_TEXTCTRL_Tilt, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelPosition,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_Tilt"));
    FlexGridSizerPositionTilt->Add(TextCtrl_Tilt, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerPosition->Add(FlexGridSizerPositionTilt, 1, wxALL|wxEXPAND, 5);
    PanelPosition->SetSizer(FlexGridSizerPosition);
    FlexGridSizerPosition->Fit(PanelPosition);
    FlexGridSizerPosition->SetSizeHints(PanelPosition);
    PanelFan = new wxPanel(Notebook1, ID_PANEL_Fan, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Fan"));
    FlexGridSizerFan = new wxFlexGridSizer(0, 3, 0, 0);
    PanelFan->SetSizer(FlexGridSizerFan);
    FlexGridSizerFan->Fit(PanelFan);
    FlexGridSizerFan->SetSizeHints(PanelFan);
    PanelMovement = new wxPanel(Notebook1, ID_PANEL_Movement, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Movement"));
    FlexGridSizerMovement = new wxFlexGridSizer(0, 3, 0, 0);
    PanelMovement->SetSizer(FlexGridSizerMovement);
    FlexGridSizerMovement->Fit(PanelMovement);
    FlexGridSizerMovement->SetSizeHints(PanelMovement);
    PanelControl = new wxPanel(Notebook1, ID_PANEL_Control, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Control"));
    FlexGridSizerControl = new wxFlexGridSizer(0, 3, 0, 0);
    PanelControl->SetSizer(FlexGridSizerControl);
    FlexGridSizerControl->Fit(PanelControl);
    FlexGridSizerControl->SetSizeHints(PanelControl);
    Notebook1->AddPage(PanelPosition, _("Position"), false);
    Notebook1->AddPage(PanelFan, _("Fan"), false);
    Notebook1->AddPage(PanelMovement, _("Movement"), false);
    Notebook1->AddPage(PanelControl, _("Control"), false);
    FlexGridSizer_Main->Add(Notebook1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    Button_Apply = new wxButton(this, ID_BUTTON_Apply, _("Apply"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Apply"));
    FlexGridSizer1->Add(Button_Apply, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer_Main->Add(FlexGridSizer1, 1, wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizer_Positions = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText_MH1 = new wxStaticText(this, ID_STATICTEXT_MH1, _("MH1:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH1"));
    FlexGridSizer_Positions->Add(StaticText_MH1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH1 = new wxTextCtrl(this, ID_TEXTCTRL_MH1, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH1"));
    TextCtrl_MH1->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH1, 1, wxALL, 5);
    StaticText_MH2 = new wxStaticText(this, ID_STATICTEXT_MH2, _("MH2:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH2"));
    FlexGridSizer_Positions->Add(StaticText_MH2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH2 = new wxTextCtrl(this, ID_TEXTCTRL_MH2, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH2"));
    TextCtrl_MH2->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH2, 1, wxALL, 5);
    StaticText_MH3 = new wxStaticText(this, ID_STATICTEXT_MH3, _("MH3:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH3"));
    FlexGridSizer_Positions->Add(StaticText_MH3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH3 = new wxTextCtrl(this, ID_TEXTCTRL_MH3, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH3"));
    TextCtrl_MH3->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH3, 1, wxALL, 5);
    StaticText_MH4 = new wxStaticText(this, ID_STATICTEXT_MH4, _("MH4:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH4"));
    FlexGridSizer_Positions->Add(StaticText_MH4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH4 = new wxTextCtrl(this, ID_TEXTCTRL_MH4, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH4"));
    TextCtrl_MH4->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH4, 1, wxALL, 5);
    StaticText_MH5 = new wxStaticText(this, ID_STATICTEXT_MH5, _("MH5:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH5"));
    FlexGridSizer_Positions->Add(StaticText_MH5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH5 = new wxTextCtrl(this, ID_TEXTCTRL_MH5, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH5"));
    TextCtrl_MH5->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH5, 1, wxALL, 5);
    StaticText_MH6 = new wxStaticText(this, ID_STATICTEXT_MH6, _("MH6:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH6"));
    FlexGridSizer_Positions->Add(StaticText_MH6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH6 = new wxTextCtrl(this, ID_TEXTCTRL_MH6, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH6"));
    TextCtrl_MH6->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH6, 1, wxALL, 5);
    StaticText_MH7 = new wxStaticText(this, ID_STATICTEXT_MH7, _("MH7:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH7"));
    FlexGridSizer_Positions->Add(StaticText_MH7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH7 = new wxTextCtrl(this, ID_TEXTCTRL_MH7, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH7"));
    TextCtrl_MH7->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH7, 1, wxALL, 5);
    StaticText_MH8 = new wxStaticText(this, ID_STATICTEXT_MH8, _("MH8:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH8"));
    FlexGridSizer_Positions->Add(StaticText_MH8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH8 = new wxTextCtrl(this, ID_TEXTCTRL_MH8, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH8"));
    TextCtrl_MH8->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH8, 1, wxALL, 5);
    FlexGridSizer_Main->Add(FlexGridSizer_Positions, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer_Main);
    FlexGridSizer_Main->Fit(this);
    FlexGridSizer_Main->SetSizeHints(this);

    Connect(ID_BUTTON_All,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButton_AllClick);
    Connect(ID_BUTTON_None,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButton_NoneClick);
    Connect(ID_BUTTON_Evens,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButton_EvensClick);
    Connect(ID_BUTTON_Odds,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButton_OddsClick);
    Connect(ID_BUTTON_Apply,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButton_ApplyClick);
    //*)

    SetName("ID_PANEL_MOVINGHEAD");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&MovingHeadPanel::OnVCChanged, nullptr, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&MovingHeadPanel::OnValidateWindow, nullptr, this);

    ValidateWindow();
}

MovingHeadPanel::~MovingHeadPanel()
{
	//(*Destroy(MovingHeadPanel)
	//*)
}

void MovingHeadPanel::ValidateWindow()
{
}

void MovingHeadPanel::OnButton_ApplyClick(wxCommandEvent& event)
{
    for( int i = 1; i <= 8; ++i ) {
        wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
        wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
        if( checkbox != nullptr ) {
            if( checkbox->IsChecked() ) {
                wxString textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d", i);
                wxTextCtrl* textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
                if( textbox != nullptr ) {
                    wxString pan_ctrl = wxString::Format("IDD_TEXTCTRL_Pan", i);
                    wxTextCtrl* pan_textbox = (wxTextCtrl*)(this->FindWindowByName(pan_ctrl));
                    wxString tilt_ctrl = wxString::Format("IDD_TEXTCTRL_Tilt", i);
                    wxTextCtrl* tilt_textbox = (wxTextCtrl*)(this->FindWindowByName(tilt_ctrl));
                    if( pan_textbox != nullptr && tilt_textbox != nullptr ) {
                        wxString value = wxString::Format("Pan: %s, Tilt: %s",pan_textbox->GetValue(), tilt_textbox->GetValue());
                        textbox->SetValue(value);
                    }
                }
            }
        }
    }
}

void MovingHeadPanel::UncheckAllFixtures()
{
    for( int i = 1; i <= 8; ++i ) {
        wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
        wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
        if( checkbox != nullptr ) {
            checkbox->SetValue(false);
        }
    }
}

void MovingHeadPanel::OnButton_AllClick(wxCommandEvent& event)
{
    UncheckAllFixtures();

    auto models = GetActiveModels();

    for (const auto& it : models) {
        if( it->GetDisplayAs() == "DmxMovingHeadAdv" ) {
            DmxMovingHeadAdv* mhead = (DmxMovingHeadAdv*)it;
            int num = mhead->GetFixtureVal();
            wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", num);
            wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
            if( checkbox != nullptr ) {
                checkbox->SetValue(true);
            }
       }
    }
    ProcessFirstFixture();
}

void MovingHeadPanel::OnButton_NoneClick(wxCommandEvent& event)
{
    UncheckAllFixtures();
}

void MovingHeadPanel::OnButton_EvensClick(wxCommandEvent& event)
{
    UncheckAllFixtures();

    auto models = GetActiveModels();

    for (const auto& it : models) {
        if( it->GetDisplayAs() == "DmxMovingHeadAdv" ) {
            DmxMovingHeadAdv* mhead = (DmxMovingHeadAdv*)it;
            int num = mhead->GetFixtureVal();
            if( num % 2 == 0 ) {
                wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", num);
                wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
                if( checkbox != nullptr ) {
                    checkbox->SetValue(true);
                }
            }
       }
    }
    ProcessFirstFixture();
}

void MovingHeadPanel::OnButton_OddsClick(wxCommandEvent& event)
{
    UncheckAllFixtures();

    auto models = GetActiveModels();

    for (const auto& it : models) {
        if( it->GetDisplayAs() == "DmxMovingHeadAdv" ) {
            DmxMovingHeadAdv* mhead = (DmxMovingHeadAdv*)it;
            int num = mhead->GetFixtureVal();
            if( num % 2 > 0 ) {
                wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", num);
                wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
                if( checkbox != nullptr ) {
                    checkbox->SetValue(true);
                }
            }
       }
    }
    ProcessFirstFixture();
}

std::list<Model*> MovingHeadPanel::GetActiveModels()
{
    std::list<Model*> res;

    auto effect = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffect();
    if (effect != nullptr) {
        if (effect->GetParentEffectLayer() != nullptr) {
            Element* element = effect->GetParentEffectLayer()->GetParentElement();
            if (element != nullptr) {
                ModelElement* me = dynamic_cast<ModelElement*>(element);
                if (me != nullptr) {
                    auto model = xLightsApp::GetFrame()->AllModels[me->GetModelName()];
                    if (model != nullptr) {
                        if (model->GetDisplayAs() == "ModelGroup") {
                            auto mg = dynamic_cast<ModelGroup*>(model);
                            if (mg != nullptr) {
                                for (const auto& it : mg->GetFlatModels(true, false)) {
                                    if (it->GetDisplayAs() != "ModelGroup" && it->GetDisplayAs() != "SubModel") {
                                        res.push_back(it);
                                    }
                                }
                            }
                        }
                        else if (model->GetDisplayAs() == "SubModel") {
                            // don't add SubModels
                        }
                        else {
                            res.push_back(model);
                        }
                    }
                }
            }
        }
    }

    return res;
}

void MovingHeadPanel::ProcessFirstFixture()
{
    for( int i = 1; i <= 8; ++i ) {
        wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
        wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
        if( checkbox != nullptr ) {
            if( checkbox->IsChecked() ) {
                wxString textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d", i);
                wxTextCtrl* textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
                if( textbox != nullptr ) {
                    std::string mh_settings = textbox->GetValue();
                    if( mh_settings != "" ) {
                        int pan_pos = mh_settings.find("Pan:");
                        int tilt_pos = mh_settings.find("Tilt:");
                        if( pan_pos >= 0 && tilt_pos >= 0 ) {
                            std::string pan_value = mh_settings.substr(pan_pos+5, mh_settings.length());
                            std::string tilt_value = mh_settings.substr(tilt_pos+6, mh_settings.length());
                            wxString pan_ctrl = wxString::Format("IDD_TEXTCTRL_Pan", i);
                            wxTextCtrl* pan_textbox = (wxTextCtrl*)(this->FindWindowByName(pan_ctrl));
                            if( pan_textbox != nullptr ) {
                                float pan_pos = atof(pan_value.c_str());
                                std::string pan_str = wxString::Format("%f", pan_pos);
                                pan_textbox->SetValue(pan_str);
                            }
                            wxString tilt_ctrl = wxString::Format("IDD_TEXTCTRL_Tilt", i);
                            wxTextCtrl* tilt_textbox = (wxTextCtrl*)(this->FindWindowByName(tilt_ctrl));
                            if( tilt_textbox != nullptr ) {
                                float tilt_pos = atof(tilt_value.c_str());
                                std::string tilt_str = wxString::Format("%f", tilt_pos);
                                tilt_textbox->SetValue(tilt_str);
                            }
                        }
                    }
                }
                break;
            }
        }
    }
}
