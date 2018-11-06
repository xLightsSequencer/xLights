#include "TextPanel.h"
#include "EffectPanelUtils.h"
#include "../FontManager.h"

//(*InternalHeaders(TextPanel)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/fontpicker.h>
#include <wx/gbsizer.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/notebook.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(TextPanel)
const long TextPanel::ID_STATICTEXT_Text = wxNewId();
const long TextPanel::ID_TEXTCTRL_Text = wxNewId();
const long TextPanel::ID_STATICTEXT1 = wxNewId();
const long TextPanel::ID_FILEPICKERCTRL_Text_File = wxNewId();
const long TextPanel::ID_FONTPICKER_Text_Font = wxNewId();
const long TextPanel::ID_BITMAPBUTTON_FONTPICKER_Text_Font = wxNewId();
const long TextPanel::ID_STATICTEXT_Text_Font = wxNewId();
const long TextPanel::ID_CHOICE_Text_Font = wxNewId();
const long TextPanel::ID_BITMAPBUTTON1 = wxNewId();
const long TextPanel::ID_STATICTEXT_Text_Dir = wxNewId();
const long TextPanel::ID_CHOICE_Text_Dir = wxNewId();
const long TextPanel::ID_BITMAPBUTTON_CHOICE_Text_Dir = wxNewId();
const long TextPanel::ID_CHECKBOX_TextToCenter = wxNewId();
const long TextPanel::ID_BITMAPBUTTON_TextToCenter = wxNewId();
const long TextPanel::ID_STATICTEXT_Text_Speed = wxNewId();
const long TextPanel::IDD_SLIDER_Text_Speed = wxNewId();
const long TextPanel::ID_TEXTCTRL_Text_Speed = wxNewId();
const long TextPanel::ID_BITMAPBUTTON_Text_Speed = wxNewId();
const long TextPanel::ID_STATICTEXT_Text_Effect = wxNewId();
const long TextPanel::ID_CHOICE_Text_Effect = wxNewId();
const long TextPanel::ID_BITMAPBUTTON_CHOICE_Text_Effect = wxNewId();
const long TextPanel::ID_STATICTEXT_Text_Count = wxNewId();
const long TextPanel::ID_CHOICE_Text_Count = wxNewId();
const long TextPanel::ID_BITMAPBUTTON_CHOICE_Text_Count = wxNewId();
const long TextPanel::ID_CHECKBOX_Text_PixelOffsets = wxNewId();
const long TextPanel::ID_STATICTEXT_Text_XStart = wxNewId();
const long TextPanel::ID_SLIDER_Text_XStart = wxNewId();
const long TextPanel::IDD_TEXTCTRL_Text_XStart = wxNewId();
const long TextPanel::ID_STATICTEXT_Text_YStart = wxNewId();
const long TextPanel::IDD_TEXTCTRL_Text_YStart = wxNewId();
const long TextPanel::ID_SLIDER_Text_YStart = wxNewId();
const long TextPanel::IDD_PANEL6 = wxNewId();
const long TextPanel::ID_STATICTEXT_Text_XEnd = wxNewId();
const long TextPanel::ID_SLIDER_Text_XEnd = wxNewId();
const long TextPanel::IDD_TEXTCTRL_Text_XEnd = wxNewId();
const long TextPanel::ID_STATICTEXT_Text_YEnd = wxNewId();
const long TextPanel::IDD_TEXTCTRL_Text_YEnd = wxNewId();
const long TextPanel::ID_SLIDER_Text_YEnd = wxNewId();
const long TextPanel::IDD_PANEL17 = wxNewId();
const long TextPanel::IDD_NOTEBOOK1 = wxNewId();
const long TextPanel::ID_PANEL_Text1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(TextPanel,wxPanel)
	//(*EventTable(TextPanel)
	//*)
END_EVENT_TABLE()

