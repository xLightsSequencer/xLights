#include "ColorPanel.h"
#include "xLightsMain.h"

#include "../include/padlock16x16-green.xpm" //-DJ
#include "../include/padlock16x16-red.xpm" //-DJ
#include "../include/padlock16x16-blue.xpm" //-DJ
#include "ColorCurve.h"
#include "ColorCurveDialog.h"

class xLightsFrame;
//(*InternalHeaders(ColorPanel)
#include <wx/bitmap.h>
#include <wx/settings.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

#include <wx/valnum.h>
#include "Color.h"
#include "effects/EffectPanelUtils.h"

//(*IdInit(ColorPanel)
const long ColorPanel::ID_BUTTON1 = wxNewId();
const long ColorPanel::ID_STATICTEXT24 = wxNewId();
const long ColorPanel::ID_SLIDER_SparkleFrequency = wxNewId();
const long ColorPanel::ID_VALUECURVE_SparkleFrequency = wxNewId();
const long ColorPanel::IDD_TEXTCTRL_SparkleFrequency = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON_SLIDER_SparkleFrequency = wxNewId();
const long ColorPanel::ID_CHECKBOX_MusicSparkles = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON_MusicSparkles = wxNewId();
const long ColorPanel::ID_STATICTEXT127 = wxNewId();
const long ColorPanel::ID_SLIDER_Brightness = wxNewId();
const long ColorPanel::ID_VALUECURVE_Brightness = wxNewId();
const long ColorPanel::IDD_TEXTCTRL_Brightness = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON_SLIDER_Brightness = wxNewId();
const long ColorPanel::ID_STATICTEXT128 = wxNewId();
const long ColorPanel::ID_SLIDER_Contrast = wxNewId();
const long ColorPanel::IDD_TEXTCTRL_Contrast = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON_SLIDER_Contrast = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON87 = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON1 = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON88 = wxNewId();
const long ColorPanel::ID_SCROLLED_ColorScroll = wxNewId();
const long ColorPanel::ID_PANEL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ColorPanel,wxPanel)
	//(*EventTable(ColorPanel)
	//*)
END_EVENT_TABLE()

