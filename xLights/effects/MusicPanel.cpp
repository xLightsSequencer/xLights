/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*InternalHeaders(MusicPanel)
 #include <wx/bitmap.h>
 #include <wx/bmpbuttn.h>
 #include <wx/checkbox.h>
 #include <wx/choice.h>
 #include <wx/image.h>
 #include <wx/intl.h>
 #include <wx/settings.h>
 #include <wx/sizer.h>
 #include <wx/slider.h>
 #include <wx/stattext.h>
 #include <wx/string.h>
 #include <wx/textctrl.h>
 //*)

#include "MusicPanel.h"
#include "EffectPanelUtils.h"
#include "MusicEffect.h"
#include "UtilFunctions.h"

//(*IdInit(MusicPanel)
const long MusicPanel::ID_STATICTEXT_Music_Bars = wxNewId();
const long MusicPanel::ID_SLIDER_Music_Bars = wxNewId();
const long MusicPanel::IDD_TEXTCTRL_Music_Bars = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_SLIDER_Music_Bars = wxNewId();
const long MusicPanel::ID_STATICTEXT_Music_Type = wxNewId();
const long MusicPanel::ID_CHOICE_Music_Type = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_CHOICE_Music_Type = wxNewId();
const long MusicPanel::ID_STATICTEXT_Music_StartNote = wxNewId();
const long MusicPanel::ID_SLIDER_Music_StartNote = wxNewId();
const long MusicPanel::IDD_TEXTCTRL_Music_StartNote = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_SLIDER_Music_StartNote = wxNewId();
const long MusicPanel::ID_STATICTEXT_Music_EndNote = wxNewId();
const long MusicPanel::ID_SLIDER_Music_EndNote = wxNewId();
const long MusicPanel::IDD_TEXTCTRL_Music_EndNote = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_SLIDER_Music_EndNote = wxNewId();
const long MusicPanel::ID_STATICTEXT_Music_Sensitivity = wxNewId();
const long MusicPanel::ID_SLIDER_Music_Sensitivity = wxNewId();
const long MusicPanel::IDD_TEXTCTRL_Music_Sensitivity = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_SLIDER_Music_Sensitivity = wxNewId();
const long MusicPanel::ID_STATICTEXT_Music_Offset = wxNewId();
const long MusicPanel::ID_SLIDER_Music_Offset = wxNewId();
const long MusicPanel::ID_VALUECURVE_Music_Offset = wxNewId();
const long MusicPanel::IDD_TEXTCTRL_Music_Offset = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_SLIDER_Music_Offset = wxNewId();
const long MusicPanel::ID_CHECKBOX_Music_Scale = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_CHECKBOX_Music_Scale = wxNewId();
const long MusicPanel::ID_STATICTEXT_Music_Scaling = wxNewId();
const long MusicPanel::ID_CHOICE_Music_Scaling = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_CHOICE_Music_Scaling = wxNewId();
const long MusicPanel::ID_STATICTEXT_Music_Colour = wxNewId();
const long MusicPanel::ID_CHOICE_Music_Colour = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_CHOICE_Music_Colour = wxNewId();
const long MusicPanel::ID_CHECKBOX_Music_Fade = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_CHECKBOX_Music_Fade = wxNewId();
const long MusicPanel::ID_CHECKBOX_Music_LogarithmicX = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_CHECKBOX_Music_LogarithmicX = wxNewId();
//*)

BEGIN_EVENT_TABLE(MusicPanel,wxPanel)
	//(*EventTable(MusicPanel)
	//*)
END_EVENT_TABLE()

