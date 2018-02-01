#include "xlColorPickerFields.h"
#include <algorithm>

//(*InternalHeaders(xlColorPickerFields)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(xlColorPickerFields)
const long xlColorPickerFields::ID_PANEL_Palette = wxNewId();
const long xlColorPickerFields::ID_PANEL_SLIDER = wxNewId();
const long xlColorPickerFields::ID_BITMAPBUTTON_Swatch1 = wxNewId();
const long xlColorPickerFields::ID_SLIDER_Left = wxNewId();
const long xlColorPickerFields::ID_SLIDER_Right = wxNewId();
const long xlColorPickerFields::ID_PANEL_CurrentColor = wxNewId();
const long xlColorPickerFields::ID_RADIOBUTTON_Hue = wxNewId();
const long xlColorPickerFields::ID_RADIOBUTTON_Saturation = wxNewId();
const long xlColorPickerFields::ID_RADIOBUTTON_Brightness = wxNewId();
const long xlColorPickerFields::ID_RADIOBUTTON_Red = wxNewId();
const long xlColorPickerFields::ID_RADIOBUTTON_Green = wxNewId();
const long xlColorPickerFields::ID_RADIOBUTTON_Blue = wxNewId();
const long xlColorPickerFields::ID_TEXTCTRL_Hue = wxNewId();
const long xlColorPickerFields::ID_TEXTCTRL_Saturation = wxNewId();
const long xlColorPickerFields::ID_TEXTCTRL_BrightnessCPF = wxNewId();
const long xlColorPickerFields::ID_TEXTCTRL_Red = wxNewId();
const long xlColorPickerFields::ID_TEXTCTRL_Green = wxNewId();
const long xlColorPickerFields::ID_TEXTCTRL_Blue = wxNewId();
const long xlColorPickerFields::ID_BITMAPBUTTON_Swatch2 = wxNewId();
const long xlColorPickerFields::ID_BITMAPBUTTON_Swatch3 = wxNewId();
const long xlColorPickerFields::ID_BITMAPBUTTON_Swatch4 = wxNewId();
const long xlColorPickerFields::ID_BITMAPBUTTON_Swatch5 = wxNewId();
const long xlColorPickerFields::ID_BITMAPBUTTON_Swatch6 = wxNewId();
const long xlColorPickerFields::ID_BITMAPBUTTON_Swatch7 = wxNewId();
const long xlColorPickerFields::ID_BITMAPBUTTON_Swatch8 = wxNewId();
const long xlColorPickerFields::ID_RADIOBUTTON_SwatchMarker = wxNewId();
//*)

wxDEFINE_EVENT(EVT_CP_SLIDER_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_CP_PALETTE_CHANGED, wxCommandEvent);

BEGIN_EVENT_TABLE(xlColorPickerFields,wxPanel)
	//(*EventTable(xlColorPickerFields)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_CP_SLIDER_CHANGED, xlColorPickerFields::ProcessSliderColorChange)
    EVT_COMMAND(wxID_ANY, EVT_CP_PALETTE_CHANGED, xlColorPickerFields::ProcessPaletteColorChange)
END_EVENT_TABLE()

