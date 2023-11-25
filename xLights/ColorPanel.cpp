/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#define ZERO 0

#include "xLightsMain.h"
#include "ColorPanel.h"
#include "ColorCurve.h"
#include "effects/EffectPanelUtils.h"
#include "UtilFunctions.h"
#include "xLightsApp.h"
#include "xLightsMain.h"
#include "ExternalHooks.h"
#include "sequencer/MainSequencer.h"

//(*InternalHeaders(ColorPanel)
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/string.h>
//*)

#include <wx/artprov.h>
#include <wx/dnd.h>
#include <wx/valnum.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/odcombo.h>
#include <wx/config.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/regex.h>

#include <log4cpp/Category.hh>

#define PALETTE_SIZE 8

//(*IdInit(ColorPanel)
const long ColorPanel::ID_BITMAPBUTTON_ReverseColours = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON_LeftShiftColours = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON_RightShiftColours = wxNewId();
const long ColorPanel::ID_CUSTOM1 = wxNewId();
const long ColorPanel::ID_BUTTON1 = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON3 = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON2 = wxNewId();
const long ColorPanel::ID_CHECKBOX_ResetColorPanel = wxNewId();
const long ColorPanel::ID_STATICTEXT1 = wxNewId();
const long ColorPanel::ID_SLIDER_ChromaSensitivity = wxNewId();
const long ColorPanel::ID_COLOURPICKERCTRL_ChromaColour = wxNewId();
const long ColorPanel::ID_CHECKBOX_Chroma = wxNewId();
const long ColorPanel::ID_STATICTEXT_SparkleFrequency = wxNewId();
const long ColorPanel::ID_SLIDER_SparkleFrequency = wxNewId();
const long ColorPanel::ID_VALUECURVE_SparkleFrequency = wxNewId();
const long ColorPanel::IDD_TEXTCTRL_SparkleFrequency = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON_SLIDER_SparkleFrequency = wxNewId();
const long ColorPanel::ID_CHECKBOX_MusicSparkles = wxNewId();
const long ColorPanel::ID_COLOURPICKERCTRL_SparklesColour = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON_MusicSparkles = wxNewId();
const long ColorPanel::ID_STATICTEXT_Brightness = wxNewId();
const long ColorPanel::ID_SLIDER_Brightness = wxNewId();
const long ColorPanel::ID_VALUECURVE_Brightness = wxNewId();
const long ColorPanel::IDD_TEXTCTRL_Brightness = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON_SLIDER_Brightness = wxNewId();
const long ColorPanel::ID_STATICTEXT_Contrast = wxNewId();
const long ColorPanel::ID_SLIDER_Contrast = wxNewId();
const long ColorPanel::IDD_TEXTCTRL_Contrast = wxNewId();
const long ColorPanel::ID_BITMAPBUTTON_SLIDER_Contrast = wxNewId();
const long ColorPanel::ID_STATICTEXT4 = wxNewId();
const long ColorPanel::ID_STATICTEXT_Color_HueAdjust = wxNewId();
const long ColorPanel::ID_SLIDER_Color_HueAdjust = wxNewId();
const long ColorPanel::ID_VALUECURVE_Color_HueAdjust = wxNewId();
const long ColorPanel::IDD_TEXTCTRL_Color_HueAdjust = wxNewId();
const long ColorPanel::ID_STATICTEXT_Color_SaturationAdjust = wxNewId();
const long ColorPanel::ID_SLIDER_Color_SaturationAdjust = wxNewId();
const long ColorPanel::ID_VALUECURVE_Color_SaturationAdjust = wxNewId();
const long ColorPanel::IDD_TEXTCTRL_Color_SaturationAdjust = wxNewId();
const long ColorPanel::ID_STATICTEXT_Color_ValueAdjust = wxNewId();
const long ColorPanel::ID_SLIDER_Color_ValueAdjust = wxNewId();
const long ColorPanel::ID_VALUECURVE_Color_ValueAdjust = wxNewId();
const long ColorPanel::IDD_TEXTCTRL_Color_ValueAdjust = wxNewId();
const long ColorPanel::ID_SCROLLED_ColorScroll = wxNewId();
const long ColorPanel::ID_PANEL1 = wxNewId();
//*)

#define SWATCH_WIDTH 11
class ColourList : public wxOwnerDrawnComboBox
{
public:
    ColourList(wxWindow *parent, wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = "ColourList") : wxOwnerDrawnComboBox(parent, id, wxEmptyString, pos, size, 0, nullptr, style | wxCB_READONLY, validator, name)
    {

    }

    virtual wxCoord OnMeasureItem(size_t item) const
    {
        return 18;
    }

    virtual wxCoord OnMeasureItemWidth(size_t item) const
    {
        return PALETTE_SIZE * SWATCH_WIDTH - 1;
    }

    virtual void OnDrawItem(wxDC &dc, const wxRect &rect, int item, int flags) const
    {
        if (item == wxNOT_FOUND)
            return;

        wxString s = GetString(item);

        if (s == "(Load)")
        {
            if (rect.GetHeight() != 18)
            {
                wxFontMetrics fm = dc.GetFontMetrics();
                dc.SetTextForeground(*wxBLACK);
                dc.DrawText(s, rect.GetLeft() + 2, rect.GetTop() + (rect.GetHeight() - fm.height) / 2);
            }
            GetVListBoxComboPopup()->UnsetToolTip();
        }
        else
        {
            wxArrayString as = wxSplit(s, ',');

            int i = 0;

            for (auto it = as.begin(); it != as.end() && i < PALETTE_SIZE; ++it)
            {
                if (it->Contains("Active"))
                {
                    ColorCurve cc(it->ToStdString());
                    dc.DrawBitmap(cc.GetImage(SWATCH_WIDTH - 1, rect.GetHeight()-1, false), i * SWATCH_WIDTH, rect.GetTop());
                }
                else
                {
                    xlColor c;
                    c.SetFromString(it->ToStdString());
                    wxPen p(c.asWxColor());
                    wxBrush b(c.asWxColor());
                    dc.SetPen(p);
                    dc.SetBrush(b);
                    dc.DrawRectangle(i * SWATCH_WIDTH, rect.GetTop(), SWATCH_WIDTH - 1, rect.GetHeight()-1);
                }
                i++;
            }

            if (flags & wxODCB_PAINTING_SELECTED)
            {
                wxString file = as.back();
                GetVListBoxComboPopup()->SetToolTip(file);
            }
        }
    }
};

class ColourText2DropTarget : public wxTextDropTarget
{
public:
    ColourText2DropTarget(BulkEditColourPickerCtrl* owner) { _owner = owner; };

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override;
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;

    BulkEditColourPickerCtrl* _owner;
};

wxDragResult ColourText2DropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
    if (_owner->IsEnabled()) {
        _owner->SetFocus();
        return wxDragCopy;
    }
    return wxDragNone;
}

bool ColourText2DropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
    if (data == "") return false;

    if (!_owner->IsEnabled()) return false;

    if (ColorCurve::IsColorCurve(data)) {
        wxMessageBox("You cannot drag a colour curve onto this colour picker.");
    }
    else {
        _owner->SetColour(data);
    }
    return true;
}

