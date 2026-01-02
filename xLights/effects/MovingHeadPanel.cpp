/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MovingHeadPanel.h"
#include "MovingHeadEffect.h"
#include "MovingHeadPanels/MHPresetBitmapButton.h"
#include "MovingHeadPanels/MHPathPresetBitmapButton.h"
#include "MovingHeadPanels/MHDimmerPresetBitmapButton.h"
#include "../models/Model.h"
#include "../models/DMX/DmxMovingHeadAdv.h"
#include "../models/DMX/DmxColorAbility.h"
#include "../xLightsMain.h"
#include "../xLightsVersion.h"
#include "../sequencer/MainSequencer.h"
#include "../sequencer/Effect.h"
#include "../sequencer/Element.h"
#include "../models/ModelGroup.h"
#include "xLightsApp.h"
#include <wx/stdpaths.h>

#include "ExternalHooks.h"

#include "./utils/spdlog_macros.h"

//(*InternalHeaders(MovingHeadPanel)
#include <wx/bitmap.h>
#include <wx/font.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MovingHeadPanel)
const wxWindowID MovingHeadPanel::ID_STATICTEXT_Fixtures = wxNewId();
const wxWindowID MovingHeadPanel::ID_BUTTON_All = wxNewId();
const wxWindowID MovingHeadPanel::ID_BUTTON_None = wxNewId();
const wxWindowID MovingHeadPanel::ID_BUTTON_Evens = wxNewId();
const wxWindowID MovingHeadPanel::ID_BUTTON_Odds = wxNewId();
const wxWindowID MovingHeadPanel::IDD_CHECKBOX_MH1 = wxNewId();
const wxWindowID MovingHeadPanel::IDD_CHECKBOX_MH2 = wxNewId();
const wxWindowID MovingHeadPanel::IDD_CHECKBOX_MH3 = wxNewId();
const wxWindowID MovingHeadPanel::IDD_CHECKBOX_MH4 = wxNewId();
const wxWindowID MovingHeadPanel::IDD_CHECKBOX_MH5 = wxNewId();
const wxWindowID MovingHeadPanel::IDD_CHECKBOX_MH6 = wxNewId();
const wxWindowID MovingHeadPanel::IDD_CHECKBOX_MH7 = wxNewId();
const wxWindowID MovingHeadPanel::IDD_CHECKBOX_MH8 = wxNewId();
const wxWindowID MovingHeadPanel::ID_STATICTEXT_Pan = wxNewId();
const wxWindowID MovingHeadPanel::ID_SLIDER_MHPan = wxNewId();
const wxWindowID MovingHeadPanel::ID_VALUECURVE_MHPan = wxNewId();
const wxWindowID MovingHeadPanel::IDD_TEXTCTRL_MHPan = wxNewId();
const wxWindowID MovingHeadPanel::ID_STATICTEXT_Tilt = wxNewId();
const wxWindowID MovingHeadPanel::ID_SLIDER_MHTilt = wxNewId();
const wxWindowID MovingHeadPanel::ID_VALUECURVE_MHTilt = wxNewId();
const wxWindowID MovingHeadPanel::IDD_TEXTCTRL_MHTilt = wxNewId();
const wxWindowID MovingHeadPanel::ID_STATICTEXT_PanOffset = wxNewId();
const wxWindowID MovingHeadPanel::ID_SLIDER_MHPanOffset = wxNewId();
const wxWindowID MovingHeadPanel::ID_VALUECURVE_MHPanOffset = wxNewId();
const wxWindowID MovingHeadPanel::IDD_TEXTCTRL_MHPanOffset = wxNewId();
const wxWindowID MovingHeadPanel::ID_STATICTEXT_TiltOffset = wxNewId();
const wxWindowID MovingHeadPanel::ID_SLIDER_MHTiltOffset = wxNewId();
const wxWindowID MovingHeadPanel::ID_VALUECURVE_MHTiltOffset = wxNewId();
const wxWindowID MovingHeadPanel::IDD_TEXTCTRL_MHTiltOffset = wxNewId();
const wxWindowID MovingHeadPanel::ID_STATICTEXT_Groupings = wxNewId();
const wxWindowID MovingHeadPanel::ID_SLIDER_MHGroupings = wxNewId();
const wxWindowID MovingHeadPanel::ID_VALUECURVE_MHGroupings = wxNewId();
const wxWindowID MovingHeadPanel::IDD_TEXTCTRL_MHGroupings = wxNewId();
const wxWindowID MovingHeadPanel::ID_STATICTEXT_MHCycles = wxNewId();
const wxWindowID MovingHeadPanel::ID_SLIDER_MHCycles = wxNewId();
const wxWindowID MovingHeadPanel::IDD_TEXTCTRL_MHCycles = wxNewId();
const wxWindowID MovingHeadPanel::ID_BUTTON_SavePreset = wxNewId();
const wxWindowID MovingHeadPanel::ID_PANEL_Position = wxNewId();
const wxWindowID MovingHeadPanel::ID_BUTTON_DimmerOn = wxNewId();
const wxWindowID MovingHeadPanel::ID_BUTTON_DimmerOff = wxNewId();
const wxWindowID MovingHeadPanel::ID_BUTTON_SaveDimmerPreset = wxNewId();
const wxWindowID MovingHeadPanel::ID_PANEL_Dimmer = wxNewId();
const wxWindowID MovingHeadPanel::ID_BUTTON_MHPathContinue = wxNewId();
const wxWindowID MovingHeadPanel::ID_BUTTON_MHPathClear = wxNewId();
const wxWindowID MovingHeadPanel::ID_BUTTON_MHPathClose = wxNewId();
const wxWindowID MovingHeadPanel::ID_TEXTCTRL_MHPathDef = wxNewId();
const wxWindowID MovingHeadPanel::ID_STATICTEXT_PathScale = wxNewId();
const wxWindowID MovingHeadPanel::ID_SLIDER_MHPathScale = wxNewId();
const wxWindowID MovingHeadPanel::ID_VALUECURVE_MHPathScale = wxNewId();
const wxWindowID MovingHeadPanel::IDD_TEXTCTRL_MHPathScale = wxNewId();
const wxWindowID MovingHeadPanel::ID_STATICTEXT_TimeOffset = wxNewId();
const wxWindowID MovingHeadPanel::ID_SLIDER_MHTimeOffset = wxNewId();
const wxWindowID MovingHeadPanel::ID_VALUECURVE_MHTimeOffset = wxNewId();
const wxWindowID MovingHeadPanel::IDD_TEXTCTRL_MHTimeOffset = wxNewId();
const wxWindowID MovingHeadPanel::ID_CHECKBOX_MHIgnorePan = wxNewId();
const wxWindowID MovingHeadPanel::ID_CHECKBOX_MHIgnoreTilt = wxNewId();
const wxWindowID MovingHeadPanel::ID_BUTTON_SavePathPreset = wxNewId();
const wxWindowID MovingHeadPanel::ID_PANEL_Pathing = wxNewId();
const wxWindowID MovingHeadPanel::ID_PANEL_Color = wxNewId();
const wxWindowID MovingHeadPanel::ID_CHECKBOX_AUTO_SHUTTER = wxNewId();
const wxWindowID MovingHeadPanel::ID_PANEL_ColorWheel = wxNewId();
const wxWindowID MovingHeadPanel::ID_NOTEBOOK2 = wxNewId();
const wxWindowID MovingHeadPanel::ID_PANEL_Control = wxNewId();
const wxWindowID MovingHeadPanel::IDD_TEXTCTRL_Status = wxNewId();
const wxWindowID MovingHeadPanel::ID_BUTTON_ResetToDefault = wxNewId();
const wxWindowID MovingHeadPanel::ID_PANEL1 = wxNewId();
const wxWindowID MovingHeadPanel::ID_NOTEBOOK1 = wxNewId();
const wxWindowID MovingHeadPanel::ID_TEXTCTRL_MH1_Settings = wxNewId();
const wxWindowID MovingHeadPanel::ID_TEXTCTRL_MH2_Settings = wxNewId();
const wxWindowID MovingHeadPanel::ID_TEXTCTRL_MH3_Settings = wxNewId();
const wxWindowID MovingHeadPanel::ID_TEXTCTRL_MH4_Settings = wxNewId();
const wxWindowID MovingHeadPanel::ID_TEXTCTRL_MH5_Settings = wxNewId();
const wxWindowID MovingHeadPanel::ID_TEXTCTRL_MH6_Settings = wxNewId();
const wxWindowID MovingHeadPanel::ID_TEXTCTRL_MH7_Settings = wxNewId();
const wxWindowID MovingHeadPanel::ID_TEXTCTRL_MH8_Settings = wxNewId();
//*)

BEGIN_EVENT_TABLE(MovingHeadPanel,wxPanel)
	//(*EventTable(MovingHeadPanel)
	//*)
END_EVENT_TABLE()

