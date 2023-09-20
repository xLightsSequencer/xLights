/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "BufferPanel.h"

//(*InternalHeaders(BufferPanel)
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/string.h>
//*)

#include <wx/config.h>

#include <vector>

#include "PixelBuffer.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "effects/EffectPanelUtils.h"
#include "ValueCurveDialog.h"
#include "SubBufferPanel.h"
#include "xLightsMain.h"
#include "xLightsApp.h"

//(*IdInit(BufferPanel)
const long BufferPanel::ID_CHECKBOX_ResetBufferPanel = wxNewId();
const long BufferPanel::ID_STATICTEXT_BufferStyle = wxNewId();
const long BufferPanel::ID_CHOICE_BufferStyle = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_CHOICE_BufferStyle = wxNewId();
const long BufferPanel::ID_STATICTEXT3 = wxNewId();
const long BufferPanel::ID_SPINCTRL_BufferStagger = wxNewId();
const long BufferPanel::ID_STATICTEXT2 = wxNewId();
const long BufferPanel::ID_CHOICE_PerPreviewCamera = wxNewId();
const long BufferPanel::ID_STATICTEXT_BufferTransform = wxNewId();
const long BufferPanel::ID_CHOICE_BufferTransform = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_CHOICE_BufferTransform = wxNewId();
const long BufferPanel::ID_STATICTEXT_Blur = wxNewId();
const long BufferPanel::ID_SLIDER_Blur = wxNewId();
const long BufferPanel::ID_VALUECURVE_Blur = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_Blur = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_SLIDER_EffectBlur = wxNewId();
const long BufferPanel::ID_CHECKBOX_OverlayBkg = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_OverlayBkg = wxNewId();
const long BufferPanel::ID_SCROLLEDWINDOW1 = wxNewId();
const long BufferPanel::ID_PANEL1 = wxNewId();
const long BufferPanel::ID_CHOICE_Preset = wxNewId();
const long BufferPanel::ID_STATICTEXT_Rotation = wxNewId();
const long BufferPanel::ID_SLIDER_Rotation = wxNewId();
const long BufferPanel::ID_VALUECURVE_Rotation = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_Rotation = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_Rotation = wxNewId();
const long BufferPanel::ID_STATICTEXT_Rotations = wxNewId();
const long BufferPanel::ID_SLIDER_Rotations = wxNewId();
const long BufferPanel::ID_VALUECURVE_Rotations = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_Rotations = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_Rotations = wxNewId();
const long BufferPanel::ID_STATICTEXT_PivotPointX = wxNewId();
const long BufferPanel::ID_SLIDER_PivotPointX = wxNewId();
const long BufferPanel::ID_VALUECURVE_PivotPointX = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_PivotPointX = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_PivotPointX = wxNewId();
const long BufferPanel::ID_STATICTEXT_PivotPointY = wxNewId();
const long BufferPanel::ID_SLIDER_PivotPointY = wxNewId();
const long BufferPanel::ID_VALUECURVE_PivotPointY = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_PivotPointY = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_PivotPointY = wxNewId();
const long BufferPanel::ID_STATICTEXT_Zoom = wxNewId();
const long BufferPanel::ID_SLIDER_Zoom = wxNewId();
const long BufferPanel::ID_VALUECURVE_Zoom = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_Zoom = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_Zoom = wxNewId();
const long BufferPanel::ID_STATICTEXT_ZoomQuality = wxNewId();
const long BufferPanel::ID_SLIDER_ZoomQuality = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_ZoomQuality = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_ZoomQuality = wxNewId();
const long BufferPanel::ID_STATICTEXT_XRotation = wxNewId();
const long BufferPanel::ID_SLIDER_XRotation = wxNewId();
const long BufferPanel::ID_VALUECURVE_XRotation = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_XRotation = wxNewId();
const long BufferPanel::ID_STATICTEXT_XPivot = wxNewId();
const long BufferPanel::ID_SLIDER_XPivot = wxNewId();
const long BufferPanel::ID_VALUECURVE_XPivot = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_XPivot = wxNewId();
const long BufferPanel::ID_STATICTEXT_YRotation = wxNewId();
const long BufferPanel::ID_SLIDER_YRotation = wxNewId();
const long BufferPanel::ID_VALUECURVE_YRotation = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_YRotation = wxNewId();
const long BufferPanel::ID_STATICTEXT_YPivot = wxNewId();
const long BufferPanel::ID_SLIDER_YPivot = wxNewId();
const long BufferPanel::ID_VALUECURVE_YPivot = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_YPivot = wxNewId();
const long BufferPanel::ID_STATICTEXT1 = wxNewId();
const long BufferPanel::ID_CHOICE_RZ_RotationOrder = wxNewId();
const long BufferPanel::ID_SCROLLEDWINDOW2 = wxNewId();
const long BufferPanel::ID_PANEL4 = wxNewId();
const long BufferPanel::ID_NOTEBOOK1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(BufferPanel,wxPanel)
	//(*EventTable(BufferPanel)
	//*)
END_EVENT_TABLE()