ColorPanel::ColorPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    __brightness = 100;
    wxIntegerValidator<int> _brightness(&__brightness, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _brightness.SetMin(0);
    _brightness.SetMax(400);

	//(*Initialize(ColorPanel)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxButton* ButtonColor1;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	Panel_Sizer = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	ColorScrollWindow = new wxScrolledWindow(Panel_Sizer, ID_SCROLLED_ColorScroll, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLED_ColorScroll"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer5 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer5->AddGrowableCol(1);
	FlexGridSizer5->AddGrowableCol(2);
	FlexGridSizer5->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	FlexGridSizer_Palette = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer5->Add(FlexGridSizer_Palette, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer5->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	ButtonColor1 = new wxButton(ColorScrollWindow, ID_BUTTON1, _("Update"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer5->Add(ButtonColor1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText22 = new wxStaticText(ColorScrollWindow, ID_STATICTEXT24, _("Sparkles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT24"));
	FlexGridSizer2->Add(StaticText22, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	Slider_SparkleFrequency = new wxSlider(ColorScrollWindow, ID_SLIDER_SparkleFrequency, 0, 0, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_SparkleFrequency"));
	FlexGridSizer8->Add(Slider_SparkleFrequency, 1, wxALL|wxEXPAND, 2);
	BitmapButton_SparkleFrequencyVC = new ValueCurveButton(ColorScrollWindow, ID_VALUECURVE_SparkleFrequency, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_SparkleFrequency"));
	FlexGridSizer8->Add(BitmapButton_SparkleFrequencyVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 2);
	txtCtrlSparkleFreq = new wxTextCtrl(ColorScrollWindow, IDD_TEXTCTRL_SparkleFrequency, _("0"), wxDefaultPosition, wxDLG_UNIT(ColorScrollWindow,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_SparkleFrequency"));
	FlexGridSizer2->Add(txtCtrlSparkleFreq, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_SparkleFrequency = new wxBitmapButton(ColorScrollWindow, ID_BITMAPBUTTON_SLIDER_SparkleFrequency, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_SparkleFrequency"));
	BitmapButton_SparkleFrequency->SetDefault();
	BitmapButton_SparkleFrequency->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_SparkleFrequency->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	FlexGridSizer2->Add(BitmapButton_SparkleFrequency, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_MusicSparkles = new wxCheckBox(ColorScrollWindow, ID_CHECKBOX_MusicSparkles, _("Sparkles reflect music"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MusicSparkles"));
	CheckBox_MusicSparkles->SetValue(false);
	FlexGridSizer2->Add(CheckBox_MusicSparkles, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_MusicSparkles = new wxBitmapButton(ColorScrollWindow, ID_BITMAPBUTTON_MusicSparkles, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_MusicSparkles"));
	BitmapButton_MusicSparkles->SetDefault();
	BitmapButton_MusicSparkles->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_MusicSparkles->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	FlexGridSizer2->Add(BitmapButton_MusicSparkles, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText126 = new wxStaticText(ColorScrollWindow, ID_STATICTEXT127, _("Brightness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT127"));
	FlexGridSizer2->Add(StaticText126, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	Slider_Brightness = new wxSlider(ColorScrollWindow, ID_SLIDER_Brightness, 100, 0, 400, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Brightness"));
	FlexGridSizer7->Add(Slider_Brightness, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCBrightness = new ValueCurveButton(ColorScrollWindow, ID_VALUECURVE_Brightness, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Brightness"));
	FlexGridSizer7->Add(BitmapButton_VCBrightness, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer2->Add(FlexGridSizer7, 1, wxEXPAND, 0);
	txtCtlBrightness = new wxTextCtrl(ColorScrollWindow, IDD_TEXTCTRL_Brightness, _("100"), wxDefaultPosition, wxDLG_UNIT(ColorScrollWindow,wxSize(20,-1)), 0, _brightness, _T("IDD_TEXTCTRL_Brightness"));
	FlexGridSizer2->Add(txtCtlBrightness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Brightness = new wxBitmapButton(ColorScrollWindow, ID_BITMAPBUTTON_SLIDER_Brightness, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Brightness"));
	BitmapButton_Brightness->SetDefault();
	BitmapButton_Brightness->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_Brightness->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	FlexGridSizer2->Add(BitmapButton_Brightness, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText127 = new wxStaticText(ColorScrollWindow, ID_STATICTEXT128, _("Contrast"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT128"));
	FlexGridSizer2->Add(StaticText127, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Contrast = new wxSlider(ColorScrollWindow, ID_SLIDER_Contrast, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Contrast"));
	FlexGridSizer2->Add(Slider_Contrast, 1, wxALL|wxEXPAND, 2);
	txtCtlContrast = new wxTextCtrl(ColorScrollWindow, IDD_TEXTCTRL_Contrast, _("0"), wxDefaultPosition, wxDLG_UNIT(ColorScrollWindow,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Contrast"));
	FlexGridSizer2->Add(txtCtlContrast, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Contrast = new wxBitmapButton(ColorScrollWindow, ID_BITMAPBUTTON_SLIDER_Contrast, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Contrast"));
	BitmapButton_Contrast->SetDefault();
	BitmapButton_Contrast->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_Contrast->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	FlexGridSizer2->Add(BitmapButton_Contrast, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer4->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer6 = new wxFlexGridSizer(0, 4, 0, 0);
	BitmapButton_normal = new wxBitmapButton(ColorScrollWindow, ID_BITMAPBUTTON87, padlock16x16_green_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON87"));
	BitmapButton_normal->SetDefault();
	BitmapButton_normal->Hide();
	FlexGridSizer6->Add(BitmapButton_normal, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_locked = new wxBitmapButton(ColorScrollWindow, ID_BITMAPBUTTON1, padlock16x16_red_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
	BitmapButton_locked->SetDefault();
	BitmapButton_locked->Hide();
	FlexGridSizer6->Add(BitmapButton_locked, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_random = new wxBitmapButton(ColorScrollWindow, ID_BITMAPBUTTON88, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON88"));
	BitmapButton_random->SetDefault();
	BitmapButton_random->Hide();
	FlexGridSizer6->Add(BitmapButton_random, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer4->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ColorScrollWindow->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(ColorScrollWindow);
	FlexGridSizer4->SetSizeHints(ColorScrollWindow);
	FlexGridSizer3->Add(ColorScrollWindow, 1, wxALL|wxEXPAND, 0);
	Panel_Sizer->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel_Sizer);
	FlexGridSizer3->SetSizeHints(Panel_Sizer);
	FlexGridSizer1->Add(Panel_Sizer, 1, wxALL|wxEXPAND, 0);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnUpdateColorClick);
	Connect(ID_SLIDER_SparkleFrequency,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ColorPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_SparkleFrequency,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_SparkleFrequency,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ColorPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_SparkleFrequency,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnLockButtonClick);
	Connect(ID_CHECKBOX_MusicSparkles,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ColorPanel::OnCheckBox_MusicSparklesClick);
	Connect(ID_BITMAPBUTTON_MusicSparkles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Brightness,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ColorPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Brightness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Brightness,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ColorPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Brightness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Contrast,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ColorPanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_Contrast,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ColorPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Contrast,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnLockButtonClick);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&ColorPanel::OnResize);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&ColorPanel::OnVCChanged, 0, this);

    BitmapButton_VCBrightness->GetValue()->SetLimits(0, 400);
    BitmapButton_SparkleFrequencyVC->GetValue()->SetLimits(0, 200);

    FlexGridSizer_Palette->SetCols(PALETTE_SIZE);
    for (int x = 0; x < PALETTE_SIZE; x++) {
        FlexGridSizer_Palette->AddGrowableCol(x);
        wxString ids = wxString::Format("ID_CHECKBOX_Palette%d", (x + 1));
        long id = wxNewId();
        wxCheckBox *cb = new wxCheckBox(ColorScrollWindow, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, ids);
        cb->SetValue(x < 2);
        FlexGridSizer_Palette->Add(cb, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
        checkBoxes.push_back(cb);
        Connect(id, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&ColorPanel::OnCheckBox_PaletteClick);
    }
    for (int x = 0; x < PALETTE_SIZE; x++) {
        wxString ids = wxString::Format("ID_BUTTON_Palette%d", (x + 1));
        long id = wxNewId();
        ColorCurveButton *bb = new ColorCurveButton(ColorScrollWindow, id, wxNullBitmap, wxDefaultPosition, wxSize(21,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, ids);
        bb->SetDefault();
        FlexGridSizer_Palette->Add(bb, 0, wxALIGN_LEFT|wxALIGN_TOP, 0);
        buttons.push_back(bb);
        //Connect(id, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ColorPanel::OnButton_PaletteNumberClick);
        //Connect(id, wxEVT_COMMAND_RIGHT_CLICK, (wxObjectEventFunction)&ColorPanel::OnButton_PaletteNumberRClick);
        Connect(wxID_ANY, EVT_CC_CHANGED, (wxObjectEventFunction)&ColorPanel::OnCCChanged, 0, this);
    }
    for (int x = 0; x < PALETTE_SIZE; x++) {
        wxString ids = wxString::Format("ID_BITMAPBUTTON_BUTTON_Palette%d", (x + 1));
        long id = wxNewId();
        wxBitmapButton *bb = new wxBitmapButton(ColorScrollWindow, id, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, ids);
        bb->SetDefault();
        bb->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
        FlexGridSizer_Palette->Add(bb, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
        Connect(id,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnLockButtonClick);
    }
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    SetDefaultPalette();

    ValidateWindow();
}

ColorPanel::~ColorPanel()
{
	//(*Destroy(ColorPanel)
	//*)
}


PANEL_EVENT_HANDLERS(ColorPanel)

void ColorPanel::SetColorCount(int count)
{
    // Disabling this as we dont really want to limit the colours to from left to right ... but it would be clearer
    return;

    if (count == -1)
    {
        for (int i = 0; i < PALETTE_SIZE; i++)
        {
            wxString ids = wxString::Format("ID_CHECKBOX_Palette%d", (i + 1));
            wxWindow* CtrlWin = wxWindow::FindWindowByName(ids, this);
            CtrlWin->Enable();
            ids = wxString::Format("ID_BUTTON_Palette%d", (i + 1));
            CtrlWin = wxWindow::FindWindowByName(ids, this);
            CtrlWin->Enable();
            ids = wxString::Format("ID_BITMAPBUTTON_BUTTON_Palette%d", (i + 1));
            CtrlWin = wxWindow::FindWindowByName(ids, this);
            CtrlWin->Enable();
        }
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            wxString ids = wxString::Format("ID_CHECKBOX_Palette%d", (i + 1));
            wxWindow* CtrlWin = wxWindow::FindWindowByName(ids, this);
            CtrlWin->Enable();
            ids = wxString::Format("ID_BUTTON_Palette%d", (i + 1));
            CtrlWin = wxWindow::FindWindowByName(ids, this);
            CtrlWin->Enable();
            ids = wxString::Format("ID_BITMAPBUTTON_BUTTON_Palette%d", (i + 1));
            CtrlWin = wxWindow::FindWindowByName(ids, this);
            CtrlWin->Enable();
        }
        for (int i = count; i < PALETTE_SIZE; i++)
        {
            wxString ids = wxString::Format("ID_CHECKBOX_Palette%d", (i + 1));
            wxWindow* CtrlWin = wxWindow::FindWindowByName(ids, this);
            CtrlWin->Disable();
            ids = wxString::Format("ID_BUTTON_Palette%d", (i + 1));
            CtrlWin = wxWindow::FindWindowByName(ids, this);
            CtrlWin->Disable();
            ids = wxString::Format("ID_BITMAPBUTTON_BUTTON_Palette%d", (i + 1));
            CtrlWin = wxWindow::FindWindowByName(ids, this);
            CtrlWin->Disable();
        }
    }
}

void ColorPanel::SetButtonColor(ColorCurveButton* btn, const std::string& cstr)
{
    if (cstr.find("Active") != std::string::npos)
    {
        btn->GetValue()->Deserialise(cstr);
        btn->UpdateState();
    }
    else
    {
        btn->SetActive(false);
        btn->SetColor(cstr);
    }
}

wxString ColorPanel::GetRandomColorString() {
    wxString ChildName,AttrName;
    wxString ret;
    // get palette
    wxColour color;
    for (int i=0; i < PALETTE_SIZE; i++) {
        color = GetPaletteColor(i);
        AttrName.Printf("C_BUTTON_Palette%d=", (i+1));
        ret += AttrName + color.GetAsString(wxC2S_HTML_SYNTAX) + ",";

        wxString v = (!EffectPanelUtils::IsLocked(GetPaletteButton(i)->GetName().ToStdString())? rand() % 2: GetPaletteCheckbox(i)->GetValue()) ? "1" : "0";
        AttrName.Printf("C_CHECKBOX_Palette%d=", (i+1));
        ret += AttrName + v + ",";
    }
    //TODO: randomize
    if (Slider_SparkleFrequency->GetValue() != 0) {
        ret+= wxString::Format("C_SLIDER_SparkleFrequency=%d,",Slider_SparkleFrequency->GetValue());
    }
    if (CheckBox_MusicSparkles->GetValue())
    {
        ret += wxString::Format("C_CHECKBOX_MusicSparkles=%d,", CheckBox_MusicSparkles->GetValue());
    }
    if (Slider_Brightness->GetValue() != 100) {
        ret+= wxString::Format("C_SLIDER_Brightness=%d,",Slider_Brightness->GetValue());
    }
    if (Slider_Contrast->GetValue() != 0) {
        ret+= wxString::Format("C_SLIDER_Contrast=%d",Slider_Contrast->GetValue());
    }
    return ret;
}

wxString ColorPanel::GetColorString()
{
    wxString s,ChildName,AttrName;
    wxColour color;
    for (int i=0; i < PALETTE_SIZE; i++)
    {
        wxString ids = wxString::Format("ID_BUTTON_Palette%d", (i + 1));
        ColorCurveButton* btn = (ColorCurveButton*)wxWindow::FindWindowByName(ids, this);
        if (btn->GetValue()->IsActive())
        {
            AttrName.Printf("C_BUTTON_Palette%d=", (i + 1));
            s += AttrName + btn->GetValue()->Serialise() + ",";
        }
        else
        {
            color = GetPaletteColor(i);
            AttrName.Printf("C_BUTTON_Palette%d=", (i + 1));
            s += AttrName + color.GetAsString(wxC2S_HTML_SYNTAX) + ",";
        }

        if (checkBoxes[i]->IsChecked()) {
            AttrName.Printf("C_CHECKBOX_Palette%d=1,",(i+1));
            s += AttrName;
        }
    }
    if (BitmapButton_SparkleFrequencyVC->GetValue()->IsActive())
    {
        wxString sparkleVC = wxString(BitmapButton_SparkleFrequencyVC->GetValue()->Serialise().c_str());
        s += "C_VALUECURVE_SparkleFrequency=";
        s += sparkleVC;
        s += ",";
    }
    else
    {
        if (Slider_SparkleFrequency->GetValue() != 0) {
            s += wxString::Format("C_SLIDER_SparkleFrequency=%d,", Slider_SparkleFrequency->GetValue());
        }
    }
    if (CheckBox_MusicSparkles->GetValue())
    {
        s += wxString::Format("C_CHECKBOX_MusicSparkles=%d,", CheckBox_MusicSparkles->GetValue());
    }
    if (BitmapButton_VCBrightness->GetValue()->IsActive())
    {
        wxString brightnessVC = wxString(BitmapButton_VCBrightness->GetValue()->Serialise().c_str());
        s += "C_VALUECURVE_Brightness=";
        s += brightnessVC;
        s += ",";
    }
    else
    {
        if (Slider_Brightness->GetValue() != 100) {
            s += wxString::Format("C_SLIDER_Brightness=%d,", Slider_Brightness->GetValue());
        }
    }

    if (Slider_Contrast->GetValue() != 0) {
        s+= wxString::Format("C_SLIDER_Contrast=%d",Slider_Contrast->GetValue());
    }
    return s;
}



wxColour ColorPanel::GetPaletteColor(int idx)
{
    if (idx < buttons.size()) {
        return buttons[idx]->GetBackgroundColour();
    }
    return *wxBLACK;
}

wxCheckBox* ColorPanel::GetPaletteCheckbox(int idx)
{
    if (idx < checkBoxes.size()) {
        return checkBoxes[idx];
    }
    return checkBoxes[0]; //0;
}

wxButton* ColorPanel::GetPaletteButton(int idx)
{
    if (idx < buttons.size()) {
        return buttons[idx];
    }
    return buttons[0]; //0;
}

void ColorPanel::SetDefaultSettings() {
    for (auto it = checkBoxes.begin(); it != checkBoxes.end(); it++) {
        (*it)->SetValue(false);
    }
    BitmapButton_SparkleFrequencyVC->GetValue()->SetDefault(0.0f, 200.0f);
    BitmapButton_SparkleFrequencyVC->UpdateState();
    Slider_SparkleFrequency->SetValue(0);
    CheckBox_MusicSparkles->SetValue(false);
    __brightness = 100;
    BitmapButton_VCBrightness->GetValue()->SetDefault(0.0f, 400.0f);
    BitmapButton_VCBrightness->UpdateState();
    Slider_Brightness->SetValue(100);
    txtCtlBrightness->SetValue("100");
    Slider_Contrast->SetValue(0);
    txtCtlContrast->SetValue("0");
    txtCtrlSparkleFreq->SetValue("0");
}

void ColorPanel::SetDefaultPalette()
{
    SetButtonColor(buttons[0],"#FFFFFF");
    SetButtonColor(buttons[1],"#FF0000");
    SetButtonColor(buttons[2],"#00FF00");
    SetButtonColor(buttons[3],"#0000FF");
    SetButtonColor(buttons[4],"#FFFF00");
    SetButtonColor(buttons[5],"#000000");
    SetButtonColor(buttons[6],"#00FFFF");
    SetButtonColor(buttons[7],"#FF00FF");
}


void ColorPanel::OnCheckBox_PaletteClick(wxCommandEvent& event)
{
    PaletteChanged=true;
}

void ColorPanel::OnCCChanged(wxCommandEvent& event)
{
    ColorCurveButton* w = (ColorCurveButton*)event.GetEventObject();
    lastColors[w->GetId()] = w->GetColor();
    Refresh();
}

void ColorPanel::OnResize(wxSizeEvent& event)
{
    wxSize s = GetSize();
    Panel_Sizer->SetSize(s);
    Panel_Sizer->SetMinSize(s);
    Panel_Sizer->SetMaxSize(s);
    Panel_Sizer->Refresh();

    ColorScrollWindow->SetSize(s);
    ColorScrollWindow->SetMinSize(s);
    ColorScrollWindow->SetMaxSize(s);

    ColorScrollWindow->FitInside();
    ColorScrollWindow->SetScrollRate(5, 5);
    ColorScrollWindow->Refresh();
}

void ColorPanel::OnUpdateColorClick(wxCommandEvent& event)
{
    wxCommandEvent eventEffectUpdated(EVT_EFFECT_PALETTE_UPDATED);
    wxPostEvent(GetParent(), eventEffectUpdated);
    Refresh();
}

void ColorPanel::OnCheckBox_MusicSparklesClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void ColorPanel::ValidateWindow()
{
}