MovingHeadPanel::MovingHeadPanel(wxWindow* parent) : xlEffectPanel(parent)
{
    //(*Initialize(MovingHeadPanel)
    BulkEditTextCtrl* TextCtrl_MHGroupings;
    BulkEditTextCtrlF1* TextCtrl_MHCycles;
    BulkEditTextCtrlF1* TextCtrl_MHPan;
    BulkEditTextCtrlF1* TextCtrl_MHPanOffset;
    BulkEditTextCtrlF1* TextCtrl_MHTilt;
    BulkEditTextCtrlF1* TextCtrl_MHTiltOffset;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizerColorMain;
    wxFlexGridSizer* FlexGridSizerColorSliders;
    wxFlexGridSizer* FlexGridSizerColorWheelMain;
    wxFlexGridSizer* FlexGridSizerColorWheelSliders;
    wxFlexGridSizer* FlexGridSizerControl;
    wxFlexGridSizer* FlexGridSizerCycles;
    wxFlexGridSizer* FlexGridSizerDimmer;
    wxFlexGridSizer* FlexGridSizerFixtures;
    wxFlexGridSizer* FlexGridSizerFixturesLabel;
    wxFlexGridSizer* FlexGridSizerFixturesSelection;
    wxFlexGridSizer* FlexGridSizerIgnoreBoxes;
    wxFlexGridSizer* FlexGridSizerPositionPan;
    wxFlexGridSizer* FlexGridSizerPositionTilt;
    wxFlexGridSizer* FlexGridSizer_Groupings;
    wxFlexGridSizer* FlexGridSizer_PanOffset;
    wxFlexGridSizer* FlexGridSizer_PathButtons;
    wxFlexGridSizer* FlexGridSizer_PathControls;
    wxFlexGridSizer* FlexGridSizer_Positions;
    wxFlexGridSizer* FlexGridSizer_TiltOffset;

    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
    FlexGridSizer_Main = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer_Main->AddGrowableCol(0);
    FlexGridSizerFixtures = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerFixtures->AddGrowableCol(0);
    FlexGridSizerFixturesLabel = new wxFlexGridSizer(0, 3, 0, 0);
    StaticTextFixtures = new wxStaticText(this, ID_STATICTEXT_Fixtures, _("Fixtures"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fixtures"));
    wxFont StaticTextFixturesFont(12,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,_T("Arial"),wxFONTENCODING_DEFAULT);
    StaticTextFixtures->SetFont(StaticTextFixturesFont);
    FlexGridSizerFixturesLabel->Add(StaticTextFixtures, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 6, 0, 0);
    Button_All = new wxButton(this, ID_BUTTON_All, _("All"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_BUTTON_All"));
    FlexGridSizer2->Add(Button_All, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_None = new wxButton(this, ID_BUTTON_None, _("None"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_BUTTON_None"));
    FlexGridSizer2->Add(Button_None, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Evens = new wxButton(this, ID_BUTTON_Evens, _("Evens"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_BUTTON_Evens"));
    FlexGridSizer2->Add(Button_Evens, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Odds = new wxButton(this, ID_BUTTON_Odds, _("Odds"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_BUTTON_Odds"));
    FlexGridSizer2->Add(Button_Odds, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerFixturesLabel->Add(FlexGridSizer2, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerFixtures->Add(FlexGridSizerFixturesLabel, 1, wxLEFT|wxRIGHT|wxEXPAND, 5);
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
    FlexGridSizerFixtures->Add(FlexGridSizerFixturesSelection, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer_Main->Add(FlexGridSizerFixtures, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 5);
    Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
    PanelPosition = new wxPanel(Notebook1, ID_PANEL_Position, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Position"));
    FlexGridSizerPosition = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerPosition->AddGrowableCol(0);
    FlexGridSizerPositionCanvas = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizerPositionCanvas->AddGrowableCol(0);
    FlexGridSizerPositionCanvas->AddGrowableRow(0);
    FlexGridSizerPosition->Add(FlexGridSizerPositionCanvas, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizerPositionPan = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizerPositionPan->AddGrowableCol(1);
    Label_Pan = new wxStaticText(PanelPosition, ID_STATICTEXT_Pan, _("Pan (deg):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Pan"));
    FlexGridSizerPositionPan->Add(Label_Pan, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHPan = new BulkEditSliderF1(PanelPosition, ID_SLIDER_MHPan, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MHPan"));
    FlexGridSizerPositionPan->Add(Slider_MHPan, 1, wxALL|wxEXPAND, 2);
    ValueCurve_MHPan = new BulkEditValueCurveButton(PanelPosition, ID_VALUECURVE_MHPan, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_MHPan"));
    FlexGridSizerPositionPan->Add(ValueCurve_MHPan, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    TextCtrl_MHPan = new BulkEditTextCtrlF1(PanelPosition, IDD_TEXTCTRL_MHPan, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelPosition,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_MHPan"));
    FlexGridSizerPositionPan->Add(TextCtrl_MHPan, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerPosition->Add(FlexGridSizerPositionPan, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizerPositionTilt = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizerPositionTilt->AddGrowableCol(1);
    Label_Tilt = new wxStaticText(PanelPosition, ID_STATICTEXT_Tilt, _("Tilt (deg): "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Tilt"));
    FlexGridSizerPositionTilt->Add(Label_Tilt, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHTilt = new BulkEditSliderF1(PanelPosition, ID_SLIDER_MHTilt, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MHTilt"));
    FlexGridSizerPositionTilt->Add(Slider_MHTilt, 1, wxALL|wxEXPAND, 2);
    ValueCurve_MHTilt = new BulkEditValueCurveButton(PanelPosition, ID_VALUECURVE_MHTilt, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_MHTilt"));
    FlexGridSizerPositionTilt->Add(ValueCurve_MHTilt, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    TextCtrl_MHTilt = new BulkEditTextCtrlF1(PanelPosition, IDD_TEXTCTRL_MHTilt, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelPosition,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_MHTilt"));
    FlexGridSizerPositionTilt->Add(TextCtrl_MHTilt, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerPosition->Add(FlexGridSizerPositionTilt, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizer_PanOffset = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer_PanOffset->AddGrowableCol(1);
    Label_PanOffset = new wxStaticText(PanelPosition, ID_STATICTEXT_PanOffset, _("Fan Pan:  "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_PanOffset"));
    FlexGridSizer_PanOffset->Add(Label_PanOffset, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHPanOffset = new BulkEditSliderF1(PanelPosition, ID_SLIDER_MHPanOffset, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MHPanOffset"));
    FlexGridSizer_PanOffset->Add(Slider_MHPanOffset, 1, wxALL|wxEXPAND, 2);
    ValueCurve_MHPanOffset = new BulkEditValueCurveButton(PanelPosition, ID_VALUECURVE_MHPanOffset, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_MHPanOffset"));
    FlexGridSizer_PanOffset->Add(ValueCurve_MHPanOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    TextCtrl_MHPanOffset = new BulkEditTextCtrlF1(PanelPosition, IDD_TEXTCTRL_MHPanOffset, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelPosition,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_MHPanOffset"));
    FlexGridSizer_PanOffset->Add(TextCtrl_MHPanOffset, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerPosition->Add(FlexGridSizer_PanOffset, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizer_TiltOffset = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer_TiltOffset->AddGrowableCol(1);
    Label_TiltOffset = new wxStaticText(PanelPosition, ID_STATICTEXT_TiltOffset, _("Fan Tilt:   "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_TiltOffset"));
    FlexGridSizer_TiltOffset->Add(Label_TiltOffset, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHTiltOffset = new BulkEditSliderF1(PanelPosition, ID_SLIDER_MHTiltOffset, 0, -1800, 1800, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MHTiltOffset"));
    FlexGridSizer_TiltOffset->Add(Slider_MHTiltOffset, 1, wxALL|wxEXPAND, 2);
    ValueCurve_MHTiltOffset = new BulkEditValueCurveButton(PanelPosition, ID_VALUECURVE_MHTiltOffset, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_MHTiltOffset"));
    FlexGridSizer_TiltOffset->Add(ValueCurve_MHTiltOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    TextCtrl_MHTiltOffset = new BulkEditTextCtrlF1(PanelPosition, IDD_TEXTCTRL_MHTiltOffset, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelPosition,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_MHTiltOffset"));
    FlexGridSizer_TiltOffset->Add(TextCtrl_MHTiltOffset, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerPosition->Add(FlexGridSizer_TiltOffset, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizer_Groupings = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer_Groupings->AddGrowableCol(1);
    StaticText_Groupings = new wxStaticText(PanelPosition, ID_STATICTEXT_Groupings, _("Groupings:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Groupings"));
    FlexGridSizer_Groupings->Add(StaticText_Groupings, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHGroupings = new BulkEditSlider(PanelPosition, ID_SLIDER_MHGroupings, 1, 1, 6, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MHGroupings"));
    FlexGridSizer_Groupings->Add(Slider_MHGroupings, 1, wxALL|wxEXPAND, 2);
    ValueCurve_MHGroupings = new BulkEditValueCurveButton(PanelPosition, ID_VALUECURVE_MHGroupings, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_MHGroupings"));
    FlexGridSizer_Groupings->Add(ValueCurve_MHGroupings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    TextCtrl_MHGroupings = new BulkEditTextCtrl(PanelPosition, IDD_TEXTCTRL_MHGroupings, _("1"), wxDefaultPosition, wxDLG_UNIT(PanelPosition,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_MHGroupings"));
    FlexGridSizer_Groupings->Add(TextCtrl_MHGroupings, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerPosition->Add(FlexGridSizer_Groupings, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizerCycles = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizerCycles->AddGrowableCol(1);
    StaticText_MHCycles = new wxStaticText(PanelPosition, ID_STATICTEXT_MHCycles, _("Cycles:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MHCycles"));
    FlexGridSizerCycles->Add(StaticText_MHCycles, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHCycles = new BulkEditSliderF1(PanelPosition, ID_SLIDER_MHCycles, 1, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MHCycles"));
    FlexGridSizerCycles->Add(Slider_MHCycles, 1, wxALL|wxEXPAND, 2);
    FlexGridSizerCycles->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    TextCtrl_MHCycles = new BulkEditTextCtrlF1(PanelPosition, IDD_TEXTCTRL_MHCycles, _("1"), wxDefaultPosition, wxDLG_UNIT(PanelPosition,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_MHCycles"));
    FlexGridSizerCycles->Add(TextCtrl_MHCycles, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerPosition->Add(FlexGridSizerCycles, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    ButtonSavePreset = new wxButton(PanelPosition, ID_BUTTON_SavePreset, _("Save Preset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SavePreset"));
    FlexGridSizerPosition->Add(ButtonSavePreset, 1, wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizerPresets = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizerPosition->Add(FlexGridSizerPresets, 1, wxALL|wxEXPAND, 5);
    PanelPosition->SetSizer(FlexGridSizerPosition);
    FlexGridSizerPosition->Fit(PanelPosition);
    FlexGridSizerPosition->SetSizeHints(PanelPosition);
    PanelDimmer = new wxPanel(Notebook1, ID_PANEL_Dimmer, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Dimmer"));
    FlexGridSizerDimmer = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerDimmer->AddGrowableCol(0);
    FlexGridSizerDimmerCanvas = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizerDimmerCanvas->AddGrowableCol(0);
    FlexGridSizerDimmerCanvas->AddGrowableRow(0);
    FlexGridSizerDimmer->Add(FlexGridSizerDimmerCanvas, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
    ButtonDimmerOn = new wxButton(PanelDimmer, ID_BUTTON_DimmerOn, _("On"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DimmerOn"));
    FlexGridSizer3->Add(ButtonDimmerOn, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonDimmerOff = new wxButton(PanelDimmer, ID_BUTTON_DimmerOff, _("Off"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DimmerOff"));
    FlexGridSizer3->Add(ButtonDimmerOff, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerDimmer->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSaveDimmerPreset = new wxButton(PanelDimmer, ID_BUTTON_SaveDimmerPreset, _("Save Preset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SaveDimmerPreset"));
    FlexGridSizerDimmer->Add(ButtonSaveDimmerPreset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerDimmerPresets = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizerDimmer->Add(FlexGridSizerDimmerPresets, 1, wxALL|wxEXPAND, 5);
    PanelDimmer->SetSizer(FlexGridSizerDimmer);
    FlexGridSizerDimmer->Fit(PanelDimmer);
    FlexGridSizerDimmer->SetSizeHints(PanelDimmer);
    PanelPathing = new wxPanel(Notebook1, ID_PANEL_Pathing, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Pathing"));
    FlexGridSizerPathing = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerPathing->AddGrowableCol(0);
    FlexGridSizerPathCanvas = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizerPathCanvas->AddGrowableCol(0);
    FlexGridSizerPathCanvas->AddGrowableRow(0);
    FlexGridSizerPathing->Add(FlexGridSizerPathCanvas, 1, wxALL|wxEXPAND, 0);
    FlexGridSizer_PathButtons = new wxFlexGridSizer(0, 4, 0, 0);
    Button_MHPathContinue = new wxButton(PanelPathing, ID_BUTTON_MHPathContinue, _("Continue"), wxDefaultPosition, wxDLG_UNIT(PanelPathing,wxSize(36,-1)), 0, wxDefaultValidator, _T("ID_BUTTON_MHPathContinue"));
    FlexGridSizer_PathButtons->Add(Button_MHPathContinue, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_MHPathClear = new wxButton(PanelPathing, ID_BUTTON_MHPathClear, _("Clear"), wxDefaultPosition, wxDLG_UNIT(PanelPathing,wxSize(36,-1)), 0, wxDefaultValidator, _T("ID_BUTTON_MHPathClear"));
    FlexGridSizer_PathButtons->Add(Button_MHPathClear, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_MHPathClose = new wxButton(PanelPathing, ID_BUTTON_MHPathClose, _("Close"), wxDefaultPosition, wxDLG_UNIT(PanelPathing,wxSize(36,-1)), 0, wxDefaultValidator, _T("ID_BUTTON_MHPathClose"));
    FlexGridSizer_PathButtons->Add(Button_MHPathClose, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MHPathDef = new wxTextCtrl(PanelPathing, ID_TEXTCTRL_MHPathDef, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MHPathDef"));
    TextCtrl_MHPathDef->Hide();
    FlexGridSizer_PathButtons->Add(TextCtrl_MHPathDef, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerPathing->Add(FlexGridSizer_PathButtons, 1, wxALL|wxEXPAND, 0);
    FlexGridSizer4 = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer4->AddGrowableCol(1);
    Label_PathScale = new wxStaticText(PanelPathing, ID_STATICTEXT_PathScale, _("Scale:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_PathScale"));
    FlexGridSizer4->Add(Label_PathScale, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHPathScale = new BulkEditSliderF1(PanelPathing, ID_SLIDER_MHPathScale, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MHPathScale"));
    FlexGridSizer4->Add(Slider_MHPathScale, 1, wxALL|wxEXPAND, 2);
    ValueCurve_MHPathScale = new BulkEditValueCurveButton(PanelPathing, ID_VALUECURVE_MHPathScale, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_MHPathScale"));
    FlexGridSizer4->Add(ValueCurve_MHPathScale, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    TextCtrl_MHPathScale = new BulkEditTextCtrlF1(PanelPathing, IDD_TEXTCTRL_MHPathScale, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelPathing,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_MHPathScale"));
    FlexGridSizer4->Add(TextCtrl_MHPathScale, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerPathing->Add(FlexGridSizer4, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizer_PathControls = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer_PathControls->AddGrowableCol(1);
    Label_TimeOffset = new wxStaticText(PanelPathing, ID_STATICTEXT_TimeOffset, _("Time Offset:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_TimeOffset"));
    FlexGridSizer_PathControls->Add(Label_TimeOffset, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_MHTimeOffset = new BulkEditSliderF1(PanelPathing, ID_SLIDER_MHTimeOffset, 0, -1000, 1000, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MHTimeOffset"));
    FlexGridSizer_PathControls->Add(Slider_MHTimeOffset, 1, wxALL|wxEXPAND, 2);
    ValueCurve_MHTimeOffset = new BulkEditValueCurveButton(PanelPathing, ID_VALUECURVE_MHTimeOffset, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_MHTimeOffset"));
    FlexGridSizer_PathControls->Add(ValueCurve_MHTimeOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    TextCtrl_MHTimeOffset = new BulkEditTextCtrlF1(PanelPathing, IDD_TEXTCTRL_MHTimeOffset, _("0"), wxDefaultPosition, wxDLG_UNIT(PanelPathing,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_MHTimeOffset"));
    FlexGridSizer_PathControls->Add(TextCtrl_MHTimeOffset, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerPathing->Add(FlexGridSizer_PathControls, 1, wxALL|wxEXPAND, 5);
    FlexGridSizerIgnoreBoxes = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox_MHIgnorePan = new BulkEditCheckBox(PanelPathing, ID_CHECKBOX_MHIgnorePan, _("Ignore Pan"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MHIgnorePan"));
    CheckBox_MHIgnorePan->SetValue(false);
    FlexGridSizerIgnoreBoxes->Add(CheckBox_MHIgnorePan, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MHIgnoreTilt = new BulkEditCheckBox(PanelPathing, ID_CHECKBOX_MHIgnoreTilt, _("Ignore Tilt"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MHIgnoreTilt"));
    CheckBox_MHIgnoreTilt->SetValue(false);
    FlexGridSizerIgnoreBoxes->Add(CheckBox_MHIgnoreTilt, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerPathing->Add(FlexGridSizerIgnoreBoxes, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    ButtonSavePathPreset = new wxButton(PanelPathing, ID_BUTTON_SavePathPreset, _("Save Preset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SavePathPreset"));
    FlexGridSizerPathing->Add(ButtonSavePathPreset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerPathPresets = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizerPathing->Add(FlexGridSizerPathPresets, 1, wxALL|wxEXPAND, 5);
    PanelPathing->SetSizer(FlexGridSizerPathing);
    FlexGridSizerPathing->Fit(PanelPathing);
    FlexGridSizerPathing->SetSizeHints(PanelPathing);
    PanelControl = new wxPanel(Notebook1, ID_PANEL_Control, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Control"));
    FlexGridSizerControl = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizerControl->AddGrowableCol(0);
    FlexGridSizerControl->AddGrowableRow(0);
    Notebook2 = new wxNotebook(PanelControl, ID_NOTEBOOK2, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK2"));
    PanelColor = new wxPanel(Notebook2, ID_PANEL_Color, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Color"));
    FlexGridSizerColorMain = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizerColorMain->AddGrowableCol(0);
    FlexGridSizerColor = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizerColor->AddGrowableCol(0);
    FlexGridSizerColor->AddGrowableRow(0);
    FlexGridSizerColorMain->Add(FlexGridSizerColor, 1, wxALL|wxEXPAND, 0);
    FlexGridSizerColorSliders = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizerColorSliders->AddGrowableCol(0);
    FlexGridSizerColorMain->Add(FlexGridSizerColorSliders, 1, wxALL|wxEXPAND, 0);
    PanelColor->SetSizer(FlexGridSizerColorMain);
    FlexGridSizerColorMain->Fit(PanelColor);
    FlexGridSizerColorMain->SetSizeHints(PanelColor);
    PanelColorWheel = new wxPanel(Notebook2, ID_PANEL_ColorWheel, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_ColorWheel"));
    PanelColorWheel->Hide();
    FlexGridSizerColorWheelMain = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizerColorWheelMain->AddGrowableCol(0);
    FlexGridSizerColorWheel = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizerColorWheel->AddGrowableCol(0);
    FlexGridSizerColorWheel->AddGrowableRow(0);
    FlexGridSizerColorWheelMain->Add(FlexGridSizerColorWheel, 1, wxALL|wxEXPAND, 0);
    FlexGridSizerColorWheelSliders = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizerColorWheelSliders->AddGrowableCol(0);
    CheckBoxAutoShutter = new wxCheckBox(PanelColorWheel, ID_CHECKBOX_AUTO_SHUTTER, _("Auto Shutter"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_AUTO_SHUTTER"));
    CheckBoxAutoShutter->SetValue(false);
    FlexGridSizerColorWheelSliders->Add(CheckBoxAutoShutter, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerColorWheelMain->Add(FlexGridSizerColorWheelSliders, 1, wxALL|wxEXPAND, 0);
    PanelColorWheel->SetSizer(FlexGridSizerColorWheelMain);
    FlexGridSizerColorWheelMain->Fit(PanelColorWheel);
    FlexGridSizerColorWheelMain->SetSizeHints(PanelColorWheel);
    Notebook2->AddPage(PanelColor, _("Color"), false);
    Notebook2->AddPage(PanelColorWheel, _("ColorWheel"), false);
    FlexGridSizerControl->Add(Notebook2, 1, wxALL|wxEXPAND, 5);
    PanelControl->SetSizer(FlexGridSizerControl);
    FlexGridSizerControl->Fit(PanelControl);
    FlexGridSizerControl->SetSizeHints(PanelControl);
    PanelStatus = new wxPanel(Notebook1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    TextCtrl_Status = new wxTextCtrl(PanelStatus, IDD_TEXTCTRL_Status, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("IDD_TEXTCTRL_Status"));
    FlexGridSizer1->Add(TextCtrl_Status, 1, wxALL|wxEXPAND, 5);
    Button_ResetToDefault = new wxButton(PanelStatus, ID_BUTTON_ResetToDefault, _("Reset to Default"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ResetToDefault"));
    FlexGridSizer1->Add(Button_ResetToDefault, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PanelStatus->SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(PanelStatus);
    FlexGridSizer1->SetSizeHints(PanelStatus);
    Notebook1->AddPage(PanelPosition, _("Position"), false);
    Notebook1->AddPage(PanelDimmer, _("Dimmer"), false);
    Notebook1->AddPage(PanelPathing, _("Pathing"), false);
    Notebook1->AddPage(PanelControl, _("Control"), false);
    Notebook1->AddPage(PanelStatus, _("Status"), false);
    FlexGridSizer_Main->Add(Notebook1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer_Positions = new wxFlexGridSizer(0, 8, 0, 0);
    TextCtrl_MH1_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH1_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH1_Settings"));
    TextCtrl_MH1_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH1_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH2_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH2_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH2_Settings"));
    TextCtrl_MH2_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH2_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH3_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH3_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH3_Settings"));
    TextCtrl_MH3_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH3_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH4_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH4_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH4_Settings"));
    TextCtrl_MH4_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH4_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH5_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH5_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH5_Settings"));
    TextCtrl_MH5_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH5_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH6_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH6_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH6_Settings"));
    TextCtrl_MH6_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH6_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_MH7_Settings = new wxTextCtrl(this, ID_TEXTCTRL_MH7_Settings, wxEmptyString, wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MH7_Settings"));
    TextCtrl_MH7_Settings->Hide();
    FlexGridSizer_Positions->Add(TextCtrl_MH7_Settings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    Connect(IDD_CHECKBOX_MH1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnCheckBox_MHClick);
    Connect(IDD_CHECKBOX_MH2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnCheckBox_MHClick);
    Connect(IDD_CHECKBOX_MH3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnCheckBox_MHClick);
    Connect(IDD_CHECKBOX_MH4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnCheckBox_MHClick);
    Connect(IDD_CHECKBOX_MH5,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnCheckBox_MHClick);
    Connect(IDD_CHECKBOX_MH6,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnCheckBox_MHClick);
    Connect(IDD_CHECKBOX_MH7,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnCheckBox_MHClick);
    Connect(IDD_CHECKBOX_MH8,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnCheckBox_MHClick);
    Connect(ID_VALUECURVE_MHTiltOffset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnValueCurve_MHTiltOffsetClick);
    Connect(ID_BUTTON_SavePreset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButtonSavePresetClick);
    Connect(ID_BUTTON_DimmerOn,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButtonDimmerOnClick);
    Connect(ID_BUTTON_DimmerOff,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButtonDimmerOffClick);
    Connect(ID_BUTTON_SaveDimmerPreset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButtonSaveDimmerPresetClick);
    Connect(ID_BUTTON_MHPathContinue,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButton_MHPathContinueClick);
    Connect(ID_BUTTON_MHPathClear,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButton_MHPathClearClick);
    Connect(ID_BUTTON_MHPathClose,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButton_MHPathCloseClick);
    Connect(ID_CHECKBOX_MHIgnorePan,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnCheckBox_MHIgnorePanClick);
    Connect(ID_CHECKBOX_MHIgnoreTilt,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnCheckBox_MHIgnoreTiltClick);
    Connect(ID_BUTTON_SavePathPreset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButtonSavePathPresetClick);
    Connect(ID_CHECKBOX_AUTO_SHUTTER,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnCheckBoxAutoShutterClick);
    Connect(ID_BUTTON_ResetToDefault,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnButton_ResetToDefaultClick);
    Connect(ID_NOTEBOOK1,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&MovingHeadPanel::OnNotebook1PageChanged);
    //*)

    SetName("ID_PANEL_MOVINGHEAD");

    // canvas
    m_movingHeadCanvasPanel = new MovingHeadCanvasPanel(this, PanelPosition, wxID_ANY, wxDefaultPosition, wxSize(250, 250));
    FlexGridSizerPositionCanvas->Add(m_movingHeadCanvasPanel, 0, wxALL | wxEXPAND);
    m_movingHeadDimmerPanel = new MovingHeadDimmerPanel(this, PanelDimmer, wxID_ANY, wxDefaultPosition, wxSize(250, 125));
    FlexGridSizerDimmerCanvas->Add(m_movingHeadDimmerPanel, 0, wxALL | wxEXPAND);
    m_sketchCanvasPanel = new SketchCanvasPanel(this, PanelPathing, wxID_ANY, wxDefaultPosition, wxSize(250, 250));
    FlexGridSizerPathCanvas->Add(m_sketchCanvasPanel, 0, wxALL | wxEXPAND);

    m_sketchCanvasPanel->UpdatePathState(SketchCanvasPathState::DefineStartPoint);
    m_sketchCanvasPanel->DrawGrid(true);

    m_rgbColorPanel = new MHRgbPickerPanel(this, PanelColor, wxID_ANY, wxDefaultPosition, wxSize(250, 250));
    FlexGridSizerColor->Add(m_rgbColorPanel, 0, wxALL | wxEXPAND);
    Notebook2->AddPage(PanelColor, _("Color"), true);
    PanelColor->Show();
    // Delete Colorwheel page
    while(Notebook2->GetPageCount()>1) {
        Notebook2->RemovePage(1);
    }
    UpdateColorPanel();

    Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(MovingHeadPanel::OnCharHook), (wxObject*) nullptr, this);
    Connect(wxEVT_SIZE,(wxObjectEventFunction)&MovingHeadPanel::OnResize);

    Connect(ID_VALUECURVE_MHPan,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_MHTilt,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_MHPanOffset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_MHTiltOffset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_MHGroupings,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_MHTimeOffset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_MHPathScale,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MovingHeadPanel::OnVCButtonClick);

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&MovingHeadPanel::OnVCChanged, nullptr, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&MovingHeadPanel::OnValidateWindow, nullptr, this);

    Connect(wxEVT_COMMAND_SLIDER_UPDATED, (wxObjectEventFunction)&MovingHeadPanel::OnSliderUpdated);
    Connect(wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&MovingHeadPanel::OnTextCtrlUpdated);

    ValueCurve_MHPan->GetValue()->SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    ValueCurve_MHPan->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);
    ValueCurve_MHTilt->GetValue()->SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    ValueCurve_MHTilt->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);
    ValueCurve_MHPanOffset->GetValue()->SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    ValueCurve_MHPanOffset->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);
    ValueCurve_MHTiltOffset->GetValue()->SetLimits(MOVING_HEAD_MIN, MOVING_HEAD_MAX);
    ValueCurve_MHTiltOffset->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);
    ValueCurve_MHGroupings->GetValue()->SetLimits(MOVING_HEAD_GROUP_MIN, MOVING_HEAD_GROUP_MAX);
    ValueCurve_MHTimeOffset->GetValue()->SetLimits(MOVING_HEAD_TIME_MIN, MOVING_HEAD_TIME_MAX);
    ValueCurve_MHTimeOffset->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);
    ValueCurve_MHTimeOffset->GetValue()->SetLimits(MOVING_HEAD_SCALE_MIN, MOVING_HEAD_SCALE_MAX);
    ValueCurve_MHTimeOffset->GetValue()->SetDivisor(MOVING_HEAD_DIVISOR);

    ValidateWindow();
}

MovingHeadPanel::~MovingHeadPanel()
{
	//(*Destroy(MovingHeadPanel)
	//*)
}

void MovingHeadPanel::SetEffectTimeRange(int startTimeMs, int endTimeMs)
{
    startTimeMs_ = startTimeMs;
    endTimeMs_ = endTimeMs;
}

void MovingHeadPanel::OnResize(wxSizeEvent& event)
{
    // If this window is not fully initialized, dismiss this event
    if ( !IsShownOnScreen() )
        return;

    wxSize old_sz = m_sketchCanvasPanel->GetSize();
    if( old_sz.GetWidth() != old_sz.GetHeight() ) {
        if( old_sz.GetWidth() > 270 ) {
            wxSize new_size = old_sz;
            new_size.SetHeight(new_size.GetWidth());
            m_sketchCanvasPanel->SetMinSize(new_size);
        }
    }
    FlexGridSizer_Main->SetSizeHints(this);
    Layout();
    event.Skip();
}

void MovingHeadPanel::ProcessPresetDir(wxDir& directory, bool subdirs)
{
    
    LOG_INFO("MovingHeadPanel Scanning directory for *.xmh files: %s.", (const char *)directory.GetNameWithSep().c_str());

    auto existing = FlexGridSizerPresets->GetChildren();
    auto existing_path = FlexGridSizerPathPresets->GetChildren();

    wxArrayString files;
    GetAllFilesInDir(directory.GetNameWithSep(), files, "*.xmh");
    int count = 0;
    for (auto &filename : files) {
        wxFileName fn(filename);
        count++;
        bool found = false;
        for (const auto& it : existing) {
            if (it->GetWindow()->GetLabel() == fn.GetFullPath()) {
                // already there
                found = true;
                break;
            }
        }
        if (!found) {
            for (const auto& it : existing_path) {
                if (it->GetWindow()->GetLabel() == fn.GetFullPath()) {
                    // already there
                    found = true;
                    break;
                }
            }
            if(!found) {
                LoadMHPreset(fn);
            }
        }
    }
    LOG_INFO("    Found %d.", count);

    if (subdirs) {
        wxString filename;
        bool cont = directory.GetFirst(&filename, "*", wxDIR_DIRS);
        while (cont) {
            wxDir dir(directory.GetNameWithSep() + filename);
            ProcessPresetDir(dir, subdirs);
            cont = directory.GetNext(&filename);
        }
    }
}

void MovingHeadPanel::LoadMHPreset(const wxFileName& fn)
{
    LoadMHPreset(fn.GetFullPath().ToStdString());
}

void MovingHeadPanel::LoadMHPreset(const std::string& fn)
{
    if (!FileExists(fn)) {
        DisplayError("Failure loading MH preset file " + fn + ".");
        return;
    }
    wxXmlDocument doc(fn);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "mhpreset")
        {
            wxArrayString heads;
            for( int i = 1; i <=8; ++i ) {
                wxString label = wxString::Format("data%i", i);
                wxString data = root->GetAttribute(label);
                heads.Add(data);
            }
            wxString iid = wxString::Format("ID_BITMAPBUTTON_%d", (int)FlexGridSizerPresets->GetItemCount());
            long id = wxNewId();
            MHPresetBitmapButton* presetBtn = new MHPresetBitmapButton(PanelPosition, id, wxNullBitmap, wxDefaultPosition, wxSize(96, 48), wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, iid);
            presetBtn->SetLabel(fn);
            presetBtn->SetToolTip(fn);
            presets.push_back( presetBtn );
            presetBtn->SetPreset(heads);
            FlexGridSizerPresets->Add(presetBtn, 1, wxALL, 5);
            Connect(id, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&MovingHeadPanel::OnButtonPresetClick);
        }
        else if (root->GetName() == "mhpathpreset")
        {
            std::string data = root->GetAttribute("data");
            wxString iid = wxString::Format("ID_BITMAPBUTTON_%d", (int)FlexGridSizerPathPresets->GetItemCount());
            long id = wxNewId();
            MHPathPresetBitmapButton* presetBtn = new MHPathPresetBitmapButton(PanelPathing, id, wxNullBitmap, wxDefaultPosition, wxSize(48, 48), wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, iid);
            presetBtn->SetLabel(fn);
            presetBtn->SetToolTip(fn);
            path_presets.push_back( presetBtn );
            presetBtn->SetPreset(data);
            FlexGridSizerPathPresets->Add(presetBtn, 1, wxALL, 5);
            Connect(id, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&MovingHeadPanel::OnButtonPathPresetClick);
        }
        else if (root->GetName() == "mhdimmerpreset")
        {
            std::string data = root->GetAttribute("data");
            wxString iid = wxString::Format("ID_BITMAPBUTTON_%d", (int)FlexGridSizerDimmerPresets->GetItemCount());
            long id = wxNewId();
            MHDimmerPresetBitmapButton* presetBtn = new MHDimmerPresetBitmapButton(PanelDimmer, id, wxNullBitmap, wxDefaultPosition, wxSize(48, 48), wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, iid);
            presetBtn->SetLabel(fn);
            presetBtn->SetToolTip(fn);
            dimmer_presets.push_back( presetBtn );
            presetBtn->SetPreset(data);
            FlexGridSizerDimmerPresets->Add(presetBtn, 1, wxALL, 5);
            Connect(id, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&MovingHeadPanel::OnButtonDimmerPresetClick);
        }
       else
        {
            DisplayError("Failure loading MH preset file " + fn + ".");
        }
    }
    else
    {
        DisplayError("Failure loading MH preset file " + fn + ".");
    }
}

void MovingHeadPanel::PopulatePresets()
{
    

    wxDir dir;
    if (wxDir::Exists(xLightsFrame::CurrentDir))
    {
        dir.Open(xLightsFrame::CurrentDir);
        ProcessPresetDir(dir, false);
    }

    wxString d = GetMHPresetFolder(xLightsFrame::CurrentDir.ToStdString());
    if (wxDir::Exists(d))
    {
        dir.Open(d);
        ProcessPresetDir(dir, true);
    }
    else
    {
        LOG_INFO("Directory for *.xmh files not found: %s.", (const char *)d.c_str());
    }

    wxStandardPaths stdp = wxStandardPaths::Get();


#ifndef __WXMSW__
    d = wxStandardPaths::Get().GetResourcesDir() + "/mhpresets";
#else
    d = wxFileName(stdp.GetExecutablePath()).GetPath() + "/mhpresets";
#endif
    if (wxDir::Exists(d))
    {
        dir.Open(d);
        ProcessPresetDir(dir, true);
    }
    else
    {
        LOG_INFO("Directory for *.xmh files not found: %s.", (const char *)d.c_str());
    }
}

std::string MovingHeadPanel::GetMHPresetFolder(const std::string& showFolder)
{
    if (showFolder == "") return "";

    std::string mhf = showFolder + "/mhpresets";
    if (!wxDir::Exists(mhf))
    {
        wxMkdir(mhf);
        if (!wxDir::Exists(mhf))
        {
            return "";
        }
    }
    return mhf;
}

void MovingHeadPanel::OnButtonSavePresetClick(wxCommandEvent& event)
{
    wxArrayString heads;
    for( int i = 1; i <= 8; ++i ) {
        std::string mh_settings = xlEMPTY_STRING;
        wxString textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d_Settings", i);
        wxTextCtrl* mh_textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
        if( mh_textbox != nullptr ) {
            mh_settings = mh_textbox->GetValue();
            heads.Add( mh_settings );
        }
    }
    SavePreset( heads );
    Layout();
    Refresh();
}

void MovingHeadPanel::OnButtonSavePathPresetClick(wxCommandEvent& event)
{
    wxArrayString path_def;
    path_def.Add(m_sketchDef);
    SavePreset( path_def, true );
    Layout();
    Refresh();
}

void MovingHeadPanel::OnButtonSaveDimmerPresetClick(wxCommandEvent& event)
{
    if( m_movingHeadDimmerPanel != nullptr ) {
        std::string dimmer_text = m_movingHeadDimmerPanel->GetDimmerCommands();
        wxArrayString dimmer_def;
        dimmer_def.Add(dimmer_text);
        SavePreset( dimmer_def, false, true );
        Layout();
        Refresh();
    }
}

void MovingHeadPanel::SavePreset(const wxArrayString& preset, bool is_path, bool is_dimmer)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    std::string mhf = GetMHPresetFolder(xLightsFrame::CurrentDir.ToStdString());
    wxString filename = wxFileSelector(_("MH Preset file to save"), mhf, wxEmptyString, wxEmptyString, "MH Preset files (*.xmh)|*.xmh", wxFD_SAVE);
    if (filename.IsEmpty()) return;

    wxFile f(filename);

    
    LOG_INFO("Saving to xcc file %s.", (const char *)filename.c_str());

    bool replace_existing {false};
    if( f.Exists(filename) ) {
        replace_existing = true;
    }

    if (!f.Create(filename, true) || !f.IsOpened())
    {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
        return;
    }

    wxString v = xlights_version_string;
    if( is_path ) {
        f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<mhpathpreset \n");
        f.Write(wxString::Format("data=\"%s\" ", (const char *)preset[0].c_str()));
        f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
        f.Write(" >\n");
        f.Write("</mhpathpreset>");
        f.Close();
    } else if( is_dimmer ) {
            f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<mhdimmerpreset \n");
            f.Write(wxString::Format("data=\"%s\" ", (const char *)preset[0].c_str()));
            f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
            f.Write(" >\n");
            f.Write("</mhdimmerpreset>");
            f.Close();
    } else {
        f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<mhpreset \n");
        for( int i = 1; i <= 8; ++i ) {
            f.Write(wxString::Format("data%i=\"%s\" ", i, (const char *)preset[i-1].c_str()));
        }
        f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
        f.Write(" >\n");
        f.Write("</mhpreset>");
        f.Close();
    }

    if( replace_existing ) {
        wxFileName fn(filename);
        if( is_path ) {
            for (auto preset_ : path_presets) {
                if( fn.GetFullPath() == preset_->GetLabel() ) {
                    preset_->SetPreset( preset[0] );
                    break;
                }
            }
        } else {
            for (auto preset_ : presets) {
                if( fn.GetFullPath() == preset_->GetLabel() ) {
                    preset_->SetPreset( preset );
                    break;
                }
            }
        }
    }
    else {
        PopulatePresets();
    }
}

void MovingHeadPanel::OnButtonPresetClick(wxCommandEvent& event)
{
    recall = true;
    MHPresetBitmapButton* btn = (MHPresetBitmapButton*)event.GetEventObject();
    wxArrayString settings = btn->GetPreset();
    for( int i = 1; i <= 8; ++i ) {
        wxString textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d_Settings", i);
        wxTextCtrl* mh_textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
        mh_textbox->SetValue(settings[i-1]);
    }
    recall = false;
    FireChangeEvent();
}

void MovingHeadPanel::OnButtonPathPresetClick(wxCommandEvent& event)
{
    recall = true;
    MHPathPresetBitmapButton* btn = (MHPathPresetBitmapButton*)event.GetEventObject();
    m_sketchDef = btn->GetPreset();
    m_sketch = SketchEffectSketch::SketchFromString(m_sketchDef);
    TextCtrl_MHPathDef->SetValue(m_sketchDef);
    m_sketchCanvasPanel->ResetHandlesState();
    if( m_sketch.paths().size() > 0 ) {
        selected_path = 0;
        m_sketchCanvasPanel->UpdateHandlesForPath(0);
    } else {
        selected_path = -1;
    }
    recall = false;
    UpdatePathSettings();
    FireChangeEvent();
}

void MovingHeadPanel::OnButtonDimmerPresetClick(wxCommandEvent& event)
{
    recall = true;
    MHDimmerPresetBitmapButton* btn = (MHDimmerPresetBitmapButton*)event.GetEventObject();
    std::string dimmer_def = btn->GetPreset();
    m_movingHeadDimmerPanel->SetDimmerCommands(dimmer_def);
    recall = false;
    UpdateDimmerSettings();
    FireChangeEvent();
}

void MovingHeadPanel::OnNotebook1PageChanged(wxNotebookEvent& event)
{
    if (Notebook1->GetPageText(Notebook1->GetSelection()) == "Status") {
        UpdateStatusPanel();
    }
}

void MovingHeadPanel::ValidateWindow()
{
    // Lets fix moving head fixture assignments
    auto models = GetActiveModels();
    if (models.empty()) return;
    bool single_model = models.size() == 1;

    // if single model make sure the effect setting is on correct head...if not move it
    auto model = models.front();
    if (single_model) {
        if( model->GetDisplayAs() == "DmxMovingHeadAdv" ||
            model->GetDisplayAs() == "DmxMovingHead") {
            auto mh = dynamic_cast<const DmxMovingHeadComm*>(model);
            int fixture = mh->GetFixtureVal();
            wxString fixture_textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d_Settings", fixture);
            wxTextCtrl* fixture_mh_textbox = (wxTextCtrl*)(this->FindWindowByName(fixture_textbox_ctrl));
            if( fixture_mh_textbox != nullptr ) {
                std::string mh_settings = fixture_mh_textbox->GetValue();
                if( mh_settings == xlEMPTY_STRING ) {
                    // need to search for settings
                    for( int i = 1; i <= 8; ++i ) {
                        if (i == fixture) {
                            continue;
                        }
                        wxString textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d_Settings", i);
                        wxTextCtrl* mh_textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
                        if( mh_textbox != nullptr ) {
                            mh_settings = mh_textbox->GetValue();
                            if( mh_settings != xlEMPTY_STRING ) {
                                for( int j = 1; j <= 8; ++j ) {
                                    textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d_Settings", j);
                                    mh_textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
                                    if( mh_textbox != nullptr ) {
                                        if (j == fixture) {
                                            wxArrayString all_cmds = wxSplit(mh_settings, ';');
                                            wxArrayString new_cmds;
                                            for (size_t k = 0; k < all_cmds.size(); ++k )
                                            {
                                                std::string cmd = all_cmds[k];
                                                if( cmd == xlEMPTY_STRING ) continue;
                                                int pos = cmd.find(":");
                                                std::string cmd_type = cmd.substr(0, pos);
                                                if( cmd_type == "Heads") {
                                                    std::string headset = "Heads: ";
                                                    headset += wxString::Format("%d", fixture);
                                                    new_cmds.push_back(headset);
                                                } else {
                                                    new_cmds.push_back(all_cmds[k]);
                                                }
                                            }
                                            std::string final_cmd = wxJoin(new_cmds, ';');
                                            fixture_mh_textbox->SetValue(final_cmd);
                                        } else {
                                            mh_textbox->SetValue(xlEMPTY_STRING);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // updates the status panel if its already active and a new effect is selected
    UpdateStatusPanel();

    // Set current timing track in Dimmer window
    const ModelManager& mgr = model->GetModelManager();
    xLightsFrame* xlights = mgr.GetXLightsFrame();
    if (xlights == nullptr) return;
    Element* active_timing = xlights->GetMainSequencer()->PanelEffectGrid->GetActiveTimingElement();
    if (active_timing == nullptr) return;
    m_movingHeadDimmerPanel->SetEffectTimeRange(startTimeMs_, endTimeMs_);
    m_movingHeadDimmerPanel->SetTimingTrack(active_timing);
}

static std::list<std::string> vcurves_pos = {"ID_VALUECURVE_MHPan", "ID_VALUECURVE_MHTilt", "ID_VALUECURVE_MHPanOffset",
                                             "ID_VALUECURVE_MHTiltOffset", "ID_VALUECURVE_MHGroupings" };
static std::list<std::string> vcurves_path = {"ID_VALUECURVE_MHPathScale", "ID_VALUECURVE_MHTimeOffset" };

void MovingHeadPanel::OnVCChanged(wxCommandEvent& event)
{
    EffectPanelUtils::OnVCChanged(event);

    BulkEditValueCurveButton* vc_btn = reinterpret_cast<BulkEditValueCurveButton*>(event.GetEventObject());

    if( vc_btn != nullptr ) {
        std::string vc_id = vc_btn->GetValue()->GetId();
        bool pos_found = (std::find(vcurves_pos.begin(), vcurves_pos.end(), vc_id) != vcurves_pos.end());
        if( pos_found ) {
            UpdateMHSettings();
        } else {
            bool path_found = (std::find(vcurves_path.begin(), vcurves_path.end(), vc_id) != vcurves_path.end());
            if( path_found ) {
                UpdatePathSettings();
            }
        }
    }

    FireChangeEvent();
}

void MovingHeadPanel::OnSliderUpdated(wxCommandEvent& event)
{
    int event_id = event.GetId();
    if (event_id == ID_SLIDER_MHPan ||
        event_id == ID_SLIDER_MHTilt ||
        event_id == ID_SLIDER_MHPanOffset ||
        event_id == ID_SLIDER_MHTiltOffset ||
        event_id == ID_SLIDER_MHGroupings ||
        event_id == ID_SLIDER_MHCycles) {
        UpdateMHSettings();
    }
    else if (event_id == ID_SLIDER_MHPathScale ||
             event_id == ID_SLIDER_MHTimeOffset) {
        UpdatePathSettings();
    }
    if (event_id == ID_SLIDER_MHPan ||
        event_id == ID_SLIDER_MHTilt )
    {
        float pan = 0.0f;
        float tilt = 0.0f;
        GetPosition("Pan", pan);
        GetPosition("Tilt", tilt);
        UpdatePositionCanvas(pan, tilt);
    }
}

void MovingHeadPanel::OnTextCtrlUpdated(wxCommandEvent& event)
{
    int event_id = event.GetId();

    if (event_id == ID_TEXTCTRL_MHPathDef) {
        if( m_sketchDef != TextCtrl_MHPathDef->GetValue() ) {
            m_sketchDef = TextCtrl_MHPathDef->GetValue();
            m_sketch = SketchEffectSketch::SketchFromString(m_sketchDef);
            if( m_sketchDef == xlEMPTY_STRING ) {
                m_sketchCanvasPanel->ResetHandlesState();
                m_sketchCanvasPanel->UpdatePathState(SketchCanvasPathState::DefineStartPoint);
                selected_path = -1;
            } else {
                if( m_sketch.paths().size() > 0 ) {
                    selected_path = 0;
                    m_sketchCanvasPanel->UpdateHandlesForPath(0);
                } else {
                    selected_path = -1;
                }
            }
        }
    }
    else if (event_id == IDD_TEXTCTRL_MHPan ||
             event_id == IDD_TEXTCTRL_MHTilt ||
             event_id == IDD_TEXTCTRL_MHPanOffset ||
             event_id == IDD_TEXTCTRL_MHTiltOffset ||
             event_id == IDD_TEXTCTRL_MHGroupings ||
             event_id == IDD_TEXTCTRL_MHCycles) {
        UpdateMHSettings();
    }
    else if (event_id == IDD_TEXTCTRL_MHTimeOffset ||
             event_id == IDD_TEXTCTRL_MHPathScale) {
        UpdatePathSettings();
    }
    if( event_id == IDD_TEXTCTRL_MHPan ) {
        float pan = 0.0f;
        float tilt = 0.0f;
        if( GetPosition("Pan", pan) ) {
            GetPosition("Tilt", tilt);
            UpdatePositionCanvas(pan, tilt);
        }
    } else if( event_id == IDD_TEXTCTRL_MHTilt ) {
        float pan = 0.0f;
        float tilt = 0.0f;
        if( GetPosition("Tilt", tilt) ) {
            GetPosition("Pan", pan);
            UpdatePositionCanvas(pan, tilt);
        }
    }
}

void MovingHeadPanel::UpdateColorPanel()
{
    DmxMovingHeadComm* first_mhead{ nullptr };
    auto models = GetActiveModels();
    bool wheel_active = false;
    unsigned int num_colors = 0;
    for (const auto& it : models) {
        if (it->GetDisplayAs() == "DmxMovingHeadAdv" || it->GetDisplayAs() == "DmxMovingHead") {
            DmxMovingHeadComm* mhead = (DmxMovingHeadComm*)it;
            bool active = IsHeadActive(mhead->GetFixtureVal()) || (models.size() == 1);
            if( active ) {
                if( mhead->HasColorAbility() ) {
                    DmxColorAbility* ptrColorAbility = mhead->GetColorAbility();
                    std::string color_type = ptrColorAbility->GetTypeName();
                    if (color_type == "ColorWheel" ) {
                        if (wheel_active) {
                            // multiple color wheels so check if num colors is the same
                            if ( ptrColorAbility->GetColors().size() != num_colors ) {
                                wheel_active = false;
                                break;
                            }
                        } else {
                            wheel_active = true;
                            num_colors = ptrColorAbility->GetColors().size();
                            if( first_mhead == nullptr ) {
                                first_mhead = mhead;
                            }
                        }
                    } else {
                        wheel_active = false;
                        break;
                    }
                } else {
                    wheel_active = false;
                    break;
                }
            }
        } else {
            wheel_active = false;
            break;
        }
    }

    if( wheel_active ) {
        DmxColorAbility* ptrColorAbility = first_mhead->GetColorAbility();
        auto colors = ptrColorAbility->GetColors();
        if( m_wheelColorPanel == nullptr) {
            m_wheelColorPanel = new MHColorWheelPanel(this, PanelColorWheel, wxID_ANY, wxDefaultPosition, wxSize(250, 250));
            m_wheelColorPanel->DefineColours(colors);
            FlexGridSizerColorWheel->Add(m_wheelColorPanel, 0, wxALL | wxEXPAND);
            PanelColorWheel->Show();
        } else {
            // check if number of colors is still the same
            m_wheelColorPanel->DefineColours(colors);
        }
        if( Notebook2->GetPageCount() == 1 ) {
            Notebook2->AddPage(PanelColorWheel, _("ColorWheel"), false);
        }
    } else {
        while(Notebook2->GetPageCount()>1) {
            Notebook2->RemovePage(1);
        }
    }
}

static std::list<std::string> possettings = {"Heads", "Pan", "Tilt", "PanOffset", "TiltOffset", "Groupings", "Cycles",
                                             "Pan VC", "Tilt VC", "PanOffset VC", "TiltOffset VC", "Groupings VC"};
static std::list<std::string> pathsettings = {"Path", "PathScale", "TimeOffset", "IgnorePan", "IgnoreTilt", "PathScale VC", "TimeOffset VC" };
static std::list<std::string> colorsettings = { "Color", "Wheel", "AutoShutter" };
static std::list<std::string> dimmersettings = {"Dimmer" };

void MovingHeadPanel::UpdateMHSettings()
{
    if( recall ) return;

    if( !presets_loaded ) { // I'd like to do this during construction but apparently the current directory is not set yet
        PopulatePresets();
        presets_loaded = true;
        if (!IsShownOnScreen()) {
            return;
        }
        FlexGridSizer_Main->SetSizeHints(this);
        Layout();
        Refresh();
    }

    UpdateColorPanel();

    RemoveSettings(possettings);

    std::string headset = xlEMPTY_STRING;

    // build a string with the selected moving heads
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
                    if (it->GetDisplayAs() == "DmxMovingHeadAdv" || it->GetDisplayAs() == "DmxMovingHead") {
                        DmxMovingHeadComm* mhead = (DmxMovingHeadComm*)it;
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

    for( int i = 1; i <= 8; ++i ) {
        wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
        wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
        if( checkbox != nullptr ) {
            if( checkbox->IsChecked() ) {
                wxString textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d_Settings", i);
                wxTextCtrl* mh_textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
                if( mh_textbox != nullptr ) {
                    std::string mh_settings = mh_textbox->GetValue();

                    AddSetting( "Pan", "Pan", mh_settings );
                    AddSetting( "Tilt", "Tilt", mh_settings );
                    AddSetting( "PanOffset", "PanOffset", mh_settings );
                    AddSetting( "TiltOffset", "TiltOffset", mh_settings );
                    AddSetting( "Groupings", "Groupings", mh_settings );
                    AddSetting( "Cycles", "Cycles", mh_settings );
                    mh_settings += ";";
                    mh_settings += headset;

                    // update the settings textbox
                    mh_textbox->SetValue(mh_settings);
                }
            }
        }
    }
    UpdateStatusPanel();
}

void MovingHeadPanel::UpdateColorSettings()
{
    if( recall ) return;

    bool rgb_active = false;
    bool wheel_active = false;
    std::string color_text{xlEMPTY_STRING};
    std::string wheel_text{xlEMPTY_STRING};

    RemoveSettings(colorsettings);

    if( m_rgbColorPanel != nullptr && m_rgbColorPanel->HasColour() ) {
        color_text = m_rgbColorPanel->GetColour();
        rgb_active = true;
    }

    if( m_wheelColorPanel != nullptr && m_wheelColorPanel->HasColour() && Notebook2->GetPageCount() > 1 ) {
        wheel_text = m_wheelColorPanel->GetColour();
        wheel_active = true;
    }

    if( color_text != xlEMPTY_STRING || wheel_text != xlEMPTY_STRING ) {
        for( int i = 1; i <= 8; ++i ) {
            wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
            wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
            if( checkbox != nullptr ) {
                if( checkbox->IsChecked() ) {
                    wxString textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d_Settings", i);
                    wxTextCtrl* mh_textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
                    if( mh_textbox != nullptr ) {
                        std::string mh_settings = mh_textbox->GetValue();
                        if( rgb_active ) {
                            mh_settings += ";";
                            mh_settings += color_text;
                        }
                        if( wheel_active ) {
                            mh_settings += ";";
                            mh_settings += wheel_text;
                            if (CheckBoxAutoShutter->IsChecked()) {
                                mh_settings += ";AutoShutter: true";
                            }
                        }
                        mh_textbox->SetValue(mh_settings);
                    }
                }
            }
        }
    }
}

void MovingHeadPanel::UpdatePathSettings()
{
    if( recall ) return;

    RemoveSettings(pathsettings);

    for( int i = 1; i <= 8; ++i ) {
        wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
        wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
        if( checkbox != nullptr ) {
            if( checkbox->IsChecked() ) {
                wxString textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d_Settings", i);
                wxTextCtrl* mh_textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
                if( mh_textbox != nullptr ) {
                    std::string mh_settings = mh_textbox->GetValue();

                    // See if there are any path segments
                    if( m_sketch.getLength() > 0.0 ) {
                        AddPath( mh_settings );
                        AddSetting( "PathScale", "PathScale", mh_settings );
                        AddSetting( "TimeOffset", "TimeOffset", mh_settings );

                        // See if pan path is ignored
                        checkbox = (wxCheckBox*)(this->FindWindowByName("ID_CHECKBOX_MHIgnorePan"));
                        if( checkbox != nullptr ) {
                            if( checkbox->IsChecked() ) {
                                mh_settings += ";IgnorePan: ";
                            }
                        }

                        // See if tilt path is ignored
                        checkbox = (wxCheckBox*)(this->FindWindowByName("ID_CHECKBOX_MHIgnoreTilt"));
                        if( checkbox != nullptr ) {
                            if( checkbox->IsChecked() ) {
                                mh_settings += ";IgnoreTilt: ";
                            }
                        }

                        // update the settings textbox
                        mh_textbox->SetValue(mh_settings);
                    }
                }
            }
        }
    }
}

void MovingHeadPanel::UpdateDimmerSettings()
{
    if( recall ) return;

    std::string dimmer_text{xlEMPTY_STRING};

    RemoveSettings(dimmersettings);

    if( m_movingHeadDimmerPanel != nullptr ) {
        dimmer_text = m_movingHeadDimmerPanel->GetDimmerCommands();
    }

    if( dimmer_text != xlEMPTY_STRING ) {
        for( int i = 1; i <= 8; ++i ) {
            wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
            wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
            if( checkbox != nullptr ) {
                if( checkbox->IsChecked() ) {
                    wxString textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d_Settings", i);
                    wxTextCtrl* mh_textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
                    if( mh_textbox != nullptr ) {
                        std::string mh_settings = mh_textbox->GetValue();
                        mh_settings += ";";
                        mh_settings += dimmer_text;
                        mh_textbox->SetValue(mh_settings);
                    }
                }
            }
        }
    }
}

void MovingHeadPanel::UpdateStatusPanel()
{
    bool headselect_set = false;
    bool hasrealvalues = false;
    GetFixturesGroups();
    Button_All->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    Button_None->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    Button_Evens->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    Button_Odds->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    wxColor btntext(0, 0, 0);
    if (IsDarkMode()) {
        btntext = wxColor (255, 255, 255);
    }
    Button_All->SetForegroundColour(wxColour(btntext));
    Button_None->SetForegroundColour(wxColour(btntext));
    Button_Evens->SetForegroundColour(wxColour(btntext));
    Button_Odds->SetForegroundColour(wxColour(btntext));

    std::string all_settings = xlEMPTY_STRING;
    for( int i = 1; i <= 8; ++i ) {
        wxString textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d_Settings", i);
        wxTextCtrl* mh_textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
        if( mh_textbox != nullptr ) {
            std::string mh_settings = mh_textbox->GetValue();
            if( mh_settings != xlEMPTY_STRING ) {
                wxString settings = wxString::Format("MH%d:\n", i);
                all_settings += settings;
                wxArrayString all_cmds = wxSplit(mh_settings, ';');
                bool pos_set = false;
                bool path_set = false;
                bool color_set = false;
                bool dimmer_set = false;
                hasrealvalues = false;
                for (size_t j = 0; j < all_cmds.size(); ++j )
                {
                    std::string cmd = all_cmds[j];
                    if( cmd == xlEMPTY_STRING ) continue;
                    int pos = cmd.find(":");
                    std::string cmd_type = cmd.substr(0, pos);
                    if (cmd_type == "Pan" || cmd_type == "Pan VC") {
                        pos_set = true;
                        if (cmd != "Pan: 0.0")
                            hasrealvalues = true;
                    } else if (cmd_type == "Tilt" || cmd_type == "Tilt VC") {
                        pos_set = true;
                        if (cmd != "Tilt: 0.0")
                            hasrealvalues = true;
                    } else if (cmd_type == "PanOffset") {
                        pos_set = true;
                        if (cmd != "PanOffset: 0.0")
                            hasrealvalues = true;
                    } else if (cmd_type == "TiltOffset") {
                        pos_set = true;
                        if (cmd != "TiltOffset: 0.0")
                            hasrealvalues = true;
                    } else if (cmd_type == "Path") {
                        path_set = true;
                    } else if (cmd_type == "Color") {
                        color_set = true;
                    } else if (cmd_type == "Dimmer") {
                        dimmer_set = true;
                    } else if (cmd_type == "Heads") {
                        std::string hascmd_heads = cmd.substr(cmd.find(':') + 2);
                        if (hasrealvalues && headselect_set == false) {
                            /*
                            auto setMH = wxSplit(hascmd_heads, ',');
                            for (auto i = 0; i < setMH.size(); i++) {
                                wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%s", setMH[i]);
                                wxCheckBox* checkbox = (wxCheckBox*)(FindWindowByName(checkbox_ctrl));
                                if (checkbox != nullptr) {
                                    checkbox->SetValue(true);
                                }
                            }
                            */
                            headselect_set = true;
                        }
                        if (hasrealvalues) {
                            if (hascmd_heads == mh_evens) {
                                Button_Evens->SetBackgroundColour(*wxBLUE);
                                Button_Evens->SetForegroundColour(wxColour(255,255,255));
                            } else if (hascmd_heads == mh_odds) {
                                Button_Odds->SetBackgroundColour(*wxBLUE);
                                Button_Odds->SetForegroundColour(wxColour(255,255,255));
                            } else if (hascmd_heads == mh_all) {
                                Button_All->SetBackgroundColour(*wxBLUE);
                                Button_All->SetForegroundColour(wxColour(255,255,255));
                            }
                        }
                    }
                }
                if (pos_set) {
                    all_settings += "Position: ";
                    for (size_t j = 0; j < all_cmds.size(); ++j )
                    {
                        std::string cmd = all_cmds[j];
                        if( cmd == xlEMPTY_STRING ) continue;
                        int pos = cmd.find(":");
                        std::string cmd_type = cmd.substr(0, pos);
                        int vc_pos = cmd_type.find("VC");
                        if (vc_pos >= 0) {
                            all_settings += cmd_type + " ";
                        } else {
                            bool found = (std::find(possettings.begin(), possettings.end(), cmd_type) != possettings.end());
                            if( found ) {
                                all_settings += (all_cmds[j]) + " ";
                            }
                        }
                    }
                    all_settings += "\n";
                }
                if (path_set) {
                    all_settings += "Path: Active\n";
                }
                if (color_set) {
                    all_settings += "Color: Active\n";
                }
                if (dimmer_set) {
                    all_settings += "Dimmer: Active\n";
                }
                all_settings += "\n";
            }
        }
    }
    TextCtrl_Status->SetValue(all_settings);
}

// Added special case to remove all path settings at once so we don't have to search several times
void MovingHeadPanel::RemoveSettings(std::list<std::string>& settings)
{
    for( int i = 1; i <= 8; ++i ) {
        wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
        wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
        if( checkbox != nullptr ) {
            if( checkbox->IsChecked() ) {
                wxString textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d_Settings", i);
                wxTextCtrl* mh_textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
                if( mh_textbox != nullptr ) {
                    std::string mh_settings = mh_textbox->GetValue();
                    if( mh_settings != xlEMPTY_STRING ) {
                        wxArrayString all_cmds = wxSplit(mh_settings, ';');
                        wxArrayString updated_cmds;
                        for (size_t j = 0; j < all_cmds.size(); ++j )
                        {
                            std::string cmd = all_cmds[j];
                            if( cmd == xlEMPTY_STRING ) continue;
                            int pos = cmd.find(":");
                            std::string cmd_type = cmd.substr(0, pos);
                            bool found = (std::find(settings.begin(), settings.end(), cmd_type) != settings.end());
                            if( !found ) {
                                updated_cmds.Add(all_cmds[j]);
                            }
                        }
                        mh_settings = wxJoin( updated_cmds, ';');
                        mh_textbox->SetValue(mh_settings);
                    }
                }
            }
        }
    }
}

void MovingHeadPanel::AddSetting(const std::string& name, const std::string& ctrl_name, std::string& mh_settings)
{
    wxTextCtrl* textbox = (wxTextCtrl*)(this->FindWindowByName("IDD_TEXTCTRL_MH" + ctrl_name));
    if( textbox != nullptr && !textbox->IsEnabled() ) {
        BulkEditValueCurveButton* vc_button = (BulkEditValueCurveButton*)(this->FindWindowByName("ID_VALUECURVE_MH" + ctrl_name));
        if( vc_button != nullptr ) {
            ValueCurve* vc = vc_button->GetValue();
            AddValueCurve(vc, name + " VC", mh_settings);
        }
    } else {
        AddTextbox("IDD_TEXTCTRL_MH" + ctrl_name, name, mh_settings);
    }
}

void MovingHeadPanel::AddPath(std::string& mh_settings)
{
    wxTextCtrl* textbox = (wxTextCtrl*)(this->FindWindowByName("ID_TEXTCTRL_MHPathDef"));
    if( textbox != nullptr ) {
        if( mh_settings != xlEMPTY_STRING ) {
            mh_settings += ";";
        }
        wxString value = wxString::Format("Path: ");
        std::string path_text = textbox->GetValue();
        std::replace( path_text.begin(), path_text.end(), ';', '@'); // custom value curves were using my delimiter
        mh_settings += value + path_text;
    }
}

void MovingHeadPanel::AddValueCurve(ValueCurve* vc, const std::string& name, std::string& mh_settings)
{
    if( mh_settings != xlEMPTY_STRING ) {
        mh_settings += ";";
    }
    std::string vc_text = vc->Serialise();
    std::replace( vc_text.begin(), vc_text.end(), ';', '@'); // custom value curves were using my delimiter
    wxString value = wxString::Format("%s: %s", name, vc_text);
    mh_settings += value;
}

void MovingHeadPanel::AddTextbox(const std::string& ctrl_id, const std::string& name, std::string& mh_settings)
{
    wxTextCtrl* textbox = (wxTextCtrl*)(this->FindWindowByName(ctrl_id));
    if( textbox != nullptr ) {
        if( mh_settings != xlEMPTY_STRING ) {
            mh_settings += ";";
        }
        wxString value = wxString::Format("%s: %s", name, textbox->GetValue());
        mh_settings += value;
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

void MovingHeadPanel::CheckAllFixtures() {
    auto models = GetActiveModels();

    for (const auto& it : models) {
        if (it->GetDisplayAs() == "DmxMovingHeadAdv" || it->GetDisplayAs() == "DmxMovingHead") {
            DmxMovingHeadComm* mhead = (DmxMovingHeadComm*)it;
            int num = mhead->GetFixtureVal();
            wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", num);
            wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
            if (checkbox != nullptr) {
                checkbox->SetValue(true);
            }
        }
    }
}

void MovingHeadPanel::OnButton_AllClick(wxCommandEvent& event)
{
    CheckAllFixtures();
    wxCommandEvent _event;
    OnCheckBox_MHClick(_event);
}

void MovingHeadPanel::OnButton_NoneClick(wxCommandEvent& event)
{
    UncheckAllFixtures();
    UpdateColorPanel();
}

void MovingHeadPanel::OnButton_EvensClick(wxCommandEvent& event)
{
    UncheckAllFixtures();

    auto models = GetActiveModels();

    for (const auto& it : models) {
        if (it->GetDisplayAs() == "DmxMovingHeadAdv" || it->GetDisplayAs() == "DmxMovingHead") {
            DmxMovingHeadComm* mhead = (DmxMovingHeadComm*)it;
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
    wxCommandEvent _event;
    OnCheckBox_MHClick(_event);
}

void MovingHeadPanel::OnButton_OddsClick(wxCommandEvent& event)
{
    UncheckAllFixtures();

    auto models = GetActiveModels();

    for (const auto& it : models) {
        if (it->GetDisplayAs() == "DmxMovingHeadAdv" || it->GetDisplayAs() == "DmxMovingHead") {
            DmxMovingHeadComm* mhead = (DmxMovingHeadComm*)it;
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
    wxCommandEvent _event;
    OnCheckBox_MHClick(_event);
}

std::list<Model*> MovingHeadPanel::GetActiveModels()
{
    std::list<Model*> res;

    if (xLightsApp::GetFrame()->GetMainSequencer() == nullptr) return res;

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

void MovingHeadPanel::OnCheckBox_MHIgnorePanClick(wxCommandEvent& event)
{
    CheckBox_MHIgnorePan->SetValue(event.GetInt());
    UpdatePathSettings();
}

void MovingHeadPanel::OnCheckBox_MHIgnoreTiltClick(wxCommandEvent& event)
{
    CheckBox_MHIgnoreTilt->SetValue(event.GetInt());
    UpdatePathSettings();
}

void MovingHeadPanel::OnCharHook(wxKeyEvent& event)
{
    if( Notebook1->GetPageText(Notebook1->GetSelection()) == "Pathing" ) {
        if( TextCtrl_MHTimeOffset->HasFocus() || TextCtrl_MHPathScale->HasFocus() ) {
            event.Skip();
        } else {
            if (m_sketchCanvasPanel != nullptr)
                m_sketchCanvasPanel->OnSketchKeyDown(event);
        }
    } else {
        event.Skip();
    }
}

void MovingHeadPanel::SetSketchDef(const std::string& sketchDef)
{
    if ( sketchDef != m_sketchDef) {
        m_sketchDef = sketchDef;
        m_sketch = SketchEffectSketch::SketchFromString(m_sketchDef);
    }
}

SketchEffectSketch& MovingHeadPanel::GetSketch()
{
    return m_sketch;
}

int MovingHeadPanel::GetSelectedPathIndex()
{
    return selected_path;
}

void MovingHeadPanel::NotifySketchUpdated()
{
    m_sketchDef = m_sketch.toString();
    TextCtrl_MHPathDef->SetValue(m_sketchDef);
    UpdatePathSettings();
}

void MovingHeadPanel::NotifySketchPathsUpdated()
{
    if( m_sketch.paths().size() > 0 ) {
        selected_path = 0;
    } else {
        selected_path = -1;
    }
}

void MovingHeadPanel::NotifyPathStateUpdated(SketchCanvasPathState state)
{
}

void MovingHeadPanel::SelectLastPath()
{
    selected_path = 0;
}

bool MovingHeadPanel::canContinuePath() const
{
    return true;
}

void MovingHeadPanel::OnButton_MHPathContinueClick(wxCommandEvent& event)
{
    m_sketchCanvasPanel->UpdatePathState(SketchCanvasPathState::LineToNewPoint);
}

void MovingHeadPanel::OnButton_MHPathClearClick(wxCommandEvent& event)
{
    m_sketchCanvasPanel->ResetHandlesState(SketchCanvasPathState::DefineStartPoint);
    SetSketchDef(xlEMPTY_STRING);
    NotifySketchUpdated();
    NotifySketchPathsUpdated();
    m_sketchCanvasPanel->Changed();
}

void MovingHeadPanel::OnButton_MHPathCloseClick(wxCommandEvent& event)
{
    m_sketchCanvasPanel->ClosePath();
    NotifySketchUpdated();
    m_sketchCanvasPanel->Changed();
}

void MovingHeadPanel::NotifyPositionUpdated()
{
    wxPoint2DDouble pt = m_movingHeadCanvasPanel->GetPosition();
    float pos_x = pt.m_x * 360.0f - 180.0f;
    float pos_y = -pt.m_y * 360.0f + 180.0f;
    UpdateTextbox("Pan", pos_x);
    UpdateTextbox("Tilt", pos_y);
    UpdateMHSettings();
    FireChangeEvent();
}

void MovingHeadPanel::UpdatePositionCanvas(float pan, float tilt)
{
    wxPoint2DDouble new_pos;
    new_pos.m_x = (pan + 180.0f) / 360.0f;
    new_pos.m_y = (180.0f - tilt) / 360.0f;
    m_movingHeadCanvasPanel->SetPosition(new_pos);
}

void MovingHeadPanel::UpdateTextbox(const std::string& ctrl_name, float pos)
{
    wxTextCtrl* textbox = (wxTextCtrl*)(this->FindWindowByName("IDD_TEXTCTRL_MH" + ctrl_name));
    if( textbox != nullptr ) {
        if( !textbox->IsEnabled() ) {
            textbox->Enable();
            BulkEditSliderF1* slider = (BulkEditSliderF1*)(this->FindWindowByName("ID_SLIDER_MH" + ctrl_name));
            if( slider != nullptr ) {
                slider->Enable();
            }
            BulkEditValueCurveButton* vc_button = (BulkEditValueCurveButton*)(this->FindWindowByName("ID_VALUECURVE_MH" + ctrl_name));
            if( vc_button != nullptr ) {
                vc_button->ToggleActive();
            }
        }
        wxString new_pos = wxString::Format("%3.1f", pos);
        textbox->SetValue(new_pos);
    }
}

void MovingHeadPanel::UpdateValueCurve(const std::string& ctrl_name, const std::string& curve_settings)
{
    BulkEditValueCurveButton* vc_button = (BulkEditValueCurveButton*)(this->FindWindowByName("ID_VALUECURVE_MH" + ctrl_name));
    wxTextCtrl* textbox = (wxTextCtrl*)(this->FindWindowByName("IDD_TEXTCTRL_MH" + ctrl_name));
    if( textbox != nullptr ) {
        if( textbox->IsEnabled() ) {
            textbox->Disable();
            BulkEditSliderF1* slider = (BulkEditSliderF1*)(this->FindWindowByName("ID_SLIDER_MH" + ctrl_name));
            if( slider != nullptr ) {
                slider->Disable();
            }
            if( vc_button != nullptr ) {
                vc_button->ToggleActive();
            }
        }
    }
    if( vc_button != nullptr ) {
        vc_button->SetValue(curve_settings);
    }
}

void MovingHeadPanel::UpdateCheckbox(const std::string& ctrl_name, bool value)
{
    wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName("ID_CHECKBOX_" + ctrl_name));
    if( checkbox != nullptr ) {
        checkbox->SetValue(value);
    }
}

bool MovingHeadPanel::GetPosition(const std::string& ctrl_name, float& pos)
{
    bool ret_val = false;
    wxTextCtrl* textbox = (wxTextCtrl*)(this->FindWindowByName("IDD_TEXTCTRL_MH" + ctrl_name));
    BulkEditSliderF1* slider = (BulkEditSliderF1*)(this->FindWindowByName("ID_SLIDER_MH" + ctrl_name));
    if( slider != nullptr && textbox != nullptr ) {
        if( slider->HasFocus() || textbox->HasFocus() ) {
            ret_val = true;
        }
        pos = wxAtof(textbox->GetValue());
    }
    return ret_val;
}

void MovingHeadPanel::NotifyColorUpdated()
{
    UpdateColorSettings();
    FireChangeEvent();
}

void MovingHeadPanel::NotifyDimmerUpdated()
{
    UpdateDimmerSettings();
    FireChangeEvent();
}

void MovingHeadPanel::OnCheckBox_MHClick(wxCommandEvent& event)
{
    // update color panels since selected heads changed
    UpdateColorPanel();

    bool all_same {true};
    std::string last_mh {xlEMPTY_STRING};
    for( int i = 1; i <= 8; ++i ) {
        wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", i);
        wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
        if( checkbox != nullptr ) {
            if( checkbox->IsChecked() ) {
                wxString textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d_Settings", i);
                wxTextCtrl* mh_textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
                if( last_mh == xlEMPTY_STRING ) {
                    last_mh = mh_textbox->GetValue();
                } else {
                    if( last_mh != mh_textbox->GetValue() ) {
                        all_same = false;
                        break;
                    }
                }
            }
        }
    }

    if( all_same && last_mh != xlEMPTY_STRING ) {
        RecallSettings(last_mh);
    }
    UpdateStatusPanel();
}

bool MovingHeadPanel::IsHeadActive(int num)
{
    wxString checkbox_ctrl = wxString::Format("IDD_CHECKBOX_MH%d", num);
    wxCheckBox* checkbox = (wxCheckBox*)(this->FindWindowByName(checkbox_ctrl));
    if( checkbox != nullptr ) {
        if( checkbox->IsChecked() ) {
            return true;
        }
    }
    return false;
}

void MovingHeadPanel::RecallSettings(const std::string mh_settings)
{
    recall = true;
    bool handled_path {false};

    UpdateCheckbox("MHIgnorePan", false);
    UpdateCheckbox("MHIgnoreTilt", false);

    wxArrayString all_cmds = wxSplit(mh_settings, ';');
    for (size_t j = 0; j < all_cmds.size(); ++j )
    {
        std::string cmd = all_cmds[j];
        if( cmd == xlEMPTY_STRING ) continue;
        int pos = cmd.find(":");
        std::string cmd_type = cmd.substr(0, pos);
        std::string settings = cmd.substr(pos+2, cmd.length());
        std::replace( settings.begin(), settings.end(), '@', ';');

        if( cmd_type == "Pan" ) {
            UpdateTextbox("Pan", wxAtof(settings.c_str()));
        } else if ( cmd_type == "Tilt" ) {
            UpdateTextbox("Tilt", wxAtof(settings.c_str()));
        } else if ( cmd_type == "Pan VC" ) {
            UpdateValueCurve("Pan", settings.c_str());
        } else if ( cmd_type == "Tilt VC" ) {
            UpdateValueCurve("Tilt", settings.c_str());
        } else if( cmd_type == "PanOffset" ) {
            UpdateTextbox("PanOffset", wxAtof(settings.c_str()));
        } else if( cmd_type == "TiltOffset" ) {
            UpdateTextbox("TiltOffset", wxAtof(settings.c_str()));
        } else if( cmd_type == ("PanOffset VC") ) {
            UpdateValueCurve("PanOffset", settings.c_str());
        } else if( cmd_type == ("TiltOffset VC") ) {
            UpdateValueCurve("TiltOffset", settings.c_str());
        } else if ( cmd_type == "IgnorePan" ) {
            UpdateCheckbox("MHIgnorePan", true);
        } else if ( cmd_type == "IgnoreTilt" ) {
            UpdateCheckbox("MHIgnoreTilt", true);
        } else if ( cmd_type == "Path" ) {
            handled_path = true;
            m_sketchDef = settings;
            m_sketch = SketchEffectSketch::SketchFromString(m_sketchDef);
            if( m_sketchDef == xlEMPTY_STRING ) {
                m_sketchCanvasPanel->ResetHandlesState();
                m_sketchCanvasPanel->UpdatePathState(SketchCanvasPathState::DefineStartPoint);
                selected_path = -1;
            } else {
                if( m_sketch.paths().size() > 0 ) {
                    selected_path = 0;
                    m_sketchCanvasPanel->UpdateHandlesForPath(0);
                } else {
                    selected_path = -1;
                }
            }
        } else if( cmd_type == "Groupings" ) {
            UpdateTextbox("Groupings", wxAtof(settings.c_str()));
        } else if( cmd_type == "Groupings VC" ) {
            UpdateValueCurve("Groupings", settings.c_str());
        } else if( cmd_type == "TimeOffset" ) {
            UpdateTextbox("TimeOffset", wxAtof(settings.c_str()));
        } else if( cmd_type == "PathScale" ) {
            UpdateTextbox("PathScale", wxAtof(settings.c_str()));
        } else if( cmd_type == "TimeOffset VC" ) {
            UpdateValueCurve("TimeOffset", settings.c_str());
        } else if( cmd_type == "PathScale VC" ) {
            UpdateValueCurve("PathScale", settings.c_str());
        } else if( cmd_type == "Color" ) {
            if( m_rgbColorPanel != nullptr ) {
                m_rgbColorPanel->SetColours(settings);
            }
        } else if( cmd_type == "Wheel" ) {
            if( m_wheelColorPanel != nullptr ) {
                m_wheelColorPanel->SetColours(settings);
            }
        } else if( cmd_type == "Dimmer" ) {
            if( m_movingHeadDimmerPanel != nullptr ) {
                m_movingHeadDimmerPanel->SetDimmerCommands(settings);
            }
        } else if (cmd_type == "AutoShutter") {
            UpdateCheckbox("AutoShutter", true);
        }
    }
    float pan = 0.0f;
    float tilt = 0.0f;
    GetPosition("Pan", pan);
    GetPosition("Tilt", tilt);
    UpdatePositionCanvas(pan, tilt);

    if( !handled_path ) {
        m_sketchDef = xlEMPTY_STRING;
        m_sketch = SketchEffectSketch::SketchFromString(m_sketchDef);
        m_sketchCanvasPanel->ResetHandlesState();
        m_sketchCanvasPanel->UpdatePathState(SketchCanvasPathState::DefineStartPoint);
        selected_path = -1;
    }

    recall = false;
}

void MovingHeadPanel::OnButton_ResetToDefaultClick(wxCommandEvent& event)
{
    std::string all_settings = xlEMPTY_STRING;

    ValueCurve_MHPan->SetActive(false);
    ValueCurve_MHTilt->SetActive(false);
    ValueCurve_MHPanOffset->SetActive(false);
    ValueCurve_MHTiltOffset->SetActive(false);
    ValueCurve_MHGroupings->SetActive(false);
    SetSliderValue(Slider_MHPan, 0.0f);
    SetSliderValue(Slider_MHTilt, 0.0f);
    SetSliderValue(Slider_MHPanOffset, 0.0f);
    SetSliderValue(Slider_MHTiltOffset, 0.0f);
    SetSliderValue(Slider_MHGroupings, 1);
    SetSliderValue(Slider_MHCycles, 10);
    NotifyPositionUpdated();

    m_movingHeadDimmerPanel->SetDimmerCommands("0.0,0.0,1.0,0.0");

    OnButton_MHPathClearClick(event);
    ValueCurve_MHPathScale->SetActive(false);
    ValueCurve_MHTimeOffset->SetActive(false);
    SetSliderValue(Slider_MHPathScale, 0.0f);
    SetSliderValue(Slider_MHTimeOffset, 0.0f);
    CheckBox_MHIgnorePan->SetValue(false);
    CheckBox_MHIgnoreTilt->SetValue(false);
    UpdatePathSettings();
    TextCtrl_Status->SetValue("");
    if (m_rgbColorPanel != nullptr) {
        m_rgbColorPanel->ResetColours();
    }
    if (m_wheelColorPanel != nullptr) {
        m_wheelColorPanel->ResetColours();
    }

    for( int i = 1; i <= 8; ++i ) {
        wxString textbox_ctrl = wxString::Format("ID_TEXTCTRL_MH%d_Settings", i);
        wxTextCtrl* mh_textbox = (wxTextCtrl*)(this->FindWindowByName(textbox_ctrl));
        if( mh_textbox != nullptr ) {
            std::string mh_settings = mh_textbox->GetValue();
            if( mh_settings != xlEMPTY_STRING ) {
                mh_textbox->SetValue(xlEMPTY_STRING);
            }
        }
    }
    CheckAllFixtures();

    FireChangeEvent();
    ValidateWindow();
}

void MovingHeadPanel::SetSliderValue(wxSlider* slider, int value) {
    slider->SetValue(value);
    wxScrollEvent event(wxEVT_SLIDER, slider->GetId());
    event.SetEventObject(slider);
    event.SetInt(value);
    slider->ProcessWindowEvent(event);
}

void MovingHeadPanel::OnButtonDimmerOnClick(wxCommandEvent& event)
{
    m_movingHeadDimmerPanel->SetDimmerCommands("0.0,1.0,1.0,1.0");
    UpdateDimmerSettings();
    FireChangeEvent();
}

void MovingHeadPanel::OnButtonDimmerOffClick(wxCommandEvent& event)
{
    m_movingHeadDimmerPanel->SetDimmerCommands("0.0,0.0,1.0,0.0");
    UpdateDimmerSettings();
    FireChangeEvent();
}

void MovingHeadPanel::OnValueCurve_MHTiltOffsetClick(wxCommandEvent& event)
{
}

void MovingHeadPanel::OnCheckBoxAutoShutterClick(wxCommandEvent& event)
{
    UpdateColorSettings();
    FireChangeEvent();
}

void MovingHeadPanel::GetFixturesGroups() {
    if (mh_evens == "" && mh_odds == "") {
        auto models = GetActiveModels();
        for (const auto& it : models) {
            if (it->GetDisplayAs() == "DmxMovingHeadAdv" || it->GetDisplayAs() == "DmxMovingHead") {
                DmxMovingHeadComm* mhead = (DmxMovingHeadComm*)it;
                int num = mhead->GetFixtureVal();
                if (!mh_all.empty()) {
                    mh_all += ",";
                }
                mh_all += std::to_string(num);
                if (num % 2 > 0) {
                    if (!mh_odds.empty()) {
                        mh_odds += ",";
                    }
                    mh_odds += std::to_string(num);
                } else {
                    if (!mh_evens.empty()) {
                        mh_evens += ",";
                    }
                    mh_evens += std::to_string(num);
                }
            }
        }
    }
}
