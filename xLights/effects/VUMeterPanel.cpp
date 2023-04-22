/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(VUMeterPanel)
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

#include "VUMeterPanel.h"
#include "EffectPanelUtils.h"
#include "VUMeterEffect.h"
#include "UtilFunctions.h"

//(*IdInit(VUMeterPanel)
const long VUMeterPanel::ID_STATICTEXT_VUMeter_Bars = wxNewId();
const long VUMeterPanel::ID_SLIDER_VUMeter_Bars = wxNewId();
const long VUMeterPanel::IDD_TEXTCTRL_VUMeter_Bars = wxNewId();
const long VUMeterPanel::ID_BITMAPBUTTON_SLIDER_VUMeter_Bars = wxNewId();
const long VUMeterPanel::ID_STATICTEXT_VUMeter_Type = wxNewId();
const long VUMeterPanel::ID_CHOICE_VUMeter_Type = wxNewId();
const long VUMeterPanel::ID_STATICTEXT5 = wxNewId();
const long VUMeterPanel::ID_BITMAPBUTTON_CHOICE_VUMeter_Type = wxNewId();
const long VUMeterPanel::ID_STATICTEXT_VUMeter_TimingTrack = wxNewId();
const long VUMeterPanel::ID_CHOICE_VUMeter_TimingTrack = wxNewId();
const long VUMeterPanel::ID_STATICTEXT4 = wxNewId();
const long VUMeterPanel::ID_BITMAPBUTTON_CHOICE_VUMeter_TimingTrack = wxNewId();
const long VUMeterPanel::ID_STATICTEXT2 = wxNewId();
const long VUMeterPanel::ID_TEXTCTRL_Filter = wxNewId();
const long VUMeterPanel::ID_CHECKBOX_Regex = wxNewId();
const long VUMeterPanel::ID_STATICTEXT_VUMeter_Sensitivity = wxNewId();
const long VUMeterPanel::ID_SLIDER_VUMeter_Sensitivity = wxNewId();
const long VUMeterPanel::IDD_TEXTCTRL_VUMeter_Sensitivity = wxNewId();
const long VUMeterPanel::ID_BITMAPBUTTON_SLIDER_VUMeter_Sensitivity = wxNewId();
const long VUMeterPanel::ID_STATICTEXT1 = wxNewId();
const long VUMeterPanel::ID_SLIDER_VUMeter_Gain = wxNewId();
const long VUMeterPanel::ID_VALUECURVE_VUMeter_Gain = wxNewId();
const long VUMeterPanel::ID_TEXTCTRL_VUMeter_Gain = wxNewId();
const long VUMeterPanel::ID_STATICTEXT_VUMeter_Shape = wxNewId();
const long VUMeterPanel::ID_CHOICE_VUMeter_Shape = wxNewId();
const long VUMeterPanel::ID_STATICTEXT8 = wxNewId();
const long VUMeterPanel::ID_BITMAPBUTTON_CHOICE_VUMeter_Shape = wxNewId();
const long VUMeterPanel::ID_STATICTEXT3 = wxNewId();
const long VUMeterPanel::ID_FILEPICKERCTRL_SVGFile = wxNewId();
const long VUMeterPanel::ID_STATICTEXT9 = wxNewId();
const long VUMeterPanel::ID_CHECKBOX_VUMeter_SlowDownFalls = wxNewId();
const long VUMeterPanel::ID_STATICTEXT10 = wxNewId();
const long VUMeterPanel::ID_BITMAPBUTTON_CHECKBOX_VUMeter_SlowDownFalls = wxNewId();
const long VUMeterPanel::ID_STATICTEXT_VUMeter_StartNote = wxNewId();
const long VUMeterPanel::ID_SLIDER_VUMeter_StartNote = wxNewId();
const long VUMeterPanel::IDD_TEXTCTRL_VUMeter_StartNote = wxNewId();
const long VUMeterPanel::ID_BITMAPBUTTON_SLIDER_VUMeter_StartNote = wxNewId();
const long VUMeterPanel::ID_STATICTEXT_VUMeter_EndNote = wxNewId();
const long VUMeterPanel::ID_SLIDER_VUMeter_EndNote = wxNewId();
const long VUMeterPanel::IDD_TEXTCTRL_VUMeter_EndNote = wxNewId();
const long VUMeterPanel::ID_BITMAPBUTTON_SLIDER_VUMeter_EndNote = wxNewId();
const long VUMeterPanel::ID_CHECKBOX_VUMeter_LogarithmicX = wxNewId();
const long VUMeterPanel::ID_STATICTEXT_VUMeter_XOffset = wxNewId();
const long VUMeterPanel::ID_SLIDER_VUMeter_XOffset = wxNewId();
const long VUMeterPanel::IDD_TEXTCTRL_VUMeter_XOffset = wxNewId();
const long VUMeterPanel::ID_BITMAPBUTTON_SLIDER_VUMeter_XOffset = wxNewId();
const long VUMeterPanel::ID_STATICTEXT_VUMeter_YOffset = wxNewId();
const long VUMeterPanel::ID_SLIDER_VUMeter_YOffset = wxNewId();
const long VUMeterPanel::ID_VALUECURVE_VUMeter_YOffset = wxNewId();
const long VUMeterPanel::IDD_TEXTCTRL_VUMeter_YOffset = wxNewId();
const long VUMeterPanel::ID_BITMAPBUTTON_SLIDER_VUMeter_YOffset = wxNewId();
//*)

