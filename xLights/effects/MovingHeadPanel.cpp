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
#include <wx/bitmap.h>
#include <wx/font.h>
#include <wx/image.h>
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
const long MovingHeadPanel::IDD_SLIDER_MHPan = wxNewId();
const long MovingHeadPanel::ID_VALUECURVE_MHPan = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MHPan = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_Tilt = wxNewId();
const long MovingHeadPanel::IDD_SLIDER_MHTilt = wxNewId();
const long MovingHeadPanel::ID_VALUECURVE_MHTilt = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MHTilt = wxNewId();
const long MovingHeadPanel::ID_PANEL_Position = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_FanPan = wxNewId();
const long MovingHeadPanel::IDD_SLIDER_MHFanPan = wxNewId();
const long MovingHeadPanel::ID_VALUECURVE_MHFanPan = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MHFanPan = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_FanTilt = wxNewId();
const long MovingHeadPanel::IDD_SLIDER_MHFanTilt = wxNewId();
const long MovingHeadPanel::ID_VALUECURVE_MHFanTilt = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MHFanTilt = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT1 = wxNewId();
const long MovingHeadPanel::IDD_SLIDER_MHPanOffset = wxNewId();
const long MovingHeadPanel::ID_VALUECURVE_MHPanOffset = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MHPanOffset = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_TiltOffset = wxNewId();
const long MovingHeadPanel::IDD_SLIDER_MHTiltOffset = wxNewId();
const long MovingHeadPanel::ID_VALUECURVE_MHTiltOffset = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MHTiltOffset = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_Groupings = wxNewId();
const long MovingHeadPanel::IDD_SLIDER_MHGroupings = wxNewId();
const long MovingHeadPanel::ID_VALUECURVE_MHGroupings = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MHGroupings = wxNewId();
const long MovingHeadPanel::ID_CHECKBOX_FanPan = wxNewId();
const long MovingHeadPanel::ID_CHECKBOX_FanTilt = wxNewId();
const long MovingHeadPanel::ID_PANEL_Fan = wxNewId();
const long MovingHeadPanel::ID_PANEL_Movement = wxNewId();
const long MovingHeadPanel::ID_PANEL_Control = wxNewId();
const long MovingHeadPanel::ID_NOTEBOOK1 = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH1 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH1 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH1_Settings = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH2 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH2 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH2_Settings = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH3 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH3 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH3_Settings = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH4 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH4 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH4_Settings = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH5 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH5 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH5_Settings = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH6 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH6 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH6_Settings = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH7 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH7 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH7_Settings = wxNewId();
const long MovingHeadPanel::ID_STATICTEXT_MH8 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH8 = wxNewId();
const long MovingHeadPanel::ID_TEXTCTRL_MH8_Settings = wxNewId();
//*)

BEGIN_EVENT_TABLE(MovingHeadPanel,wxPanel)
	//(*EventTable(MovingHeadPanel)
	//*)
END_EVENT_TABLE()

