/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ui/color/ColorPanel.h"

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/clrpicker.h>
#include <wx/config.h>
#include <wx/file.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/odcombo.h>
#include <wx/regex.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/stdpaths.h>
#include <wx/textctrl.h>
#include <wx/textdlg.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>

#include <spdlog/spdlog.h>

#include "ui/color/wxColorCurveButton.h"
#include "ui/color/wxColorCurveRenderer.h"
#include "ai/AIColorPaletteDialog.h"
#include "ui/effectpanels/EffectPanelManager.h"
#include "ui/effectpanels/EffectPanelUtils.h"
#include "ui/shared/utils/xlLockButton.h"
#include "ui/shared/utils/wxUtilities.h"
#include "ui/sequencer/MainSequencer.h"
#include "render/ColorCurve.h"
#include "utils/Color.h"
#include "utils/ExternalHooks.h"
#include "utils/FileUtils.h"
#include "UtilFunctions.h"
#include "xLightsApp.h"
#include "xLightsMain.h"

#define PALETTE_SIZE 8

// Standard sizes
#define SWATCH_WIDTH_STANDARD 11
#define SWATCH_HEIGHT_STANDARD 18
#define PALETTE_BUTTON_SIZE_STANDARD 21
#define PALETTE_CC_SIZE_STANDARD 13

// Large sizes
#define SWATCH_WIDTH_LARGE 28
#define SWATCH_HEIGHT_LARGE 36
#define PALETTE_BUTTON_SIZE_LARGE 32
#define PALETTE_CC_SIZE_LARGE 18

const wxWindowID ColorPanel::ID_MNU_UPDATE = wxNewId();
const wxWindowID ColorPanel::ID_MNU_SAVE = wxNewId();
const wxWindowID ColorPanel::ID_MNU_SAVE_AS = wxNewId();
const wxWindowID ColorPanel::ID_MNU_DELETE = wxNewId();
const wxWindowID ColorPanel::ID_MNU_IMPORT = wxNewId();
const wxWindowID ColorPanel::ID_MNU_GENERATE = wxNewId();

static bool IsLargePalette() {
    return wxConfigBase::Get()->Read("PaletteSize", "Normal") == "Large";
}

class ColourList : public wxOwnerDrawnComboBox {
public:
    ColourList(wxWindow* parent, wxWindowID id,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxValidator& validator = wxDefaultValidator,
               const wxString& name = "ColourList") :
        wxOwnerDrawnComboBox(parent, id, wxEmptyString, pos, size, 0, nullptr,
                             style | wxCB_READONLY | wxCC_STD_BUTTON | wxODCB_STD_CONTROL_PAINT, validator, name) {}

    int GetSwatchWidth() const {
        return IsLargePalette() ? SWATCH_WIDTH_LARGE : SWATCH_WIDTH_STANDARD;
    }

    virtual wxCoord OnMeasureItem(size_t item) const override {
        return IsLargePalette() ? SWATCH_HEIGHT_LARGE : SWATCH_HEIGHT_STANDARD;
    }

    virtual wxCoord OnMeasureItemWidth(size_t item) const override {
        int swatchWidth = IsLargePalette() ? SWATCH_WIDTH_LARGE : SWATCH_WIDTH_STANDARD;
        return PALETTE_SIZE * swatchWidth - 1;
    }

    virtual void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const override {
        if (item == wxNOT_FOUND) return;

        wxString s = GetString(item);
        if (s == "") {
            GetVListBoxComboPopup()->UnsetToolTip();
        } else {
            wxArrayString as = wxSplit(s, ',');
            int i = 0;
            int swatchWidth = GetSwatchWidth();

            for (auto it = as.begin(); it != as.end() && i < PALETTE_SIZE; ++it) {
                if (it->Contains("Active")) {
                    ColorCurve cc(it->ToStdString());
                    dc.DrawBitmap(wxColorCurveRenderer::GetColorCurveImage(cc, swatchWidth - 1, rect.GetHeight() - 1, false),
                                  i * swatchWidth, rect.GetTop());
                } else {
                    xlColor c;
                    c.SetFromString(it->ToStdString());
                    wxPen p(xlColorToWxColour(c));
                    wxBrush b(xlColorToWxColour(c));
                    dc.SetPen(p);
                    dc.SetBrush(b);
                    dc.DrawRectangle(i * swatchWidth, rect.GetTop(), swatchWidth - 1, rect.GetHeight() - 1);
                }
                i++;
            }

            if (flags & wxODCB_PAINTING_SELECTED) {
                wxString file = as.back();
                GetVListBoxComboPopup()->SetToolTip(file);
            }
        }
    }
};

class ColourText2DropTarget : public wxTextDropTarget {
public:
    ColourText2DropTarget(BulkEditColourPickerCtrl* owner) { _owner = owner; }
    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override;
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;
    BulkEditColourPickerCtrl* _owner;
};

wxDragResult ColourText2DropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def) {
    if (_owner->IsEnabled()) {
        _owner->SetFocus();
        return wxDragCopy;
    }
    return wxDragNone;
}

bool ColourText2DropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data) {
    if (data == "") return false;
    if (!_owner->IsEnabled()) return false;
    if (ColorCurve::IsColorCurve(data)) {
        wxMessageBox("You cannot drag a colour curve onto this colour picker.");
    } else {
        _owner->SetColour(data);
    }
    return true;
}

class ColourTextDropTarget : public wxTextDropTarget {
public:
    ColourTextDropTarget(ColorCurveButton* owner) { _owner = owner; }
    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override;
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;
    ColorCurveButton* _owner;
};

wxDragResult ColourTextDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def) {
    if (_owner->IsEnabled()) {
        _owner->SetFocus();
        return wxDragCopy;
    }
    return wxDragNone;
}

bool ColourTextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data) {
    if (data == "") return false;
    if (!_owner->IsEnabled()) return false;
    if (ColorCurve::IsColorCurve(data)) {
        _owner->SetValue(data);
        _owner->SetActive(true);
    } else {
        _owner->SetColor(data, false);
    }
    _owner->NotifyChange();
    return true;
}

namespace {
nlohmann::json LoadColorMetadata() {
    std::string metaDir = EffectPanelManager::GetMetadataDirectory();
    if (metaDir.empty()) return {};
    return JsonEffectPanel::LoadMetadata(metaDir + "/shared/Color.json");
}
} // namespace

ColorPanel::ColorPanel(wxWindow* parent, wxWindowID /*id*/,
                       const wxPoint& /*pos*/, const wxSize& /*size*/) :
    JsonEffectPanel(parent, LoadColorMetadata(), /*deferBuild*/ true) {
    BuildFromJson(metadata_);

    SetName("Color");

    // Restore the 'Reset panel when changing effects' preference.
    wxConfigBase* config = wxConfigBase::Get();
    bool reset = true;
    if (config) config->Read("xLightsResetColorPanel", &reset, true);
    if (_resetColorPanelCheck) _resetColorPanelCheck->SetValue(reset);

    _lastShowDir = xLightsFrame::CurrentDir;

    // SetDefaultPalette also calls LoadAllPalettes the first time.
    SetDefaultPalette();

    ValidateWindow();
    SetMinSize(wxSize(50, 50));
}