BufferPanel::BufferPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size) : xlEffectPanel(parent)
{
	//(*Initialize(BufferPanel)
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer11;
	wxFlexGridSizer* FlexGridSizer12;
	wxFlexGridSizer* FlexGridSizer13;
	wxFlexGridSizer* FlexGridSizer15;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer9;
	wxPanel* Panel3;
	wxPanel* Panel4;
	wxStaticText* StaticText11;
	xlLockButton* BitmapButtonBufferStyle;
	xlLockButton* BitmapButton_BufferTransform;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	CheckBox_ResetBufferPanel = new wxCheckBox(this, ID_CHECKBOX_ResetBufferPanel, _("Reset panel when changing effects"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ResetBufferPanel"));
	CheckBox_ResetBufferPanel->SetValue(true);
	FlexGridSizer1->Add(CheckBox_ResetBufferPanel, 1, wxALL|wxEXPAND, 5);
	Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	Panel3 = new wxPanel(Notebook1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer11 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer11->AddGrowableCol(0);
	FlexGridSizer11->AddGrowableRow(0);
	ScrolledWindow1 = new wxScrolledWindow(Panel3, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
	FullBufferSizer = new wxFlexGridSizer(2, 1, 0, 0);
	FullBufferSizer->AddGrowableCol(0);
	FullBufferSizer->AddGrowableRow(1);
	BufferSizer = new wxFlexGridSizer(0, 3, 0, 0);
	BufferSizer->AddGrowableCol(1);
	StaticText4 = new wxStaticText(ScrolledWindow1, ID_STATICTEXT_BufferStyle, _("Render Style"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_BufferStyle"));
	BufferSizer->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BufferStyleChoice = new BulkEditChoice(ScrolledWindow1, ID_CHOICE_BufferStyle, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_BufferStyle"));
	BufferStyleChoice->SetSelection( BufferStyleChoice->Append(_("Default")) );
	BufferStyleChoice->Append(_("Per Preview"));
	BufferSizer->Add(BufferStyleChoice, 1, wxALL|wxEXPAND, 2);
	BitmapButtonBufferStyle = new xlLockButton(ScrolledWindow1, ID_BITMAPBUTTON_CHOICE_BufferStyle, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_BufferStyle"));
	BitmapButtonBufferStyle->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	BitmapButtonBufferStyle->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	BufferSizer->Add(BitmapButtonBufferStyle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText17 = new wxStaticText(ScrolledWindow1, ID_STATICTEXT3, _("Buffer Stagger"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	BufferSizer->Add(StaticText17, 1, wxALL|wxEXPAND, 2);
	SpinCtrl_BufferStagger = new wxSpinCtrl(ScrolledWindow1, ID_SPINCTRL_BufferStagger, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -100, 100, 0, _T("ID_SPINCTRL_BufferStagger"));
	SpinCtrl_BufferStagger->SetValue(_T("0"));
	BufferSizer->Add(SpinCtrl_BufferStagger, 1, wxALL|wxEXPAND, 2);
	BufferSizer->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	StaticText16 = new wxStaticText(ScrolledWindow1, ID_STATICTEXT2, _("Camera"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	BufferSizer->Add(StaticText16, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_PerPreviewCamera = new BulkEditChoice(ScrolledWindow1, ID_CHOICE_PerPreviewCamera, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PerPreviewCamera"));
	Choice_PerPreviewCamera->SetSelection( Choice_PerPreviewCamera->Append(_("2D")) );
	BufferSizer->Add(Choice_PerPreviewCamera, 1, wxALL|wxEXPAND, 2);
	BufferSizer->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(ScrolledWindow1, ID_STATICTEXT_BufferTransform, _("Transformation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_BufferTransform"));
	BufferSizer->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BufferTransform = new BulkEditChoice(ScrolledWindow1, ID_CHOICE_BufferTransform, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_BufferTransform"));
	BufferTransform->SetSelection( BufferTransform->Append(_("None")) );
	BufferTransform->Append(_("Rotate CC 90"));
	BufferTransform->Append(_("Rotate CW 90"));
	BufferTransform->Append(_("Rotate 180"));
	BufferTransform->Append(_("Flip Vertical"));
	BufferTransform->Append(_("Flip Horizontal"));
	BufferTransform->Append(_("Rotate CC 90 Flip Horizontal"));
	BufferTransform->Append(_("Rotate CW 90 Flip Horizontal"));
	BufferSizer->Add(BufferTransform, 1, wxALL|wxEXPAND, 2);
	BitmapButton_BufferTransform = new xlLockButton(ScrolledWindow1, ID_BITMAPBUTTON_CHOICE_BufferTransform, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_BufferTransform"));
	BitmapButton_BufferTransform->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	BitmapButton_BufferTransform->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	BufferSizer->Add(BitmapButton_BufferTransform, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(ScrolledWindow1, ID_STATICTEXT_Blur, _("Blur"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Blur"));
	BufferSizer->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	Slider_EffectBlur = new BulkEditSlider(ScrolledWindow1, ID_SLIDER_Blur, 1, 1, 15, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Blur"));
	Slider_EffectBlur->SetMinSize(wxDLG_UNIT(ScrolledWindow1,wxSize(20,-1)));
	FlexGridSizer6->Add(Slider_EffectBlur, 1, wxALL|wxEXPAND, 0);
	BitmapButton_Blur = new BulkEditValueCurveButton(ScrolledWindow1, ID_VALUECURVE_Blur, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Blur"));
	FlexGridSizer6->Add(BitmapButton_Blur, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl_EffectBlur = new BulkEditTextCtrl(ScrolledWindow1, IDD_TEXTCTRL_Blur, _("1"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow1,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Blur"));
	TextCtrl_EffectBlur->SetMaxLength(2);
	FlexGridSizer6->Add(TextCtrl_EffectBlur, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	BufferSizer->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 0);
	BitmapButton_EffectBlur = new xlLockButton(ScrolledWindow1, ID_BITMAPBUTTON_SLIDER_EffectBlur, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_EffectBlur"));
	BitmapButton_EffectBlur->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	BitmapButton_EffectBlur->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	BufferSizer->Add(BitmapButton_EffectBlur, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_OverlayBkg = new BulkEditCheckBox(ScrolledWindow1, ID_CHECKBOX_OverlayBkg, _("Persistent"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_OverlayBkg"));
	CheckBox_OverlayBkg->SetValue(false);
	BufferSizer->Add(CheckBox_OverlayBkg, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BufferSizer->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	BitmapButton_OverlayBkg = new xlLockButton(ScrolledWindow1, ID_BITMAPBUTTON_OverlayBkg, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_OverlayBkg"));
	BitmapButton_OverlayBkg->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	BitmapButton_OverlayBkg->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	BufferSizer->Add(BitmapButton_OverlayBkg, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FullBufferSizer->Add(BufferSizer, 1, wxALL|wxEXPAND, 0);
	SubBufferPanelSizer = new wxFlexGridSizer(0, 1, 0, 0);
	SubBufferPanelSizer->AddGrowableCol(0);
	SubBufferPanelSizer->AddGrowableRow(0);
	FullBufferSizer->Add(SubBufferPanelSizer, 1, wxALL|wxEXPAND, 0);
	ScrolledWindow1->SetSizer(FullBufferSizer);
	FlexGridSizer11->Add(ScrolledWindow1, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 0);
	Panel3->SetSizer(FlexGridSizer11);
	Panel4 = new wxPanel(Notebook1, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	FlexGridSizer15 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer15->AddGrowableCol(0);
	FlexGridSizer15->AddGrowableRow(0);
	ScrolledWindow2 = new wxScrolledWindow(Panel4, ID_SCROLLEDWINDOW2, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW2"));
	RotoZoomSizer = new wxFlexGridSizer(0, 3, 0, 0);
	RotoZoomSizer->AddGrowableCol(1);
	StaticText11 = new wxStaticText(ScrolledWindow2, wxID_ANY, _("Preset"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	RotoZoomSizer->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Preset = new wxChoice(ScrolledWindow2, ID_CHOICE_Preset, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Preset"));
	Choice_Preset->SetSelection( Choice_Preset->Append(wxEmptyString) );
	Choice_Preset->Append(_("None - Reset"));
	Choice_Preset->Append(_("1 Rev CW"));
	Choice_Preset->Append(_("1 Rev CCW"));
	Choice_Preset->Append(_("Explode"));
	Choice_Preset->Append(_("Collapse"));
	Choice_Preset->Append(_("Explode + Spin CW"));
	Choice_Preset->Append(_("Spin CW Accelerate"));
	Choice_Preset->Append(_("Shake"));
	RotoZoomSizer->Add(Choice_Preset, 1, wxALL, 2);
	RotoZoomSizer->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(ScrolledWindow2, ID_STATICTEXT_Rotation, _("Z Rotation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Rotation"));
	RotoZoomSizer->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Rotation = new BulkEditSlider(ScrolledWindow2, ID_SLIDER_Rotation, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Rotation"));
	Slider_Rotation->SetMinSize(wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)));
	FlexGridSizer2->Add(Slider_Rotation, 1, wxALL|wxEXPAND, 1);
	BitmapButton_VCRotation = new BulkEditValueCurveButton(ScrolledWindow2, ID_VALUECURVE_Rotation, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Rotation"));
	FlexGridSizer2->Add(BitmapButton_VCRotation, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Rotation = new BulkEditTextCtrl(ScrolledWindow2, IDD_TEXTCTRL_Rotation, _("0"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Rotation"));
	TextCtrl_Rotation->SetMaxLength(3);
	FlexGridSizer2->Add(TextCtrl_Rotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	RotoZoomSizer->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	BitmapButton_Rotation = new xlLockButton(ScrolledWindow2, ID_BITMAPBUTTON_Rotation, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_Rotation"));
	BitmapButton_Rotation->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	BitmapButton_Rotation->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	RotoZoomSizer->Add(BitmapButton_Rotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(ScrolledWindow2, ID_STATICTEXT_Rotations, _("Z Rotations"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Rotations"));
	RotoZoomSizer->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	Slider_Rotations = new BulkEditSliderF1(ScrolledWindow2, ID_SLIDER_Rotations, 0, 0, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Rotations"));
	Slider_Rotations->SetMinSize(wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)));
	FlexGridSizer5->Add(Slider_Rotations, 1, wxALL|wxEXPAND, 1);
	BitmapButton_VCRotations = new BulkEditValueCurveButton(ScrolledWindow2, ID_VALUECURVE_Rotations, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Rotations"));
	FlexGridSizer5->Add(BitmapButton_VCRotations, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Rotations = new BulkEditTextCtrlF1(ScrolledWindow2, IDD_TEXTCTRL_Rotations, _("0.0"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Rotations"));
	FlexGridSizer5->Add(TextCtrl_Rotations, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	RotoZoomSizer->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 0);
	BitmapButton_Rotations = new xlLockButton(ScrolledWindow2, ID_BITMAPBUTTON_Rotations, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_Rotations"));
	BitmapButton_Rotations->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	BitmapButton_Rotations->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	RotoZoomSizer->Add(BitmapButton_Rotations, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText8 = new wxStaticText(ScrolledWindow2, ID_STATICTEXT_PivotPointX, _("Z Pivot Point X"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_PivotPointX"));
	RotoZoomSizer->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer9 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	Slider_PivotPointX = new BulkEditSlider(ScrolledWindow2, ID_SLIDER_PivotPointX, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_PivotPointX"));
	Slider_PivotPointX->SetMinSize(wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)));
	FlexGridSizer9->Add(Slider_PivotPointX, 1, wxALL|wxEXPAND, 1);
	BitmapButton_VCPivotPointX = new BulkEditValueCurveButton(ScrolledWindow2, ID_VALUECURVE_PivotPointX, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_PivotPointX"));
	FlexGridSizer9->Add(BitmapButton_VCPivotPointX, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_PivotPointX = new BulkEditTextCtrl(ScrolledWindow2, IDD_TEXTCTRL_PivotPointX, _("50"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_PivotPointX"));
	FlexGridSizer9->Add(TextCtrl_PivotPointX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RotoZoomSizer->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 0);
	BitmapButton_PivotPointX = new xlLockButton(ScrolledWindow2, ID_BITMAPBUTTON_PivotPointX, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_PivotPointX"));
	BitmapButton_PivotPointX->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	BitmapButton_PivotPointX->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	RotoZoomSizer->Add(BitmapButton_PivotPointX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText9 = new wxStaticText(ScrolledWindow2, ID_STATICTEXT_PivotPointY, _("Z Pivot Point Y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_PivotPointY"));
	RotoZoomSizer->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer10->AddGrowableCol(0);
	Slider_PivotPointY = new BulkEditSlider(ScrolledWindow2, ID_SLIDER_PivotPointY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_PivotPointY"));
	Slider_PivotPointY->SetMinSize(wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)));
	FlexGridSizer10->Add(Slider_PivotPointY, 1, wxALL|wxEXPAND, 1);
	BitmapButton_VCPivotPointY = new BulkEditValueCurveButton(ScrolledWindow2, ID_VALUECURVE_PivotPointY, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_PivotPointY"));
	FlexGridSizer10->Add(BitmapButton_VCPivotPointY, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_PivotPointY = new BulkEditTextCtrl(ScrolledWindow2, IDD_TEXTCTRL_PivotPointY, _("50"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_PivotPointY"));
	FlexGridSizer10->Add(TextCtrl_PivotPointY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RotoZoomSizer->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 0);
	BitmapButton_PivotPointY = new xlLockButton(ScrolledWindow2, ID_BITMAPBUTTON_PivotPointY, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_PivotPointY"));
	BitmapButton_PivotPointY->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	BitmapButton_PivotPointY->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	RotoZoomSizer->Add(BitmapButton_PivotPointY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText6 = new wxStaticText(ScrolledWindow2, ID_STATICTEXT_Zoom, _("Zoom"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Zoom"));
	RotoZoomSizer->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	Slider_Zoom = new BulkEditSliderF1(ScrolledWindow2, ID_SLIDER_Zoom, 10, 0, 30, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Zoom"));
	Slider_Zoom->SetMinSize(wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)));
	FlexGridSizer7->Add(Slider_Zoom, 1, wxALL|wxEXPAND, 1);
	BitmapButton_VCZoom = new BulkEditValueCurveButton(ScrolledWindow2, ID_VALUECURVE_Zoom, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Zoom"));
	FlexGridSizer7->Add(BitmapButton_VCZoom, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Zoom = new BulkEditTextCtrlF1(ScrolledWindow2, IDD_TEXTCTRL_Zoom, _("1.0"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Zoom"));
	FlexGridSizer7->Add(TextCtrl_Zoom, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RotoZoomSizer->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 0);
	BitmapButton_Zoom = new xlLockButton(ScrolledWindow2, ID_BITMAPBUTTON_Zoom, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_Zoom"));
	BitmapButton_Zoom->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	BitmapButton_Zoom->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	RotoZoomSizer->Add(BitmapButton_Zoom, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText7 = new wxStaticText(ScrolledWindow2, ID_STATICTEXT_ZoomQuality, _("Zoom Quality"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ZoomQuality"));
	RotoZoomSizer->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	Slider_ZoomQuality = new BulkEditSlider(ScrolledWindow2, ID_SLIDER_ZoomQuality, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_ZoomQuality"));
	Slider_ZoomQuality->SetMinSize(wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)));
	FlexGridSizer8->Add(Slider_ZoomQuality, 1, wxALL|wxEXPAND, 1);
	TextCtrl_ZoomQuality = new BulkEditTextCtrl(ScrolledWindow2, IDD_TEXTCTRL_ZoomQuality, _("1"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_ZoomQuality"));
	FlexGridSizer8->Add(TextCtrl_ZoomQuality, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RotoZoomSizer->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 0);
	BitmapButton_ZoomQuality = new xlLockButton(ScrolledWindow2, ID_BITMAPBUTTON_ZoomQuality, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_ZoomQuality"));
	BitmapButton_ZoomQuality->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	BitmapButton_ZoomQuality->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	RotoZoomSizer->Add(BitmapButton_ZoomQuality, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText10 = new wxStaticText(ScrolledWindow2, ID_STATICTEXT_XRotation, _("X Rotation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_XRotation"));
	RotoZoomSizer->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_XRotation = new BulkEditSlider(ScrolledWindow2, ID_SLIDER_XRotation, 0, 0, 360, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_XRotation"));
	Slider_XRotation->SetMinSize(wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)));
	FlexGridSizer3->Add(Slider_XRotation, 1, wxALL|wxEXPAND, 1);
	BitmapButton_XRotation = new BulkEditValueCurveButton(ScrolledWindow2, ID_VALUECURVE_XRotation, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_XRotation"));
	FlexGridSizer3->Add(BitmapButton_XRotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_XRotation = new BulkEditTextCtrl(ScrolledWindow2, IDD_TEXTCTRL_XRotation, _("0"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_XRotation"));
	TextCtrl_XRotation->SetMaxLength(3);
	FlexGridSizer3->Add(TextCtrl_XRotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RotoZoomSizer->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	RotoZoomSizer->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText13 = new wxStaticText(ScrolledWindow2, ID_STATICTEXT_XPivot, _("X Pivot Point"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_XPivot"));
	RotoZoomSizer->Add(StaticText13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer12->AddGrowableCol(0);
	Slider_XPivot = new BulkEditSlider(ScrolledWindow2, ID_SLIDER_XPivot, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_XPivot"));
	Slider_XPivot->SetMinSize(wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)));
	FlexGridSizer12->Add(Slider_XPivot, 1, wxALL|wxEXPAND, 1);
	BitmapButton_XPivot = new BulkEditValueCurveButton(ScrolledWindow2, ID_VALUECURVE_XPivot, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_XPivot"));
	FlexGridSizer12->Add(BitmapButton_XPivot, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_XPivot = new BulkEditTextCtrl(ScrolledWindow2, IDD_TEXTCTRL_XPivot, _("50"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_XPivot"));
	TextCtrl_XPivot->SetMaxLength(3);
	FlexGridSizer12->Add(TextCtrl_XPivot, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RotoZoomSizer->Add(FlexGridSizer12, 1, wxALL|wxEXPAND, 0);
	RotoZoomSizer->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText12 = new wxStaticText(ScrolledWindow2, ID_STATICTEXT_YRotation, _("Y Rotation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_YRotation"));
	RotoZoomSizer->Add(StaticText12, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_YRotation = new BulkEditSlider(ScrolledWindow2, ID_SLIDER_YRotation, 0, 0, 360, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_YRotation"));
	Slider_YRotation->SetMinSize(wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)));
	FlexGridSizer4->Add(Slider_YRotation, 1, wxALL|wxEXPAND, 1);
	BitmapButton_YRotation = new BulkEditValueCurveButton(ScrolledWindow2, ID_VALUECURVE_YRotation, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_YRotation"));
	FlexGridSizer4->Add(BitmapButton_YRotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_YRotation = new BulkEditTextCtrl(ScrolledWindow2, IDD_TEXTCTRL_YRotation, _("0"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_YRotation"));
	TextCtrl_YRotation->SetMaxLength(3);
	FlexGridSizer4->Add(TextCtrl_YRotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RotoZoomSizer->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 0);
	RotoZoomSizer->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText14 = new wxStaticText(ScrolledWindow2, ID_STATICTEXT_YPivot, _("Y Pivot Point"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_YPivot"));
	RotoZoomSizer->Add(StaticText14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer13 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer13->AddGrowableCol(0);
	Slider_YPivot = new BulkEditSlider(ScrolledWindow2, ID_SLIDER_YPivot, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_YPivot"));
	Slider_YPivot->SetMinSize(wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)));
	FlexGridSizer13->Add(Slider_YPivot, 1, wxALL|wxEXPAND, 1);
	BitmapButton_YPivot = new BulkEditValueCurveButton(ScrolledWindow2, ID_VALUECURVE_YPivot, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_YPivot"));
	FlexGridSizer13->Add(BitmapButton_YPivot, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_YPivot = new BulkEditTextCtrl(ScrolledWindow2, IDD_TEXTCTRL_YPivot, _("50"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_YPivot"));
	TextCtrl_YPivot->SetMaxLength(3);
	FlexGridSizer13->Add(TextCtrl_YPivot, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RotoZoomSizer->Add(FlexGridSizer13, 1, wxALL|wxEXPAND, 0);
	RotoZoomSizer->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText15 = new wxStaticText(ScrolledWindow2, ID_STATICTEXT1, _("Application Order"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	RotoZoomSizer->Add(StaticText15, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	ChoiceRotateOrder = new BulkEditChoice(ScrolledWindow2, ID_CHOICE_RZ_RotationOrder, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_RZ_RotationOrder"));
	ChoiceRotateOrder->SetSelection( ChoiceRotateOrder->Append(_("X-Y-Z")) );
	ChoiceRotateOrder->Append(_("X-Z-Y"));
	ChoiceRotateOrder->Append(_("Y-X-Z"));
	ChoiceRotateOrder->Append(_("Y-Z-X"));
	ChoiceRotateOrder->Append(_("Z-X-Y"));
	ChoiceRotateOrder->Append(_("Z-Y-X"));
	RotoZoomSizer->Add(ChoiceRotateOrder, 1, wxALL, 1);
	RotoZoomSizer->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ScrolledWindow2->SetSizer(RotoZoomSizer);
	FlexGridSizer15->Add(ScrolledWindow2, 1, wxALL|wxEXPAND, 2);
	Panel4->SetSizer(FlexGridSizer15);
	Notebook1->AddPage(Panel3, _("Buffer"), false);
	Notebook1->AddPage(Panel4, _("Roto-Zoom"), false);
	FlexGridSizer1->Add(Notebook1, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer1);

	Connect(ID_CHECKBOX_ResetBufferPanel,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&BufferPanel::OnCheckBox_ResetBufferPanelClick);
	Connect(ID_CHOICE_BufferStyle,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&BufferPanel::OnBufferStyleChoiceSelect);
	Connect(ID_BITMAPBUTTON_CHOICE_BufferStyle,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_CHOICE_BufferTransform,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&BufferPanel::OnBufferTransformSelect);
	Connect(ID_BITMAPBUTTON_CHOICE_BufferTransform,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Blur,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_EffectBlur,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_OverlayBkg,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_CHOICE_Preset,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&BufferPanel::OnChoice_PresetSelect);
	Connect(ID_VALUECURVE_Rotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_Rotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Rotations,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_Rotations,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_PivotPointX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_PivotPointX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_PivotPointY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_PivotPointY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Zoom,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_Zoom,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_ZoomQuality,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_XRotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_XPivot,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_YRotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_YPivot,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnVCButtonClick);
	//*)

    SetName("Buffer");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&BufferPanel::OnVCChanged, nullptr, this);

    BitmapButton_Blur->GetValue()->SetLimits(BLUR_MIN, BLUR_MAX);
    BitmapButton_VCRotation->GetValue()->SetLimits(RZ_ROTATION_MIN, RZ_ROTATION_MAX);
    BitmapButton_VCZoom->GetValue()->SetLimits(RZ_ZOOM_MIN, RZ_ZOOM_MAX);
    BitmapButton_VCZoom->GetValue()->SetDivisor(RZ_ZOOM_DIVISOR);
    BitmapButton_VCRotations->GetValue()->SetLimits(RZ_ROTATIONS_MIN, RZ_ROTATIONS_MAX);
    BitmapButton_VCRotations->GetValue()->SetDivisor(RZ_ROTATIONS_DIVISOR);
    BitmapButton_VCPivotPointX->GetValue()->SetLimits(RZ_PIVOTX_MIN, RZ_PIVOTX_MAX);
    BitmapButton_VCPivotPointY->GetValue()->SetLimits(RZ_PIVOTY_MIN, RZ_PIVOTY_MAX);
    BitmapButton_XRotation->GetValue()->SetLimits(RZ_XROTATION_MIN, RZ_XROTATION_MAX);
    BitmapButton_YRotation->GetValue()->SetLimits(RZ_YROTATION_MIN, RZ_YROTATION_MAX);
    BitmapButton_XPivot->GetValue()->SetLimits(RZ_XPIVOT_MIN, RZ_XPIVOT_MAX);
    BitmapButton_YPivot->GetValue()->SetLimits(RZ_YPIVOT_MIN, RZ_YPIVOT_MAX);

    subBufferPanel = new SubBufferPanel(ScrolledWindow1, true, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    subBufferPanel->SetMinSize(wxDLG_UNIT(ScrolledWindow1, wxSize(30,30)));
    SubBufferPanelSizer->Insert(0, subBufferPanel,1, wxALL|wxEXPAND, 2);

    wxConfigBase* config = wxConfigBase::Get();
    bool reset;
    config->Read("xLightsResetBufferPanel", &reset, true);
    CheckBox_ResetBufferPanel->SetValue(reset);

    FullBufferSizer->Layout();
    RotoZoomSizer->Layout();

    ScrolledWindow1->SetSizer(FullBufferSizer);
    ScrolledWindow1->FitInside();
    ScrolledWindow1->SetScrollRate(5, 5);

    ScrolledWindow2->SetSizer(RotoZoomSizer);
    ScrolledWindow2->FitInside();
    ScrolledWindow2->SetScrollRate(5, 5);

    
    Connect(subBufferPanel->GetId(),SUBBUFFER_RANGE_CHANGED,(wxObjectEventFunction)&BufferPanel::HandleCommandChange);
    
    ValidateWindow();
}

BufferPanel::~BufferPanel()
{
	//(*Destroy(BufferPanel)
	//*)
}

wxString BufferPanel::GetBufferString() {
    wxString s = "";
    // Persistent
    if (CheckBox_OverlayBkg->GetValue()) {
        s += "B_CHECKBOX_OverlayBkg=1,";
    }
    if (BufferStyleChoice->GetSelection() != 0) {
        s += "B_CHOICE_BufferStyle=";
        s += BufferStyleChoice->GetStringSelection();
        s += ",";
    }
    if (Choice_PerPreviewCamera->GetStringSelection() != "2D") {
        s += "B_CHOICE_PerPreviewCamera=";
        s += Choice_PerPreviewCamera->GetStringSelection();
        s += ",";
    }
    if (BufferTransform->GetSelection() != 0) {
        s += "B_CHOICE_BufferTransform=";
        s += BufferTransform->GetStringSelection();
        s += ",";
    }

    if (SpinCtrl_BufferStagger->GetValue() != 0) {
        s += wxString::Format("B_SPINCTRL_BufferStagger=%d,", SpinCtrl_BufferStagger->GetValue());
    }

    wxString subB = subBufferPanel->GetValue();
    if (subB.size() > 0) {
        s += "B_CUSTOM_SubBuffer=";
        s += subB;
        s += ",";
    }

    if (BitmapButton_Blur->GetValue()->IsActive())
    {
        wxString blurVC = wxString(BitmapButton_Blur->GetValue()->Serialise().c_str());
        s += "B_VALUECURVE_Blur=";
        s += blurVC;
        s += ",";
    }
    else
    {
        // Blur
        if (Slider_EffectBlur->GetValue() > 1) {
            s += wxString::Format("B_SLIDER_Blur=%d,", Slider_EffectBlur->GetValue());
        }
    }
    if (BitmapButton_VCRotation->GetValue()->IsActive())
    {
        wxString rotationVC = wxString(BitmapButton_VCRotation->GetValue()->Serialise().c_str());
        s += "B_VALUECURVE_Rotation=";
        s += rotationVC;
        s += ",";
    }
    else
    {
        if (Slider_Rotation->GetValue() > 0) {
            s += wxString::Format("B_SLIDER_Rotation=%d,", Slider_Rotation->GetValue());
        }
    }
    if (BitmapButton_XRotation->GetValue()->IsActive())
    {
        wxString rotationXVC = wxString(BitmapButton_XRotation->GetValue()->Serialise().c_str());
        s += "B_VALUECURVE_XRotation=";
        s += rotationXVC;
        s += ",";
    }
    else
    {
        if (Slider_XRotation->GetValue() > 0) {
            s += wxString::Format("B_SLIDER_XRotation=%d,", Slider_XRotation->GetValue());
        }
    }
    if (BitmapButton_XPivot->GetValue()->IsActive())
    {
        wxString pivotXVC = wxString(BitmapButton_XPivot->GetValue()->Serialise().c_str());
        s += "B_VALUECURVE_XPivot=";
        s += pivotXVC;
        s += ",";
    }
    else
    {
        if (Slider_XPivot->GetValue() != 50) {
            s += wxString::Format("B_SLIDER_XPivot=%d,", Slider_XPivot->GetValue());
        }
    }
    if (BitmapButton_YRotation->GetValue()->IsActive())
    {
        wxString rotationYVC = wxString(BitmapButton_YRotation->GetValue()->Serialise().c_str());
        s += "B_VALUECURVE_YRotation=";
        s += rotationYVC;
        s += ",";
    }
    else
    {
        if (Slider_YRotation->GetValue() > 0) {
            s += wxString::Format("B_SLIDER_YRotation=%d,", Slider_YRotation->GetValue());
        }
    }
    if (BitmapButton_YPivot->GetValue()->IsActive())
    {
        wxString pivotYVC = wxString(BitmapButton_YPivot->GetValue()->Serialise().c_str());
        s += "B_VALUECURVE_YPivot=";
        s += pivotYVC;
        s += ",";
    }
    else
    {
        if (Slider_YPivot->GetValue() != 50) {
            s += wxString::Format("B_SLIDER_YPivot=%d,", Slider_YPivot->GetValue());
        }
    }
    if (BitmapButton_VCZoom->GetValue()->IsActive())
    {
        wxString zoomVC = wxString(BitmapButton_VCZoom->GetValue()->Serialise().c_str());
        s += "B_VALUECURVE_Zoom=";
        s += zoomVC;
        s += ",";
    }
    else
    {
        if (Slider_Zoom->GetValue() != 10) {
            s += wxString::Format("B_SLIDER_Zoom=%d,", Slider_Zoom->GetValue());
        }
    }
    if (BitmapButton_VCRotations->GetValue()->IsActive())
    {
        wxString rotationsVC = wxString(BitmapButton_VCRotations->GetValue()->Serialise().c_str());
        s += "B_VALUECURVE_Rotations=";
        s += rotationsVC;
        s += ",";
    }
    else
    {
        if (Slider_Rotations->GetValue() != 0) {
            s += wxString::Format("B_SLIDER_Rotations=%d,", Slider_Rotations->GetValue());
        }
    }
    if (Slider_ZoomQuality->GetValue() != 1) {
        s += wxString::Format("B_SLIDER_ZoomQuality=%d,", Slider_ZoomQuality->GetValue());
    }
    if (BitmapButton_VCPivotPointX->GetValue()->IsActive())
    {
        wxString pivotpointxVC = wxString(BitmapButton_VCPivotPointX->GetValue()->Serialise().c_str());
        s += "B_VALUECURVE_PivotPointX=";
        s += pivotpointxVC;
        s += ",";
    }
    else
    {
        if (Slider_PivotPointX->GetValue() != 50) {
            s += wxString::Format("B_SLIDER_PivotPointX=%d,", Slider_PivotPointX->GetValue());
        }
    }
    if (ChoiceRotateOrder->GetStringSelection() != "X-Y-Z")
    {
        s += "B_CHOICE_RZ_RotationOrder=";
        s += ChoiceRotateOrder->GetStringSelection();
        s += ",";
    }
    if (BitmapButton_VCPivotPointY->GetValue()->IsActive())
    {
        wxString pivotpointyVC = wxString(BitmapButton_VCPivotPointY->GetValue()->Serialise().c_str());
        s += "B_VALUECURVE_PivotPointY=";
        s += pivotpointyVC;
        s += ",";
    }
    else
    {
        if (Slider_PivotPointY->GetValue() != 50) {
            s += wxString::Format("B_SLIDER_PivotPointY=%d,", Slider_PivotPointY->GetValue());
        }
    }
    return s;
}

void BufferPanel::UpdateCamera(const Model* model)
{
    _defaultCamera = "2D";
    Choice_PerPreviewCamera->SetStringSelection("2D");
    if (model != nullptr) {
        auto mg = dynamic_cast<const ModelGroup*>(model);
        if (mg != nullptr) {
            Choice_PerPreviewCamera->SetStringSelection(mg->GetDefaultCamera());
            _defaultCamera = mg->GetDefaultCamera();
        }
    }

    _mg = (model->GetDisplayAs() == "ModelGroup");
}

void BufferPanel::UpdateBufferStyles(const Model* model)
{
    auto sel = BufferStyleChoice->GetStringSelection();
    BufferStyleChoice->Clear();
    if (model != nullptr) {
        const std::vector<std::string> &types = model->GetBufferStyles();
        for (const auto& it : types) {
            BufferStyleChoice->Append(it);
        }
    }
    if (BufferStyleChoice->IsEmpty()) {
        BufferStyleChoice->Append("Default");
    }
    BufferStyleChoice->SetStringSelection(sel);

    _mg = (model->GetDisplayAs() == "ModelGroup");
}

void BufferPanel::SetDefaultControls(const Model *model, bool optionbased) {
    if (!optionbased || CheckBox_ResetBufferPanel->GetValue())
    {
        Slider_EffectBlur->SetValue(1);
        CheckBox_OverlayBkg->SetValue(false);
        BufferStyleChoice->Clear();
        if (model != nullptr) {
            const std::vector<std::string> &types = model->GetBufferStyles();
            for (const auto& it : types) {
                BufferStyleChoice->Append(it);
            }
        }
        if (BufferStyleChoice->IsEmpty()) {
            BufferStyleChoice->Append("Default");
        }

        Choice_PerPreviewCamera->SetStringSelection("2D");
        if (model != nullptr) {
            _mg = (model->GetDisplayAs() == "ModelGroup");
            auto mg = dynamic_cast<const ModelGroup*>(model);
            if (mg != nullptr) {
                Choice_PerPreviewCamera->SetStringSelection(mg->GetDefaultCamera());
            }
        }

        subBufferPanel->SetDefaults();

        Slider_EffectBlur->SetValue(1);
        TextCtrl_EffectBlur->SetValue("1");
        BitmapButton_Blur->GetValue()->SetDefault(1.0f, 15.0f);
        BitmapButton_Blur->UpdateState();

        SpinCtrl_BufferStagger->SetValue(0);

        BufferStyleChoice->SetSelection(0);

        BufferTransform->SetSelection(0);

        Slider_Rotation->SetValue(0);
        TextCtrl_Rotation->SetValue("0");
        BitmapButton_VCRotation->GetValue()->SetDefault(0.0f, 100.0f);
        BitmapButton_VCRotation->UpdateState();

        Slider_XRotation->SetValue(0);
        TextCtrl_XRotation->SetValue("0");
        BitmapButton_XRotation->GetValue()->SetDefault(0.0f, 360.0f);
        BitmapButton_XRotation->UpdateState();

        Slider_XPivot->SetValue(50);
        TextCtrl_XPivot->SetValue("50");
        BitmapButton_XPivot->GetValue()->SetDefault(0.0f, 100.0f);
        BitmapButton_XPivot->UpdateState();

        Slider_YRotation->SetValue(0);
        TextCtrl_YRotation->SetValue("0");
        BitmapButton_YRotation->GetValue()->SetDefault(0.0f, 360.0f);
        BitmapButton_YRotation->UpdateState();

        Slider_YPivot->SetValue(50);
        TextCtrl_YPivot->SetValue("50");
        BitmapButton_YPivot->GetValue()->SetDefault(0.0f, 100.0f);
        BitmapButton_YPivot->UpdateState();

        Slider_Zoom->SetValue(10);
        TextCtrl_Zoom->SetValue("1.0");
        BitmapButton_VCZoom->GetValue()->SetDefault(0.0f, 30.0f);
        BitmapButton_VCZoom->GetValue()->SetDivisor(10);
        BitmapButton_VCZoom->UpdateState();

        Slider_Rotations->SetValue(0);
        TextCtrl_Rotations->SetValue("0.0");
        BitmapButton_VCRotations->GetValue()->SetDefault(0.0f, 200.0f);
        BitmapButton_VCRotations->GetValue()->SetDivisor(10);
        BitmapButton_VCRotations->UpdateState();

        Slider_ZoomQuality->SetValue(1);
        TextCtrl_ZoomQuality->SetValue("1");

        Slider_PivotPointX->SetValue(50);
        TextCtrl_PivotPointX->SetValue("50");
        BitmapButton_VCPivotPointX->GetValue()->SetDefault(0.0f, 100.0f);
        BitmapButton_VCPivotPointX->UpdateState();

        Slider_PivotPointY->SetValue(50);
        TextCtrl_PivotPointY->SetValue("50");
        BitmapButton_VCPivotPointY->GetValue()->SetDefault(0.0f, 100.0f);
        BitmapButton_VCPivotPointY->UpdateState();

        ChoiceRotateOrder->SetSelection(0);

        ValidateWindow();
    }
}

void BufferPanel::ValidateWindow()
{
    auto bs = BufferStyleChoice->GetStringSelection();
    if (bs == "Per Preview" ||
        bs == "Per Model Per Preview")
    {
        Choice_PerPreviewCamera->Enable();
    }
    else
    {
        Choice_PerPreviewCamera->Disable();
    }

    // Only some buffer shapes support stagger
    if (_mg &&
        (bs == "Horizontal Stack"
        || bs == "Vertical Stack"
        //|| bs == "Horizontal Stack - Scaled"
        //|| bs == "Vertical Stack - Scaled"
        //|| bs == "Horizontal Per Model"
        //|| bs == "Vertical Per Model"
        //|| bs == "Horizontal Per Model/Strand"
        //|| bs == "Vertical Per Model/Strand"
        ))
    {
        SpinCtrl_BufferStagger->Enable();
    }
    else
    {
        SpinCtrl_BufferStagger->Disable();
    }

    if (BitmapButton_Blur->GetValue()->IsActive())
    {
        Slider_EffectBlur->Disable();
        TextCtrl_EffectBlur->Disable();
    }
    else
    {
        Slider_EffectBlur->Enable();
        TextCtrl_EffectBlur->Enable();
    }

    if (BitmapButton_VCRotation->GetValue()->IsActive())
    {
        Slider_Rotation->Disable();
        TextCtrl_Rotation->Disable();
    }
    else
    {
        Slider_Rotation->Enable();
        TextCtrl_Rotation->Enable();
    }

    if (BitmapButton_VCZoom->GetValue()->IsActive())
    {
        Slider_Zoom->Disable();
        TextCtrl_Zoom->Disable();
    }
    else
    {
        Slider_Zoom->Enable();
        TextCtrl_Zoom->Enable();
    }

    if (BitmapButton_VCRotations->GetValue()->IsActive())
    {
        Slider_Rotations->Disable();
        TextCtrl_Rotations->Disable();
    }
    else
    {
        Slider_Rotations->Enable();
        TextCtrl_Rotations->Enable();
    }

    if (BitmapButton_VCPivotPointX->GetValue()->IsActive())
    {
        Slider_PivotPointX->Disable();
        TextCtrl_PivotPointX->Disable();
    }
    else
    {
        Slider_PivotPointX->Enable();
        TextCtrl_PivotPointX->Enable();
    }

    if (BitmapButton_VCPivotPointY->GetValue()->IsActive())
    {
        Slider_PivotPointY->Disable();
        TextCtrl_PivotPointY->Disable();
    }
    else
    {
        Slider_PivotPointY->Enable();
        TextCtrl_PivotPointY->Enable();
    }
}

void BufferPanel::OnBufferTransformSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void BufferPanel::OnChoice_PresetSelect(wxCommandEvent& event)
{
    wxString preset = Choice_Preset->GetStringSelection();

    Slider_Zoom->SetValue(10);
    TextCtrl_Zoom->SetValue("1.0");
    Slider_PivotPointX->SetValue(50);
    TextCtrl_PivotPointX->SetValue("50");
    Slider_PivotPointY->SetValue(50);
    TextCtrl_PivotPointY->SetValue("50");
    Slider_Rotation->SetValue(0);
    TextCtrl_Rotation->SetValue("0");
    Slider_Rotations->SetValue(0);
    TextCtrl_Rotations->SetValue("0.0");
    Slider_ZoomQuality->SetValue(1);
    TextCtrl_ZoomQuality->SetValue("1");
    BitmapButton_VCPivotPointX->GetValue()->SetDefault();
    BitmapButton_VCPivotPointY->GetValue()->SetDefault();
    BitmapButton_VCZoom->GetValue()->SetDefault();
    BitmapButton_VCRotation->GetValue()->SetDefault();
    BitmapButton_VCRotations->GetValue()->SetDefault();
    BitmapButton_VCPivotPointX->SetActive(false);
    BitmapButton_VCPivotPointY->SetActive(false);
    BitmapButton_VCZoom->SetActive(false);
    BitmapButton_VCRotation->SetActive(false);
    BitmapButton_VCRotations->SetActive(false);
    Slider_XPivot->SetValue(50);
    TextCtrl_XPivot->SetValue("50");
    Slider_YPivot->SetValue(50);
    TextCtrl_YPivot->SetValue("50");
    Slider_XRotation->SetValue(0);
    TextCtrl_XRotation->SetValue("0");
    Slider_YRotation->SetValue(0);
    TextCtrl_YRotation->SetValue("0");
    BitmapButton_XPivot->GetValue()->SetDefault();
    BitmapButton_YPivot->GetValue()->SetDefault();
    BitmapButton_XRotation->GetValue()->SetDefault();
    BitmapButton_YRotation->GetValue()->SetDefault();
    BitmapButton_XPivot->GetValue()->SetActive(false);
    BitmapButton_YPivot->GetValue()->SetActive(false);
    BitmapButton_XRotation->GetValue()->SetActive(false);
    BitmapButton_YRotation->GetValue()->SetActive(false);

    if (preset == "None - Reset")
    {
        // dont do anything
    }
    else if (preset == "1 Rev CW")
    {
        Slider_Rotations->SetValue(10);
        TextCtrl_Rotations->SetValue("1.0");
        BitmapButton_VCRotation->GetValue()->SetType("Ramp");
        BitmapButton_VCRotation->GetValue()->SetParameter1(0);
        BitmapButton_VCRotation->GetValue()->SetParameter2(100);
        BitmapButton_VCRotation->SetActive(true);
        Slider_ZoomQuality->SetValue(2);
    }
    else if (preset == "1 Rev CCW")
    {
        Slider_Rotations->SetValue(10);
        TextCtrl_Rotations->SetValue("1.0");
        BitmapButton_VCRotation->GetValue()->SetType("Ramp");
        BitmapButton_VCRotation->GetValue()->SetParameter1(100);
        BitmapButton_VCRotation->GetValue()->SetParameter2(0);
        BitmapButton_VCRotation->SetActive(true);
        Slider_ZoomQuality->SetValue(2);
    }
    else if (preset == "Explode")
    {
        BitmapButton_VCZoom->GetValue()->SetType("Ramp");
        BitmapButton_VCZoom->GetValue()->SetParameter1(0);
        BitmapButton_VCZoom->GetValue()->SetParameter2(10);
        BitmapButton_VCZoom->SetActive(true);
    }
    else if (preset == "Collapse")
    {
        BitmapButton_VCZoom->GetValue()->SetType("Ramp");
        BitmapButton_VCZoom->GetValue()->SetParameter1(10);
        BitmapButton_VCZoom->GetValue()->SetParameter2(0);
        BitmapButton_VCZoom->SetActive(true);
    }
    else if (preset == "Explode + Spin CW")
    {
        BitmapButton_VCZoom->GetValue()->SetType("Ramp");
        BitmapButton_VCZoom->GetValue()->SetParameter1(0);
        BitmapButton_VCZoom->GetValue()->SetParameter2(10);
        BitmapButton_VCZoom->SetActive(true);
        Slider_Rotations->SetValue(10);
        TextCtrl_Rotations->SetValue("1.0");
        BitmapButton_VCRotation->GetValue()->SetType("Ramp");
        BitmapButton_VCRotation->GetValue()->SetParameter1(0);
        BitmapButton_VCRotation->GetValue()->SetParameter2(100);
        BitmapButton_VCRotation->SetActive(true);
        Slider_ZoomQuality->SetValue(2);
        TextCtrl_ZoomQuality->SetValue("2");
    }
    else if (preset == "Shake")
    {
        Slider_Rotations->SetValue(10);
        TextCtrl_Rotations->SetValue("1.0");
        BitmapButton_VCRotation->GetValue()->SetType("Sine");
        BitmapButton_VCRotation->GetValue()->SetParameter1(0);
        BitmapButton_VCRotation->GetValue()->SetParameter2(10);
        BitmapButton_VCRotation->GetValue()->SetParameter3(50);
        BitmapButton_VCRotation->GetValue()->SetParameter4(25);
        BitmapButton_VCRotation->GetValue()->SetWrap(true);
        BitmapButton_VCRotation->SetActive(true);
        Slider_ZoomQuality->SetValue(2);
        TextCtrl_ZoomQuality->SetValue("2");
    }
    else if (preset == "Spin CW Accelerate")
    {
        BitmapButton_VCRotation->GetValue()->SetType("Ramp");
        BitmapButton_VCRotation->GetValue()->SetParameter1(0);
        BitmapButton_VCRotation->GetValue()->SetParameter2(100);
        BitmapButton_VCRotation->SetActive(true);
        BitmapButton_VCRotations->GetValue()->SetType("Ramp");
        BitmapButton_VCRotations->GetValue()->SetParameter1(0);
        BitmapButton_VCRotations->GetValue()->SetParameter2(10);
        BitmapButton_VCRotations->SetActive(true);
        Slider_ZoomQuality->SetValue(2);
        TextCtrl_ZoomQuality->SetValue("2");
    }
    Choice_Preset->SetStringSelection("");
    ChoiceRotateOrder->SetSelection(0);
    ValidateWindow();
}

void BufferPanel::OnButton_ResetClick(wxCommandEvent& event)
{
    subBufferPanel->SetDefaults();
    subBufferPanel->Refresh();
    Refresh();
}

void BufferPanel::OnCheckBox_ResetBufferPanelClick(wxCommandEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsResetBufferPanel", CheckBox_ResetBufferPanel->IsChecked());
}

bool BufferPanel::CanRenderBufferUseCamera(const std::string& rb)
{
    return (rb == "Per Preview" || rb == "Per Model Per Preview");
}

void BufferPanel::OnBufferStyleChoiceSelect(wxCommandEvent& event)
{
    auto bs = BufferStyleChoice->GetStringSelection();

    if (BufferPanel::CanRenderBufferUseCamera(bs))
    {
        Choice_PerPreviewCamera->Clear();

        Choice_PerPreviewCamera->Append("2D");

        // load the camera positions
        xLightsFrame* frame = xLightsApp::GetFrame();
        for (size_t i = 0; i < frame->viewpoint_mgr.GetNum3DCameras(); ++i)
        {
            Choice_PerPreviewCamera->Append(frame->viewpoint_mgr.GetCamera3D(i)->GetName());
        }

        Choice_PerPreviewCamera->SetStringSelection(_defaultCamera);
        if (Choice_PerPreviewCamera->GetStringSelection() != _defaultCamera) {
            Choice_PerPreviewCamera->SetStringSelection("2D");
        }
    }
    else
    {
        Choice_PerPreviewCamera->SetStringSelection("2D");
    }

    ValidateWindow();
}