MovingHeadPanel::MovingHeadPanel(wxWindow* parent) : xlEffectPanel(parent)
{
    //(*Initialize(MovingHeadPanel)
    BulkEditTextCtrlF1* TextCtrl_MHFanPan;
    BulkEditTextCtrlF1* TextCtrl_MHFanTilt;
    BulkEditTextCtrlF1* TextCtrl_MHGroupings;
    BulkEditTextCtrlF1* TextCtrl_MHPan;
    BulkEditTextCtrlF1* TextCtrl_MHPanOffset;
    BulkEditTextCtrlF1* TextCtrl_MHTilt;
    BulkEditTextCtrlF1* TextCtrl_MHTiltOffset;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizerControl;
    wxFlexGridSizer* FlexGridSizerFan;
    wxFlexGridSizer* FlexGridSizerFixtures;
    wxFlexGridSizer* FlexGridSizerFixturesLabel;
    wxFlexGridSizer* FlexGridSizerFixturesSelection;
    wxFlexGridSizer* FlexGridSizerMovement;
    wxFlexGridSizer* FlexGridSizerPosition;
    wxFlexGridSizer* FlexGridSizerPositionPan;
    wxFlexGridSizer* FlexGridSizerPositionTilt;
    wxFlexGridSizer* FlexGridSizer_FanActive;
    wxFlexGridSizer* FlexGridSizer_FanPan;
    wxFlexGridSizer* FlexGridSizer_FanTilt;
    wxFlexGridSizer* FlexGridSizer_Groupings;
    wxFlexGridSizer* FlexGridSizer_PanOffset;
    wxFlexGridSizer* FlexGridSizer_Positions;
    wxFlexGridSizer* FlexGridSizer_TiltOffset;
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
    FlexGridSizerPositionPan = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizerPositionPan->AddGrowableCol(1);
    Label_Pan = new wxStaticText(PanelPosition, ID_STATICTEXT_Pan, _("Pan (deg):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Pan"));
    FlexGridSizerPositionPan->Add(Label_Pan, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHPan = new BulkEditSliderF1(PanelPosition, IDD_SLIDER_MHPan, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_MHPan"));
    FlexGridSizerPositionPan->Add(Slider_MHPan, 1, wxALL|wxEXPAND, 2);
    ValueCurve_MHPan = new BulkEditValueCurveButton(PanelPosition, ID_VALUECURVE_MHPan, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_MHPan"));
    FlexGridSizerPositionPan->Add(ValueCurve_MHPan, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MHPan = new BulkEditTextCtrlF1(PanelPosition, ID_TEXTCTRL_MHPan, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelPosition,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MHPan"));
    FlexGridSizerPositionPan->Add(TextCtrl_MHPan, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerPosition->Add(FlexGridSizerPositionPan, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizerPositionTilt = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizerPositionTilt->AddGrowableCol(1);
    Label_Tilt = new wxStaticText(PanelPosition, ID_STATICTEXT_Tilt, _("Tilt (deg):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Tilt"));
    FlexGridSizerPositionTilt->Add(Label_Tilt, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHTilt = new BulkEditSliderF1(PanelPosition, IDD_SLIDER_MHTilt, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_MHTilt"));
    FlexGridSizerPositionTilt->Add(Slider_MHTilt, 1, wxALL|wxEXPAND, 2);
    ValueCurve_MHTilt = new BulkEditValueCurveButton(PanelPosition, ID_VALUECURVE_MHTilt, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_MHTilt"));
    FlexGridSizerPositionTilt->Add(ValueCurve_MHTilt, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MHTilt = new BulkEditTextCtrlF1(PanelPosition, ID_TEXTCTRL_MHTilt, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelPosition,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MHTilt"));
    FlexGridSizerPositionTilt->Add(TextCtrl_MHTilt, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerPosition->Add(FlexGridSizerPositionTilt, 1, wxALL|wxEXPAND, 5);
    PanelPosition->SetSizer(FlexGridSizerPosition);
    FlexGridSizerPosition->Fit(PanelPosition);
    FlexGridSizerPosition->SetSizeHints(PanelPosition);
    PanelFan = new wxPanel(Notebook1, ID_PANEL_Fan, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Fan"));
    FlexGridSizerFan = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer_FanPan = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer_FanPan->AddGrowableCol(1);
    Label_FanPan = new wxStaticText(PanelFan, ID_STATICTEXT_FanPan, _("Fan - Pan (deg):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_FanPan"));
    FlexGridSizer_FanPan->Add(Label_FanPan, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHFanPan = new BulkEditSliderF1(PanelFan, IDD_SLIDER_MHFanPan, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_MHFanPan"));
    FlexGridSizer_FanPan->Add(Slider_MHFanPan, 1, wxALL|wxEXPAND, 2);
    ValueCurve_MHFanPan = new BulkEditValueCurveButton(PanelFan, ID_VALUECURVE_MHFanPan, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_MHFanPan"));
    FlexGridSizer_FanPan->Add(ValueCurve_MHFanPan, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MHFanPan = new BulkEditTextCtrlF1(PanelFan, ID_TEXTCTRL_MHFanPan, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelFan,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MHFanPan"));
    FlexGridSizer_FanPan->Add(TextCtrl_MHFanPan, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerFan->Add(FlexGridSizer_FanPan, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer_FanTilt = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer_FanTilt->AddGrowableCol(1);
    Label_FanTilt = new wxStaticText(PanelFan, ID_STATICTEXT_FanTilt, _("Fan - Tilt (deg):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_FanTilt"));
    FlexGridSizer_FanTilt->Add(Label_FanTilt, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHFanTilt = new BulkEditSliderF1(PanelFan, IDD_SLIDER_MHFanTilt, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_MHFanTilt"));
    FlexGridSizer_FanTilt->Add(Slider_MHFanTilt, 1, wxALL|wxEXPAND, 2);
    ValueCurve_MHFanTilt = new BulkEditValueCurveButton(PanelFan, ID_VALUECURVE_MHFanTilt, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_MHFanTilt"));
    FlexGridSizer_FanTilt->Add(ValueCurve_MHFanTilt, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MHFanTilt = new BulkEditTextCtrlF1(PanelFan, ID_TEXTCTRL_MHFanTilt, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelFan,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MHFanTilt"));
    FlexGridSizer_FanTilt->Add(TextCtrl_MHFanTilt, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerFan->Add(FlexGridSizer_FanTilt, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizer_PanOffset = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer_PanOffset->AddGrowableCol(1);
    StaticText1 = new wxStaticText(PanelFan, ID_STATICTEXT1, _("Pan Offset:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer_PanOffset->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHPanOffset = new BulkEditSliderF1(PanelFan, IDD_SLIDER_MHPanOffset, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_MHPanOffset"));
    FlexGridSizer_PanOffset->Add(Slider_MHPanOffset, 1, wxALL|wxEXPAND, 2);
    ValueCurve_MHPanOffset = new BulkEditValueCurveButton(PanelFan, ID_VALUECURVE_MHPanOffset, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_MHPanOffset"));
    FlexGridSizer_PanOffset->Add(ValueCurve_MHPanOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MHPanOffset = new BulkEditTextCtrlF1(PanelFan, ID_TEXTCTRL_MHPanOffset, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelFan,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MHPanOffset"));
    FlexGridSizer_PanOffset->Add(TextCtrl_MHPanOffset, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerFan->Add(FlexGridSizer_PanOffset, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizer_TiltOffset = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer_TiltOffset->AddGrowableCol(1);
    Label_TiltOffset = new wxStaticText(PanelFan, ID_STATICTEXT_TiltOffset, _("Tilt Offset:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_TiltOffset"));
    FlexGridSizer_TiltOffset->Add(Label_TiltOffset, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHTiltOffset = new BulkEditSliderF1(PanelFan, IDD_SLIDER_MHTiltOffset, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_MHTiltOffset"));
    FlexGridSizer_TiltOffset->Add(Slider_MHTiltOffset, 1, wxALL|wxEXPAND, 2);
    ValueCurve_MHTiltOffset = new BulkEditValueCurveButton(PanelFan, ID_VALUECURVE_MHTiltOffset, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_MHTiltOffset"));
    FlexGridSizer_TiltOffset->Add(ValueCurve_MHTiltOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MHTiltOffset = new BulkEditTextCtrlF1(PanelFan, ID_TEXTCTRL_MHTiltOffset, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelFan,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MHTiltOffset"));
    FlexGridSizer_TiltOffset->Add(TextCtrl_MHTiltOffset, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerFan->Add(FlexGridSizer_TiltOffset, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizer_Groupings = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer_Groupings->AddGrowableCol(1);
    StaticText_Groupings = new wxStaticText(PanelFan, ID_STATICTEXT_Groupings, _("Groupings:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Groupings"));
    FlexGridSizer_Groupings->Add(StaticText_Groupings, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHGroupings = new BulkEditSlider(PanelFan, IDD_SLIDER_MHGroupings, 1, 1, 6, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_MHGroupings"));
    FlexGridSizer_Groupings->Add(Slider_MHGroupings, 1, wxALL|wxEXPAND, 2);
    ValueCurve_MHGroupings = new BulkEditValueCurveButton(PanelFan, ID_VALUECURVE_MHGroupings, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_MHGroupings"));
    FlexGridSizer_Groupings->Add(ValueCurve_MHGroupings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MHGroupings = new BulkEditTextCtrlF1(PanelFan, ID_TEXTCTRL_MHGroupings, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelFan,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MHGroupings"));
    FlexGridSizer_Groupings->Add(TextCtrl_MHGroupings, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerFan->Add(FlexGridSizer_Groupings, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizer_FanActive = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox_FanPan = new wxCheckBox(PanelFan, ID_CHECKBOX_FanPan, _("Fan - Pan: Active"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FanPan"));
    CheckBox_FanPan->SetValue(false);
    FlexGridSizer_FanActive->Add(CheckBox_FanPan, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer_FanActive->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_FanTilt = new wxCheckBox(PanelFan, ID_CHECKBOX_FanTilt, _("Fan - Tilt: Active"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FanTilt"));
    CheckBox_FanTilt->SetValue(false);
    FlexGridSizer_FanActive->Add(CheckBox_FanTilt, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerFan->Add(FlexGridSizer_FanActive, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
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
    FlexGridSizer_Positions = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText_MH1 = new wxStaticText(this, ID_STATICTEXT_MH1, _("MH1:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH1"));
    FlexGridSizer_Positions->Add(StaticText_MH1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH1 = new wxTextCtrl(this, ID_TEXTCTRL_MH1, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH1"));
    TextCtrl_MH1->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH1, 1, wxALL, 5);
    TextCtrl_MH1_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH1_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH1_Settings"));
    TextCtrl_MH1_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH1_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_MH2 = new wxStaticText(this, ID_STATICTEXT_MH2, _("MH2:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH2"));
    FlexGridSizer_Positions->Add(StaticText_MH2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH2 = new wxTextCtrl(this, ID_TEXTCTRL_MH2, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH2"));
    TextCtrl_MH2->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH2, 1, wxALL, 5);
    TextCtrl_MH2_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH2_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH2_Settings"));
    TextCtrl_MH2_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH2_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_MH3 = new wxStaticText(this, ID_STATICTEXT_MH3, _("MH3:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH3"));
    FlexGridSizer_Positions->Add(StaticText_MH3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH3 = new wxTextCtrl(this, ID_TEXTCTRL_MH3, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH3"));
    TextCtrl_MH3->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH3, 1, wxALL, 5);
    TextCtrl_MH3_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH3_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH3_Settings"));
    TextCtrl_MH3_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH3_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_MH4 = new wxStaticText(this, ID_STATICTEXT_MH4, _("MH4:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH4"));
    FlexGridSizer_Positions->Add(StaticText_MH4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH4 = new wxTextCtrl(this, ID_TEXTCTRL_MH4, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH4"));
    TextCtrl_MH4->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH4, 1, wxALL, 5);
    TextCtrl_MH4_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH4_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH4_Settings"));
    TextCtrl_MH4_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH4_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_MH5 = new wxStaticText(this, ID_STATICTEXT_MH5, _("MH5:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH5"));
    FlexGridSizer_Positions->Add(StaticText_MH5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH5 = new wxTextCtrl(this, ID_TEXTCTRL_MH5, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH5"));
    TextCtrl_MH5->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH5, 1, wxALL, 5);
    TextCtrl_MH5_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH5_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH5_Settings"));
    TextCtrl_MH5_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH5_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_MH6 = new wxStaticText(this, ID_STATICTEXT_MH6, _("MH6:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH6"));
    FlexGridSizer_Positions->Add(StaticText_MH6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH6 = new wxTextCtrl(this, ID_TEXTCTRL_MH6, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH6"));
    TextCtrl_MH6->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH6, 1, wxALL, 5);
    TextCtrl_MH6_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH6_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH6_Settings"));
    TextCtrl_MH6_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH6_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_MH7 = new wxStaticText(this, ID_STATICTEXT_MH7, _("MH7:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH7"));
    FlexGridSizer_Positions->Add(StaticText_MH7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH7 = new wxTextCtrl(this, ID_TEXTCTRL_MH7, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH7"));
    TextCtrl_MH7->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH7, 1, wxALL, 5);
    TextCtrl_MH7_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH7_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH7_Settings"));
    TextCtrl_MH7_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH7_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_MH8 = new wxStaticText(this, ID_STATICTEXT_MH8, _("MH8:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MH8"));
    FlexGridSizer_Positions->Add(StaticText_MH8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH8 = new wxTextCtrl(this, ID_TEXTCTRL_MH8, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(140,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_MH8"));
    TextCtrl_MH8->Disable();
    FlexGridSizer_Positions->Add(TextCtrl_MH8, 1, wxALL, 5);
    TextCtrl_MH8_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH8_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH8_Settings"));
    TextCtrl_MH8_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH8_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer_Main->Add(FlexGridSizer_Positions, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    SetSizer(FlexGridSizer_Main);
    FlexGridSizer_Main->Fit(this);
    FlexGridSizer_Main->SetSizeHints(this);

    Connect(ID_BUTTON_All,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButton_AllClick);
    Connect(ID_BUTTON_None,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButton_NoneClick);
    Connect(ID_BUTTON_Evens,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButton_EvensClick);
    Connect(ID_BUTTON_Odds,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButton_OddsClick);
    Connect(ID_NOTEBOOK1,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&MovingHeadPanel::OnNotebook1PageChanged);
    //*)

    SetName("ID_PANEL_MOVINGHEAD");

    Connect(ID_VALUECURVE_MHPan,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_MHTilt,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_MHFanPan,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_MHFanTilt,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_MHPanOffset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_MHTiltOffset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_MHGroupings,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&MovingHeadPanel::OnVCChanged, nullptr, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&MovingHeadPanel::OnValidateWindow, nullptr, this);

    Connect(wxEVT_COMMAND_SLIDER_UPDATED, (wxObjectEventFunction)&MovingHeadPanel::OnSliderUpdated);
    Connect(wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&MovingHeadPanel::OnTextCtrlUpdated);

    ValueCurve_MHPan->GetValue()->SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    ValueCurve_MHPan->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);
    ValueCurve_MHTilt->GetValue()->SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    ValueCurve_MHTilt->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);
    ValueCurve_MHFanPan->GetValue()->SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    ValueCurve_MHFanPan->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);
    ValueCurve_MHFanTilt->GetValue()->SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    ValueCurve_MHFanTilt->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);
    ValueCurve_MHPanOffset->GetValue()->SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    ValueCurve_MHPanOffset->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);
    ValueCurve_MHTiltOffset->GetValue()->SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    ValueCurve_MHTiltOffset->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);
    ValueCurve_MHGroupings->GetValue()->SetLimits(MOVING_HEAD_GROUP_MIN, MOVING_HEAD_GROUP_MAX);
    ValueCurve_MHGroupings->GetValue()->SetDivisor(MOVING_HEAD_GROUP_DIVISOR);

    ValidateWindow();
}

MovingHeadPanel::~MovingHeadPanel()
{
	//(*Destroy(MovingHeadPanel)
	//*)
}

void MovingHeadPanel::OnNotebook1PageChanged(wxNotebookEvent& event)
{
}

void MovingHeadPanel::ValidateWindow()
{
}

void MovingHeadPanel::OnVCChanged(wxCommandEvent& event)
{
    EffectPanelUtils::OnVCChanged(event);

    int event_id = event.GetId();

    UpdateMHSettings();

    FireChangeEvent();
}

void MovingHeadPanel::OnSliderUpdated(wxCommandEvent& event)
{
    UpdateMHSettings();
}

void MovingHeadPanel::OnTextCtrlUpdated(wxCommandEvent& event)
{
    int event_id = event.GetId();

    if (event_id == ID_TEXTCTRL_MHPan || event_id == ID_TEXTCTRL_MHTilt) {
        UpdateMHSettings();
    }
}

void MovingHeadPanel::UpdateMHSettings()
{
    bool has_fanpan = false;
    bool has_fantilt = false;
    std::string headset = xlEMPTY_STRING;

    if( CheckBox_FanPan->IsChecked() ) {
        has_fanpan = true;
    }
    if( CheckBox_FanTilt->IsChecked() ) {
        has_fantilt = true;
    }

    // build a string with the selected moving heads
    if( has_fanpan || has_fantilt ) {
        bool add_comma = false;
        headset = "Heads:";
        for( int i = 1; i <= 8; ++i ) {
            wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
            wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
            if( checkbox != nullptr ) {
                if( checkbox->IsChecked() ) {
                    if( add_comma ) {
                        headset += ",";
                    } else {
                        headset += " ";
                    }
                    int fixture_num = 1;
                    auto models = GetActiveModels();
                    for (const auto& it : models) {
                        if( it->GetDisplayAs() == "DmxMovingHeadAdv" ) {
                            DmxMovingHeadAdv* mhead = (DmxMovingHeadAdv*)it;
                            if( mhead->GetFixtureVal() == i ) {
                                fixture_num = mhead->GetFixtureVal();
                            }
                        }
                    }
                    headset += wxString::Format("%d", fixture_num);
                    add_comma = true;
                }
            }
        }
    }

    for( int i = 1; i <= 8; ++i ) {
        wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
        wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
        if( checkbox != nullptr ) {
            if( checkbox->IsChecked() ) {
                std::string ugly_settings = xlEMPTY_STRING;
                std::string pretty_settings = xlEMPTY_STRING;

                // Add pan settings
                if( has_fanpan ){
                    AddSetting( "FanPan", ugly_settings, pretty_settings );
                    AddSetting( "PanOffset", ugly_settings, pretty_settings );
                } else {
                    AddSetting( "Pan", ugly_settings, pretty_settings );
                }

                // Add tilt settings
                if( has_fantilt ) {
                    AddSetting( "FanTilt", ugly_settings, pretty_settings );
                    AddSetting( "TiltOffset", ugly_settings, pretty_settings );
                } else {
                    AddSetting( "Tilt", ugly_settings, pretty_settings );
                }

                if( has_fanpan || has_fantilt ) {
                    AddSetting( "Groupings", ugly_settings, pretty_settings );
                    ugly_settings += ";";
                    ugly_settings += headset;
                }

                // update the settings textbox
                wxString textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d_Settings", i);
                wxTextCtrl* mh_textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
                if( mh_textbox != nullptr ) {
                    if( ugly_settings != xlEMPTY_STRING ) {
                        mh_textbox->SetValue(ugly_settings);
                    }
                }
                textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d", i);
                mh_textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
                if( mh_textbox != nullptr ) {
                    if( pretty_settings != xlEMPTY_STRING ) {
                        mh_textbox->SetValue(pretty_settings);
                    }
                }
            }
        }
    }
}

void MovingHeadPanel::AddSetting(const std::string& name, std::string& ugly_settings, std::string& pretty_settings)
{
    wxTextCtrl* textbox = (wxTextCtrl*)(this->FindWindowByName("ID_TEXTCTRL_MH" + name));
    if( textbox != nullptr && !textbox->IsEnabled() ) {
        BulkEditValueCurveButton* vc_button = (BulkEditValueCurveButton*)(this->FindWindowByName("ID_VALUECURVE_MH" + name));
        if( vc_button != nullptr ) {
            ValueCurve* vc = vc_button->GetValue();
            AddValueCurve(vc, name + " VC", ugly_settings, pretty_settings);
        }
    } else {
        AddTextbox("ID_TEXTCTRL_MH" + name, name, ugly_settings, pretty_settings);
    }
}

void MovingHeadPanel::AddValueCurve(ValueCurve* vc, const std::string& name, std::string& ugly_settings, std::string& pretty_settings)
{
    if( ugly_settings != xlEMPTY_STRING ) {
        ugly_settings += ";";
        pretty_settings += "; ";
    }
    std::string vc_text = vc->Serialise();
    std::replace( vc_text.begin(), vc_text.end(), ';', '@'); // custom value curves were using my delimiter
    wxString value = wxString::Format("%s: %s", name, vc_text);
    ugly_settings += value;

    // create the pretty string
    value = wxString::Format("%s: ", name);
    pretty_settings += value;
    std::string vc_type = vc->GetType();
    pretty_settings += vc_type;
    pretty_settings += " ";
    if (vc_type == "Ramp" || vc_type == "Ramp Up/Down")
    {
        float p1 = vc->GetParameter1();
        float p2 = vc->GetParameter2();
        value = wxString::Format("P1=%3.1f,P2=%3.1f", p1,  p2);
        pretty_settings += value;
        if (vc_type == "Ramp Up/Down")
        {
            float p3 = vc->GetParameter3();
            value = wxString::Format(",P3=%3.1f", p3);
            pretty_settings += value;
        }
    }
}

void MovingHeadPanel::AddTextbox(const std::string& ctrl_id, const std::string& name, std::string& ugly_settings, std::string& pretty_settings)
{
    wxTextCtrl* textbox = (wxTextCtrl*)(this->FindWindowByName(ctrl_id));
    if( textbox != nullptr ) {
        if( ugly_settings != xlEMPTY_STRING ) {
            ugly_settings += ";";
            pretty_settings += "; ";
        }
        wxString value = wxString::Format("%s: %s", name, textbox->GetValue());
        ugly_settings += value;
        pretty_settings += value;
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
    //ProcessFirstFixture();
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
    //ProcessFirstFixture();
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
    //ProcessFirstFixture();
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
                            wxString pan_ctrl = wxString::Format("ID_TEXTCTRL_Pan", i);
                            wxTextCtrl* pan_textbox = (wxTextCtrl*)(this->FindWindowByName(pan_ctrl));
                            if( pan_textbox != nullptr ) {
                                float pan_pos = atof(pan_value.c_str());
                                std::string pan_str = wxString::Format("%f", pan_pos);
                                pan_textbox->SetValue(pan_str);
                            }
                            wxString tilt_ctrl = wxString::Format("ID_TEXTCTRL_Tilt", i);
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