wxWindow* ColorPanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                           const nlohmann::json& prop, int /*cols*/) {
    std::string id = prop.value("id", "");
    if (id == "PaletteHeaderRow") return BuildPaletteHeaderRow(parentWin, sizer);
    if (id == "ResetPanelRow") return BuildResetPanelRow(parentWin, sizer);
    if (id == "ChromaKeyRow") return BuildChromaKeyRow(parentWin, sizer);
    if (id == "SparklesRow") return BuildSparklesRow(parentWin, sizer);
    if (id == "BrightnessLevelRow") return BuildBrightnessLevelRow(parentWin, sizer);
    return nullptr;
}

wxWindow* ColorPanel::BuildPaletteHeaderRow(wxWindow* parentWin, wxSizer* sizer) {
    // Top row: [palette grid][reverse/shift buttons column][colour list + menu].
    // Matches the legacy layout: the palette swatches are leftmost, the
    // reverse/shift vertical button stack sits to the immediate right, and
    // the recent-palettes dropdown + menu button end the row.
    auto* row = new wxFlexGridSizer(0, 3, 0, 0);
    row->AddGrowableCol(2);

    // Vertical stack of reverse / left-shift / right-shift buttons. Built
    // first so the pointers are ready when the row is assembled below.
    auto* btnCol = new wxBoxSizer(wxVERTICAL);
    _reverseColoursButton = new xlSizedBitmapButton(
        parentWin, wxNewId(),
        wxArtProvider::GetBitmapBundle("xlART_colorpanel_reverse_xpm", wxART_BUTTON),
        wxDefaultPosition, wxSize(26, 16),
        wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator,
        _T("ID_BITMAPBUTTON_ReverseColours"));
    btnCol->Add(_reverseColoursButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 2);
    _reverseColoursButton->Bind(wxEVT_BUTTON, &ColorPanel::OnBitmapButton_ReverseColoursClick, this);

    _leftShiftColoursButton = new xlSizedBitmapButton(
        parentWin, wxNewId(),
        wxArtProvider::GetBitmapBundle("xlART_colorpanel_left_shift_xpm", wxART_BUTTON),
        wxDefaultPosition, wxSize(26, 16), wxBU_AUTODRAW, wxDefaultValidator,
        _T("ID_BITMAPBUTTON_LeftShiftColours"));
    btnCol->Add(_leftShiftColoursButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 2);
    _leftShiftColoursButton->Bind(wxEVT_BUTTON, &ColorPanel::OnBitmapButton_ShiftColoursLeftClick, this);

    _rightShiftColoursButton = new xlSizedBitmapButton(
        parentWin, wxNewId(),
        wxArtProvider::GetBitmapBundle("xlART_colorpanel_right_shift_xpm", wxART_BUTTON),
        wxDefaultPosition, wxSize(26, 16), wxBU_AUTODRAW, wxDefaultValidator,
        _T("ID_BITMAPBUTTON_RightShiftColours"));
    btnCol->Add(_rightShiftColoursButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 2);
    _rightShiftColoursButton->Bind(wxEVT_BUTTON, &ColorPanel::OnBitmapButton_ShiftColoursRightClick, this);

    // Column 1: 4-row palette grid (8 checkboxes, 8 color-curve buttons, 8 lock buttons, 8 cc activation buttons).
    _paletteGridSizer = new wxFlexGridSizer(0, PALETTE_SIZE, 0, 0);
    for (int x = 0; x < PALETTE_SIZE; x++) {
        _paletteGridSizer->AddGrowableCol(x);
    }
    // Row 1: enable checkboxes
    for (int x = 0; x < PALETTE_SIZE; x++) {
        wxString ids = wxString::Format("ID_CHECKBOX_Palette%d", x + 1);
        auto* cb = new wxCheckBox(parentWin, wxNewId(), wxEmptyString,
                                   wxDefaultPosition, wxDefaultSize, wxNO_BORDER,
                                   wxDefaultValidator, ids);
        cb->SetValue(x < 2);
        _paletteGridSizer->Add(cb, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
        checkBoxes.push_back(cb);
        cb->Bind(wxEVT_CHECKBOX, &ColorPanel::OnCheckBox_PaletteClick, this);
    }
    // Row 2: ColorCurveButtons
    for (int x = 0; x < PALETTE_SIZE; x++) {
        wxString ids = wxString::Format("ID_BUTTON_Palette%d", x + 1);
        int btnSize = IsLargePalette() ? PALETTE_BUTTON_SIZE_LARGE : PALETTE_BUTTON_SIZE_STANDARD;
        auto* bb = new ColorCurveButton(parentWin, wxNewId(), wxNullBitmap,
                                         wxDefaultPosition, FromDIP(wxSize(btnSize, btnSize)),
                                         wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, ids);
        bb->SetDropTarget(new ColourTextDropTarget(bb));
        _paletteGridSizer->Add(bb, 0, wxALIGN_LEFT | wxALIGN_TOP, 0);
        buttons.push_back(bb);
    }
    Connect(wxID_ANY, EVT_CC_CHANGED, (wxObjectEventFunction)&ColorPanel::OnCCChanged, 0, this);
    // Row 3: lock buttons
    for (int x = 0; x < PALETTE_SIZE; x++) {
        wxString ids = wxString::Format("ID_BITMAPBUTTON_BUTTON_Palette%d", x + 1);
        auto* bb = new xlLockButton(parentWin, wxNewId(),
                                     wxArtProvider::GetBitmapBundle("xlART_PADLOCK_OPEN", wxART_BUTTON),
                                     wxDefaultPosition, wxDefaultSize,
                                     wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, ids);
        _paletteGridSizer->Add(bb, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
        bb->Bind(wxEVT_BUTTON, &ColorPanel::OnLockButtonClick, this);
    }
    // Row 4: color-curve activation buttons
    for (int x = 0; x < PALETTE_SIZE; x++) {
        wxString ids = wxString::Format("ID_BITMAPBUTTON_BUTTON_PaletteCC%d", x + 1);
        int ccSize = IsLargePalette() ? PALETTE_CC_SIZE_LARGE : PALETTE_CC_SIZE_STANDARD;
        auto* bb = new xlSizedBitmapButton(parentWin, wxNewId(),
                                            wxArtProvider::GetBitmapBundle("xlART_cc_na_xpm", wxART_BUTTON),
                                            wxDefaultPosition, wxSize(ccSize, ccSize),
                                            wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, ids);
        bb->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
        _paletteGridSizer->Add(bb, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
        bb->Bind(wxEVT_BUTTON, &ColorPanel::OnCCButtonClick, this);
    }
    row->Add(_paletteGridSizer, 0, wxALL | wxALIGN_TOP, 2);

    // Column 2: the reverse / shift vertical button stack (built above).
    row->Add(btnCol, 0, wxALL | wxALIGN_TOP, 2);

    // Column 3: ColourList dropdown + menu button
    auto* menuCol = new wxBoxSizer(wxHORIZONTAL);
    _colourList = new ColourList(parentWin, wxNewId(), wxDefaultPosition, wxDefaultSize,
                                  0, wxDefaultValidator, _T("ID_CUSTOM1"));
    menuCol->Add(_colourList, 1, wxALL, 2);
    _colourList->Bind(wxEVT_COMBOBOX_DROPDOWN, &ColorPanel::OnColourChoiceDropDown, this);
    _colourList->Bind(wxEVT_COMBOBOX, &ColorPanel::OnColourChoiceSelect, this);

    _menuPaletteButton = new wxButton(parentWin, wxNewId(), L"\u2630",
                                       wxDefaultPosition, wxDLG_UNIT(parentWin, wxSize(18, -1)),
                                       0, wxDefaultValidator, _T("ID_BUTTON1"));
    _menuPaletteButton->SetMinSize(wxDLG_UNIT(parentWin, wxSize(18, -1)));
    _menuPaletteButton->SetMaxSize(wxDLG_UNIT(parentWin, wxSize(18, -1)));
    menuCol->Add(_menuPaletteButton, 0, wxALL | wxALIGN_TOP, 5);
    _menuPaletteButton->Bind(wxEVT_BUTTON, &ColorPanel::OnBitmapButton_MenuPaletteClick, this);

    row->Add(menuCol, 0, wxALL | wxALIGN_LEFT, 2);

    sizer->Add(row, 1, wxALL | wxEXPAND, 2);
    // Return the reverse-colours button as the "anchor" control so the
    // framework sees a non-null result. The JSON has no tooltip for this
    // compound row, so the specific widget chosen here doesn't matter —
    // only that it's non-null to satisfy the framework's CreateCustomControl
    // contract and avoid a spurious warning.
    return _reverseColoursButton;
}

wxWindow* ColorPanel::BuildResetPanelRow(wxWindow* parentWin, wxSizer* sizer) {
    _resetColorPanelCheck = new wxCheckBox(parentWin, wxNewId(),
                                            "Reset panel when changing effects",
                                            wxDefaultPosition, wxDefaultSize, 0,
                                            wxDefaultValidator,
                                            _T("IDD_CHECKBOX_ResetColorPanel"));
    _resetColorPanelCheck->SetValue(true);
    sizer->Add(_resetColorPanelCheck, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    _resetColorPanelCheck->Bind(wxEVT_CHECKBOX, &ColorPanel::OnCheckBox_ResetColorPanelClick, this);
    return _resetColorPanelCheck;
}

wxWindow* ColorPanel::BuildChromaKeyRow(wxWindow* parentWin, wxSizer* sizer) {
    auto* row = new wxFlexGridSizer(0, 4, 0, 0);
    row->AddGrowableCol(1);

    auto* label = new wxStaticText(parentWin, wxID_ANY, "Chroma Key");
    row->Add(label, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

    _chromaSensitivity = new BulkEditSlider(parentWin, wxNewId(), 1, 1, 255,
                                             wxDefaultPosition, wxDefaultSize, 0,
                                             wxDefaultValidator,
                                             _T("ID_SLIDER_ChromaSensitivity"));
    row->Add(_chromaSensitivity, 1, wxALL | wxEXPAND, 2);

    _chromaColour = new BulkEditColourPickerCtrl(parentWin, wxNewId(), wxColour(0, 0, 0),
                                                  wxDefaultPosition, wxDefaultSize, 0,
                                                  wxDefaultValidator,
                                                  _T("ID_COLOURPICKERCTRL_ChromaColour"));
    _chromaColour->SetDropTarget(new ColourText2DropTarget(_chromaColour));
    row->Add(_chromaColour, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    _enableChromaCheck = new BulkEditCheckBox(parentWin, wxNewId(), wxEmptyString,
                                               wxDefaultPosition, wxDefaultSize, 0,
                                               wxDefaultValidator,
                                               _T("ID_CHECKBOX_Chroma"));
    _enableChromaCheck->SetValue(false);
    row->Add(_enableChromaCheck, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    _enableChromaCheck->Bind(wxEVT_CHECKBOX, &ColorPanel::OnCheckBox_EnableChromakeyClick, this);

    sizer->Add(row, 1, wxALL | wxEXPAND, 2);
    return _chromaSensitivity;
}

wxWindow* ColorPanel::BuildSparklesRow(wxWindow* parentWin, wxSizer* sizer) {
    // Vertical stack of two rows:
    //   Row 1: [label] [slider + VC] [text] [lock]       — matches the legacy
    //                                                      standard 4-col slider layout.
    //   Row 2: [spacer] [Sparkles reflect music checkbox + sparkles color picker + lock]
    auto* outer = new wxBoxSizer(wxVERTICAL);

    auto* topRow = new wxFlexGridSizer(0, 4, 0, 0);
    topRow->AddGrowableCol(1);

    auto* label = new wxStaticText(parentWin, wxID_ANY, "Sparkles",
                                    wxDefaultPosition, wxDefaultSize, 0,
                                    _T("ID_STATICTEXT_SparkleFrequency"));
    topRow->Add(label, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

    auto* sliderSizer = new wxBoxSizer(wxHORIZONTAL);
    _sparkleFrequency = new BulkEditSlider(parentWin, wxNewId(), 0, 0, 200,
                                            wxDefaultPosition, wxDefaultSize, 0,
                                            wxDefaultValidator,
                                            _T("ID_SLIDER_SparkleFrequency"));
    sliderSizer->Add(_sparkleFrequency, 1, wxALL | wxEXPAND, 0);
    _sparkleFrequencyVC = new BulkEditValueCurveButton(parentWin, wxNewId(),
                                                        GetValueCurveNotSelectedBitmap(),
                                                        wxDefaultPosition, wxDefaultSize,
                                                        wxBU_AUTODRAW | wxBORDER_NONE,
                                                        wxDefaultValidator,
                                                        _T("ID_VALUECURVE_SparkleFrequency"));
    _sparkleFrequencyVC->GetValue()->SetLimits(COLORPANEL_SPARKLE_MIN, COLORPANEL_SPARKLE_MAX);
    sliderSizer->Add(_sparkleFrequencyVC, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    _sparkleFrequencyVC->Bind(wxEVT_BUTTON, &ColorPanel::OnVCButtonClick, this);
    topRow->Add(sliderSizer, 1, wxALL | wxEXPAND, 0);

    _sparkleFrequencyText = new BulkEditTextCtrl(parentWin, wxNewId(), _T("0"),
                                                  wxDefaultPosition,
                                                  wxDLG_UNIT(parentWin, wxSize(20, -1)), 0,
                                                  wxDefaultValidator,
                                                  _T("IDD_TEXTCTRL_SparkleFrequency"));
    topRow->Add(_sparkleFrequencyText, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    _sparkleFrequencyLock = new xlLockButton(parentWin, wxNewId(), wxNullBitmap,
                                               wxDefaultPosition, wxSize(14, 14),
                                               wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator,
                                               _T("ID_BITMAPBUTTON_SLIDER_SparkleFrequency"));
    topRow->Add(_sparkleFrequencyLock, 0, wxALL | wxALIGN_CENTER_VERTICAL, 0);
    _sparkleFrequencyLock->Bind(wxEVT_BUTTON, &ColorPanel::OnLockButtonClick, this);

    outer->Add(topRow, 0, wxALL | wxEXPAND, 0);

    // Row 2: music checkbox + sparkles color picker + lock, indented under
    // the Sparkles label.
    auto* musicRow = new wxBoxSizer(wxHORIZONTAL);
    _musicSparklesCheck = new BulkEditCheckBox(parentWin, wxNewId(), "Sparkles reflect music",
                                                wxDefaultPosition, wxDefaultSize, 0,
                                                wxDefaultValidator,
                                                _T("ID_CHECKBOX_MusicSparkles"));
    _musicSparklesCheck->SetValue(false);
    musicRow->Add(_musicSparklesCheck, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    _musicSparklesCheck->Bind(wxEVT_CHECKBOX, &ColorPanel::OnCheckBox_MusicSparklesClick, this);

    _sparklesColour = new BulkEditColourPickerCtrl(parentWin, wxNewId(),
                                                    wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT),
                                                    wxDefaultPosition, wxDefaultSize, 0,
                                                    wxDefaultValidator,
                                                    _T("ID_COLOURPICKERCTRL_SparklesColour"));
    _sparklesColour->SetDropTarget(new ColourText2DropTarget(_sparklesColour));
    musicRow->Add(_sparklesColour, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    _musicSparklesLock = new xlLockButton(parentWin, wxNewId(), wxNullBitmap,
                                           wxDefaultPosition, wxSize(14, 14),
                                           wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator,
                                           _T("ID_BITMAPBUTTON_MusicSparkles"));
    musicRow->Add(_musicSparklesLock, 0, wxALL | wxALIGN_CENTER_VERTICAL, 0);
    _musicSparklesLock->Bind(wxEVT_BUTTON, &ColorPanel::OnLockButtonClick, this);

    outer->Add(musicRow, 0, wxLEFT | wxEXPAND, 8);

    sizer->Add(outer, 1, wxALL | wxEXPAND, 2);
    return _sparkleFrequency;
}

wxWindow* ColorPanel::BuildBrightnessLevelRow(wxWindow* parentWin, wxSizer* sizer) {
    // Legacy serializes this key as "CHECKBOXBRIGHTNESSLEVEL" (no underscore
    // between CHECKBOX and BRIGHTNESSLEVEL). The framework's standard
    // checkbox row would produce "ID_CHECKBOX_BRIGHTNESSLEVEL" which maps to
    // "E_CHECKBOX_BRIGHTNESSLEVEL" — but the render engine reads the no-
    // underscore form. Build manually with the legacy name to preserve
    // round-trip compatibility.
    _brightnessLevelCheck = new BulkEditCheckBox(parentWin, wxNewId(), "Brightness Level",
                                                  wxDefaultPosition, wxDefaultSize, 0,
                                                  wxDefaultValidator,
                                                  _T("ID_CHECKBOXBRIGHTNESSLEVEL"));
    _brightnessLevelCheck->SetValue(false);
    sizer->Add(_brightnessLevelCheck, 0, wxALL | wxEXPAND, 5);
    return _brightnessLevelCheck;
}

// -------- Palette management --------

void ColorPanel::LoadAllPalettes() {
    _loadedPalettes.clear();

    wxDir dir;
    if (wxDir::Exists(xLightsFrame::CurrentDir)) {
        dir.Open(xLightsFrame::CurrentDir);
        LoadPalettes(dir, false);
    }

    wxString d = xLightsFrame::CurrentDir + "/Palettes";
    if (wxDir::Exists(d)) {
        dir.Open(d);
        LoadPalettes(dir, true);
    }

    wxStandardPaths stdp = wxStandardPaths::Get();
#ifndef __WXMSW__
    d = wxStandardPaths::Get().GetResourcesDir() + "/palettes";
#else
    d = wxFileName(stdp.GetExecutablePath()).GetPath() + "/palettes";
#endif
    if (wxDir::Exists(d)) {
        dir.Open(d);
        LoadPalettes(dir, true);
    }

    if (_colourList && _colourList->GetCount() != 0) {
        _colourList->Clear();
    }
    if (_colourList) {
        _colourList->AppendString("");
        for (auto it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it) {
            _colourList->AppendString(*it);
        }
    }
    FireChangeEvent();
}

void ColorPanel::LoadPalettes(wxDir& directory, bool subdirs) {
    static wxRegEx cregex("^\\$[^:]*: rgba\\(([^)]*)\\)");

    wxArrayString files;
    GetAllFilesInDir(directory.GetName(), files, "*.xpalette");
    for (auto& filename : files) {
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
                    if (p.BeforeLast(',') == scomp) { found = true; break; }
                }
                if (!found) {
                    _loadedPalettes.push_back(s.ToStdString() + fn.GetFullName().ToStdString());
                }
            }
        }
    }

    files.clear();
    GetAllFilesInDir(directory.GetNameWithSep(), files, "*.scss");
    for (auto& filename : files) {
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
                                                     wxAtoi(comp[0]), wxAtoi(comp[1]), wxAtoi(comp[2]));
                            cols++;
                        }
                    }
                }
                if (cols > 0) {
                    while (cols < 8) { pal += "#FFFFFF,"; cols++; }
                    bool found = false;
                    for (auto it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it) {
                        wxString p(*it);
                        if (p.BeforeLast(',') == pal) { found = true; break; }
                    }
                    if (!found) {
                        _loadedPalettes.push_back(pal.ToStdString() + fn.GetFullName().ToStdString());
                    }
                }
            }
        }
    }

    if (subdirs) {
        wxString subdir;
        bool cont = directory.GetFirst(&subdir, wxEmptyString, wxDIR_DIRS);
        while (cont) {
            wxDir sub;
            if (sub.Open(directory.GetNameWithSep() + subdir)) {
                LoadPalettes(sub, true);
            }
            cont = directory.GetNext(&subdir);
        }
    }
}

void ColorPanel::SetDefaultPalette() {
    if (buttons.size() < 8) return;
    SetButtonColor(buttons[0], "#FFFFFF");
    SetButtonColor(buttons[1], "#FF0000");
    SetButtonColor(buttons[2], "#00FF00");
    SetButtonColor(buttons[3], "#0000FF");
    SetButtonColor(buttons[4], "#FFFF00");
    SetButtonColor(buttons[5], "#000000");
    SetButtonColor(buttons[6], "#00FFFF");
    SetButtonColor(buttons[7], "#FF00FF");

    if (_colourList && _colourList->GetCount() < 2) {
        LoadAllPalettes();
    }
    FireChangeEvent();
    ValidateWindow();
}

wxColour ColorPanel::GetPaletteColor(int idx) const {
    if (idx < (int)buttons.size()) {
        return buttons[idx]->GetBackgroundColour();
    }
    return *wxBLACK;
}

wxCheckBox* ColorPanel::GetPaletteCheckbox(int idx) {
    if (idx < (int)checkBoxes.size()) return checkBoxes[idx];
    return checkBoxes[0];
}

wxButton* ColorPanel::GetPaletteButton(int idx) {
    if (idx < (int)buttons.size()) return buttons[idx];
    return buttons[0];
}

std::string ColorPanel::GetCurrentPalette() const {
    std::string res;
    for (size_t i = 0; i < PALETTE_SIZE; i++) {
        wxString ids = wxString::Format("ID_BUTTON_Palette%d", (int)i + 1);
        auto* btn = (ColorCurveButton*)wxWindow::FindWindowByName(ids, const_cast<ColorPanel*>(this));
        if (btn && btn->GetValue()->IsActive()) {
            res += btn->GetValue()->Serialise() + ",";
        } else {
            wxColor color = GetPaletteColor(i);
            color.Set(color.Red(), color.Green(), color.Blue(), wxALPHA_OPAQUE);
            res += color.GetAsString(wxC2S_HTML_SYNTAX).ToStdString() + ",";
        }
    }
    return res;
}

void ColorPanel::SetButtonColor(int btn, const xlColor& v, bool notify) {
    SetButtonColor(buttons[btn], v, notify);
}

void ColorPanel::SetButtonColor(ColorCurveButton* btn, const std::string& cstr, bool notify) {
    if (cstr.find("Active") != std::string::npos) {
        btn->GetValue()->Deserialise(cstr);
        btn->UpdateState(notify);
    } else {
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

// -------- Color string builders (render-time) --------

wxString ColorPanel::GetRandomColorString() {
    wxString AttrName;
    wxString ret;
    wxColour color;
    for (int i = 0; i < PALETTE_SIZE; i++) {
        color = GetPaletteColor(i);
        AttrName.Printf("C_BUTTON_Palette%d=", i + 1);
        ret += AttrName + color.GetAsString(wxC2S_HTML_SYNTAX) + ",";

        wxString v = (!EffectPanelUtils::IsLocked(GetPaletteButton(i)->GetName().ToStdString())
                      ? rand() % 2 : GetPaletteCheckbox(i)->GetValue()) ? "1" : "0";
        AttrName.Printf("C_CHECKBOX_Palette%d=", i + 1);
        ret += AttrName + v + ",";
    }
    if (_sparkleFrequency && _sparkleFrequency->GetValue() != 0) {
        ret += wxString::Format("C_SLIDER_SparkleFrequency=%d,", _sparkleFrequency->GetValue());
    }
    if (_musicSparklesCheck && _musicSparklesCheck->GetValue()) {
        ret += wxString::Format("C_CHECKBOX_MusicSparkles=%d,", _musicSparklesCheck->GetValue());
    }
    if (_brightnessLevelCheck && _brightnessLevelCheck->GetValue()) {
        ret += wxString::Format("C_CHECKBOXBRIGHTNESSLEVEL=%d,", _brightnessLevelCheck->GetValue());
    }
    // Emit C_SLIDER_* (not C_TEXTCTRL_*): PixelBuffer reads SLIDER_Brightness /
    // SLIDER_Contrast / SLIDER_Color_HueAdjust etc., and Effect::CopySettingsMap's
    // palette filter only keeps keys whose name[2] is S / C / V, so TEXTCTRL
    // entries would be dropped from the palette map before reaching the renderer.
    auto sliderValue = [this](const char* propId) -> long {
        auto* p = GetPropertyInfo(propId);
        if (p == nullptr) return 0;
        if (p->slider) return p->slider->GetValue();
        return 0;
    };
    long brightness = sliderValue("Brightness");
    if (brightness != 100) ret += wxString::Format("C_SLIDER_Brightness=%ld,", brightness);
    long contrast = sliderValue("Contrast");
    if (contrast != 0) ret += wxString::Format("C_SLIDER_Contrast=%ld,", contrast);
    auto appendHSV = [&](const char* propId, const char* key) {
        long v = sliderValue(propId);
        if (v != 0) ret += wxString::Format("%s=%ld,", key, v);
    };
    appendHSV("Color_HueAdjust", "C_SLIDER_Color_HueAdjust");
    appendHSV("Color_SaturationAdjust", "C_SLIDER_Color_SaturationAdjust");
    appendHSV("Color_ValueAdjust", "C_SLIDER_Color_ValueAdjust");
    if (_chromaSensitivity && _chromaSensitivity->GetValue() != 1) {
        ret += wxString::Format("C_SLIDER_ChromaSensitivity=%d,", _chromaSensitivity->GetValue());
    }
    if (_enableChromaCheck && _enableChromaCheck->GetValue()) {
        ret += "C_CHECKBOX_Chroma=1,";
    }
    if (_chromaColour && _chromaColour->GetColour() != *wxBLACK) {
        ret += "C_COLOURPICKERCTRL_ChromaColour=" + _chromaColour->GetStringValue() + ",";
    }
    if (_sparklesColour && _sparklesColour->GetColour() != *wxWHITE) {
        ret += "C_COLOURPICKERCTRL_SparklesColour=" + _sparklesColour->GetStringValue() + ",";
    }
    return ret;
}

wxString ColorPanel::GetColorString(bool colourOnly) {
    wxString s, AttrName;
    for (int i = 0; i < PALETTE_SIZE; i++) {
        auto* btn = buttons[i];
        if (btn->GetValue()->IsActive()) {
            AttrName.Printf("C_BUTTON_Palette%d=", i + 1);
            s += AttrName + btn->GetValue()->Serialise() + ",";
        } else {
            wxColour color = GetPaletteColor(i);
            AttrName.Printf("C_BUTTON_Palette%d=", i + 1);
            s += AttrName + color.GetAsString(wxC2S_HTML_SYNTAX) + ",";
        }
        if (checkBoxes[i]->IsChecked()) {
            AttrName.Printf("C_CHECKBOX_Palette%d=1,", i + 1);
            s += AttrName;
        }
    }

    if (colourOnly) return s;

    // Sparkles (manual because compound custom row)
    if (_sparkleFrequencyVC && _sparkleFrequencyVC->GetValue()->IsActive()) {
        s += "C_VALUECURVE_SparkleFrequency=";
        s += wxString(_sparkleFrequencyVC->GetValue()->Serialise().c_str());
        s += ",";
    } else if (_sparkleFrequency && _sparkleFrequency->GetValue() != 0) {
        s += wxString::Format("C_SLIDER_SparkleFrequency=%d,", _sparkleFrequency->GetValue());
    }
    if (_musicSparklesCheck && _musicSparklesCheck->GetValue()) {
        s += wxString::Format("C_CHECKBOX_MusicSparkles=%d,", _musicSparklesCheck->GetValue());
    }
    if (_brightnessLevelCheck && _brightnessLevelCheck->GetValue()) {
        s += wxString::Format("C_CHECKBOXBRIGHTNESSLEVEL=%d,", _brightnessLevelCheck->GetValue());
    }

    // Framework-built sliders (Brightness / Contrast / HSV). Use the
    // framework's filtered effect string but with the C_ prefix swap.
    wxString frameworkSettings = JsonEffectPanel::GetEffectString();
    frameworkSettings.Replace(",E_", ",C_");
    if (frameworkSettings.StartsWith("E_")) {
        frameworkSettings = "C_" + frameworkSettings.Mid(2);
    }
    if (!frameworkSettings.empty()) {
        s += frameworkSettings;
        if (!s.EndsWith(",")) s += ",";
    }

    // Chroma key (manual)
    if (_enableChromaCheck && _enableChromaCheck->GetValue()) {
        if (_chromaSensitivity && _chromaSensitivity->GetValue() != 1) {
            s += wxString::Format("C_SLIDER_ChromaSensitivity=%d,", _chromaSensitivity->GetValue());
        }
        if (_chromaColour && _chromaColour->GetColour() != *wxBLACK) {
            s += "C_COLOURPICKERCTRL_ChromaColour=" + _chromaColour->GetStringValue() + ",";
        }
        s += "C_CHECKBOX_Chroma=1,";
    }

    if (_sparklesColour && _sparklesColour->GetColour() != *wxWHITE) {
        s += "C_COLOURPICKERCTRL_SparklesColour=" + _sparklesColour->GetStringValue() + ",";
    }

    return s;
}

// -------- Reset / validate --------

void ColorPanel::SetDefaultSettings(bool optionbased) {
    if (!optionbased) {
        for (const auto& it : checkBoxes) {
            it->SetValue(false);
        }
    }

    if (!optionbased || (_resetColorPanelCheck && _resetColorPanelCheck->GetValue())) {
        if (_sparkleFrequencyVC) {
            _sparkleFrequencyVC->GetValue()->SetDefault(COLORPANEL_SPARKLE_MIN, COLORPANEL_SPARKLE_MAX);
            _sparkleFrequencyVC->UpdateState();
        }
        if (_sparkleFrequencyText) _sparkleFrequencyText->SetValue("0");
        if (_sparkleFrequency) _sparkleFrequency->SetValue(0);

        if (_musicSparklesCheck) _musicSparklesCheck->SetValue(false);
        if (_brightnessLevelCheck) _brightnessLevelCheck->SetValue(false);

        if (_chromaSensitivity) _chromaSensitivity->SetValue(1);
        if (_chromaColour) _chromaColour->SetColour(*wxBLACK);
        if (_enableChromaCheck) _enableChromaCheck->SetValue(false);
        if (_sparklesColour) _sparklesColour->SetColour(*wxWHITE);

        // Framework-built slider defaults
        JsonEffectPanel::SetDefaultParameters();

#ifdef __XLIGHTS_HAS_TOUCHBARS__
        if (touchBar) touchBar->SetSparkles(0);
#endif
    }
    FireChangeEvent();
    ValidateWindow();
}

void ColorPanel::ValidateWindow() {
    JsonEffectPanel::ValidateWindow();

    // Update the color-curve activation button bitmaps to reflect current
    // time-curve state on each palette color-curve button.
    for (int x = 0; x < PALETTE_SIZE; x++) {
        if (x >= (int)buttons.size()) break;
        auto* ccb = buttons[x];
        wxString tsids = wxString::Format("ID_BITMAPBUTTON_BUTTON_PaletteCC%d", x + 1);
        auto* ts = (wxBitmapButton*)wxWindow::FindWindowByName(tsids, this);
        if (ts == nullptr) continue;

        if (ccb->GetValue()->IsActive()) {
            if (!_supportslinear && !_supportsradial) {
                ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_timelocked_xpm", wxART_BUTTON));
            } else {
                switch (ccb->GetValue()->GetTimeCurve()) {
                case TC_TIME:      ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_time_xpm", wxART_BUTTON)); break;
                case TC_LEFT:      ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_left_xpm", wxART_BUTTON)); break;
                case TC_RIGHT:     ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_right_xpm", wxART_BUTTON)); break;
                case TC_UP:        ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_up_xpm", wxART_BUTTON)); break;
                case TC_DOWN:      ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_down_xpm", wxART_BUTTON)); break;
                case TC_RADIALOUT: ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_radialout_xpm", wxART_BUTTON)); break;
                case TC_RADIALIN:  ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_radialin_xpm", wxART_BUTTON)); break;
                case TC_CW:        ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_cw_xpm", wxART_BUTTON)); break;
                case TC_CCW:       ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_ccw_xpm", wxART_BUTTON)); break;
                }
            }
        } else {
            ts->SetBitmap(wxArtProvider::GetBitmapBundle("xlART_cc_na_xpm", wxART_BUTTON));
        }
    }

    if (_enableChromaCheck && _enableChromaCheck->GetValue()) {
        if (_chromaSensitivity) _chromaSensitivity->Enable();
        if (_chromaColour) _chromaColour->Enable();
    } else {
        if (_chromaSensitivity) _chromaSensitivity->Enable(false);
        if (_chromaColour) _chromaColour->Enable(false);
    }
}

void ColorPanel::SetColorCount(int /*count*/) {
    // Legacy behavior was #if 0'd out — no-op. Preserved for API stability.
}

void ColorPanel::SetSupports(bool linear, bool radial) {
    _supportslinear = linear;
    _supportsradial = radial;
    for (int i = 0; i < PALETTE_SIZE && i < (int)buttons.size(); ++i) {
        buttons[i]->GetValue()->SetValidTimeCurve(linear, radial);
    }
    FireChangeEvent();
    ValidateWindow();
}

void ColorPanel::RefreshPaletteSize() {
    int btnSize = IsLargePalette() ? PALETTE_BUTTON_SIZE_LARGE : PALETTE_BUTTON_SIZE_STANDARD;
    for (auto* btn : buttons) {
        btn->SetSize(FromDIP(wxSize(btnSize, btnSize)));
        btn->SetMinSize(FromDIP(wxSize(btnSize, btnSize)));
    }

    int ccSize = IsLargePalette() ? PALETTE_CC_SIZE_LARGE : PALETTE_CC_SIZE_STANDARD;
    for (int x = 0; x < PALETTE_SIZE; x++) {
        wxString ids = wxString::Format("ID_BITMAPBUTTON_BUTTON_PaletteCC%d", x + 1);
        auto* ccBtn = (wxBitmapButton*)wxWindow::FindWindowByName(ids, this);
        if (ccBtn) {
            ccBtn->SetSize(wxSize(ccSize, ccSize));
            ccBtn->SetMinSize(wxSize(ccSize, ccSize));
        }
    }

    int swatchWidth = IsLargePalette() ? SWATCH_WIDTH_LARGE : SWATCH_WIDTH_STANDARD;
    int dropdownWidth = PALETTE_SIZE * swatchWidth + 20;

    if (_colourList) {
        _colourList->Dismiss();
        _colourList->Clear();
        _colourList->AppendString("");
        for (auto it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it) {
            _colourList->AppendString(*it);
        }
        _colourList->SetMinSize(wxSize(dropdownWidth, -1));
        _colourList->SetSize(wxSize(dropdownWidth, -1));
        _colourList->SetPopupMinWidth(dropdownWidth);
        _colourList->SetSelection(0);
    }

    if (_paletteGridSizer) _paletteGridSizer->Layout();
    Layout();
}

// -------- Handlers --------

void ColorPanel::OnCheckBox_PaletteClick(wxCommandEvent& /*event*/) {
    FireChangeEvent();
    PaletteChanged = true;
}

void ColorPanel::OnCCChanged(wxCommandEvent& event) {
    auto* w = (ColorCurveButton*)event.GetEventObject();
    lastColors[w->GetId()] = w->GetColor();

#ifdef __XLIGHTS_HAS_TOUCHBARS__
    for (int x = 0; x < PALETTE_SIZE; x++) {
        if (buttons[x] == w) {
            wxColor c = w->GetBackgroundColour();
            touchBar->SetColor(x, w->GetBitmap(), c);
        }
    }
#endif

    if (event.GetInt() != 0) {
        wxCommandEvent e(EVT_COLOUR_CHANGED);
        e.SetInt(-1);
        wxPostEvent(xLightsApp::GetFrame(), e);
    }

    FireChangeEvent();
    Refresh();
    ValidateWindow();
}

void ColorPanel::UpdateColor() {
    int alleffects = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("");
    if (alleffects > 1) {
        if (!xLightsApp::GetFrame()->IsSuppressColorWarn()) {
            if (wxMessageBox("Are you sure you want to change the colours on all selected effects?", "Update all", wxYES_NO | wxCENTRE, this) == wxNO) return;
        }
    }
    wxCommandEvent eventEffectUpdated(EVT_EFFECT_PALETTE_UPDATED);
    wxPostEvent(GetParent(), eventEffectUpdated);
    FireChangeEvent();
    Refresh();
    ValidateWindow();
}

void ColorPanel::OnCheckBox_MusicSparklesClick(wxCommandEvent& /*event*/) {
    FireChangeEvent();
    ValidateWindow();
}

void ColorPanel::OnCheckBox_ResetColorPanelClick(wxCommandEvent& /*event*/) {
    wxConfigBase* config = wxConfigBase::Get();
    if (config && _resetColorPanelCheck) {
        config->Write("xLightsResetColorPanel", _resetColorPanelCheck->IsChecked());
    }
}

void ColorPanel::OnCheckBox_EnableChromakeyClick(wxCommandEvent& /*event*/) {
    ValidateWindow();
    // Chroma key is serialized, so a toggle must trigger a save.
    FireChangeEvent();
}

void ColorPanel::OnColourChoiceDropDown(wxCommandEvent& /*event*/) {
    if (_lastShowDir != xLightsFrame::CurrentDir) {
        _lastShowDir = xLightsFrame::CurrentDir;
        LoadAllPalettes();
        ValidateWindow();
        FireChangeEvent();
    }
}

void ColorPanel::OnColourChoiceSelect(wxCommandEvent& event) {
    long sel = event.GetInt();
    if (!_colourList) return;
    wxString s = _colourList->GetString(sel);
    if (s != "") {
        wxArrayString as = wxSplit(s, ',');
        for (size_t i = 0; i < std::min(as.size(), buttons.size()); i++) {
            if (as[i].Contains("Active")) {
                buttons[i]->GetValue()->Deserialise(as[i].ToStdString());
                buttons[i]->SetActive(true);
                buttons[i]->Refresh();
            } else {
                buttons[i]->SetColor(as[i].ToStdString());
                buttons[i]->Refresh();
            }
        }
    }
    _colourList->SetSelection(0);
    FireChangeEvent();
    ValidateWindow();
}

// -------- Palette menu actions --------

wxString ColorPanel::RemoveNonAlphanumeric(wxString const& str) const {
    wxString result;
    for (wxString::const_iterator it = str.begin(); it != str.end(); ++it) {
        if (wxIsalnum(*it)) result.Append(*it);
    }
    return result;
}

void ColorPanel::OnBitmapButton_MenuPaletteClick(wxCommandEvent& /*event*/) {
    wxMenu mnuLayer;
    wxMenuItem* updateItem = mnuLayer.Append(ID_MNU_UPDATE, "Update Palette");
    wxMenuItem* saveItem = mnuLayer.Append(ID_MNU_SAVE, "Save Palette");
    wxMenuItem* saveAsItem = mnuLayer.Append(ID_MNU_SAVE_AS, "Save Palette As");
    wxMenuItem* deleteItem = mnuLayer.Append(ID_MNU_DELETE, "Delete Palette");
    mnuLayer.Append(ID_MNU_IMPORT, "Import Palette");
    if (xLightsApp::GetFrame()->GetAIService(aiType::COLORPALETTES) != nullptr) {
        mnuLayer.Append(ID_MNU_GENERATE, "Generate Palette");
    }
    mnuLayer.Connect(wxEVT_COMMAND_MENU_SELECTED,
                     (wxObjectEventFunction)&ColorPanel::OnListPopup, nullptr, this);

    const int alleffects = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("");
    if (alleffects == 0) updateItem->Enable(false);
    deleteItem->Enable(false);

    wxString pal = wxString(GetCurrentPalette()).BeforeLast(',');
    for (auto it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it) {
        wxString ss(it->c_str());
        if (ss.BeforeLast(',') == pal) {
            saveItem->Enable(false);
            saveAsItem->Enable(false);
            if (FindPaletteFile(ss.AfterLast(','), pal + ",") != "") {
                deleteItem->Enable(true);
            } else {
                deleteItem->Enable(false);
            }
        }
    }
    PopupMenu(&mnuLayer);
}

void ColorPanel::OnListPopup(wxCommandEvent& event) {
    if (event.GetId() == ID_MNU_SAVE) SavePalette(false);
    else if (event.GetId() == ID_MNU_SAVE_AS) SavePalette(true);
    else if (event.GetId() == ID_MNU_DELETE) DeletePalette();
    else if (event.GetId() == ID_MNU_IMPORT) ImportPalette();
    else if (event.GetId() == ID_MNU_GENERATE) GeneratePalette();
    else if (event.GetId() == ID_MNU_UPDATE) UpdateColor();
}

bool ColorPanel::ValidateAndFormatPaletteString(wxString& input, wxString& /*errorMsg*/) {
    input = input.Trim().Trim(false);
    input.Replace(" ", "");
    input = input.Upper();

    wxArrayString colors = wxSplit(input, ',');
    size_t nonEmptyCount = 0;
    for (const wxString& color : colors) {
        if (!color.IsEmpty()) nonEmptyCount++;
    }
    if (nonEmptyCount < 1 || nonEmptyCount > 8) return false;

    for (const wxString& color : colors) {
        if (color.IsEmpty()) continue;
        if (color.length() != 7 || color[0] != '#') return false;
        for (size_t i = 1; i < color.length(); ++i) {
            wxChar c = color[i];
            if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) return false;
        }
    }
    return true;
}

void ColorPanel::LoadColorsToButtons(const wxString& colorString) {
    wxArrayString colors = wxSplit(colorString, ',');
    size_t buttonIndex = 0;
    for (const wxString& color : colors) {
        if (color.IsEmpty()) { buttonIndex++; continue; }
        if (buttonIndex < 8) {
            std::string colorStr = color.ToStdString();
            SetButtonColor(buttons[buttonIndex], colorStr);
            buttonIndex++;
        }
    }
}

void ColorPanel::GeneratePalette() {
    AIColorPaletteDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK) {
        int idx = 0;
        for (auto& color : dlg.GetColorStrings()) {
            if (idx < 8) {
                SetButtonColor(buttons[idx], color.ToStdString());
            }
            idx++;
        }
        FireChangeEvent();
        ValidateWindow();
    }
}

void ColorPanel::ImportPalette() {
    wxString lastInput = "";
    bool validInput = false;
    while (!validInput) {
        wxTextEntryDialog dialog(this,
                                 wxString::Format("Import Palette Text String (e.g. #000000,#454545,#565657) %s",
                                                  lastInput.IsEmpty() ? "" : "\nInvalid input: Please correct your entry."),
                                 "Palette Text String");
        dialog.SetValue(lastInput);
        if (dialog.ShowModal() == wxID_CANCEL) return;
        lastInput = dialog.GetValue();
        wxString errorMsg;
        if (ValidateAndFormatPaletteString(lastInput, errorMsg)) validInput = true;
    }
    LoadColorsToButtons(lastInput);
    wxLogMessage("Processed palette: %s", lastInput);
    FireChangeEvent();
    ValidateWindow();
}

void ColorPanel::SavePalette(bool saveAs) {
    if (_colourList && _colourList->GetCount() == 1) {
        LoadAllPalettes();
        ValidateWindow();
    }
    if (!wxDir::Exists(xLightsFrame::CurrentDir + "/Palettes")) {
        wxDir::Make(xLightsFrame::CurrentDir + "/Palettes");
    }

    int i = 1;
    wxString fn = "PAL001.xpalette";
    if (saveAs) {
        wxTextEntryDialog dialog(this, "Set Palette Name", "Set Palette Name");
        if (dialog.ShowModal() == wxID_OK) {
            fn = wxString::Format("%s.xpalette", RemoveNonAlphanumeric(dialog.GetValue()));
            if (FileExists(xLightsFrame::CurrentDir + "/Palettes/" + fn)) {
                wxString msg = fn + " File Already Exists. Override?";
                if (wxMessageBox(msg, "Override", wxYES_NO, this) != wxYES) return;
            }
        }
    } else {
        while (FileExists(xLightsFrame::CurrentDir + "/Palettes/" + fn)) {
            i++;
            fn = wxString::Format("PAL%03d.xpalette", i);
        }
    }

    wxFile f;
    f.Create(xLightsFrame::CurrentDir + "/Palettes/" + fn);
    if (f.IsOpened()) {
        std::string pal = GetCurrentPalette();
        f.Write(wxString(pal.c_str()));
        f.Close();
        _loadedPalettes.push_back(pal);
    } else {
        spdlog::error("Unable to create file {}.", fn.ToStdString());
    }
    LoadAllPalettes();
    ValidateWindow();
}

wxString ColorPanel::FindPaletteFile(const wxString& filename, const wxString& palette) const {
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
    if (FileExists(xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + "Palettes" + wxFileName::GetPathSeparator() + filename)) {
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

void ColorPanel::DeletePalette() {
    std::string pal = GetCurrentPalette();
    for (auto it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it) {
        wxString ss(it->c_str());
        if (ss.BeforeLast(',') + "," == pal) {
            wxString filename = FindPaletteFile(ss.AfterLast(','), pal);
            if (filename != "") {
                ::wxRemoveFile(filename);
            }
        }
    }
    LoadAllPalettes();
    ValidateWindow();
}

void ColorPanel::OnBitmapButton_ReverseColoursClick(wxCommandEvent& /*event*/) {
    std::string pal = GetCurrentPalette();
    wxArrayString as = wxSplit(pal, ',');
    for (size_t i = 0; i < PALETTE_SIZE; ++i) {
        if (as[i].Contains("Active")) {
            buttons[PALETTE_SIZE - i - 1]->GetValue()->Deserialise(as[i].ToStdString());
            buttons[PALETTE_SIZE - i - 1]->SetActive(true);
            buttons[PALETTE_SIZE - i - 1]->Refresh();
        } else {
            buttons[PALETTE_SIZE - i - 1]->SetColor(as[i].ToStdString());
            buttons[PALETTE_SIZE - i - 1]->Refresh();
        }
    }
    FireChangeEvent();
    ValidateWindow();
}

void ColorPanel::OnBitmapButton_ShiftColoursLeftClick(wxCommandEvent& /*event*/) {
    std::string pal = GetCurrentPalette();
    wxArrayString as = wxSplit(pal, ',');
    bool zeroActive = as[0].Contains("Active");
    std::string zeroItem = as[0].ToStdString();

    for (size_t i = 0; i < PALETTE_SIZE - 1; ++i) {
        if (as[i + 1].Contains("Active")) {
            buttons[i]->GetValue()->Deserialise(as[i + 1].ToStdString());
            buttons[i]->SetActive(true);
            buttons[i]->Refresh();
        } else {
            buttons[i]->SetColor(as[i + 1].ToStdString());
            buttons[i]->Refresh();
        }
    }
    if (zeroActive) {
        buttons[PALETTE_SIZE - 1]->GetValue()->Deserialise(zeroItem);
        buttons[PALETTE_SIZE - 1]->SetActive(true);
        buttons[PALETTE_SIZE - 1]->Refresh();
    } else {
        buttons[PALETTE_SIZE - 1]->SetColor(zeroItem);
        buttons[PALETTE_SIZE - 1]->Refresh();
    }
    FireChangeEvent();
    ValidateWindow();
}

void ColorPanel::OnBitmapButton_ShiftColoursRightClick(wxCommandEvent& /*event*/) {
    std::string pal = GetCurrentPalette();
    wxArrayString as = wxSplit(pal, ',');
    bool lastActive = as[PALETTE_SIZE - 1].Contains("Active");
    std::string lastItem = as[PALETTE_SIZE - 1].ToStdString();

    for (size_t i = PALETTE_SIZE - 1; i > 0; --i) {
        if (as[i - 1].Contains("Active")) {
            buttons[i]->GetValue()->Deserialise(as[i - 1].ToStdString());
            buttons[i]->SetActive(true);
            buttons[i]->Refresh();
        } else {
            buttons[i]->SetColor(as[i - 1].ToStdString());
            buttons[i]->Refresh();
        }
    }
    if (lastActive) {
        buttons[0]->GetValue()->Deserialise(lastItem);
        buttons[0]->SetActive(true);
        buttons[0]->Refresh();
    } else {
        buttons[0]->SetColor(lastItem);
        buttons[0]->Refresh();
    }
    FireChangeEvent();
    ValidateWindow();
}

void ColorPanel::OnCCButtonClick(wxCommandEvent& event) {
    auto* bb = static_cast<wxBitmapButton*>(event.GetEventObject());
    int id = wxAtoi(bb->GetName().Right(1));
    wxString ccbids = wxString::Format("ID_BUTTON_Palette%d", id);
    auto* ccb = (ColorCurveButton*)wxWindow::FindWindowByName(ccbids, this);
    if (ccb && ccb->GetValue()->IsActive()) {
        ccb->GetValue()->NextTimeCurve(_supportslinear, _supportsradial);
    }
    FireChangeEvent();
    ValidateWindow();
}

#ifdef __XLIGHTS_HAS_TOUCHBARS__
ColorPanelTouchBar* ColorPanel::SetupTouchBar(xlTouchBarSupport& tbs) {
    if (touchBar == nullptr && tbs.HasTouchBar()) {
        touchBar = std::unique_ptr<ColorPanelTouchBar>(new ColorPanelTouchBar(
            [this](int idx, wxColor c) {
                this->SetButtonColor(idx, wxColourToXlColor(c), false);
            },
            [this](int v) {
                if (_sparkleFrequencyVC) {
                    _sparkleFrequencyVC->SetValue(wxString::Format("%d", v));
                    _sparkleFrequencyVC->GetValue()->SetDefault(0.0f, 200.0f);
                }
                if (_sparkleFrequency) _sparkleFrequency->SetValue(v);
                if (_sparkleFrequencyText) _sparkleFrequencyText->SetValue(wxString::Format("%d", v));
            },
            tbs));
    }
    return touchBar.get();
}

void ColorPanel::UpdateTouchBarSlider(wxScrollEvent& event) {
    if (touchBar != nullptr) {
        touchBar->SetSparkles(event.GetPosition());
    }
    if (_sparkleFrequency) _sparkleFrequency->OnSlider_SliderUpdated(event);
}
#endif