xlColorPickerFields::xlColorPickerFields(wxWindow* parent, wxWindowID id,const wxPoint& pos,const wxSize& size, long style, const wxString& name)
: xlColorPicker(parent, id, pos, size, style, name),
  mDisplayMode(MODE_SATURATION)
{
	//(*Initialize(xlColorPickerFields)
	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER, _T("wxID_ANY"));
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	Panel_Palette = new xlColorCanvas(this, ID_PANEL_Palette, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL, _T("ID_PANEL_Palette"));
	Panel_Palette->SetMinSize(wxSize(256,256));
	GridBagSizer1->Add(Panel_Palette, wxGBPosition(0, 0), wxGBSpan(11, 10), wxTOP|wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(10,0)).GetWidth());
	Panel_Slider = new xlColorCanvas(this, ID_PANEL_SLIDER, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL, _T("ID_PANEL_SLIDER"));
	Panel_Slider->SetMinSize(wxSize(25,256));
	GridBagSizer1->Add(Panel_Slider, wxGBPosition(0, 11), wxGBSpan(11, 1), wxTOP|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(10,0)).GetWidth());
	BitmapButton_Swatch1 = new wxBitmapButton(this, ID_BITMAPBUTTON_Swatch1, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Swatch1"));
	BitmapButton_Swatch1->SetMinSize(wxSize(20,20));
	BitmapButton_Swatch1->SetBackgroundColour(wxColour(255,0,0));
	GridBagSizer1->Add(BitmapButton_Swatch1, wxGBPosition(11, 1), wxDefaultSpan, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	SliderLeft = new xlSlider(this, ID_SLIDER_Left, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL, wxDefaultValidator, _T("ID_SLIDER_Left"));
	SliderLeft->SetMinSize(wxSize(25,277));
	GridBagSizer1->Add(SliderLeft, wxGBPosition(0, 10), wxGBSpan(11, 1), wxTOP|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(10,0)).GetWidth());
	SliderRight = new xlSlider(this, ID_SLIDER_Right, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LEFT, wxDefaultValidator, _T("ID_SLIDER_Right"));
	SliderRight->SetMinSize(wxSize(25,277));
	GridBagSizer1->Add(SliderRight, wxGBPosition(0, 12), wxGBSpan(11, 1), wxTOP|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(10,0)).GetWidth());
	Panel_CurrentColor = new wxPanel(this, ID_PANEL_CurrentColor, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER, _T("ID_PANEL_CurrentColor"));
	Panel_CurrentColor->SetMinSize(wxSize(75,55));
	GridBagSizer1->Add(Panel_CurrentColor, wxGBPosition(0, 13), wxGBSpan(3, 2), wxTOP|wxRIGHT|wxALIGN_LEFT|wxALIGN_TOP, wxDLG_UNIT(this,wxSize(17,0)).GetWidth());
	RadioButton_Hue = new wxRadioButton(this, ID_RADIOBUTTON_Hue, _("H:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxDefaultValidator, _T("ID_RADIOBUTTON_Hue"));
	wxFont RadioButton_HueFont(13,wxFONTFAMILY_MODERN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	RadioButton_Hue->SetFont(RadioButton_HueFont);
	GridBagSizer1->Add(RadioButton_Hue, wxGBPosition(4, 13), wxDefaultSpan, wxBOTTOM|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	RadioButton_Saturation = new wxRadioButton(this, ID_RADIOBUTTON_Saturation, _("S:"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Saturation"));
	RadioButton_Saturation->SetValue(true);
	wxFont RadioButton_SaturationFont(13,wxFONTFAMILY_MODERN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	RadioButton_Saturation->SetFont(RadioButton_SaturationFont);
	GridBagSizer1->Add(RadioButton_Saturation, wxGBPosition(5, 13), wxDefaultSpan, wxBOTTOM|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	RadioButton_Brightness = new wxRadioButton(this, ID_RADIOBUTTON_Brightness, _("B:"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Brightness"));
	wxFont RadioButton_BrightnessFont(13,wxFONTFAMILY_MODERN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	RadioButton_Brightness->SetFont(RadioButton_BrightnessFont);
	GridBagSizer1->Add(RadioButton_Brightness, wxGBPosition(6, 13), wxDefaultSpan, wxBOTTOM|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	RadioButton_Red = new wxRadioButton(this, ID_RADIOBUTTON_Red, _("R:"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Red"));
	wxFont RadioButton_RedFont(13,wxFONTFAMILY_MODERN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	RadioButton_Red->SetFont(RadioButton_RedFont);
	GridBagSizer1->Add(RadioButton_Red, wxGBPosition(7, 13), wxDefaultSpan, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	RadioButton_Green = new wxRadioButton(this, ID_RADIOBUTTON_Green, _("G:"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Green"));
	wxFont RadioButton_GreenFont(13,wxFONTFAMILY_MODERN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	RadioButton_Green->SetFont(RadioButton_GreenFont);
	GridBagSizer1->Add(RadioButton_Green, wxGBPosition(8, 13), wxDefaultSpan, wxBOTTOM|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	RadioButton_Blue = new wxRadioButton(this, ID_RADIOBUTTON_Blue, _("B:"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Blue"));
	wxFont RadioButton_BlueFont(13,wxFONTFAMILY_MODERN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	RadioButton_Blue->SetFont(RadioButton_BlueFont);
	GridBagSizer1->Add(RadioButton_Blue, wxGBPosition(9, 13), wxDefaultSpan, wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	TextCtrl_Hue = new wxTextCtrl(this, ID_TEXTCTRL_Hue, _("0"), wxDefaultPosition, wxDefaultSize, wxTE_LEFT|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_TEXTCTRL_Hue"));
	TextCtrl_Hue->SetMinSize(wxSize(40,23));
	wxFont TextCtrl_HueFont(12,wxFONTFAMILY_MODERN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	TextCtrl_Hue->SetFont(TextCtrl_HueFont);
	GridBagSizer1->Add(TextCtrl_Hue, wxGBPosition(4, 14), wxDefaultSpan, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	TextCtrl_Saturation = new wxTextCtrl(this, ID_TEXTCTRL_Saturation, _("100"), wxDefaultPosition, wxDefaultSize, wxTE_LEFT|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_TEXTCTRL_Saturation"));
	TextCtrl_Saturation->SetMinSize(wxSize(40,23));
	wxFont TextCtrl_SaturationFont(12,wxFONTFAMILY_MODERN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	TextCtrl_Saturation->SetFont(TextCtrl_SaturationFont);
	GridBagSizer1->Add(TextCtrl_Saturation, wxGBPosition(5, 14), wxDefaultSpan, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	TextCtrl_Brightness = new wxTextCtrl(this, ID_TEXTCTRL_BrightnessCPF, _("100"), wxDefaultPosition, wxDefaultSize, wxTE_LEFT|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_TEXTCTRL_BrightnessCPF"));
	TextCtrl_Brightness->SetMinSize(wxSize(40,23));
	wxFont TextCtrl_BrightnessFont(12,wxFONTFAMILY_MODERN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	TextCtrl_Brightness->SetFont(TextCtrl_BrightnessFont);
	GridBagSizer1->Add(TextCtrl_Brightness, wxGBPosition(6, 14), wxDefaultSpan, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	TextCtrl_Red = new wxTextCtrl(this, ID_TEXTCTRL_Red, _("255"), wxDefaultPosition, wxDefaultSize, wxTE_LEFT|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_TEXTCTRL_Red"));
	TextCtrl_Red->SetMinSize(wxSize(40,23));
	wxFont TextCtrl_RedFont(12,wxFONTFAMILY_MODERN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	TextCtrl_Red->SetFont(TextCtrl_RedFont);
	GridBagSizer1->Add(TextCtrl_Red, wxGBPosition(7, 14), wxDefaultSpan, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	TextCtrl_Green = new wxTextCtrl(this, ID_TEXTCTRL_Green, _("0"), wxDefaultPosition, wxDefaultSize, wxTE_LEFT|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_TEXTCTRL_Green"));
	TextCtrl_Green->SetMinSize(wxSize(40,23));
	wxFont TextCtrl_GreenFont(12,wxFONTFAMILY_MODERN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	TextCtrl_Green->SetFont(TextCtrl_GreenFont);
	GridBagSizer1->Add(TextCtrl_Green, wxGBPosition(8, 14), wxDefaultSpan, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	TextCtrl_Blue = new wxTextCtrl(this, ID_TEXTCTRL_Blue, _("0"), wxDefaultPosition, wxDefaultSize, wxTE_LEFT|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_TEXTCTRL_Blue"));
	TextCtrl_Blue->SetMinSize(wxSize(40,23));
	wxFont TextCtrl_BlueFont(12,wxFONTFAMILY_MODERN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	TextCtrl_Blue->SetFont(TextCtrl_BlueFont);
	GridBagSizer1->Add(TextCtrl_Blue, wxGBPosition(9, 14), wxDefaultSpan, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	BitmapButton_Swatch2 = new wxBitmapButton(this, ID_BITMAPBUTTON_Swatch2, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Swatch2"));
	BitmapButton_Swatch2->SetMinSize(wxSize(20,20));
	BitmapButton_Swatch2->SetBackgroundColour(wxColour(0,255,0));
	GridBagSizer1->Add(BitmapButton_Swatch2, wxGBPosition(11, 2), wxDefaultSpan, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	BitmapButton_Swatch3 = new wxBitmapButton(this, ID_BITMAPBUTTON_Swatch3, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Swatch3"));
	BitmapButton_Swatch3->SetMinSize(wxSize(20,20));
	BitmapButton_Swatch3->SetBackgroundColour(wxColour(0,0,255));
	GridBagSizer1->Add(BitmapButton_Swatch3, wxGBPosition(11, 3), wxDefaultSpan, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	BitmapButton_Swatch4 = new wxBitmapButton(this, ID_BITMAPBUTTON_Swatch4, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Swatch4"));
	BitmapButton_Swatch4->SetMinSize(wxSize(20,20));
	BitmapButton_Swatch4->SetBackgroundColour(wxColour(255,255,0));
	GridBagSizer1->Add(BitmapButton_Swatch4, wxGBPosition(11, 4), wxDefaultSpan, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	BitmapButton_Swatch5 = new wxBitmapButton(this, ID_BITMAPBUTTON_Swatch5, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Swatch5"));
	BitmapButton_Swatch5->SetMinSize(wxSize(20,20));
	BitmapButton_Swatch5->SetBackgroundColour(wxColour(255,128,0));
	GridBagSizer1->Add(BitmapButton_Swatch5, wxGBPosition(11, 5), wxDefaultSpan, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	BitmapButton_Swatch6 = new wxBitmapButton(this, ID_BITMAPBUTTON_Swatch6, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Swatch6"));
	BitmapButton_Swatch6->SetMinSize(wxSize(20,20));
	BitmapButton_Swatch6->SetBackgroundColour(wxColour(0,255,255));
	GridBagSizer1->Add(BitmapButton_Swatch6, wxGBPosition(11, 6), wxDefaultSpan, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	BitmapButton_Swatch7 = new wxBitmapButton(this, ID_BITMAPBUTTON_Swatch7, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Swatch7"));
	BitmapButton_Swatch7->SetMinSize(wxSize(20,20));
	BitmapButton_Swatch7->SetBackgroundColour(wxColour(255,0,255));
	GridBagSizer1->Add(BitmapButton_Swatch7, wxGBPosition(11, 7), wxDefaultSpan, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	BitmapButton_Swatch8 = new wxBitmapButton(this, ID_BITMAPBUTTON_Swatch8, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Swatch8"));
	BitmapButton_Swatch8->SetMinSize(wxSize(20,20));
	BitmapButton_Swatch8->SetBackgroundColour(wxColour(128,0,255));
	GridBagSizer1->Add(BitmapButton_Swatch8, wxGBPosition(11, 8), wxDefaultSpan, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	RadioButton_SwatchMarker = new wxRadioButton(this, ID_RADIOBUTTON_SwatchMarker, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxDefaultValidator, _T("ID_RADIOBUTTON_SwatchMarker"));
	RadioButton_SwatchMarker->SetValue(true);
	RadioButton_SwatchMarker->Disable();
	GridBagSizer1->Add(RadioButton_SwatchMarker, wxGBPosition(12, 1), wxDefaultSpan, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	SetSizer(GridBagSizer1);
	GridBagSizer1->Fit(this);
	GridBagSizer1->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_Swatch1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xlColorPickerFields::OnBitmapButton_SwatchClick);
	Connect(ID_SLIDER_Left,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xlColorPickerFields::OnSliderLeftCmdSliderUpdated);
	Connect(ID_SLIDER_Right,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xlColorPickerFields::OnSliderRightCmdSliderUpdated);
	Connect(ID_RADIOBUTTON_Hue,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xlColorPickerFields::OnRadioButton_ModeSelect);
	Connect(ID_RADIOBUTTON_Saturation,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xlColorPickerFields::OnRadioButton_ModeSelect);
	Connect(ID_RADIOBUTTON_Brightness,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xlColorPickerFields::OnRadioButton_ModeSelect);
	Connect(ID_RADIOBUTTON_Red,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xlColorPickerFields::OnRadioButton_ModeSelect);
	Connect(ID_RADIOBUTTON_Green,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xlColorPickerFields::OnRadioButton_ModeSelect);
	Connect(ID_RADIOBUTTON_Blue,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xlColorPickerFields::OnRadioButton_ModeSelect);
	Connect(ID_TEXTCTRL_Hue,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xlColorPickerFields::OnTextCtrl_Text);
	Connect(ID_TEXTCTRL_Saturation,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xlColorPickerFields::OnTextCtrl_Text);
	Connect(ID_TEXTCTRL_BrightnessCPF,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xlColorPickerFields::OnTextCtrl_Text);
	Connect(ID_TEXTCTRL_Red,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xlColorPickerFields::OnTextCtrl_Text);
	Connect(ID_TEXTCTRL_Green,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xlColorPickerFields::OnTextCtrl_Text);
	Connect(ID_TEXTCTRL_Blue,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xlColorPickerFields::OnTextCtrl_Text);
	Connect(ID_BITMAPBUTTON_Swatch2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xlColorPickerFields::OnBitmapButton_SwatchClick);
	Connect(ID_BITMAPBUTTON_Swatch3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xlColorPickerFields::OnBitmapButton_SwatchClick);
	Connect(ID_BITMAPBUTTON_Swatch4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xlColorPickerFields::OnBitmapButton_SwatchClick);
	Connect(ID_BITMAPBUTTON_Swatch5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xlColorPickerFields::OnBitmapButton_SwatchClick);
	Connect(ID_BITMAPBUTTON_Swatch6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xlColorPickerFields::OnBitmapButton_SwatchClick);
	Connect(ID_BITMAPBUTTON_Swatch7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xlColorPickerFields::OnBitmapButton_SwatchClick);
	Connect(ID_BITMAPBUTTON_Swatch8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xlColorPickerFields::OnBitmapButton_SwatchClick);
	//*)

	Panel_Slider->SetType(xlColorCanvas::TYPE_SLIDER);
	SliderLeft->SetValue(0);
	SliderRight->SetValue(0);
    Panel_Palette->SetHSV(Panel_Slider->GetHSV());
    mActiveButton = BitmapButton_Swatch1;
}

xlColorPickerFields::~xlColorPickerFields()
{
	//(*Destroy(xlColorPickerFields)
	//*)
}

void xlColorPickerFields::SetColor(xlColor& color )
{
    mActiveButton->SetBackgroundColour(color.asWxColor());
    Panel_CurrentColor->SetBackgroundColour(color.asWxColor());
    mCurrentColor = color;
    Panel_Slider->SetRGB(mCurrentColor);
    Panel_Palette->SetRGB(mCurrentColor);
    int position = GetSliderPos();
    SliderLeft->SetValue(position);
    SliderRight->SetValue(position);
    Refresh(false);
    Update();
}

int xlColorPickerFields::GetSliderPos()
{
    HSVValue hsv = Panel_Slider->GetHSV();
    xlColor rgb = Panel_Slider->GetRGB();
    int range = Panel_Slider->GetSize().y-1;
    int position = 0;

    switch( mDisplayMode )
    {
    case MODE_HUE:
        position = range - (int)(hsv.hue * (double)range);
        break;
    case MODE_SATURATION:
        position = range - (int)(hsv.saturation * (double)range);
        break;
    case MODE_BRIGHTNESS:
        position = range - (int)(hsv.value * (double)range);
        break;
    case MODE_RED:
        position = range - (int)((double)rgb.red/255.0 * (double)range);
        break;
    case MODE_GREEN:
        position = range - (int)((double)rgb.green/255.0 * (double)range);
        break;
    case MODE_BLUE:
        position = range - (int)((double)rgb.blue/255.0 * (double)range);
        break;
    }
    return position;
}

void xlColorPickerFields::SetColorFromSliderPos(int position)
{
    HSVValue hsv = Panel_Slider->GetHSV();
    xlColor rgb = Panel_Slider->GetRGB();
    int range = Panel_Slider->GetSize().y-1;

    switch( mDisplayMode )
    {
    case MODE_HUE:
        hsv.hue = 1.0 - (double)position/(double)range;
        Panel_Slider->SetHSV(hsv);
        Panel_Palette->SetHSV(hsv);
        break;
    case MODE_SATURATION:
        hsv.saturation = 1.0 - (double)position/(double)range;
        Panel_Slider->SetHSV(hsv);
        Panel_Palette->SetHSV(hsv);
        break;
    case MODE_BRIGHTNESS:
        hsv.value = 1.0 - (double)position/(double)range;
        Panel_Slider->SetHSV(hsv);
        Panel_Palette->SetHSV(hsv);
        break;
    case MODE_RED:
        rgb.red = (255.0 * (1.0 - (double)position/(double)range));
        Panel_Slider->SetRGB(rgb);
        Panel_Palette->SetRGB(rgb);
        break;
    case MODE_GREEN:
        rgb.green = (255.0 * (1.0 - (double)position/(double)range));
        Panel_Slider->SetRGB(rgb);
        Panel_Palette->SetRGB(rgb);
        break;
    case MODE_BLUE:
        rgb.blue = (255.0 * (1.0 - (double)position/(double)range));
        Panel_Slider->SetRGB(rgb);
        Panel_Palette->SetRGB(rgb);
        break;
    }
}

void xlColorPickerFields::UpdateTextFields()
{
    HSVValue hsv = Panel_Slider->GetHSV();
    xlColor rgb = Panel_Slider->GetRGB();
    TextCtrl_Hue->ChangeValue(wxString::Format("%d", int(hsv.hue*359.0)));
    TextCtrl_Saturation->ChangeValue(wxString::Format("%d", int(hsv.saturation*100.0)));
    TextCtrl_Brightness->ChangeValue(wxString::Format("%d", int(hsv.value*100.0)));
    TextCtrl_Red->ChangeValue(wxString::Format("%d", rgb.red));
    TextCtrl_Green->ChangeValue(wxString::Format("%d", rgb.green));
    TextCtrl_Blue->ChangeValue(wxString::Format("%d", rgb.blue));
}

void xlColorPickerFields::ProcessSliderColorChange(wxCommandEvent& event)
{
    Panel_Palette->SetHSV(Panel_Slider->GetHSV());
    SliderLeft->SetValue(GetSliderPos());
    SliderRight->SetValue(GetSliderPos());
    mCurrentColor = Panel_Slider->GetRGB();
    Panel_CurrentColor->SetBackgroundColour(mCurrentColor.asWxColor());
    mActiveButton->SetBackgroundColour(mCurrentColor.asWxColor());
    NotifyColorChange();
    UpdateTextFields();
    Refresh(false);
    Update();
}

void xlColorPickerFields::ProcessPaletteColorChange(wxCommandEvent& event)
{
    Panel_Slider->SetHSV(Panel_Palette->GetHSV());
    mCurrentColor = Panel_Slider->GetRGB();
    Panel_CurrentColor->SetBackgroundColour(mCurrentColor.asWxColor());
    mActiveButton->SetBackgroundColour(mCurrentColor.asWxColor());
    NotifyColorChange();
    UpdateTextFields();
    Refresh(false);
    Update();
}

void xlColorPickerFields::OnSliderLeftCmdSliderUpdated(wxScrollEvent& event)
{
    SliderRight->SetValue(event.GetPosition());
    SetColorFromSliderPos(event.GetPosition());
    mCurrentColor = Panel_Slider->GetRGB();
    Panel_CurrentColor->SetBackgroundColour(mCurrentColor.asWxColor());
    mActiveButton->SetBackgroundColour(mCurrentColor.asWxColor());
    NotifyColorChange();
    UpdateTextFields();
    Refresh(false);
    Update();
}

void xlColorPickerFields::OnSliderRightCmdSliderUpdated(wxScrollEvent& event)
{
    SliderLeft->SetValue(event.GetPosition());
    SetColorFromSliderPos(event.GetPosition());
    mCurrentColor = Panel_Slider->GetRGB();
    Panel_CurrentColor->SetBackgroundColour(mCurrentColor.asWxColor());
    mActiveButton->SetBackgroundColour(mCurrentColor.asWxColor());
    NotifyColorChange();
    UpdateTextFields();
    Refresh(false);
    Update();
}

void xlColorPickerFields::OnRadioButton_ModeSelect(wxCommandEvent& event)
{
    wxRadioButton * radio_button = (wxRadioButton*)event.GetEventObject();
    wxString name = radio_button->GetName();
    if (name.Contains("ID_RADIOBUTTON_Hue")) {
        mDisplayMode = MODE_HUE;
    }
    else if (name.Contains("ID_RADIOBUTTON_Saturation")) {
        mDisplayMode = MODE_SATURATION;
    }
    else if (name.Contains("ID_RADIOBUTTON_Brightness")) {
        mDisplayMode = MODE_BRIGHTNESS;
    }
    else if (name.Contains("ID_RADIOBUTTON_Red")) {
        mDisplayMode = MODE_RED;
    }
    else if (name.Contains("ID_RADIOBUTTON_Green")) {
        mDisplayMode = MODE_GREEN;
    }
    else if (name.Contains("ID_RADIOBUTTON_Blue")) {
        mDisplayMode = MODE_BLUE;
    }
    Panel_Slider->SetMode(mDisplayMode);
    Panel_Palette->SetMode(mDisplayMode);
    int position = GetSliderPos();
    SliderLeft->SetValue(position);
    SliderRight->SetValue(position);
    Refresh(false);
    Update();
}

void xlColorPickerFields::OnTextCtrl_Text(wxCommandEvent& event)
{
    wxTextCtrl * text_ctrl = (wxTextCtrl*)event.GetEventObject();
    wxString name = text_ctrl->GetName();
    HSVValue hsv = Panel_Slider->GetHSV();
    xlColor rgb = Panel_Slider->GetRGB();
    if (name.Contains("ID_TEXTCTRL_Hue")) {
        int hue = RangeLimit( TextCtrl_Hue, 0, 359 );
        hsv.hue = double(hue) / 359.0;
        Panel_Slider->SetHSV(hsv);
        Panel_Palette->SetHSV(hsv);
    }
    else if (name.Contains("ID_TEXTCTRL_Saturation")) {
        int sat = RangeLimit( TextCtrl_Saturation, 0, 100 );
        hsv.saturation = double(sat) / 100.0;
        Panel_Slider->SetHSV(hsv);
        Panel_Palette->SetHSV(hsv);
    }
    else if (name.Contains("ID_TEXTCTRL_BrightnessCPF")) {
        int val = RangeLimit( TextCtrl_Brightness, 0, 100 );
        hsv.value = double(val) / 100.0;
        Panel_Slider->SetHSV(val);
        Panel_Palette->SetHSV(hsv);
    }
    else if (name.Contains("ID_TEXTCTRL_Red")) {
        int red = RangeLimit( TextCtrl_Red, 0, 255 );
        rgb.red = double(red) / 255.0;
        Panel_Slider->SetRGB(rgb);
        Panel_Palette->SetRGB(rgb);
    }
    else if (name.Contains("ID_TEXTCTRL_Green")) {
        int green = RangeLimit( TextCtrl_Green, 0, 255 );
        rgb.green = double(green) / 255.0;
        Panel_Slider->SetRGB(rgb);
        Panel_Palette->SetRGB(rgb);
    }
    else if (name.Contains("ID_TEXTCTRL_Blue")) {
        int blue = RangeLimit( TextCtrl_Blue, 0, 255 );
        rgb.blue = double(blue) / 255.0;
        Panel_Slider->SetRGB(rgb);
        Panel_Palette->SetRGB(rgb);
    }
    int position = GetSliderPos();
    SliderLeft->SetValue(position);
    SliderRight->SetValue(position);
    Refresh(false);
    Update();
}

int xlColorPickerFields::RangeLimit( wxTextCtrl* text_ctrl, int lower_limit, int upper_limit )
{
    int value = wxAtoi(text_ctrl->GetValue());
    int value_old = value;
    value = std::max(value,lower_limit);
    value = std::min(value,upper_limit);
    if( value != value_old )
    {
        text_ctrl->SetValue(wxString::Format("%d", value));
    }
    return value;
}

void xlColorPickerFields::OnBitmapButton_SwatchClick(wxCommandEvent& event)
{
    mActiveButton = (wxBitmapButton*)event.GetEventObject();
    wxString name = mActiveButton->GetName();
    name.Replace("ID_BITMAPBUTTON_Swatch", "");
    int selected_column = wxAtoi(name);
	GridBagSizer1->SetItemPosition(RadioButton_SwatchMarker, wxGBPosition(12, selected_column));
	GridBagSizer1->Layout();
    wxColor button_color = mActiveButton->GetBackgroundColour();
    Panel_CurrentColor->SetBackgroundColour(button_color);
    mCurrentColor = button_color;
    Panel_Slider->SetRGB(mCurrentColor);
    Panel_Palette->SetRGB(mCurrentColor);
    int position = GetSliderPos();
    SliderLeft->SetValue(position);
    SliderRight->SetValue(position);
    NotifyColorChange();
    Refresh(false);
    Update();
}

void xlColorPickerFields::NotifyColorChange()
{
    wxCommandEvent eventColor(EVT_PAINT_COLOR);
    eventColor.SetClientData(&mCurrentColor);
    wxPostEvent(GetParent(), eventColor);
}