BEGIN_EVENT_TABLE(VUMeterPanel,wxPanel)
	//(*EventTable(VUMeterPanel)
	//*)
END_EVENT_TABLE()

VUMeterPanel::VUMeterPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(VUMeterPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer31;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer42;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer42->AddGrowableCol(0);
	FlexGridSizer31 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer31->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_VUMeter_Bars, _("Bars"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_VUMeter_Bars"));
	FlexGridSizer31->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_VUMeter_Bars = new BulkEditSlider(this, ID_SLIDER_VUMeter_Bars, 20, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_VUMeter_Bars"));
	FlexGridSizer4->Add(Slider_VUMeter_Bars, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
	TextCtrl_VUMeter_Bars = new BulkEditTextCtrl(this, IDD_TEXTCTRL_VUMeter_Bars, _("20"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_VUMeter_Bars"));
	FlexGridSizer31->Add(TextCtrl_VUMeter_Bars, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VUMeter_Bars = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_VUMeter_Bars, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_VUMeter_Bars"));
	BitmapButton_VUMeter_Bars->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_VUMeter_Bars, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT_VUMeter_Type, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_VUMeter_Type"));
	FlexGridSizer31->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_VUMeter_Type = new BulkEditChoice(this, ID_CHOICE_VUMeter_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_VUMeter_Type"));
	FlexGridSizer31->Add(Choice_VUMeter_Type, 1, wxALL|wxEXPAND, 2);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer31->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_VUMeter_Type = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_VUMeter_Type, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_VUMeter_Type"));
	BitmapButton_VUMeter_Type->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_VUMeter_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT_VUMeter_TimingTrack, _("Timing Track"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_VUMeter_TimingTrack"));
	FlexGridSizer31->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_VUMeter_TimingTrack = new BulkEditChoice(this, ID_CHOICE_VUMeter_TimingTrack, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_VUMeter_TimingTrack"));
	Choice_VUMeter_TimingTrack->Disable();
	FlexGridSizer31->Add(Choice_VUMeter_TimingTrack, 1, wxALL|wxEXPAND, 2);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer31->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_VUMeter_TimingTrack = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_VUMeter_TimingTrack, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_VUMeter_TimingTrack"));
	BitmapButton_VUMeter_TimingTrack->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_VUMeter_TimingTrack, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText16 = new wxStaticText(this, ID_STATICTEXT2, _("Filter Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer31->Add(StaticText16, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Filter = new wxTextCtrl(this, ID_TEXTCTRL_Filter, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Filter"));
	FlexGridSizer31->Add(TextCtrl_Filter, 1, wxALL|wxEXPAND, 2);
	CheckBox_Regex = new wxCheckBox(this, ID_CHECKBOX_Regex, _("Regex"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Regex"));
	CheckBox_Regex->SetValue(false);
	FlexGridSizer31->Add(CheckBox_Regex, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT_VUMeter_Sensitivity, _("Sensitivity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_VUMeter_Sensitivity"));
	FlexGridSizer31->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(2, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_VUMeter_Sensitivity = new BulkEditSlider(this, ID_SLIDER_VUMeter_Sensitivity, 70, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_VUMeter_Sensitivity"));
	FlexGridSizer3->Add(Slider_VUMeter_Sensitivity, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	TextCtrl_VUMeter_Sensitivity = new BulkEditTextCtrl(this, IDD_TEXTCTRL_VUMeter_Sensitivity, _("70"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_VUMeter_Sensitivity"));
	FlexGridSizer31->Add(TextCtrl_VUMeter_Sensitivity, 1, wxALL|wxEXPAND, 5);
	BitmapButton_VUMeter_Sensitivity = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_VUMeter_Sensitivity, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_VUMeter_Sensitivity"));
	BitmapButton_VUMeter_Sensitivity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_VUMeter_Sensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText15 = new wxStaticText(this, ID_STATICTEXT1, _("Gain"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer31->Add(StaticText15, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	Slider_VUMeter_Gain = new BulkEditSlider(this, ID_SLIDER_VUMeter_Gain, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_VUMeter_Gain"));
	FlexGridSizer5->Add(Slider_VUMeter_Gain, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VUMeter_Gain = new BulkEditValueCurveButton(this, ID_VALUECURVE_VUMeter_Gain, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_VUMeter_Gain"));
	FlexGridSizer5->Add(BitmapButton_VUMeter_Gain, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer31->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 2);
	TextCtrl_VUMeter_Gain = new BulkEditTextCtrl(this, ID_TEXTCTRL_VUMeter_Gain, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_VUMeter_Gain"));
	FlexGridSizer31->Add(TextCtrl_VUMeter_Gain, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT_VUMeter_Shape, _("Shape"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_VUMeter_Shape"));
	FlexGridSizer31->Add(StaticText7, 1, wxALL, 2);
	Choice_VUMeter_Shape = new BulkEditChoice(this, ID_CHOICE_VUMeter_Shape, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_VUMeter_Shape"));
	Choice_VUMeter_Shape->SetSelection( Choice_VUMeter_Shape->Append(_("Circle")) );
	Choice_VUMeter_Shape->Append(_("Filled Circle"));
	Choice_VUMeter_Shape->Append(_("Square"));
	Choice_VUMeter_Shape->Append(_("Filled Square"));
	Choice_VUMeter_Shape->Append(_("Diamond"));
	Choice_VUMeter_Shape->Append(_("Filled Diamond"));
	Choice_VUMeter_Shape->Append(_("Star"));
	Choice_VUMeter_Shape->Append(_("Filled Star"));
	Choice_VUMeter_Shape->Append(_("Tree"));
	Choice_VUMeter_Shape->Append(_("Filled Tree"));
	Choice_VUMeter_Shape->Append(_("Crucifix"));
	Choice_VUMeter_Shape->Append(_("Filled Crucifix"));
	Choice_VUMeter_Shape->Append(_("Present"));
	Choice_VUMeter_Shape->Append(_("Filled Present"));
	Choice_VUMeter_Shape->Append(_("Candy Cane"));
	Choice_VUMeter_Shape->Append(_("Snowflake"));
	Choice_VUMeter_Shape->Append(_("Heart"));
	Choice_VUMeter_Shape->Append(_("Filled Heart"));
	Choice_VUMeter_Shape->Append(_("SVG"));
	FlexGridSizer31->Add(Choice_VUMeter_Shape, 1, wxALL|wxEXPAND, 2);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer31->Add(StaticText8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_VUMeter_Shape = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_VUMeter_Shape, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_VUMeter_Shape"));
	BitmapButton_VUMeter_Shape->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_VUMeter_Shape, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText17 = new wxStaticText(this, ID_STATICTEXT3, _("SVG File"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer31->Add(StaticText17, 1, wxALL|wxEXPAND, 2);
	FilePickerCtrl_SVGFile = new BulkEditFilePickerCtrl(this, ID_FILEPICKERCTRL_SVGFile, wxEmptyString, _("Select a file"), _T("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL_SVGFile"));
	FlexGridSizer31->Add(FilePickerCtrl_SVGFile, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer31->Add(0,0,1, wxALL|wxEXPAND, 5);
	FlexGridSizer31->Add(0,0,1, wxALL|wxEXPAND, 5);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer31->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_VUMeter_SlowDownFalls = new BulkEditCheckBox(this, ID_CHECKBOX_VUMeter_SlowDownFalls, _("Slow Down Falls"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_VUMeter_SlowDownFalls"));
	CheckBox_VUMeter_SlowDownFalls->SetValue(true);
	FlexGridSizer31->Add(CheckBox_VUMeter_SlowDownFalls, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer31->Add(StaticText10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_VUMeter_SlowDownFalls = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_VUMeter_SlowDownFalls, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_VUMeter_SlowDownFalls"));
	BitmapButton_VUMeter_SlowDownFalls->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_VUMeter_SlowDownFalls, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText12 = new wxStaticText(this, ID_STATICTEXT_VUMeter_StartNote, _("Start Note"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_VUMeter_StartNote"));
	FlexGridSizer31->Add(StaticText12, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_VUMeter_StartNote = new BulkEditSlider(this, ID_SLIDER_VUMeter_StartNote, 36, 0, 127, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_VUMeter_StartNote"));
	FlexGridSizer31->Add(Slider_VUMeter_StartNote, 1, wxALL|wxEXPAND, 2);
	TextCtrl_VUMeter_StartNote = new BulkEditTextCtrl(this, IDD_TEXTCTRL_VUMeter_StartNote, _("36"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_VUMeter_StartNote"));
	FlexGridSizer31->Add(TextCtrl_VUMeter_StartNote, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VUMeter_StartNote = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_VUMeter_StartNote, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_VUMeter_StartNote"));
	BitmapButton_VUMeter_StartNote->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_VUMeter_StartNote, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT_VUMeter_EndNote, _("End Note"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_VUMeter_EndNote"));
	FlexGridSizer31->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_VUMeter_EndNote = new BulkEditSlider(this, ID_SLIDER_VUMeter_EndNote, 84, 0, 127, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_VUMeter_EndNote"));
	FlexGridSizer31->Add(Slider_VUMeter_EndNote, 1, wxALL|wxEXPAND, 2);
	TextCtrl_VUMeter_EndNote = new BulkEditTextCtrl(this, IDD_TEXTCTRL_VUMeter_EndNote, _("84"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_VUMeter_EndNote"));
	FlexGridSizer31->Add(TextCtrl_VUMeter_EndNote, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VUMeter_EndNote = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_VUMeter_EndNote, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_VUMeter_EndNote"));
	BitmapButton_VUMeter_EndNote->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_VUMeter_EndNote, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	CheckBox_LogarithmicXAxis = new BulkEditCheckBox(this, ID_CHECKBOX_VUMeter_LogarithmicX, _("Logarithmic X axis"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_VUMeter_LogarithmicX"));
	CheckBox_LogarithmicXAxis->SetValue(false);
	FlexGridSizer31->Add(CheckBox_LogarithmicXAxis, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	StaticText13 = new wxStaticText(this, ID_STATICTEXT_VUMeter_XOffset, _("Horizontal Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_VUMeter_XOffset"));
	FlexGridSizer31->Add(StaticText13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_VUMeter_XOffset = new BulkEditSlider(this, ID_SLIDER_VUMeter_XOffset, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_VUMeter_XOffset"));
	FlexGridSizer2->Add(Slider_VUMeter_XOffset, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	TextCtrl_VUMeter_XOffset = new BulkEditTextCtrl(this, IDD_TEXTCTRL_VUMeter_XOffset, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_VUMeter_XOffset"));
	FlexGridSizer31->Add(TextCtrl_VUMeter_XOffset, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VUMeter_XOffset = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_VUMeter_XOffset, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_VUMeter_XOffset"));
	BitmapButton_VUMeter_XOffset->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_VUMeter_XOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText14 = new wxStaticText(this, ID_STATICTEXT_VUMeter_YOffset, _("Vertical Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_VUMeter_YOffset"));
	FlexGridSizer31->Add(StaticText14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_VUMeter_YOffset = new BulkEditSlider(this, ID_SLIDER_VUMeter_YOffset, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_VUMeter_YOffset"));
	FlexGridSizer1->Add(Slider_VUMeter_YOffset, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VUMeter_YOffsetVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_VUMeter_YOffset, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_VUMeter_YOffset"));
	FlexGridSizer1->Add(BitmapButton_VUMeter_YOffsetVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer31->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
	TextCtrl_VUMeter_YOffset = new BulkEditTextCtrl(this, IDD_TEXTCTRL_VUMeter_YOffset, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_VUMeter_YOffset"));
	FlexGridSizer31->Add(TextCtrl_VUMeter_YOffset, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VUMeter_YOffset = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_VUMeter_YOffset, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_VUMeter_YOffset"));
	BitmapButton_VUMeter_YOffset->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_VUMeter_YOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer42->Add(FlexGridSizer31, 1, wxEXPAND, 2);
	SetSizer(FlexGridSizer42);
	FlexGridSizer42->Fit(this);
	FlexGridSizer42->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_VUMeter_Bars,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnLockButtonClick);
	Connect(ID_CHOICE_VUMeter_Type,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&VUMeterPanel::OnChoice_VUMeter_TypeSelect);
	Connect(ID_BITMAPBUTTON_CHOICE_VUMeter_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_VUMeter_TimingTrack,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_VUMeter_Sensitivity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_VUMeter_Gain,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnVCButtonClick);
	Connect(ID_CHOICE_VUMeter_Shape,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&VUMeterPanel::OnChoice_VUMeter_TypeSelect);
	Connect(ID_BITMAPBUTTON_CHOICE_VUMeter_Shape,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_VUMeter_SlowDownFalls,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnLockButtonClick);
	Connect(ID_SLIDER_VUMeter_StartNote,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&VUMeterPanel::OnSlider_VUMeter_StartNoteCmdSliderUpdated);
	Connect(IDD_TEXTCTRL_VUMeter_StartNote,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&VUMeterPanel::OnTextCtrl_VUMeter_StartNoteText);
	Connect(ID_BITMAPBUTTON_SLIDER_VUMeter_StartNote,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnLockButtonClick);
	Connect(ID_SLIDER_VUMeter_EndNote,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&VUMeterPanel::OnSlider_VUMeter_StartNoteCmdSliderUpdated);
	Connect(IDD_TEXTCTRL_VUMeter_EndNote,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&VUMeterPanel::OnTextCtrl_VUMeter_StartNoteText);
	Connect(ID_BITMAPBUTTON_SLIDER_VUMeter_EndNote,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_VUMeter_XOffset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_VUMeter_YOffset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_VUMeter_YOffset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnLockButtonClick);
	//*)
    
    SetName("ID_PANEL_VUMeter");

    Choice_VUMeter_Type->Append(_("Spectrogram"));
    Choice_VUMeter_Type->Append(_("Spectrogram Peak"));
    Choice_VUMeter_Type->Append(_("Spectrogram Line"));
    Choice_VUMeter_Type->Append(_("Spectrogram Circle Line"));
    Choice_VUMeter_Type->Append(_("Volume Bars"));
    Choice_VUMeter_Type->Append(_("Waveform"));
    Choice_VUMeter_Type->Append(_("Frame Waveform"));
    Choice_VUMeter_Type->Append(_("On"));
    Choice_VUMeter_Type->Append(_("Color On"));
    Choice_VUMeter_Type->Append(_("Dominant Frequency Colour"));
    Choice_VUMeter_Type->Append(_("Dominant Frequency Colour Gradient"));
    Choice_VUMeter_Type->Append(_("Intensity Wave"));
    Choice_VUMeter_Type->Append(_("Pulse"));
    Choice_VUMeter_Type->Append(_("Level Bar"));
    Choice_VUMeter_Type->Append(_("Level Random Bar"));
    Choice_VUMeter_Type->Append(_("Level Color"));
    Choice_VUMeter_Type->Append(_("Level Pulse"));
    Choice_VUMeter_Type->Append(_("Level Jump"));
    Choice_VUMeter_Type->Append(_("Level Jump 100"));
    Choice_VUMeter_Type->Append(_("Level Pulse Color"));
    Choice_VUMeter_Type->Append(_("Level Shape"));
    Choice_VUMeter_Type->Append(_("Timing Event Bar"));
    Choice_VUMeter_Type->Append(_("Timing Event Random Bar"));
    Choice_VUMeter_Type->Append(_("Timing Event Bars"));
    Choice_VUMeter_Type->Append(_("Timing Event Spike"));
    Choice_VUMeter_Type->Append(_("Timing Event Sweep"));
    Choice_VUMeter_Type->Append(_("Timing Event Sweep 2"));
    Choice_VUMeter_Type->Append(_("Timing Event Timed Sweep"));
    Choice_VUMeter_Type->Append(_("Timing Event Timed Sweep 2"));
    Choice_VUMeter_Type->Append(_("Timing Event Alternate Timed Sweep"));
    Choice_VUMeter_Type->Append(_("Timing Event Alternate Timed Sweep 2"));
    Choice_VUMeter_Type->Append(_("Timing Event Timed Chase From Middle"));
    Choice_VUMeter_Type->Append(_("Timing Event Timed Chase To Middle"));
    Choice_VUMeter_Type->Append(_("Timing Event Color"));
    Choice_VUMeter_Type->Append(_("Timing Event Jump"));
    Choice_VUMeter_Type->Append(_("Timing Event Jump 100"));
    Choice_VUMeter_Type->Append(_("Timing Event Pulse"));
    Choice_VUMeter_Type->Append(_("Timing Event Pulse Color"));
    Choice_VUMeter_Type->Append(_("Note On"));
    Choice_VUMeter_Type->Append(_("Note Level Pulse"));
    Choice_VUMeter_Type->Append(_("Note Level Jump"));
    Choice_VUMeter_Type->Append(_("Note Level Jump 100"));
    Choice_VUMeter_Type->Append(_("Note Level Bar"));
    Choice_VUMeter_Type->Append(_("Note Level Random Bar"));

    Choice_VUMeter_Type->SetStringSelection(_("Waveform"));

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&VUMeterPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&VUMeterPanel::OnValidateWindow, 0, this);

    TextCtrl_VUMeter_StartNote->Bind(wxEVT_KILL_FOCUS, (wxObjectEventFunction)&VUMeterPanel::OnTextCtrl_VUMeter_StartNoteKillFocus, this);
    TextCtrl_VUMeter_EndNote->Bind(wxEVT_KILL_FOCUS, (wxObjectEventFunction)&VUMeterPanel::OnTextCtrl_VUMeter_StartNoteKillFocus, this);

    TextCtrl_VUMeter_StartNote->Bind(wxEVT_TEXT_ENTER, (wxObjectEventFunction)&VUMeterPanel::OnTextCtrl_VUMeter_StartNoteEnter, this);
    TextCtrl_VUMeter_EndNote->Bind(wxEVT_TEXT_ENTER, (wxObjectEventFunction)&VUMeterPanel::OnTextCtrl_VUMeter_StartNoteEnter, this);

    BitmapButton_VUMeter_YOffsetVC->GetValue()->SetLimits(VUMETER_OFFSET_MIN, VUMETER_OFFSET_MAX);
    BitmapButton_VUMeter_Gain->GetValue()->SetLimits(VUMETER_GAIN_MIN, VUMETER_GAIN_MAX);

    TextCtrl_Filter->SetToolTip("Only trigger on timing events which contain this token in their text. Blank matches all.");

	ValidateWindow();
}

VUMeterPanel::~VUMeterPanel()
{
	//(*Destroy(VUMeterPanel)
	//*)
}

void VUMeterPanel::ValidateWindow()
{
    auto type = Choice_VUMeter_Type->GetStringSelection();

    if (type == "Spectrogram" || 
        type == "Spectrogram Peak" ||
        type == "Spectrogram Circle Line" ||
        type == "Spectrogram Line")
    {
        CheckBox_LogarithmicXAxis->Enable();
    }
    else
    {
        CheckBox_LogarithmicXAxis->Disable();
    }

    if (type == "Volume Bars" ||
        type == "Waveform" ||
        type == "Frame Waveform" ||
        type == "On" ||
        type == "Color On" ||
        type == "Intensity Wave" ||
        type == "Level Bar" ||
        type == "Level Random Bar" ||
        type == "Level Color" ||
        type == "Level Pulse" || 
        type == "Level Jump" ||
        type == "Level Jump 100" ||
        type == "Level Pulse Color" ||
        type == "Timing Event Jump" ||
        type == "Note On" ||
        type == "Note Level Bar" ||
        type == "Note Level Random Bar" ||
        type == "Note Level Pulse" ||
        type == "Note Level Jump" ||
        type == "Note Level Jump 100" ||
        type == "Spectrogram Circle Line" ||
        type == "Level Shape")
    {
        Slider_VUMeter_Gain->Enable();
        TextCtrl_VUMeter_Gain->Enable();
        BitmapButton_VUMeter_Gain->Enable();
    }
    else
    {
        Slider_VUMeter_Gain->Disable();
        TextCtrl_VUMeter_Gain->Disable();
        BitmapButton_VUMeter_Gain->Disable();
    }

    if (type == "Timing Event Spike" ||
        type == "Timing Event Sweep" ||
        type == "Timing Event Sweep 2" ||
        type == "Timing Event Timed Sweep" ||
        type == "Timing Event Timed Sweep 2" ||
        type == "Timing Event Alternate Timed Sweep" ||
        type == "Timing Event Alternate Timed Sweep 2" ||
        type == "Timing Event Timed Chase To Middle" ||
        type == "Timing Event Timed Chase From Middle" ||
        type == "Pulse" ||
        type == "Timing Event Color" ||
        type == "Timing Event Pulse" ||
        type == "Timing Event Bar" ||
        type == "Timing Event Random Bar" ||
        type == "Timing Event Bars" ||
        type == "Timing Event Jump 100" ||
        type == "Timing Event Pulse Color" ||
        type == "Timing Event Jump")
    {
        Choice_VUMeter_TimingTrack->Enable();
        CheckBox_Regex->Enable();
        TextCtrl_Filter->Enable();
    }
    else
    {
        Choice_VUMeter_TimingTrack->Disable();
        CheckBox_Regex->Disable();
        TextCtrl_Filter->Disable();
    }

    if (type == "Level Pulse" ||
        type == "Level Jump" ||
        type == "Level Jump 100" ||
        type == "Level Pulse Color" ||
        type == "Level Shape" ||
        type == "Level Bar" ||
        type == "Level Random Bar" ||
        type == "Level Color" ||
        type == "Note Level Bar" ||
        type == "Note Level Random Bar" ||
        type == "Spectrogram Peak" ||
        type == "Spectrogram Line" ||
        type == "Spectrogram Circle Line" ||
        type == "Note Level Jump" ||
        type == "Note Level Jump 100" ||
        type == "Note Level Pulse" ||
        type == "Timing Event Color" ||
        type == "Dominant Frequency Colour" ||
        type == "Dominant Frequency Colour Gradient"
        )
    {
        Slider_VUMeter_Sensitivity->Enable();
        TextCtrl_VUMeter_Sensitivity->Enable();
    }
    else
    {
        Slider_VUMeter_Sensitivity->Disable();
        TextCtrl_VUMeter_Sensitivity->Disable();
    }

    if (type == "Level Shape" ||
        type == "Spectrogram" ||
        type == "Spectrogram Line" ||
        type == "Spectrogram Circle Line" ||
        type == "Spectrogram Peak")
    {
        CheckBox_VUMeter_SlowDownFalls->Enable();
    }
    else
    {
        CheckBox_VUMeter_SlowDownFalls->Disable();
    }

    if (type == "Level Shape")
    {
        Choice_VUMeter_Shape->Enable();
        auto shape = Choice_VUMeter_Shape->GetStringSelection();
        if (shape == "Star" || 
            shape == "Filled Star" || 
			shape == "Snowflake")
        {
            // we use bars to set number of points
            Slider_VUMeter_Bars->Enable();
            TextCtrl_VUMeter_Bars->Enable();
        }
        else
        {
            Slider_VUMeter_Bars->Disable();
            TextCtrl_VUMeter_Bars->Disable();
        }

        if (shape == "SVG")
        {
            FilePickerCtrl_SVGFile->Enable();
        }
        else
        {
            FilePickerCtrl_SVGFile->Disable();
        }
    }
    else
    {
        FilePickerCtrl_SVGFile->Disable();
        Choice_VUMeter_Shape->Disable();

        if (type == "On" ||
            type == "Color On" ||
            type == "Timing Event Color" ||
            type == "Level Color" ||
            type == "Note On")
        {
            Slider_VUMeter_Bars->Disable();
            TextCtrl_VUMeter_Bars->Disable();
        }
        else
        {
            Slider_VUMeter_Bars->Enable();
            TextCtrl_VUMeter_Bars->Enable();
        }
    }

    if (type == "Spectrogram" ||
        type == "Spectrogram Line" ||
        type == "Spectrogram Circle Line" ||
        type == "Spectrogram Peak" ||
        type == "Note On" ||
        type == "Note Level Bar" ||
        type == "Note Level Random Bar" ||
        type == "Note Level Jump" ||
        type == "Note Level Jump 100" ||
        type == "Note Level Pulse" ||
        type == "Dominant Frequency Colour" ||
        type == "Dominant Frequency Colour Gradient"
        )
    {
        Slider_VUMeter_EndNote->Enable();
        Slider_VUMeter_StartNote->Enable();
        TextCtrl_VUMeter_EndNote->Enable();
        TextCtrl_VUMeter_StartNote->Enable();
    }
    else
    {
        Slider_VUMeter_EndNote->Disable();
        Slider_VUMeter_StartNote->Disable();
        TextCtrl_VUMeter_EndNote->Disable();
        TextCtrl_VUMeter_StartNote->Disable();
    }

    if (type == "Spectrogram" || 
        type == "Spectrogram Line" ||
        type == "Spectrogram Circle Line" ||
        type == "Spectrogram Peak" ||
        type == "Level Shape")
    {
        Slider_VUMeter_XOffset->Enable();
        TextCtrl_VUMeter_XOffset->Enable();
    }
    else
    {
        Slider_VUMeter_XOffset->Disable();
        TextCtrl_VUMeter_XOffset->Disable();
    }

    if (type == "Level Shape" ||
        type == "Spectrogram Circle Line" ||
        type == "Frame Waveform" ||
        type == "Waveform")
    {
        Slider_VUMeter_YOffset->Enable();
        TextCtrl_VUMeter_YOffset->Enable();
        BitmapButton_VUMeter_YOffsetVC->Enable();
    }
    else
    {
        BitmapButton_VUMeter_YOffsetVC->Disable();
        Slider_VUMeter_YOffset->Disable();
        TextCtrl_VUMeter_YOffset->Disable();
    }

    Slider_VUMeter_StartNote->SetToolTip(wxString(DecodeMidi(Slider_VUMeter_StartNote->GetValue()).c_str()));
    Slider_VUMeter_EndNote->SetToolTip(wxString(DecodeMidi(Slider_VUMeter_EndNote->GetValue()).c_str()));
}

void VUMeterPanel::OnChoice_VUMeter_TypeSelect(wxCommandEvent& event)
{
	ValidateWindow();
}

void VUMeterPanel::OnSlider_VUMeter_StartNoteCmdSliderUpdated(wxScrollEvent& event)
{
    int start = Slider_VUMeter_StartNote->GetValue();
    int end = Slider_VUMeter_EndNote->GetValue();
    if (event.GetEventObject() == Slider_VUMeter_StartNote)
    {
        if (end < start)
        {
            end = start;
        }
    }
    else
    {
        if (end < start)
        {
            start = end;
        }
    }

    if (end != Slider_VUMeter_EndNote->GetValue())
    {
        Slider_VUMeter_EndNote->SetValue(end);
    }
    wxString e = wxString::Format("%d", end);
    if (e != TextCtrl_VUMeter_EndNote->GetValue())
    {
        TextCtrl_VUMeter_EndNote->SetValue(e);
    }
    if (start != Slider_VUMeter_StartNote->GetValue())
    {
        Slider_VUMeter_StartNote->SetValue(start);
    }
    wxString s = wxString::Format("%d", start);
    if (s != TextCtrl_VUMeter_StartNote->GetValue())
    {
        TextCtrl_VUMeter_StartNote->SetValue(s);
    }
    ValidateWindow();
}

void VUMeterPanel::OnTextCtrl_VUMeter_StartNoteText(wxCommandEvent& event)
{
}

void VUMeterPanel::OnTextCtrl_VUMeter_StartNoteEnter(wxCommandEvent& event)
{
    ApplyText(event);
}

void VUMeterPanel::ApplyText(wxEvent& event)
{
    int start = wxAtoi(TextCtrl_VUMeter_StartNote->GetValue());
    int end = wxAtoi(TextCtrl_VUMeter_EndNote->GetValue());
    if (event.GetEventObject() == TextCtrl_VUMeter_StartNote) {
        if (end < start) {
            end = start;
        }
    }
    else {
        if (end < start) {
            start = end;
        }
    }

    if (end != Slider_VUMeter_EndNote->GetValue()) {
        Slider_VUMeter_EndNote->SetValue(end);
    }
    wxString e = wxString::Format("%d", end);
    if (e != TextCtrl_VUMeter_EndNote->GetValue()) {
        TextCtrl_VUMeter_EndNote->SetValue(e);
    }
    if (start != Slider_VUMeter_StartNote->GetValue()) {
        Slider_VUMeter_StartNote->SetValue(start);
    }
    wxString s = wxString::Format("%d", start);
    if (s != TextCtrl_VUMeter_StartNote->GetValue()) {
        TextCtrl_VUMeter_StartNote->SetValue(s);
    }

    ValidateWindow();

}

void VUMeterPanel::OnTextCtrl_VUMeter_StartNoteKillFocus(wxFocusEvent& event)
{
    ApplyText(event);
    event.Skip();
}