class ColourTextDropTarget : public wxTextDropTarget
{
public:
    ColourTextDropTarget(ColorCurveButton* owner) { _owner = owner; };

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override;
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;

    ColorCurveButton* _owner;
};

wxDragResult ColourTextDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
    if (_owner->IsEnabled())
    {
        _owner->SetFocus();
        return wxDragCopy;
    }
    return wxDragNone;
}

bool ColourTextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
    if (data == "") return false;

    if (!_owner->IsEnabled()) return false;

    if (ColorCurve::IsColorCurve(data))
    {
        _owner->SetValue(data);
        _owner->SetActive(true);
    }
    else
    {
        _owner->SetColor(data, false);
    }
    _owner->NotifyChange();

    return true;
}

BEGIN_EVENT_TABLE(ColorPanel,wxPanel)
	//(*EventTable(ColorPanel)
	//*)
END_EVENT_TABLE()

ColorPanel::ColorPanel(wxWindow* parent, wxWindowID id,const wxPoint& pos,const wxSize& size) : xlEffectPanel(parent)
{
    _supportslinear = false;
    _supportsradial = false;

	//(*Initialize(ColorPanel)
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer3;
	wxButton* ButtonColor1;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer11;
	wxFlexGridSizer* FlexGridSizer12;
	wxFlexGridSizer* FlexGridSizer13;
	wxFlexGridSizer* FlexGridSizer14;
	wxFlexGridSizer* FlexGridSizer15;
	wxFlexGridSizer* FlexGridSizer16;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer9;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	Panel_Sizer = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	ColorScrollWindow = new wxScrolledWindow(Panel_Sizer, ID_SCROLLED_ColorScroll, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLED_ColorScroll"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer9 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	FlexGridSizer9->AddGrowableCol(1);
	FlexGridSizer9->AddGrowableCol(2);
	FlexGridSizer9->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	FlexGridSizer10 = new wxFlexGridSizer(0, 0, 0, 0);
	FlexGridSizer10->AddGrowableCol(0);
	FlexGridSizer_Palette = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer10->Add(FlexGridSizer_Palette, 1, wxALL, 2);
	FlexGridSizer9->Add(FlexGridSizer10, 1, wxALL|wxALIGN_RIGHT, 2);
	FlexGridSizer11 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer11->AddGrowableCol(1);
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BitmapButton_ReverseColours = new xlSizedBitmapButton(ColorScrollWindow, ID_BITMAPBUTTON_ReverseColours, wxArtProvider::GetBitmapBundle("xlART_colorpanel_reverse_xpm", wxART_BUTTON), wxDefaultPosition, wxSize(26,16), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_ReverseColours"));
	BoxSizer1->Add(BitmapButton_ReverseColours, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_LeftShiftColours = new xlSizedBitmapButton(ColorScrollWindow, ID_BITMAPBUTTON_LeftShiftColours, wxArtProvider::GetBitmapBundle("xlART_colorpanel_left_shift_xpm", wxART_BUTTON), wxDefaultPosition, wxSize(26,16), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_LeftShiftColours"));
	BoxSizer1->Add(BitmapButton_LeftShiftColours, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_RightShiftColours = new xlSizedBitmapButton(ColorScrollWindow, ID_BITMAPBUTTON_RightShiftColours, wxArtProvider::GetBitmapBundle("xlART_colorpanel_right_shift_xpm", wxART_BUTTON), wxDefaultPosition, wxSize(26,16), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_RightShiftColours"));
	BoxSizer1->Add(BitmapButton_RightShiftColours, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer11->Add(BoxSizer1, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	BoxSizer2 = new wxBoxSizer(wxVERTICAL);
	BitmapButton_ColourChoice = new ColourList(ColorScrollWindow,ID_CUSTOM1,wxDefaultPosition,wxDefaultSize,ZERO,wxDefaultValidator,_T("ID_CUSTOM1"));
	BoxSizer2->Add(BitmapButton_ColourChoice, 1, wxALL|wxEXPAND, 2);
	ButtonColor1 = new wxButton(ColorScrollWindow, ID_BUTTON1, _("Update"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer2->Add(ButtonColor1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer11->Add(BoxSizer2, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	BoxSizer3 = new wxBoxSizer(wxVERTICAL);
	BitmapButton_SavePalette = new xlSizedBitmapButton(ColorScrollWindow, ID_BITMAPBUTTON3, wxArtProvider::GetBitmapBundle("xlART_colorpanel_save_xpm", wxART_BUTTON), wxDefaultPosition, wxSize(24,24), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON3"));
	BoxSizer3->Add(BitmapButton_SavePalette, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_DeletePalette = new xlSizedBitmapButton(ColorScrollWindow, ID_BITMAPBUTTON2, wxArtProvider::GetBitmapBundle("xlART_colorpanel_delete_xpm", wxART_BUTTON), wxDefaultPosition, wxSize(24,24), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON2"));
	BoxSizer3->Add(BitmapButton_DeletePalette, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer11->Add(BoxSizer3, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	FlexGridSizer9->Add(FlexGridSizer11, 1, wxALL|wxALIGN_LEFT, 2);
	FlexGridSizer9->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	FlexGridSizer5->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer16 = new wxFlexGridSizer(0, 3, 0, 0);
	CheckBox_ResetColorPanel = new wxCheckBox(ColorScrollWindow, ID_CHECKBOX_ResetColorPanel, _("Reset panel when changing effects"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ResetColorPanel"));
	CheckBox_ResetColorPanel->SetValue(true);
	FlexGridSizer16->Add(CheckBox_ResetColorPanel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4->Add(FlexGridSizer16, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText5 = new wxStaticText(ColorScrollWindow, ID_STATICTEXT1, _("Chroma Key"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_ChromaSensitivity = new BulkEditSlider(ColorScrollWindow, ID_SLIDER_ChromaSensitivity, 1, 1, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_ChromaSensitivity"));
	FlexGridSizer2->Add(Slider_ChromaSensitivity, 1, wxALL|wxEXPAND, 2);
	ColourPickerCtrl_ChromaColour = new BulkEditColourPickerCtrl(ColorScrollWindow, ID_COLOURPICKERCTRL_ChromaColour, wxColour(0,0,0), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_COLOURPICKERCTRL_ChromaColour"));
	FlexGridSizer2->Add(ColourPickerCtrl_ChromaColour, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_EnableChromakey = new BulkEditCheckBox(ColorScrollWindow, ID_CHECKBOX_Chroma, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Chroma"));
	CheckBox_EnableChromakey->SetValue(false);
	FlexGridSizer2->Add(CheckBox_EnableChromakey, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText22 = new wxStaticText(ColorScrollWindow, ID_STATICTEXT_SparkleFrequency, _("Sparkles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_SparkleFrequency"));
	FlexGridSizer2->Add(StaticText22, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	Slider_SparkleFrequency = new BulkEditSlider(ColorScrollWindow, ID_SLIDER_SparkleFrequency, 0, 0, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_SparkleFrequency"));
	FlexGridSizer8->Add(Slider_SparkleFrequency, 1, wxALL|wxEXPAND, 0);
	BitmapButton_SparkleFrequencyVC = new BulkEditValueCurveButton(ColorScrollWindow, ID_VALUECURVE_SparkleFrequency, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_SparkleFrequency"));
	FlexGridSizer8->Add(BitmapButton_SparkleFrequencyVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 0);
	txtCtrlSparkleFreq = new BulkEditTextCtrl(ColorScrollWindow, IDD_TEXTCTRL_SparkleFrequency, _("0"), wxDefaultPosition, wxDLG_UNIT(ColorScrollWindow,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_SparkleFrequency"));
	FlexGridSizer2->Add(txtCtrlSparkleFreq, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_SparkleFrequency = new xlLockButton(ColorScrollWindow, ID_BITMAPBUTTON_SLIDER_SparkleFrequency, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_SparkleFrequency"));
	BitmapButton_SparkleFrequency->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer2->Add(BitmapButton_SparkleFrequency, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	CheckBox_MusicSparkles = new BulkEditCheckBox(ColorScrollWindow, ID_CHECKBOX_MusicSparkles, _("Sparkles reflect music"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MusicSparkles"));
	CheckBox_MusicSparkles->SetValue(false);
	FlexGridSizer2->Add(CheckBox_MusicSparkles, 1, wxALL|wxEXPAND, 2);
	ColourPickerCtrl_SparklesColour = new BulkEditColourPickerCtrl(ColorScrollWindow, ID_COLOURPICKERCTRL_SparklesColour, wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_COLOURPICKERCTRL_SparklesColour"));
	FlexGridSizer2->Add(ColourPickerCtrl_SparklesColour, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_MusicSparkles = new xlLockButton(ColorScrollWindow, ID_BITMAPBUTTON_MusicSparkles, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_MusicSparkles"));
	BitmapButton_MusicSparkles->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer2->Add(BitmapButton_MusicSparkles, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText126 = new wxStaticText(ColorScrollWindow, ID_STATICTEXT_Brightness, _("Brightness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Brightness"));
	FlexGridSizer2->Add(StaticText126, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	Slider_Brightness = new BulkEditSlider(ColorScrollWindow, ID_SLIDER_Brightness, 100, 0, 400, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Brightness"));
	FlexGridSizer7->Add(Slider_Brightness, 1, wxALL|wxEXPAND, 0);
	BitmapButton_VCBrightness = new BulkEditValueCurveButton(ColorScrollWindow, ID_VALUECURVE_Brightness, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Brightness"));
	FlexGridSizer7->Add(BitmapButton_VCBrightness, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer2->Add(FlexGridSizer7, 1, wxEXPAND, 0);
	txtCtlBrightness = new BulkEditTextCtrl(ColorScrollWindow, IDD_TEXTCTRL_Brightness, _("100"), wxDefaultPosition, wxDLG_UNIT(ColorScrollWindow,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Brightness"));
	FlexGridSizer2->Add(txtCtlBrightness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Brightness = new xlLockButton(ColorScrollWindow, ID_BITMAPBUTTON_SLIDER_Brightness, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Brightness"));
	BitmapButton_Brightness->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer2->Add(BitmapButton_Brightness, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText127 = new wxStaticText(ColorScrollWindow, ID_STATICTEXT_Contrast, _("Contrast"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Contrast"));
	FlexGridSizer2->Add(StaticText127, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Contrast = new BulkEditSlider(ColorScrollWindow, ID_SLIDER_Contrast, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Contrast"));
	FlexGridSizer2->Add(Slider_Contrast, 1, wxALL|wxEXPAND, 0);
	txtCtlContrast = new BulkEditTextCtrl(ColorScrollWindow, IDD_TEXTCTRL_Contrast, _("0"), wxDefaultPosition, wxDLG_UNIT(ColorScrollWindow,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Contrast"));
	FlexGridSizer2->Add(txtCtlContrast, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Contrast = new xlLockButton(ColorScrollWindow, ID_BITMAPBUTTON_SLIDER_Contrast, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Contrast"));
	BitmapButton_Contrast->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer2->Add(BitmapButton_Contrast, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer4->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer12->AddGrowableCol(1);
	StaticText4 = new wxStaticText(ColorScrollWindow, ID_STATICTEXT4, _("Adjustment"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer12->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer12->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer12->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText1 = new wxStaticText(ColorScrollWindow, ID_STATICTEXT_Color_HueAdjust, _("Hue"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Color_HueAdjust"));
	FlexGridSizer12->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer13 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer13->AddGrowableCol(0);
	Slider_Color_HueAdjust = new BulkEditSlider(ColorScrollWindow, ID_SLIDER_Color_HueAdjust, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Color_HueAdjust"));
	FlexGridSizer13->Add(Slider_Color_HueAdjust, 1, wxALL|wxEXPAND, 0);
	BitmapButton_Color_HueAdjust = new BulkEditValueCurveButton(ColorScrollWindow, ID_VALUECURVE_Color_HueAdjust, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Color_HueAdjust"));
	FlexGridSizer13->Add(BitmapButton_Color_HueAdjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer12->Add(FlexGridSizer13, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Color_HueAdjust = new BulkEditTextCtrl(ColorScrollWindow, IDD_TEXTCTRL_Color_HueAdjust, _("0"), wxDefaultPosition, wxDLG_UNIT(ColorScrollWindow,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Color_HueAdjust"));
	TextCtrl_Color_HueAdjust->SetMaxLength(4);
	FlexGridSizer12->Add(TextCtrl_Color_HueAdjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText2 = new wxStaticText(ColorScrollWindow, ID_STATICTEXT_Color_SaturationAdjust, _("Saturation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Color_SaturationAdjust"));
	FlexGridSizer12->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer15 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer15->AddGrowableCol(0);
	Slider_Color_SaturationAdjust = new BulkEditSlider(ColorScrollWindow, ID_SLIDER_Color_SaturationAdjust, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Color_SaturationAdjust"));
	FlexGridSizer15->Add(Slider_Color_SaturationAdjust, 1, wxALL|wxEXPAND, 0);
	BitmapButton_Color_SaturationAdjust = new BulkEditValueCurveButton(ColorScrollWindow, ID_VALUECURVE_Color_SaturationAdjust, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Color_SaturationAdjust"));
	FlexGridSizer15->Add(BitmapButton_Color_SaturationAdjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer12->Add(FlexGridSizer15, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Color_SaturationAdjust = new BulkEditTextCtrl(ColorScrollWindow, IDD_TEXTCTRL_Color_SaturationAdjust, _("0"), wxDefaultPosition, wxDLG_UNIT(ColorScrollWindow,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Color_SaturationAdjust"));
	TextCtrl_Color_SaturationAdjust->SetMaxLength(4);
	FlexGridSizer12->Add(TextCtrl_Color_SaturationAdjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(ColorScrollWindow, ID_STATICTEXT_Color_ValueAdjust, _("Value"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Color_ValueAdjust"));
	FlexGridSizer12->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer14 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer14->AddGrowableCol(0);
	Slider_Color_ValueAdjust = new BulkEditSlider(ColorScrollWindow, ID_SLIDER_Color_ValueAdjust, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Color_ValueAdjust"));
	FlexGridSizer14->Add(Slider_Color_ValueAdjust, 1, wxALL|wxEXPAND, 0);
	BitmapButton_Color_ValueAdjust = new BulkEditValueCurveButton(ColorScrollWindow, ID_VALUECURVE_Color_ValueAdjust, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Color_ValueAdjust"));
	FlexGridSizer14->Add(BitmapButton_Color_ValueAdjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer12->Add(FlexGridSizer14, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Color_ValueAdjust = new BulkEditTextCtrl(ColorScrollWindow, IDD_TEXTCTRL_Color_ValueAdjust, _("0"), wxDefaultPosition, wxDLG_UNIT(ColorScrollWindow,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Color_ValueAdjust"));
	TextCtrl_Color_ValueAdjust->SetMaxLength(4);
	FlexGridSizer12->Add(TextCtrl_Color_ValueAdjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4->Add(FlexGridSizer12, 1, wxALL|wxEXPAND, 2);
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

    Connect(ID_BITMAPBUTTON_ReverseColours, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ColorPanel::OnBitmapButton_ReverseColoursClick);
    Connect(ID_BITMAPBUTTON_LeftShiftColours, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ColorPanel::OnBitmapButton_ShiftColoursLeftClick);
    Connect(ID_BITMAPBUTTON_RightShiftColours,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnBitmapButton_ShiftColoursRightClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnUpdateColorClick);
	Connect(ID_BITMAPBUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnBitmapButton_SavePaletteClick);
	Connect(ID_BITMAPBUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnBitmapButton_DeletePaletteClick);
	Connect(ID_CHECKBOX_ResetColorPanel,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ColorPanel::OnCheckBox_ResetColorPanelClick);
	Connect(ID_CHECKBOX_Chroma,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ColorPanel::OnCheckBox_EnableChromakeyClick);
	Connect(ID_VALUECURVE_SparkleFrequency,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_SparkleFrequency,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_MusicSparkles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Brightness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Brightness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Contrast,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Color_HueAdjust,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Color_SaturationAdjust,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Color_ValueAdjust,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnVCButtonClick);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&ColorPanel::OnResize);
	//*)

    SetName("Color");

    Connect(wxID_ANY, wxEVT_COMBOBOX_DROPDOWN, (wxObjectEventFunction)&ColorPanel::OnColourChoiceDropDown, 0, this);
    Connect(wxID_ANY, wxEVT_COMBOBOX, (wxObjectEventFunction)&ColorPanel::OnColourChoiceSelect, 0, this);

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&ColorPanel::OnVCChanged, 0, this);

    BitmapButton_VCBrightness->GetValue()->SetLimits(COLORPANEL_BRIGHTNESS_MIN, COLORPANEL_BRIGHTNESS_MAX);
    BitmapButton_SparkleFrequencyVC->GetValue()->SetLimits(COLORPANEL_SPARKLE_MIN, COLORPANEL_SPARKLE_MAX);
    BitmapButton_Color_HueAdjust->GetValue()->SetLimits(COLORPANEL_HUE_MIN, COLORPANEL_HUE_MAX);
    BitmapButton_Color_SaturationAdjust->GetValue()->SetLimits(COLORPANEL_SATURATION_MIN, COLORPANEL_SATURATION_MAX);
    BitmapButton_Color_ValueAdjust->GetValue()->SetLimits(COLORPANEL_VALUE_MIN, COLORPANEL_VALUE_MAX);

    ColourPickerCtrl_ChromaColour->SetDropTarget(new ColourText2DropTarget(ColourPickerCtrl_ChromaColour));
    ColourPickerCtrl_SparklesColour->SetDropTarget(new ColourText2DropTarget(ColourPickerCtrl_SparklesColour));

    FlexGridSizer_Palette->SetCols(PALETTE_SIZE);
    for (int x = 0; x < PALETTE_SIZE; x++) {
        FlexGridSizer_Palette->AddGrowableCol(x);
        wxString ids = wxString::Format("ID_CHECKBOX_Palette%d", (x + 1));
        long id2 = wxNewId();
        wxCheckBox *cb = new wxCheckBox(ColorScrollWindow, id2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, ids);
        cb->SetValue(x < 2);
        FlexGridSizer_Palette->Add(cb, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
        checkBoxes.push_back(cb);
        Connect(id2, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&ColorPanel::OnCheckBox_PaletteClick);
    }
    for (int x = 0; x < PALETTE_SIZE; x++) {
        wxString ids = wxString::Format("ID_BUTTON_Palette%d", (x + 1));
        long id2 = wxNewId();
        ColorCurveButton *bb = new ColorCurveButton(ColorScrollWindow, id2, wxNullBitmap, wxDefaultPosition, FromDIP(wxSize(21, 21)), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, ids);
        bb->SetDropTarget(new ColourTextDropTarget(bb));
        FlexGridSizer_Palette->Add(bb, 0, wxALIGN_LEFT|wxALIGN_TOP, 0);
        buttons.push_back(bb);
        Connect(wxID_ANY, EVT_CC_CHANGED, (wxObjectEventFunction)&ColorPanel::OnCCChanged, 0, this);
    }
    for (int x = 0; x < PALETTE_SIZE; x++) {
        wxString ids = wxString::Format("ID_BITMAPBUTTON_BUTTON_Palette%d", (x + 1));
        long id2 = wxNewId();
        wxBitmapButton *bb = new xlLockButton(ColorScrollWindow, id2, wxArtProvider::GetBitmapBundle("xlART_PADLOCK_OPEN", wxART_BUTTON),
                                                wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, ids);
        FlexGridSizer_Palette->Add(bb, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
        Connect(id2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorPanel::OnLockButtonClick);
    }
    for (int x = 0; x < PALETTE_SIZE; x++) {
        wxString ids = wxString::Format("ID_BITMAPBUTTON_BUTTON_PaletteCC%d", (x + 1));
        long id2 = wxNewId();
        wxBitmapButton *bb = new xlSizedBitmapButton(ColorScrollWindow, id2, wxArtProvider::GetBitmapBundle("xlART_cc_na_xpm", wxART_BUTTON), wxDefaultPosition,
                                                wxSize(13, 13), wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, ids);
        bb->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
        FlexGridSizer_Palette->Add(bb, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
        Connect(id2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ColorPanel::OnCCButtonClick);
    }
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    _lastShowDir = xLightsFrame::CurrentDir;

    //SetDefaultPallette will call LoadAllPalettes in this case
    SetDefaultPalette();

    wxConfigBase* config = wxConfigBase::Get();
    bool reset;
    config->Read("xLightsResetColorPanel", &reset, true);
    CheckBox_ResetColorPanel->SetValue(reset);

    ValidateWindow();
}
#ifdef __XLIGHTS_HAS_TOUCHBARS__
ColorPanelTouchBar* ColorPanel::SetupTouchBar(xlTouchBarSupport &tbs) {
    if (touchBar == nullptr && tbs.HasTouchBar()) {
        touchBar = std::unique_ptr<ColorPanelTouchBar>(new ColorPanelTouchBar(
            [this](int idx, xlColor c) {
                this->SetButtonColor(idx, c, false);
            },
            [this](int v) {
                this->BitmapButton_SparkleFrequencyVC->SetValue(wxString::Format("%d", v));
                BitmapButton_SparkleFrequencyVC->GetValue()->SetDefault(0.0f, 200.0f);
                Slider_SparkleFrequency->SetValue(v);
                txtCtrlSparkleFreq->SetValue(wxString::Format("%d", v));
            }, tbs));
    }
    return touchBar.get();
}

void ColorPanel::UpdateTouchBarSlider(wxScrollEvent& event) {
    if (touchBar != nullptr) {
        touchBar->SetSparkles(event.GetPosition());
    }
    Slider_SparkleFrequency->OnSlider_SliderUpdated(event);
}
#endif

void ColorPanel::LoadAllPalettes()
{
    _loadedPalettes.clear();

    wxDir dir;
    if (wxDir::Exists(xLightsFrame::CurrentDir))
    {
        dir.Open(xLightsFrame::CurrentDir);
        LoadPalettes(dir, false);
    }

    wxString d = xLightsFrame::CurrentDir + "/Palettes";
    if (wxDir::Exists(d))
    {
        dir.Open(d);
        LoadPalettes(dir, true);
    }

    wxStandardPaths stdp = wxStandardPaths::Get();
#ifndef __WXMSW__
    d = wxStandardPaths::Get().GetResourcesDir() + "/palettes";
#else
    d = wxFileName(stdp.GetExecutablePath()).GetPath() + "/palettes";
#endif
    if (wxDir::Exists(d))
    {
        dir.Open(d);
        LoadPalettes(dir, true);
    }

    if (BitmapButton_ColourChoice->GetCount() != 0) {
        BitmapButton_ColourChoice->Clear();
    }
    BitmapButton_ColourChoice->AppendString("(Load)");
    for(auto it=  _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it)
    {
        BitmapButton_ColourChoice->AppendString(*it);
    }
    FireChangeEvent();
}

void ColorPanel::LoadPalettes(wxDir& directory, bool subdirs)
{
    static wxRegEx cregex("^\\$[^:]*: rgba\\(([^)]*)\\)");

    wxArrayString files;
    GetAllFilesInDir(directory.GetName(), files, "*.xpalette");
    for (auto &filename : files) {
        if (FileExists(filename)) {
            wxFileName fn(filename);
            wxFileInputStream input(fn.GetFullPath());
            if (input.IsOk()) {
                wxTextInputStream text(input);
                wxString s = text.ReadLine();
                wxString scomp = s.BeforeLast(',');

                bool found = false;
                for (auto it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it) {
                    wxString p(*it);
                    if (p.BeforeLast(',') == scomp) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    _loadedPalettes.push_back(s.ToStdString() + fn.GetFullName().ToStdString());
                }
            }
        }
    }

    files.clear();
    GetAllFilesInDir(directory.GetNameWithSep(), files, "*.scss");
    for (auto &filename : files) {
        if (FileExists(filename)) {
            wxFileName fn(filename);
            wxFileInputStream input(fn.GetFullPath());
            if (input.IsOk()) {
                wxString pal;
                int cols = 0;
                wxTextInputStream text(input);
                while (!input.Eof()) {
                    wxString line = text.ReadLine();
                    if (cregex.Matches(line)) {
                        wxString rgb = cregex.GetMatch(line, 1);
                        wxArrayString comp = wxSplit(rgb, ',');
                        if (comp.size() == 4) {
                            pal += wxString::Format("#%2x%2x%2x,",
                                wxAtoi(comp[0]),
                                wxAtoi(comp[1]),
                                wxAtoi(comp[2])
                            );
                            cols++;
                        }
                    }
                }
                if (cols > 0) {
                    while (cols < 8) {
                        pal += "#FFFFFF,";
                        cols++;
                    }
                    bool found = false;
                    for (auto it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it) {
                        wxString p(*it);
                        if (p.BeforeLast(',') == pal) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        _loadedPalettes.push_back(pal.ToStdString() + fn.GetFullName().ToStdString());
                    }
                }
            }
        }
    }
    files.clear();
    GetAllFilesInDir(directory.GetNameWithSep(), files, "*.svg");
    for (auto &filename : files) {
        if (FileExists(filename)) {
            wxFileName fn(filename);
            wxXmlDocument svg;
            svg.Load(filename);

            if (svg.IsOk()) {
                wxString pal;
                int cols = 0;
                for (auto n = svg.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext()) {
                    if (n->GetName() == "rect") {
                        if (n->HasAttribute("fill")) {
                            pal += n->GetAttribute("fill") + ",";
                            cols++;
                        }
                    }
                }
                if (cols > 0) {
                    while (cols < 8) {
                        pal += "#FFFFFF,";
                        cols++;
                    }
                    bool found = false;
                    for (auto it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it) {
                        wxString p(*it);
                        if (p.BeforeLast(',') == pal) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        _loadedPalettes.push_back(pal.ToStdString() + fn.GetFullName().ToStdString());
                    }
                }
            }
        }
    }

    if (subdirs) {
        wxString filename;
        bool cont = directory.GetFirst(&filename, "*", wxDIR_DIRS);
        while (cont) {
            wxDir dir(directory.GetNameWithSep() + filename);
            LoadPalettes(dir, subdirs);
            cont = directory.GetNext(&filename);
        }
    }
    FireChangeEvent();
}

ColorPanel::~ColorPanel()
{
	//(*Destroy(ColorPanel)
	//*)
}


void ColorPanel::SetColorCount(int count)
{
    // Disabling this as we dont really want to limit the colours to from left to right ... but it would be clearer
#if 0
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
#endif
}
void ColorPanel::SetButtonColor(int btn, const xlColor &v, bool notify) {
    SetButtonColor(buttons[btn], v, notify);
}

void ColorPanel::SetButtonColor(ColorCurveButton* btn, const std::string& cstr, bool notify)
{
    if (cstr.find("Active") != std::string::npos)
    {
        btn->GetValue()->Deserialise(cstr);
        btn->UpdateState(notify);
    }
    else
    {
        btn->SetActive(false, false);
        btn->SetColor(cstr, notify);
        btn->SetDefaultCC(cstr);
    }
#ifdef __XLIGHTS_HAS_TOUCHBARS__
    if (touchBar.get()) {
        for (int x = 0; x < PALETTE_SIZE; x++) {
            if (buttons[x] == btn) {
                wxColor c = buttons[x]->GetBackgroundColour();
                touchBar->SetColor(x, btn->GetBitmap(), c);
            }
        }
    }
#endif
    FireChangeEvent();
    ValidateWindow();
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
        ret+= wxString::Format("C_SLIDER_Contrast=%d,",Slider_Contrast->GetValue());
    }
    if (Slider_Color_HueAdjust->GetValue() != 0) {
        ret += wxString::Format("C_SLIDER_Color_HueAdjust=%d,", Slider_Color_HueAdjust->GetValue());
    }
    if (Slider_Color_SaturationAdjust->GetValue() != 0) {
        ret += wxString::Format("C_SLIDER_Color_SaturationAdjust=%d,", Slider_Color_SaturationAdjust->GetValue());
    }
    if (Slider_Color_ValueAdjust->GetValue() != 0) {
        ret += wxString::Format("C_SLIDER_Color_ValueAdjust=%d,", Slider_Color_ValueAdjust->GetValue());
    }
    if (Slider_ChromaSensitivity->GetValue() != 1) {
        ret += wxString::Format("C_SLIDER_ChromaSensitivity=%d,", Slider_ChromaSensitivity->GetValue());
    }
    if (CheckBox_EnableChromakey->GetValue()) {
        ret += wxString::Format("C_CHECKBOX_Chroma=1,");
    }
    if (ColourPickerCtrl_ChromaColour->GetColour() != *wxBLACK) {
        ret += "C_COLOURPICKERCTRL_ChromaColour=" + ColourPickerCtrl_ChromaColour->GetStringValue() + ",";
    }
    if (ColourPickerCtrl_SparklesColour->GetColour() != *wxWHITE) {
        ret += "C_COLOURPICKERCTRL_SparklesColour=" + ColourPickerCtrl_SparklesColour->GetStringValue() + ",";
    }
    return ret;
}

std::string ColorPanel::GetCurrentPalette() const
{
    std::string res;
    for (size_t i = 0; i < PALETTE_SIZE; i++)
    {
        wxString ids = wxString::Format("ID_BUTTON_Palette%d", ((int)i + 1));
        ColorCurveButton* btn = (ColorCurveButton*)wxWindow::FindWindowByName(ids, this);
        if (btn->GetValue()->IsActive())
        {
            res += btn->GetValue()->Serialise() + ",";
        }
        else
        {
            wxColor color = GetPaletteColor(i);
            color.Set(color.Red(), color.Green(), color.Blue(), wxALPHA_OPAQUE);
            res += color.GetAsString(wxC2S_HTML_SYNTAX).ToStdString() + ",";
        }
    }

    return res;
}

wxString ColorPanel::GetColorString(bool colourOnly)
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

    if (colourOnly)
    {
        return s;
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

    if (BitmapButton_Color_HueAdjust->GetValue()->IsActive())
    {
        wxString hueVC = wxString(BitmapButton_Color_HueAdjust->GetValue()->Serialise().c_str());
        s += "C_VALUECURVE_Color_HueAdjust=";
        s += hueVC;
        s += ",";
    }
    else
    {
        if (Slider_Color_HueAdjust->GetValue() != 0) {
            s += wxString::Format("C_SLIDER_Color_HueAdjust=%d,", Slider_Color_HueAdjust->GetValue());
        }
    }
    if (BitmapButton_Color_SaturationAdjust->GetValue()->IsActive())
    {
        wxString satVC = wxString(BitmapButton_Color_SaturationAdjust->GetValue()->Serialise().c_str());
        s += "C_VALUECURVE_Color_SaturationAdjust=";
        s += satVC;
        s += ",";
    }
    else
    {
        if (Slider_Color_SaturationAdjust->GetValue() != 0) {
            s += wxString::Format("C_SLIDER_Color_SaturationAdjust=%d,", Slider_Color_SaturationAdjust->GetValue());
        }
    }
    if (BitmapButton_Color_ValueAdjust->GetValue()->IsActive())
    {
        wxString valueVC = wxString(BitmapButton_Color_ValueAdjust->GetValue()->Serialise().c_str());
        s += "C_VALUECURVE_Color_ValueAdjust=";
        s += valueVC;
        s += ",";
    }
    else
    {
        if (Slider_Color_ValueAdjust->GetValue() != 0) {
            s += wxString::Format("C_SLIDER_Color_ValueAdjust=%d,", Slider_Color_ValueAdjust->GetValue());
        }
    }

    if (Slider_Contrast->GetValue() != 0) {
        s+= wxString::Format("C_SLIDER_Contrast=%d",Slider_Contrast->GetValue());
    }

    if (CheckBox_EnableChromakey->GetValue()) {
        if (Slider_ChromaSensitivity->GetValue() != 1) {
            s += wxString::Format("C_SLIDER_ChromaSensitivity=%d,", Slider_ChromaSensitivity->GetValue());
        }

        if (ColourPickerCtrl_ChromaColour->GetColour() != *wxBLACK) {
            s += "C_COLOURPICKERCTRL_ChromaColour=" + ColourPickerCtrl_ChromaColour->GetStringValue() + ",";
        }

        s += wxString::Format("C_CHECKBOX_Chroma=1", Slider_ChromaSensitivity->GetValue());
    }

    if (ColourPickerCtrl_SparklesColour->GetColour() != *wxWHITE) {
        s += "C_COLOURPICKERCTRL_SparklesColour=" + ColourPickerCtrl_SparklesColour->GetStringValue() + ",";
    }

    return s;
}

wxColour ColorPanel::GetPaletteColor(int idx) const
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

void ColorPanel::SetDefaultSettings(bool optionbased)
{
    if (!optionbased)
    {
        for (const auto& it : checkBoxes) {
            it->SetValue(false);
        }
    }

    if (!optionbased || CheckBox_ResetColorPanel->GetValue())
    {
        BitmapButton_SparkleFrequencyVC->GetValue()->SetDefault(COLORPANEL_SPARKLE_MIN, COLORPANEL_SPARKLE_MAX);
        BitmapButton_SparkleFrequencyVC->UpdateState();
        //Slider_SparkleFrequency->SetValue(0);
        txtCtrlSparkleFreq->SetValue("0");

        CheckBox_MusicSparkles->SetValue(false);

        BitmapButton_VCBrightness->GetValue()->SetDefault(COLORPANEL_BRIGHTNESS_MIN, COLORPANEL_BRIGHTNESS_MAX);
        BitmapButton_VCBrightness->UpdateState();
        //Slider_Brightness->SetValue(100);
        txtCtlBrightness->SetValue("100");

        Slider_ChromaSensitivity->SetValue(1);
        ColourPickerCtrl_ChromaColour->SetColour(*wxBLACK);
        CheckBox_EnableChromakey->SetValue(false);
        ColourPickerCtrl_SparklesColour->SetColour(*wxWHITE);

        //Slider_Contrast->SetValue(0);
        txtCtlContrast->SetValue("0");

        //Slider_Color_HueAdjust->SetValue(0);
        TextCtrl_Color_HueAdjust->SetValue("0");

        //Slider_Color_SaturationAdjust->SetValue(0);
        TextCtrl_Color_SaturationAdjust->SetValue("0");

        //Slider_Color_ValueAdjust->SetValue(0);
        TextCtrl_Color_ValueAdjust->SetValue("0");

        BitmapButton_Color_HueAdjust->GetValue()->SetDefault(COLORPANEL_HUE_MIN, COLORPANEL_HUE_MAX);
        BitmapButton_Color_HueAdjust->UpdateState();
        BitmapButton_Color_SaturationAdjust->GetValue()->SetDefault(COLORPANEL_SATURATION_MIN, COLORPANEL_SATURATION_MAX);
        BitmapButton_Color_SaturationAdjust->UpdateState();
        BitmapButton_Color_ValueAdjust->GetValue()->SetDefault(COLORPANEL_VALUE_MIN, COLORPANEL_VALUE_MAX);
        BitmapButton_Color_ValueAdjust->UpdateState();

#ifdef __XLIGHTS_HAS_TOUCHBARS__
        touchBar->SetSparkles(0);
#endif
    }
    FireChangeEvent();
    ValidateWindow();
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

    if (BitmapButton_ColourChoice->GetCount() < 2)
    {
        LoadAllPalettes();
    }
    FireChangeEvent();
    ValidateWindow();
}


void ColorPanel::OnCheckBox_PaletteClick(wxCommandEvent& event)
{
    FireChangeEvent();
    PaletteChanged=true;
}

void ColorPanel::OnCCChanged(wxCommandEvent& event)
{
    ColorCurveButton* w = (ColorCurveButton*)event.GetEventObject();
    lastColors[w->GetId()] = w->GetColor();

#ifdef __XLIGHTS_HAS_TOUCHBARS__
    for (int x = 0; x < PALETTE_SIZE; x++) {
        if (buttons[x] == w) {
            wxColor c = w->GetBackgroundColour();
            touchBar->SetColor(x, w->GetBitmap(), c);
        }
    }
#endif

    // only where a new colour has been added or a colour curve changed or exported we need to do this
    // This is signified by the event int being non zero
    if (event.GetInt() != 0)
    {
        wxCommandEvent e(EVT_COLOUR_CHANGED);
        e.SetInt(-1);
        wxPostEvent(xLightsApp::GetFrame(), e);
    }

    FireChangeEvent();
    Refresh();
    ValidateWindow();
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
    int alleffects = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("");
    if (alleffects > 1) {
        if (wxMessageBox("Are you sure you want to change the colours on all selected effects?", "Update all", wxYES_NO | wxCENTRE, this) == wxNO)
            return;
    }

    wxCommandEvent eventEffectUpdated(EVT_EFFECT_PALETTE_UPDATED);
    wxPostEvent(GetParent(), eventEffectUpdated);
    FireChangeEvent();
    Refresh();
    ValidateWindow();
}

void ColorPanel::OnCheckBox_MusicSparklesClick(wxCommandEvent& event)
{
    FireChangeEvent();
    ValidateWindow();
}

void ColorPanel::ValidateWindow()
{
    for (int x = 0; x < PALETTE_SIZE; x++) {
        wxString ccbids = wxString::Format("ID_BUTTON_Palette%d", (x + 1));
        ColorCurveButton* ccb = (ColorCurveButton*)wxWindow::FindWindowByName(ccbids, this);
        wxString tsids = wxString::Format("ID_BITMAPBUTTON_BUTTON_PaletteCC%d", (x + 1));
        wxBitmapButton* ts = (wxBitmapButton*)wxWindow::FindWindowByName(tsids, this);

        if (ccb->GetValue()->IsActive())
        {
            if (!_supportslinear && !_supportsradial)
            {
                ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_timelocked_xpm", wxART_BUTTON));
            }
            else
            {
                switch (ccb->GetValue()->GetTimeCurve())
                {
                case TC_TIME:
                    ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_time_xpm", wxART_BUTTON));
                    break;
                case TC_LEFT:
                    ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_left_xpm", wxART_BUTTON));
                    break;
                case TC_RIGHT:
                    ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_right_xpm", wxART_BUTTON));
                    break;
                case TC_UP:
                    ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_up_xpm", wxART_BUTTON));
                    break;
                case TC_DOWN:
                    ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_down_xpm", wxART_BUTTON));
                    break;
                case TC_RADIALOUT:
                    ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_radialout_xpm", wxART_BUTTON));
                    break;
                case TC_RADIALIN:
                    ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_radialin_xpm", wxART_BUTTON));
                    break;
                case TC_CW:
                    ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_cw_xpm", wxART_BUTTON));
                    break;
                case TC_CCW:
                    ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_ccw_xpm", wxART_BUTTON));
                    break;
                }
            }
        }
        else
        {
            // it should already be this
            ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_na_xpm", wxART_BUTTON));
        }
    }

    if (CheckBox_EnableChromakey->GetValue())
    {
        Slider_ChromaSensitivity->Enable();
        ColourPickerCtrl_ChromaColour->Enable();
    }
    else
    {
        Slider_ChromaSensitivity->Enable(false);
        ColourPickerCtrl_ChromaColour->Enable(false);
    }

    // only enable save if this palette was not loaded from disk or has been saved to disk
    wxString pal = wxString(GetCurrentPalette()).BeforeLast(',');
    for (auto it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it)
    {
        wxString ss(it->c_str());
        if (ss.BeforeLast(',') == pal)
        {
            BitmapButton_SavePalette->Disable();
            if (FindPaletteFile(ss.AfterLast(','), pal + ",") != "")
            {
                BitmapButton_DeletePalette->Enable();
            }
            else
            {
                BitmapButton_DeletePalette->Disable();
            }
            return;
        }
    }
    BitmapButton_SavePalette->Enable();
    BitmapButton_DeletePalette->Disable();
}

void ColorPanel::OnColourChoiceDropDown(wxCommandEvent& WXUNUSED(event))
{
    if (_lastShowDir != xLightsFrame::CurrentDir)
    {
        _lastShowDir = xLightsFrame::CurrentDir;
        LoadAllPalettes();
        ValidateWindow();
        FireChangeEvent();
    }
}

void ColorPanel::OnBitmapButton_SavePaletteClick(wxCommandEvent& event)
{
    // Double check that this has not been saved before
    if (BitmapButton_ColourChoice->GetCount() == 1) {
        LoadAllPalettes();
        ValidateWindow();
        if (!BitmapButton_SavePalette->IsEnabled()) {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.error("Already saved xpalette ... skipped.");
            return;
        }
    }

    if (!wxDir::Exists(xLightsFrame::CurrentDir + "/Palettes")) {
        wxDir::Make(xLightsFrame::CurrentDir + "/Palettes");
    }

    int i = 1;
    wxString fn = "PAL001.xpalette";

    while (FileExists(xLightsFrame::CurrentDir + "/Palettes/" + fn)) {
        i++;
        fn = wxString::Format("PAL%03d.xpalette", i);
    }

    wxFile f;
    f.Create(xLightsFrame::CurrentDir + "/Palettes/" + fn);

    if (f.IsOpened()) {
        std::string pal = GetCurrentPalette();

        f.Write(wxString(pal.c_str()));
        f.Close();

        _loadedPalettes.push_back(pal);
    } else {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.error("Unable to create file %s.", (const char *)fn.c_str());
    }

    LoadAllPalettes();

    ValidateWindow();
}

void ColorPanel::OnColourChoiceSelect(wxCommandEvent& event)
{
    long sel = event.GetInt();
    wxString s = BitmapButton_ColourChoice->GetString(sel);

    if (s != "(Load)") {
        wxArrayString as = wxSplit(s, ',');

        for (size_t i = 0; i < std::min(as.size(), buttons.size()); i++)
        {
            if (as[i].Contains("Active"))
            {
                buttons[i]->GetValue()->Deserialise(as[i].ToStdString());
                buttons[i]->SetActive(true);
                buttons[i]->Refresh();
            }
            else
            {
                buttons[i]->SetColor(as[i].ToStdString());
                buttons[i]->Refresh();
            }
        }
    }

    BitmapButton_ColourChoice->SetSelection(0);
    FireChangeEvent();
    ValidateWindow();
}

wxString ColorPanel::FindPaletteFile(const wxString& filename, const wxString& palette) const
{
    if (FileExists(xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + filename)) {
        wxFileInputStream input(xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + filename);
        if (input.IsOk()) {
            wxTextInputStream text(input);
            wxString s = text.ReadLine();
            if (s == palette) {
                return xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + filename;
            }
        }
    }

    if (FileExists(xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + "Palettes"  + wxFileName::GetPathSeparator() + filename)) {
        wxFileInputStream input(xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + "Palettes" + wxFileName::GetPathSeparator() + filename);
        if (input.IsOk()) {
            wxTextInputStream text(input);
            wxString s = text.ReadLine();
            if (s == palette) {
                return xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + "Palettes" + wxFileName::GetPathSeparator() + filename;
            }
        }
    }

    return "";
}

void ColorPanel::OnBitmapButton_DeletePaletteClick(wxCommandEvent& event)
{
    std::string pal = GetCurrentPalette();

    for (auto it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it)
    {
        wxString ss(it->c_str());
        if (ss.BeforeLast(',')+"," == pal)
        {
            // found it
            wxString filename = FindPaletteFile(ss.AfterLast(','), pal);
            if (filename != "")
            {
                ::wxRemoveFile(filename);
            }
        }
    }
    LoadAllPalettes();
    ValidateWindow();
}

void ColorPanel::OnBitmapButton_ReverseColoursClick(wxCommandEvent& event)
{
    std::string pal = GetCurrentPalette();

    wxArrayString as = wxSplit(pal, ',');

    for (size_t i = 0; i < PALETTE_SIZE; ++i)
    {
        if (as[i].Contains("Active"))
        {
            buttons[PALETTE_SIZE - i - 1]->GetValue()->Deserialise(as[i].ToStdString());
            buttons[PALETTE_SIZE - i - 1]->SetActive(true);
            buttons[PALETTE_SIZE - i - 1]->Refresh();
        }
        else
        {
            buttons[PALETTE_SIZE - i - 1]->SetColor(as[i].ToStdString());
            buttons[PALETTE_SIZE - i - 1]->Refresh();
        }
    }
    FireChangeEvent();
    ValidateWindow();
}

void ColorPanel::OnBitmapButton_ShiftColoursLeftClick(wxCommandEvent& event)
{
    std::string pal = GetCurrentPalette();

    wxArrayString as = wxSplit(pal, ',');

    bool zeroActive = as[0].Contains("Active");
    std::string zeroItem = as[0].ToStdString();

    for (size_t i = 0; i < PALETTE_SIZE - 1; ++i)
    {
        if (as[i + 1].Contains("Active"))
        {
            buttons[i]->GetValue()->Deserialise(as[i + 1].ToStdString());
            buttons[i]->SetActive(true);
            buttons[i]->Refresh();
        }
        else
        {
            buttons[i]->SetColor(as[i + 1].ToStdString());
            buttons[i]->Refresh();
        }
    }
    if (zeroActive)
    {
        buttons[PALETTE_SIZE - 1]->GetValue()->Deserialise(zeroItem);
        buttons[PALETTE_SIZE - 1]->SetActive(true);
        buttons[PALETTE_SIZE - 1]->Refresh();
    }
    else
    {
        buttons[PALETTE_SIZE - 1]->SetColor(zeroItem);
        buttons[PALETTE_SIZE - 1]->Refresh();
    }
    FireChangeEvent();
    ValidateWindow();
}

void ColorPanel::OnBitmapButton_ShiftColoursRightClick(wxCommandEvent& event)
{
    std::string pal = GetCurrentPalette();

    wxArrayString as = wxSplit(pal, ',');

    bool lastActive = as[PALETTE_SIZE - 1].Contains("Active");
    std::string lastItem = as[PALETTE_SIZE - 1].ToStdString();

    for (size_t i = PALETTE_SIZE - 1; i > 0; --i)
    {
        if (as[i - 1].Contains("Active"))
        {
            buttons[i]->GetValue()->Deserialise(as[i - 1].ToStdString());
            buttons[i]->SetActive(true);
            buttons[i]->Refresh();
        }
        else
        {
            buttons[i]->SetColor(as[i - 1].ToStdString());
            buttons[i]->Refresh();
        }
    }
    if (lastActive)
    {
        buttons[0]->GetValue()->Deserialise(lastItem);
        buttons[0]->SetActive(true);
        buttons[0]->Refresh();
    }
    else
    {
        buttons[0]->SetColor(lastItem);
        buttons[0]->Refresh();
    }
    FireChangeEvent();
    ValidateWindow();
}

void ColorPanel::OnCCButtonClick(wxCommandEvent& event)
{
    wxBitmapButton* bb = static_cast<wxBitmapButton*>(event.GetEventObject());
    int id = wxAtoi(bb->GetName().Right(1));
    wxString ccbids = wxString::Format("ID_BUTTON_Palette%d", id);
    ColorCurveButton* ccb = (ColorCurveButton*)wxWindow::FindWindowByName(ccbids, this);

    if (ccb->GetValue()->IsActive())
    {
        ccb->GetValue()->NextTimeCurve(_supportslinear, _supportsradial);
    }

    FireChangeEvent();
    ValidateWindow();
}

void ColorPanel::SetSupports(bool linear, bool radial)
{
    _supportslinear = linear;
    _supportsradial = radial;

    for (size_t i = 0; i < PALETTE_SIZE; ++i)
    {
        wxString ccbids = wxString::Format("ID_BUTTON_Palette%d", (int)(i + 1));  // 64bit version on Windows gets a debug alert without the (int) cast
        ColorCurveButton* ccb = (ColorCurveButton*)wxWindow::FindWindowByName(ccbids, this);
        ccb->GetValue()->SetValidTimeCurve(linear, radial);
    }

    FireChangeEvent();
    ValidateWindow();
}

void ColorPanel::OnCheckBox_ResetColorPanelClick(wxCommandEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsResetColorPanel", CheckBox_ResetColorPanel->IsChecked());
}

void ColorPanel::OnCheckBox_EnableChromakeyClick(wxCommandEvent& event)
{
    ValidateWindow();
}
