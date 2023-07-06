#include "MovingHeadPanel.h"
#include "MovingHeadEffect.h"

//(*InternalHeaders(MovingHeadPanel)
#include <wx/bitmap.h>
#include <wx/font.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MovingHeadPanel)
const long MovingHeadPanel::ID_STATICTEXT_Fixtures = wxNewId();
const long MovingHeadPanel::ID_CHECKBOX_MH1 = wxNewId();
const long MovingHeadPanel::ID_CHECKBOX_MH2 = wxNewId();
const long MovingHeadPanel::ID_CHECKBOX_MH3 = wxNewId();
const long MovingHeadPanel::ID_CHECKBOX_MH4 = wxNewId();
const long MovingHeadPanel::ID_CHECKBOX_MH5 = wxNewId();
const long MovingHeadPanel::ID_CHECKBOX_MH6 = wxNewId();
const long MovingHeadPanel::ID_CHECKBOX_MH7 = wxNewId();
const long MovingHeadPanel::ID_CHECKBOX_MH8 = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_Pan = wxNewId();
const long MovingHeadPanel::IDD_SLIDER_Pan = wxNewId();
const long MovingHeadPanel::ID_VALUECURVE_Pan = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_Pan = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_Tilt = wxNewId();
const long MovingHeadPanel::IDD_SLIDER_Tilt = wxNewId();
const long MovingHeadPanel::ID_VALUECURVE_Tilt = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_Tilt = wxNewId();
const long MovingHeadPanel::ID_PANEL_Position = wxNewId();
const long MovingHeadPanel::ID_PANEL_Fan = wxNewId();
const long MovingHeadPanel::ID_PANEL_Movement = wxNewId();
const long MovingHeadPanel::ID_PANEL_Control = wxNewId();
const long MovingHeadPanel::ID_NOTEBOOK1 = wxNewId();
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
    wxFlexGridSizer* FlexGridSizerControl;
    wxFlexGridSizer* FlexGridSizerFan;
    wxFlexGridSizer* FlexGridSizerFixtures;
    wxFlexGridSizer* FlexGridSizerFixturesLabel;
    wxFlexGridSizer* FlexGridSizerFixturesSelection;
    wxFlexGridSizer* FlexGridSizerMovement;
    wxFlexGridSizer* FlexGridSizerPosiitonPan;
    wxFlexGridSizer* FlexGridSizerPosition;
    wxFlexGridSizer* FlexGridSizerPositionTilt;

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
    CheckBox_MH1 = new wxCheckBox(this, ID_CHECKBOX_MH1, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MH1"));
    CheckBox_MH1->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MH2 = new wxCheckBox(this, ID_CHECKBOX_MH2, _("2"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MH2"));
    CheckBox_MH2->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MH3 = new wxCheckBox(this, ID_CHECKBOX_MH3, _("3"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MH3"));
    CheckBox_MH3->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MH4 = new wxCheckBox(this, ID_CHECKBOX_MH4, _("4"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MH4"));
    CheckBox_MH4->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MH5 = new wxCheckBox(this, ID_CHECKBOX_MH5, _("5"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MH5"));
    CheckBox_MH5->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MH6 = new wxCheckBox(this, ID_CHECKBOX_MH6, _("6"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MH6"));
    CheckBox_MH6->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MH7 = new wxCheckBox(this, ID_CHECKBOX_MH7, _("7"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MH7"));
    CheckBox_MH7->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MH8 = new wxCheckBox(this, ID_CHECKBOX_MH8, _("8"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MH8"));
    CheckBox_MH8->SetValue(false);
    FlexGridSizerFixturesSelection->Add(CheckBox_MH8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerFixtures->Add(FlexGridSizerFixturesSelection, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer_Main->Add(FlexGridSizerFixtures, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 5);
    Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
    PanelPosition = new wxPanel(Notebook1, ID_PANEL_Position, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Position"));
    FlexGridSizerPosition = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerPosiitonPan = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizerPosiitonPan->AddGrowableCol(1);
    Label_Pan = new wxStaticText(PanelPosition, ID_STATICTEXT_Pan, _("Pan (deg):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Pan"));
    FlexGridSizerPosiitonPan->Add(Label_Pan, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Pan = new BulkEditSliderF1(PanelPosition, IDD_SLIDER_Pan, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Pan"));
    FlexGridSizerPosiitonPan->Add(Slider_Pan, 1, wxALL|wxEXPAND, 2);
    ValueCurve_Pan = new BulkEditValueCurveButton(PanelPosition, ID_VALUECURVE_Pan, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_Pan"));
    FlexGridSizerPosiitonPan->Add(ValueCurve_Pan, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Pan = new BulkEditTextCtrlF1(PanelPosition, ID_TEXTCTRL_Pan, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelPosition,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Pan"));
    FlexGridSizerPosiitonPan->Add(TextCtrl_Pan, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerPosition->Add(FlexGridSizerPosiitonPan, 1, wxALL|wxEXPAND, 5);
    FlexGridSizerPositionTilt = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizerPositionTilt->AddGrowableCol(1);
    Label_Tilt = new wxStaticText(PanelPosition, ID_STATICTEXT_Tilt, _("Tilt (deg):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Tilt"));
    FlexGridSizerPositionTilt->Add(Label_Tilt, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Tilt = new BulkEditSliderF1(PanelPosition, IDD_SLIDER_Tilt, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tilt"));
    FlexGridSizerPositionTilt->Add(Slider_Tilt, 1, wxALL|wxEXPAND, 2);
    ValueCurve_Tilt = new BulkEditValueCurveButton(PanelPosition, ID_VALUECURVE_Tilt, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_Tilt"));
    FlexGridSizerPositionTilt->Add(ValueCurve_Tilt, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Tilt = new BulkEditTextCtrlF1(PanelPosition, ID_TEXTCTRL_Tilt, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelPosition,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Tilt"));
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
    SetSizer(FlexGridSizer_Main);
    FlexGridSizer_Main->Fit(this);
    FlexGridSizer_Main->SetSizeHints(this);
    //*)
    
    SetName("ID_PANEL_MOVINGHEAD");

    Connect(ID_VALUECURVE_Pan,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Tilt,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&MovingHeadPanel::OnVCChanged, nullptr, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&MovingHeadPanel::OnValidateWindow, nullptr, this);

    ValueCurve_Pan->GetValue()->SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    ValueCurve_Pan->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);

    ValueCurve_Tilt->GetValue()->SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    ValueCurve_Tilt->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);

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