TextPanel::TextPanel(wxWindow* parent)
{
	//(*Initialize(TextPanel)
	BulkEditCheckBox* CheckBox_TextToCenter;
	BulkEditTextCtrl* TextCtrl72;
	BulkEditTextCtrl* TextCtrl91;
	BulkEditTextCtrl* TextCtrl92;
	BulkEditTextCtrl* TextCtrl93;
	BulkEditTextCtrl* TextCtrl94;
	wxFlexGridSizer* FlexGridSizer119;
	wxFlexGridSizer* FlexGridSizer141;
	wxFlexGridSizer* FlexGridSizer142;
	wxFlexGridSizer* FlexGridSizer143;
	wxFlexGridSizer* FlexGridSizer144;
	wxFlexGridSizer* FlexGridSizer145;
	wxFlexGridSizer* FlexGridSizer46;
	wxFlexGridSizer* FlexGridSizer48;
	wxFlexGridSizer* FlexGridSizer66;
	wxFlexGridSizer* FlexGridSizer69;
	wxFontPickerCtrl* FontPickerCtrl;
	wxGridBagSizer* GridBagSizer6;
	wxGridBagSizer* GridBagSizer7;
	wxNotebook* Notebook6;
	wxPanel* Panel16;
	wxStaticText* StaticText162;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer46 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer46->AddGrowableCol(0);
	Panel_Text1 = new wxPanel(this, ID_PANEL_Text1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Text1"));
	FlexGridSizer69 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer69->AddGrowableCol(0);
	FlexGridSizer119 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer119->AddGrowableCol(1);
	StaticText53 = new wxStaticText(Panel_Text1, ID_STATICTEXT_Text, _("Text"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Text"));
	FlexGridSizer119->Add(StaticText53, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Text = new BulkEditTextCtrl(Panel_Text1, ID_TEXTCTRL_Text, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Text"));
	TextCtrl_Text->SetMaxLength(256);
	FlexGridSizer119->Add(TextCtrl_Text, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer119->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(Panel_Text1, ID_STATICTEXT1, _("From File"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer119->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FilePickerCtrl1 = new wxFilePickerCtrl(Panel_Text1, ID_FILEPICKERCTRL_Text_File, wxEmptyString, _("Select a text file"), _T("*.txt"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL_Text_File"));
	FlexGridSizer119->Add(FilePickerCtrl1, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer119->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText162 = new wxStaticText(Panel_Text1, wxID_ANY, _("Font"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer119->Add(StaticText162, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FontPickerCtrl = new wxFontPickerCtrl(Panel_Text1, ID_FONTPICKER_Text_Font, wxNullFont, wxDefaultPosition, wxDefaultSize, wxFNTP_FONTDESC_AS_LABEL|wxFNTP_USEFONT_FOR_LABEL, wxDefaultValidator, _T("ID_FONTPICKER_Text_Font"));
	FlexGridSizer119->Add(FontPickerCtrl, 1, wxALL|wxEXPAND, 2);
	BitmapButton_TextFont = new xlLockButton(Panel_Text1, ID_BITMAPBUTTON_FONTPICKER_Text_Font, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_FONTPICKER_Text_Font"));
	BitmapButton_TextFont->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer119->Add(BitmapButton_TextFont, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText1 = new wxStaticText(Panel_Text1, ID_STATICTEXT_Text_Font, _("XL Font"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Text_Font"));
	FlexGridSizer119->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Text_Font = new BulkEditChoice(Panel_Text1, ID_CHOICE_Text_Font, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Font"));
	FlexGridSizer119->Add(Choice_Text_Font, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton1 = new xlLockButton(Panel_Text1, ID_BITMAPBUTTON1, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
	BitmapButton1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer119->Add(BitmapButton1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText78 = new wxStaticText(Panel_Text1, ID_STATICTEXT_Text_Dir, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Text_Dir"));
	FlexGridSizer119->Add(StaticText78, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer48 = new wxFlexGridSizer(0, 3, 0, 0);
	Choice_Text_Dir = new BulkEditChoice(Panel_Text1, ID_CHOICE_Text_Dir, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Dir"));
	Choice_Text_Dir->SetSelection( Choice_Text_Dir->Append(_("none")) );
	Choice_Text_Dir->Append(_("left"));
	Choice_Text_Dir->Append(_("right"));
	Choice_Text_Dir->Append(_("up"));
	Choice_Text_Dir->Append(_("down"));
	Choice_Text_Dir->Append(_("vector"));
	Choice_Text_Dir->Append(_("up-left"));
	Choice_Text_Dir->Append(_("down-left"));
	Choice_Text_Dir->Append(_("up-right"));
	Choice_Text_Dir->Append(_("down-right"));
	Choice_Text_Dir->Append(_("wavey"));
	FlexGridSizer48->Add(Choice_Text_Dir, 1, wxALL|wxEXPAND, 2);
	BitmapButton_TextDir = new xlLockButton(Panel_Text1, ID_BITMAPBUTTON_CHOICE_Text_Dir, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Dir"));
	BitmapButton_TextDir->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer48->Add(BitmapButton_TextDir, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_TextToCenter = new BulkEditCheckBox(Panel_Text1, ID_CHECKBOX_TextToCenter, _("C"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_TextToCenter"));
	CheckBox_TextToCenter->SetValue(false);
	CheckBox_TextToCenter->SetToolTip(_("Move to center and stop"));
	FlexGridSizer48->Add(CheckBox_TextToCenter, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer119->Add(FlexGridSizer48, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_TextToCenter = new xlLockButton(Panel_Text1, ID_BITMAPBUTTON_TextToCenter, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TextToCenter"));
	BitmapButton_TextToCenter->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer119->Add(BitmapButton_TextToCenter, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText186 = new wxStaticText(Panel_Text1, ID_STATICTEXT_Text_Speed, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Text_Speed"));
	FlexGridSizer119->Add(StaticText186, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer66 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer66->AddGrowableCol(0);
	Slider_Text_Speed = new BulkEditSlider(Panel_Text1, IDD_SLIDER_Text_Speed, 10, 0, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Text_Speed"));
	FlexGridSizer66->Add(Slider_Text_Speed, 1, wxALL|wxEXPAND, 1);
	TextCtrl72 = new BulkEditTextCtrl(Panel_Text1, ID_TEXTCTRL_Text_Speed, _("10"), wxDefaultPosition, wxDLG_UNIT(Panel_Text1,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Text_Speed"));
	TextCtrl72->SetMaxLength(3);
	FlexGridSizer66->Add(TextCtrl72, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer119->Add(FlexGridSizer66, 1, wxALL|wxEXPAND, 1);
	BitmapButton_Text_Speed = new xlLockButton(Panel_Text1, ID_BITMAPBUTTON_Text_Speed, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Text_Speed"));
	BitmapButton_Text_Speed->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer119->Add(BitmapButton_Text_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText107 = new wxStaticText(Panel_Text1, ID_STATICTEXT_Text_Effect, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Text_Effect"));
	FlexGridSizer119->Add(StaticText107, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Text_Effect = new BulkEditChoice(Panel_Text1, ID_CHOICE_Text_Effect, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Effect"));
	Choice_Text_Effect->SetSelection( Choice_Text_Effect->Append(_("normal")) );
	Choice_Text_Effect->Append(_("vert text up"));
	Choice_Text_Effect->Append(_("vert text down"));
	Choice_Text_Effect->Append(_("rotate up 45"));
	Choice_Text_Effect->Append(_("rotate up 90"));
	Choice_Text_Effect->Append(_("rotate down 45"));
	Choice_Text_Effect->Append(_("rotate down 90"));
	FlexGridSizer119->Add(Choice_Text_Effect, 1, wxALL|wxEXPAND, 2);
	BitmapButton_TextEffect = new xlLockButton(Panel_Text1, ID_BITMAPBUTTON_CHOICE_Text_Effect, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Effect"));
	BitmapButton_TextEffect->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer119->Add(BitmapButton_TextEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText98 = new wxStaticText(Panel_Text1, ID_STATICTEXT_Text_Count, _("Count down"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Text_Count"));
	FlexGridSizer119->Add(StaticText98, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Text_Count = new BulkEditChoice(Panel_Text1, ID_CHOICE_Text_Count, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Count"));
	Choice_Text_Count->SetSelection( Choice_Text_Count->Append(_("none")) );
	Choice_Text_Count->Append(_("seconds"));
	Choice_Text_Count->Append(_("minutes seconds"));
	Choice_Text_Count->Append(_("to date \'d h m s\'"));
	Choice_Text_Count->Append(_("to date \'h:m:s\'"));
	Choice_Text_Count->Append(_("to date \'m\' or \'s\'"));
	Choice_Text_Count->Append(_("to date \'s\'"));
	Choice_Text_Count->Append(_("!to date!%fmt"));
	Choice_Text_Count->SetToolTip(_("seconds: enter time in seconds\nminutes seconds: enter time as mm:ss. To prepend or append text to the counter, delimit the time with / (The next show begins in /15:30/! Stay Tuned.)\n\nto date \'d h m s\'\nto date \'h:m:s\'\nto date \'m\' or \'s\'\nto date \'s\'\n\nThese options count down to given date based on system date and time. Enter Date in on of the following formats: \nFri, 25 Dec 2015 00:00:00 +0100\nFri, 25 Dec 2015 00:00:00 CST\nFri, 25 Dec 2015 00:00:00 MST\n\n\n!to date!%fmt\n\nThis option allows you to choose the output format. i.e. days only.\nEnter Date in the following format to display days only:\n/Fri, 25 Dec 2015 00:00:00 +0100/ %D"));
	FlexGridSizer119->Add(Choice_Text_Count, 1, wxALL|wxEXPAND, 2);
	BitmapButton_TextCount = new xlLockButton(Panel_Text1, ID_BITMAPBUTTON_CHOICE_Text_Count, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Count"));
	BitmapButton_TextCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer119->Add(BitmapButton_TextCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer69->Add(FlexGridSizer119, 1, wxALL|wxEXPAND, 1);
	FlexGridSizer141 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer141->AddGrowableCol(0);
	CheckBox_Text_PixelOffsets = new BulkEditCheckBox(Panel_Text1, ID_CHECKBOX_Text_PixelOffsets, _("Offsets In Pixels"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Text_PixelOffsets"));
	CheckBox_Text_PixelOffsets->SetValue(false);
	FlexGridSizer141->Add(CheckBox_Text_PixelOffsets, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
	Notebook6 = new wxNotebook(Panel_Text1, IDD_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("IDD_NOTEBOOK1"));
	Panel16 = new wxPanel(Notebook6, IDD_PANEL6, wxPoint(15,49), wxDefaultSize, wxTAB_TRAVERSAL, _T("IDD_PANEL6"));
	FlexGridSizer142 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer142->AddGrowableCol(0);
	FlexGridSizer143 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer143->AddGrowableCol(1);
	StaticText211 = new wxStaticText(Panel16, ID_STATICTEXT_Text_XStart, _("X"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Text_XStart"));
	FlexGridSizer143->Add(StaticText211, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Text_XStart = new BulkEditSlider(Panel16, ID_SLIDER_Text_XStart, 0, -200, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Text_XStart"));
	FlexGridSizer143->Add(Slider_Text_XStart, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer143->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl91 = new BulkEditTextCtrl(Panel16, IDD_TEXTCTRL_Text_XStart, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel16,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Text_XStart"));
	TextCtrl91->SetMaxLength(4);
	FlexGridSizer143->Add(TextCtrl91, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer142->Add(FlexGridSizer143, 1, wxALL|wxEXPAND, 5);
	GridBagSizer6 = new wxGridBagSizer(0, 0);
	StaticText212 = new wxStaticText(Panel16, ID_STATICTEXT_Text_YStart, _("Y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Text_YStart"));
	GridBagSizer6->Add(StaticText212, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl92 = new BulkEditTextCtrl(Panel16, IDD_TEXTCTRL_Text_YStart, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel16,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Text_YStart"));
	TextCtrl92->SetMaxLength(4);
	GridBagSizer6->Add(TextCtrl92, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Text_YStart = new BulkEditSlider(Panel16, ID_SLIDER_Text_YStart, 0, -200, 200, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER_Text_YStart"));
	GridBagSizer6->Add(Slider_Text_YStart, wxGBPosition(0, 1), wxGBSpan(4, 1), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer142->Add(GridBagSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel16->SetSizer(FlexGridSizer142);
	FlexGridSizer142->Fit(Panel16);
	FlexGridSizer142->SetSizeHints(Panel16);
	Panel17 = new wxPanel(Notebook6, IDD_PANEL17, wxPoint(104,13), wxDefaultSize, wxTAB_TRAVERSAL, _T("IDD_PANEL17"));
	FlexGridSizer144 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer144->AddGrowableCol(0);
	FlexGridSizer145 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer145->AddGrowableCol(1);
	StaticText213 = new wxStaticText(Panel17, ID_STATICTEXT_Text_XEnd, _("X"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Text_XEnd"));
	FlexGridSizer145->Add(StaticText213, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Text_XEnd = new BulkEditSlider(Panel17, ID_SLIDER_Text_XEnd, 0, -200, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Text_XEnd"));
	FlexGridSizer145->Add(Slider_Text_XEnd, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer145->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl93 = new BulkEditTextCtrl(Panel17, IDD_TEXTCTRL_Text_XEnd, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel17,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Text_XEnd"));
	TextCtrl93->SetMaxLength(4);
	FlexGridSizer145->Add(TextCtrl93, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer144->Add(FlexGridSizer145, 1, wxALL|wxEXPAND, 5);
	GridBagSizer7 = new wxGridBagSizer(0, 0);
	StaticText214 = new wxStaticText(Panel17, ID_STATICTEXT_Text_YEnd, _("Y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Text_YEnd"));
	GridBagSizer7->Add(StaticText214, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl94 = new BulkEditTextCtrl(Panel17, IDD_TEXTCTRL_Text_YEnd, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel17,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Text_YEnd"));
	TextCtrl94->SetMaxLength(4);
	GridBagSizer7->Add(TextCtrl94, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Text_YEnd = new BulkEditSlider(Panel17, ID_SLIDER_Text_YEnd, 0, -200, 200, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER_Text_YEnd"));
	GridBagSizer7->Add(Slider_Text_YEnd, wxGBPosition(0, 1), wxGBSpan(4, 1), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer144->Add(GridBagSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel17->SetSizer(FlexGridSizer144);
	FlexGridSizer144->Fit(Panel17);
	FlexGridSizer144->SetSizeHints(Panel17);
	Notebook6->AddPage(Panel16, _("Start Position"), false);
	Notebook6->AddPage(Panel17, _("End Position"), false);
	FlexGridSizer141->Add(Notebook6, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer69->Add(FlexGridSizer141, 1, wxALL|wxEXPAND, 1);
	Panel_Text1->SetSizer(FlexGridSizer69);
	FlexGridSizer69->Fit(Panel_Text1);
	FlexGridSizer69->SetSizeHints(Panel_Text1);
	FlexGridSizer46->Add(Panel_Text1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer46);
	FlexGridSizer46->Fit(this);
	FlexGridSizer46->SetSizeHints(this);

	Connect(ID_FILEPICKERCTRL_Text_File,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&TextPanel::OnFilePickerCtrl1FileChanged);
	Connect(ID_BITMAPBUTTON_FONTPICKER_Text_Font,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TextPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TextPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Text_Dir,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TextPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_TextToCenter,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TextPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_Text_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TextPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Text_Effect,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TextPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Text_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TextPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_TEXT");

   	Choice_Text_Font->SetSelection( Choice_Text_Font->Append(_("Use OS Fonts")) );
   	FontManager& font_mgr(FontManager::instance());
    wxArrayString xl_font_names = font_mgr.get_font_names();
   	font_mgr.init();
    for( int i = 0; i < xl_font_names.size(); i++ )
    {
        Choice_Text_Font->Append( xl_font_names[i] );
    }
    ValidateWindow();
}

TextPanel::~TextPanel()
{
	//(*Destroy(TextPanel)
	//*)
}

PANEL_EVENT_HANDLERS(TextPanel)

void TextPanel::OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event)
{
    ValidateWindow();
}

void TextPanel::ValidateWindow()
{
    if (!wxFile::Exists(FilePickerCtrl1->GetFileName().GetFullPath()))
    {
        TextCtrl_Text->Enable();
    }
    else
    {
        TextCtrl_Text->Disable();
    }
}