MusicPanel::MusicPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(MusicPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer31;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer42;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer42->AddGrowableCol(0);
	FlexGridSizer31 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer31->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Music_Bars, _("Bars"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Music_Bars"));
	FlexGridSizer31->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Music_Bars = new BulkEditSlider(this, ID_SLIDER_Music_Bars, 20, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Music_Bars"));
	FlexGridSizer1->Add(Slider_Music_Bars, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer31->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Music_Bars = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Music_Bars, _("20"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Music_Bars"));
	FlexGridSizer31->Add(TextCtrl_Music_Bars, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Music_Bars = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Music_Bars, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Music_Bars"));
	BitmapButton_Music_Bars->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_Music_Bars, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT_Music_Type, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Music_Type"));
	FlexGridSizer31->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Music_Type = new BulkEditChoice(this, ID_CHOICE_Music_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Music_Type"));
	Choice_Music_Type->SetSelection( Choice_Music_Type->Append(_("Morph")) );
	Choice_Music_Type->Append(_("Bounce"));
	Choice_Music_Type->Append(_("Collide"));
	Choice_Music_Type->Append(_("Separate"));
	Choice_Music_Type->Append(_("On"));
	FlexGridSizer31->Add(Choice_Music_Type, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Music_Type = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_Music_Type, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Music_Type"));
	BitmapButton_Music_Type->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_Music_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText12 = new wxStaticText(this, ID_STATICTEXT_Music_StartNote, _("Start Note"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Music_StartNote"));
	FlexGridSizer31->Add(StaticText12, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Music_StartNote = new BulkEditSlider(this, ID_SLIDER_Music_StartNote, 60, 0, 127, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Music_StartNote"));
	FlexGridSizer31->Add(Slider_Music_StartNote, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Music_StartNote = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Music_StartNote, _("60"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_Music_StartNote"));
	FlexGridSizer31->Add(TextCtrl_Music_StartNote, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Music_StartNote = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Music_StartNote, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Music_StartNote"));
	BitmapButton_Music_StartNote->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_Music_StartNote, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT_Music_EndNote, _("End Note"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Music_EndNote"));
	FlexGridSizer31->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Music_EndNote = new BulkEditSlider(this, ID_SLIDER_Music_EndNote, 80, 0, 127, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Music_EndNote"));
	FlexGridSizer31->Add(Slider_Music_EndNote, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Music_EndNote = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Music_EndNote, _("80"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_Music_EndNote"));
	FlexGridSizer31->Add(TextCtrl_Music_EndNote, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Music_EndNote = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Music_EndNote, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Music_EndNote"));
	BitmapButton_Music_EndNote->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_Music_EndNote, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT_Music_Sensitivity, _("Sensitivity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Music_Sensitivity"));
	FlexGridSizer31->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Music_Sensitivity = new BulkEditSlider(this, ID_SLIDER_Music_Sensitivity, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Music_Sensitivity"));
	FlexGridSizer2->Add(Slider_Music_Sensitivity, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Music_Sensitivity = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Music_Sensitivity, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Music_Sensitivity"));
	FlexGridSizer31->Add(TextCtrl_Music_Sensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Music_Sensitivity = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Music_Sensitivity, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Music_Sensitivity"));
	BitmapButton_Music_Sensitivity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_Music_Sensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT_Music_Offset, _("Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Music_Offset"));
	FlexGridSizer31->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Music_Offset = new BulkEditSlider(this, ID_SLIDER_Music_Offset, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Music_Offset"));
	FlexGridSizer3->Add(Slider_Music_Offset, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Music_OffsetVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Music_Offset, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Music_Offset"));
	FlexGridSizer3->Add(BitmapButton_Music_OffsetVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer31->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Music_Offset = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Music_Offset, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Music_Offset"));
	FlexGridSizer31->Add(TextCtrl_Music_Offset, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Music_Offset = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Music_Offset, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Music_Offset"));
	BitmapButton_Music_Offset->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_Music_Offset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Music_Scale = new BulkEditCheckBox(this, ID_CHECKBOX_Music_Scale, _("Scale Bars"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Music_Scale"));
	CheckBox_Music_Scale->SetValue(false);
	FlexGridSizer31->Add(CheckBox_Music_Scale, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Music_Scale = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Music_Scale, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Music_Scale"));
	BitmapButton_Music_Scale->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_Music_Scale, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT_Music_Scaling, _("Notes Scaling"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Music_Scaling"));
	FlexGridSizer31->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Music_Scaling = new BulkEditChoice(this, ID_CHOICE_Music_Scaling, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Music_Scaling"));
	Choice_Music_Scaling->SetSelection( Choice_Music_Scaling->Append(_("None")) );
	Choice_Music_Scaling->Append(_("Individual Notes"));
	Choice_Music_Scaling->Append(_("All Notes"));
	FlexGridSizer31->Add(Choice_Music_Scaling, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Music_Scaling = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_Music_Scaling, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Music_Scaling"));
	BitmapButton_Music_Scaling->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_Music_Scaling, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT_Music_Colour, _("Color"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Music_Colour"));
	FlexGridSizer31->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Music_Colour = new BulkEditChoice(this, ID_CHOICE_Music_Colour, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Music_Colour"));
	Choice_Music_Colour->SetSelection( Choice_Music_Colour->Append(_("Distinct")) );
	Choice_Music_Colour->Append(_("Blend"));
	Choice_Music_Colour->Append(_("Cycle"));
	FlexGridSizer31->Add(Choice_Music_Colour, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Music_Colour = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_Music_Colour, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Music_Colour"));
	BitmapButton_Music_Colour->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_Music_Colour, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Music_Fade = new BulkEditCheckBox(this, ID_CHECKBOX_Music_Fade, _("Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Music_Fade"));
	CheckBox_Music_Fade->SetValue(false);
	FlexGridSizer31->Add(CheckBox_Music_Fade, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Music_Fade = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Music_Fade, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Music_Fade"));
	BitmapButton_Music_Fade->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_Music_Fade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Music_LogarithmicXAxis = new BulkEditCheckBox(this, ID_CHECKBOX_Music_LogarithmicX, _("Logarithmic X axis"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Music_LogarithmicX"));
	CheckBox_Music_LogarithmicXAxis->SetValue(false);
	FlexGridSizer31->Add(CheckBox_Music_LogarithmicXAxis, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Music_LogarithmicXAxis = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Music_LogarithmicX, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Music_LogarithmicX"));
	BitmapButton_Music_LogarithmicXAxis->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer31->Add(BitmapButton_Music_LogarithmicXAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer42->Add(FlexGridSizer31, 1, wxEXPAND, 2);
	SetSizer(FlexGridSizer42);
	FlexGridSizer42->Fit(this);
	FlexGridSizer42->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Music_Bars,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(ID_CHOICE_Music_Type,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&MusicPanel::OnChoice_Music_TypeSelect);
	Connect(ID_BITMAPBUTTON_CHOICE_Music_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Music_StartNote,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&MusicPanel::OnSlider_Music_StartNoteCmdSliderUpdated);
	Connect(IDD_TEXTCTRL_Music_StartNote,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&MusicPanel::OnTextCtrl_Music_StartNoteText);
	Connect(ID_BITMAPBUTTON_SLIDER_Music_StartNote,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Music_EndNote,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&MusicPanel::OnSlider_Music_StartNoteCmdSliderUpdated);
	Connect(IDD_TEXTCTRL_Music_EndNote,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&MusicPanel::OnTextCtrl_Music_StartNoteText);
	Connect(ID_BITMAPBUTTON_SLIDER_Music_EndNote,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Music_Sensitivity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Music_Offset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Music_Offset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Music_Scale,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Music_Scaling,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Music_Colour,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Music_Fade,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Music_LogarithmicX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	//*)

    SetName("ID_PANEL_Music");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&MusicPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&MusicPanel::OnValidateWindow, 0, this);

    BitmapButton_Music_OffsetVC->GetValue()->SetLimits(MUSIC_OFFSET_MIN, MUSIC_OFFSET_MAX);

	TextCtrl_Music_StartNote->Bind(wxEVT_KILL_FOCUS, (wxObjectEventFunction)&MusicPanel::OnTextCtrl_Music_StartNoteKillFocus, this);
	TextCtrl_Music_EndNote->Bind(wxEVT_KILL_FOCUS, (wxObjectEventFunction)&MusicPanel::OnTextCtrl_Music_StartNoteKillFocus, this);
	TextCtrl_Music_StartNote->Bind(wxEVT_TEXT_ENTER, (wxObjectEventFunction)&MusicPanel::OnTextCtrl_Music_StartNoteEnter, this);
	TextCtrl_Music_EndNote->Bind(wxEVT_TEXT_ENTER, (wxObjectEventFunction)&MusicPanel::OnTextCtrl_Music_StartNoteEnter, this);

	ValidateWindow();
}

MusicPanel::~MusicPanel()
{
	//(*Destroy(MusicPanel)
	//*)
}

void MusicPanel::ValidateWindow()
{
    Slider_Music_StartNote->SetToolTip(wxString(DecodeMidi(Slider_Music_StartNote->GetValue()).c_str()));
    Slider_Music_EndNote->SetToolTip(wxString(DecodeMidi(Slider_Music_EndNote->GetValue()).c_str()));
}

void MusicPanel::OnChoice_Music_TypeSelect(wxCommandEvent& event)
{
	ValidateWindow();
}

void MusicPanel::OnSlider_Music_StartNoteCmdSliderUpdated(wxScrollEvent& event)
{
    int start = Slider_Music_StartNote->GetValue();
    int end = Slider_Music_EndNote->GetValue();
    if (event.GetEventObject() == Slider_Music_StartNote)
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

    if (end != Slider_Music_EndNote->GetValue())
    {
        Slider_Music_EndNote->SetValue(end);
    }
    wxString e = wxString::Format("%d", end);
    if (e != TextCtrl_Music_EndNote->GetValue())
    {
        TextCtrl_Music_EndNote->SetValue(e);
    }
    if (start != Slider_Music_StartNote->GetValue())
    {
        Slider_Music_StartNote->SetValue(start);
    }
    wxString s = wxString::Format("%d", start);
    if (s != TextCtrl_Music_StartNote->GetValue())
    {
        TextCtrl_Music_StartNote->SetValue(s);
    }
    ValidateWindow();
}

void MusicPanel::OnTextCtrl_Music_StartNoteText(wxCommandEvent& event)
{
	// do nothing as the text changes ... wait until focus is lost
}

void MusicPanel::OnTextCtrl_Music_StartNoteEnter(wxCommandEvent& event)
{
	ApplyText(event);
}

void MusicPanel::ApplyText(wxEvent& event)
{
	int start = wxAtoi(TextCtrl_Music_StartNote->GetValue());
	int end = wxAtoi(TextCtrl_Music_EndNote->GetValue());
	if (event.GetEventObject() == TextCtrl_Music_StartNote) {
		if (end < start) {
			end = start;
		}
	}
	else {
		if (end < start) {
			start = end;
		}
	}

	if (end != Slider_Music_EndNote->GetValue()) {
		Slider_Music_EndNote->SetValue(end);
	}
	wxString e = wxString::Format("%d", end);
	if (e != TextCtrl_Music_EndNote->GetValue()) {
		TextCtrl_Music_EndNote->SetValue(e);
	}
	if (start != Slider_Music_StartNote->GetValue()) {
		Slider_Music_StartNote->SetValue(start);
	}
	wxString s = wxString::Format("%d", start);
	if (s != TextCtrl_Music_StartNote->GetValue()) {
		TextCtrl_Music_StartNote->SetValue(s);
	}

	ValidateWindow();
}

void MusicPanel::OnTextCtrl_Music_StartNoteKillFocus(wxFocusEvent& event)
{
	ApplyText(event);
	event.Skip();
}
