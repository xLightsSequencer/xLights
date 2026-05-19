/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "xlColourPickerDialog.h"
#include "xlColorCanvas.h"
#include "xlColourData.h"
#include "shared/utils/wxUtilities.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/spinctrl.h>

#include <wx/colordlg.h>
#include <cstdlib>
#include <chrono>

// ---------------------------------------------------------------------------
// All CSS named colors — 140 entries, 20 cols × 7 rows (no gaps in grid)
// ---------------------------------------------------------------------------
struct NamedColor { const char* name; uint8_t r, g, b; };
static const NamedColor CSS_COLORS[] = {
    {"AliceBlue",             240,248,255}, {"AntiqueWhite",          250,235,215},
    {"Aqua",                    0,255,255}, {"Aquamarine",             127,255,212},
    {"Azure",                 240,255,255}, {"Beige",                  245,245,220},
    {"Bisque",                255,228,196}, {"Black",                    0,  0,  0},
    {"BlanchedAlmond",        255,235,205}, {"Blue",                     0,  0,255},
    {"BlueViolet",            138, 43,226}, {"Brown",                  165, 42, 42},
    {"BurlyWood",             222,184,135}, {"CadetBlue",               95,158,160},
    {"Chartreuse",            127,255,  0}, {"Chocolate",              210,105, 30},
    {"Coral",                 255,127, 80}, {"CornflowerBlue",         100,149,237},
    {"Cornsilk",              255,248,220}, {"Crimson",                220, 20, 60},
    {"Cyan",                    0,255,255}, {"DarkBlue",                 0,  0,139},
    {"DarkCyan",                0,139,139}, {"DarkGoldenRod",           184,134, 11},
    {"DarkGray",              169,169,169}, {"DarkGreen",                0,100,  0},
    {"DarkKhaki",             189,183,107}, {"DarkMagenta",             139,  0,139},
    {"DarkOliveGreen",         85,107, 47}, {"DarkOrange",             255,140,  0},
    {"DarkOrchid",            153, 50,204}, {"DarkRed",                 139,  0,  0},
    {"DarkSalmon",            233,150,122}, {"DarkSeaGreen",            143,188,143},
    {"DarkSlateBlue",          72, 61,139}, {"DarkSlateGray",            47, 79, 79},
    {"DarkTurquoise",           0,206,209}, {"DarkViolet",              148,  0,211},
    {"DeepPink",              255, 20,147}, {"DeepSkyBlue",               0,191,255},
    {"DimGray",               105,105,105}, {"DodgerBlue",               30,144,255},
    {"FireBrick",             178, 34, 34}, {"FloralWhite",             255,250,240},
    {"ForestGreen",            34,139, 34}, {"Fuchsia",                 255,  0,255},
    {"Gainsboro",             220,220,220}, {"GhostWhite",              248,248,255},
    {"Gold",                  255,215,  0}, {"GoldenRod",               218,165, 32},
    {"Gray",                  128,128,128}, {"Green",                     0,128,  0},
    {"GreenYellow",           173,255, 47}, {"HoneyDew",                240,255,240},
    {"HotPink",               255,105,180}, {"IndianRed",               205, 92, 92},
    {"Indigo",                 75,  0,130}, {"Ivory",                   255,255,240},
    {"Khaki",                 240,230,140}, {"Lavender",                230,230,250},
    {"LavenderBlush",         255,240,245}, {"LawnGreen",               124,252,  0},
    {"LemonChiffon",          255,250,205}, {"LightBlue",               173,216,230},
    {"LightCoral",            240,128,128}, {"LightCyan",               224,255,255},
    {"LightGoldenRodYellow",  250,250,210}, {"LightGray",               211,211,211},
    {"LightGreen",            144,238,144}, {"LightPink",               255,182,193},
    {"LightSalmon",           255,160,122}, {"LightSeaGreen",            32,178,170},
    {"LightSkyBlue",          135,206,250}, {"LightSlateGray",          119,136,153},
    {"LightSteelBlue",        176,196,222}, {"LightYellow",             255,255,224},
    {"Lime",                    0,255,  0}, {"LimeGreen",                50,205, 50},
    {"Linen",                 250,240,230}, {"Magenta",                 255,  0,255},
    {"Maroon",                128,  0,  0}, {"MediumAquaMarine",        102,205,170},
    {"MediumBlue",              0,  0,205}, {"MediumOrchid",            186, 85,211},
    {"MediumPurple",          147,112,219}, {"MediumSeaGreen",           60,179,113},
    {"MediumSlateBlue",       123,104,238}, {"MediumSpringGreen",         0,250,154},
    {"MediumTurquoise",        72,209,204}, {"MediumVioletRed",          199, 21,133},
    {"MidnightBlue",           25, 25,112}, {"MintCream",               245,255,250},
    {"MistyRose",             255,228,225}, {"Moccasin",                255,228,181},
    {"NavajoWhite",           255,222,173}, {"Navy",                      0,  0,128},
    {"OldLace",               253,245,230}, {"Olive",                   128,128,  0},
    {"OliveDrab",             107,142, 35}, {"Orange",                  255,165,  0},
    {"OrangeRed",             255, 69,  0}, {"Orchid",                  218,112,214},
    {"PaleGoldenRod",         238,232,170}, {"PaleGreen",               152,251,152},
    {"PaleTurquoise",         175,238,238}, {"PaleVioletRed",           219,112,147},
    {"PapayaWhip",            255,239,213}, {"PeachPuff",               255,218,185},
    {"Peru",                  205,133, 63}, {"Pink",                    255,192,203},
    {"Plum",                  221,160,221}, {"PowderBlue",              176,224,230},
    {"Purple",                128,  0,128}, {"Red",                     255,  0,  0},
    {"RosyBrown",             188,143,143}, {"RoyalBlue",                65,105,225},
    {"SaddleBrown",           139, 69, 19}, {"Salmon",                  250,128,114},
    {"SandyBrown",            244,164, 96}, {"SeaGreen",                 46,139, 87},
    {"SeaShell",              255,245,238}, {"Sienna",                  160, 82, 45},
    {"Silver",                192,192,192}, {"SkyBlue",                 135,206,235},
    {"SlateBlue",             106, 90,205}, {"SlateGray",               112,128,144},
    {"Snow",                  255,250,250}, {"SpringGreen",               0,255,127},
    {"SteelBlue",              70,130,180}, {"Tan",                     210,180,140},
    {"Teal",                    0,128,128}, {"Thistle",                 216,191,216},
    {"Tomato",                255, 99, 71}, {"Turquoise",                64,224,208},
    {"Violet",                238,130,238}, {"Wheat",                   245,222,179},
    {"White",                 255,255,255}, {"WhiteSmoke",              245,245,245},
    {"Yellow",                255,255,  0}, {"YellowGreen",             154,205, 50},
};
static constexpr int CSS_COUNT   = (int)(sizeof(CSS_COLORS)/sizeof(CSS_COLORS[0]));  // 140
static constexpr int CSS_COLS    = 20;   // 140 / 20 = 7 rows exactly, no gaps
static constexpr int RECENT_COUNT = 16;
static constexpr int SWATCH_W    = 16;
static constexpr int SWATCH_H    = 16;  // square swatches

// ---------------------------------------------------------------------------
// Event IDs
// ---------------------------------------------------------------------------
namespace {
    const long ID_DISC         = wxNewId();
    const long ID_VALUE_SLIDER = wxNewId();
    const long ID_HUE          = wxNewId();
    const long ID_SAT          = wxNewId();
    const long ID_VAL          = wxNewId();
    const long ID_RED          = wxNewId();
    const long ID_GRN          = wxNewId();
    const long ID_BLU          = wxNewId();
    const long ID_HEX          = wxNewId();
    const long ID_RANDOMIZE      = wxNewId();
    const long ID_SYSTEM_PICKER  = wxNewId();
    const long ID_RESET          = wxNewId();
    const long ID_OK_BTN       = wxNewId();
    const long ID_CANCEL_BTN   = wxNewId();
}

BEGIN_EVENT_TABLE(xlColourPickerDialog, wxDialog)
    // Use wxID_ANY: the canvas fires these events with no specific ID set.
    EVT_COMMAND(wxID_ANY, EVT_CP_PALETTE_CHANGED, xlColourPickerDialog::OnDiscChanged)
    EVT_COMMAND(wxID_ANY, EVT_CP_SLIDER_CHANGED,  xlColourPickerDialog::OnSliderChanged)
    EVT_SPINCTRL(ID_HUE, xlColourPickerDialog::OnHSVSpin)
    EVT_SPINCTRL(ID_SAT, xlColourPickerDialog::OnHSVSpin)
    EVT_SPINCTRL(ID_VAL, xlColourPickerDialog::OnHSVSpin)
    EVT_SPINCTRL(ID_RED, xlColourPickerDialog::OnRGBSpin)
    EVT_SPINCTRL(ID_GRN, xlColourPickerDialog::OnRGBSpin)
    EVT_SPINCTRL(ID_BLU, xlColourPickerDialog::OnRGBSpin)
    EVT_TEXT_ENTER(ID_HEX,   xlColourPickerDialog::OnHexText)
    EVT_BUTTON(ID_RANDOMIZE,     xlColourPickerDialog::OnRandomize)
    EVT_BUTTON(ID_SYSTEM_PICKER, xlColourPickerDialog::OnSystemPicker)
    EVT_BUTTON(ID_RESET,         xlColourPickerDialog::OnReset)
    EVT_BUTTON(ID_OK_BTN,     xlColourPickerDialog::OnOK)
    EVT_BUTTON(ID_CANCEL_BTN, xlColourPickerDialog::OnCancel)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------
xlColourPickerDialog::xlColourPickerDialog(wxWindow* parent, const wxColour& initial,
                                           wxWindowID id, const wxString& title)
    : wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_initialColor(initial)
{
    m_color = wxColourToXlColor(initial);
    std::srand((unsigned)std::chrono::steady_clock::now().time_since_epoch().count());

    // ── Helpers ───────────────────────────────────────────────────────────
    auto lbl = [&](const wxString& s) {
        return new wxStaticText(this, wxID_ANY, s);
    };
    // Fixed-width spin: 4 digits visible, scaled to 150% of that base width
    auto spin = [&](long sid, int lo, int hi, int val) {
        auto* s = new wxSpinCtrl(this, sid, wxEmptyString, wxDefaultPosition,
                                 wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, lo, hi, val);
        int base = s->GetTextExtent("8888").GetWidth() + 30;
        s->SetMinSize(wxSize(base * 3 / 2, -1));
        return s;
    };

    // ── HSV disc — expands to fill available space ─────────────────────────
    m_disc = new xlColorCanvas(this, ID_DISC, wxDefaultPosition, wxDefaultSize,
                               wxBORDER_SUNKEN | wxTAB_TRAVERSAL);
    m_disc->SetType(xlColorCanvas::DisplayType::TYPE_WHEEL);
    m_disc->SetMinSize(wxSize(160, 160));

    // ── Value / brightness slider — fixed width, stretches tall with disc ──
    m_valueSlider = new xlColorCanvas(this, ID_VALUE_SLIDER, wxDefaultPosition, wxDefaultSize,
                                      wxBORDER_SUNKEN | wxTAB_TRAVERSAL);
    m_valueSlider->SetType(xlColorCanvas::DisplayType::TYPE_SLIDER);
    m_valueSlider->SetMode(MODE_BRIGHTNESS);
    m_valueSlider->SetMinSize(wxSize(25, 160));

    auto* sliderCol = new wxBoxSizer(wxVERTICAL);
    sliderCol->Add(lbl("Value:"), 0, wxBOTTOM | wxALIGN_CENTER_HORIZONTAL, 2);
    sliderCol->Add(m_valueSlider, 1, wxEXPAND);

    // ── Right column ──────────────────────────────────────────────────────
    m_hueSpin = spin(ID_HUE, 0, 360, 0);
    m_satSpin = spin(ID_SAT, 0, 255, 255);
    m_valSpin = spin(ID_VAL, 0, 255, 255);
    m_redSpin = spin(ID_RED, 0, 255, 255);
    m_grnSpin = spin(ID_GRN, 0, 255, 0);
    m_bluSpin = spin(ID_BLU, 0, 255, 0);

    // 3-row layout: HSV on left two columns, RGB on right two columns
    auto* spinGrid = new wxFlexGridSizer(3, 4, 3, 6);
    spinGrid->Add(lbl("H:"), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
    spinGrid->Add(m_hueSpin);
    spinGrid->Add(lbl("R:"), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxLEFT, 8);
    spinGrid->Add(m_redSpin);
    spinGrid->Add(lbl("S:"), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
    spinGrid->Add(m_satSpin);
    spinGrid->Add(lbl("G:"), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxLEFT, 8);
    spinGrid->Add(m_grnSpin);
    spinGrid->Add(lbl("V:"), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
    spinGrid->Add(m_valSpin);
    spinGrid->Add(lbl("B:"), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxLEFT, 8);
    spinGrid->Add(m_bluSpin);

    // Hex field — textctrl expands to fill the full width of the right column
    m_hexField = new wxTextCtrl(this, ID_HEX, "#FF0000", wxDefaultPosition,
                                wxDefaultSize, wxTE_PROCESS_ENTER);
    auto* hexRow = new wxBoxSizer(wxHORIZONTAL);
    hexRow->Add(lbl("Hex:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    hexRow->Add(m_hexField,  1, wxALIGN_CENTER_VERTICAL);

    // Old / New preview panels — side by side
    m_beforePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(44, 32), wxBORDER_SUNKEN);
    m_afterPanel  = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(44, 32), wxBORDER_SUNKEN);
    m_beforePanel->SetBackgroundColour(initial);
    m_afterPanel->SetBackgroundColour(initial);

    auto* previewGrid = new wxBoxSizer(wxHORIZONTAL);
    previewGrid->Add(lbl("Old:"),     0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 3);
    previewGrid->Add(m_beforePanel,   1, wxEXPAND | wxRIGHT, 8);
    previewGrid->Add(lbl("New:"),     0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 3);
    previewGrid->Add(m_afterPanel,    1, wxEXPAND);

    // Assemble right column
    auto* rightCol = new wxBoxSizer(wxVERTICAL);
    rightCol->Add(spinGrid,    0, wxBOTTOM, 6);
    rightCol->Add(new wxStaticLine(this), 0, wxEXPAND | wxBOTTOM, 4);
    rightCol->Add(hexRow,      0, wxEXPAND | wxBOTTOM, 6);
    rightCol->Add(new wxStaticLine(this), 0, wxEXPAND | wxBOTTOM, 4);
    rightCol->Add(previewGrid, 0, wxBOTTOM, 6);
    rightCol->Add(new wxStaticLine(this), 0, wxEXPAND | wxBOTTOM, 6);
    rightCol->Add(new wxButton(this, ID_RANDOMIZE, "Randomize"), 0, wxEXPAND);

    // ── Top section: disc expands, slider + right col are fixed ──────────
    auto* topRow = new wxBoxSizer(wxHORIZONTAL);
    topRow->Add(m_disc,     1, wxEXPAND | wxRIGHT, 6);   // proportion=1 → expands
    topRow->Add(sliderCol,  0, wxEXPAND | wxRIGHT, 10);
    topRow->Add(rightCol,   0, wxALIGN_TOP);

    // Helper — creates a bordered colour swatch panel
    auto makeSwatchPanel = [&](const wxColour& color) {
        auto* p = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(SWATCH_W, SWATCH_H),
                              wxBORDER_NONE | wxFULL_REPAINT_ON_RESIZE);
        p->SetBackgroundColour(color);
        p->SetBackgroundStyle(wxBG_STYLE_PAINT);
        p->SetCursor(wxCURSOR_HAND);
        p->Bind(wxEVT_PAINT, [p](wxPaintEvent&) {
            wxPaintDC dc(p);
            wxSize sz = p->GetSize();
            dc.SetBackground(wxBrush(p->GetBackgroundColour()));
            dc.Clear();
            dc.SetPen(*wxBLACK_PEN);
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());
        });
        return p;
    };

    // ── CSS named color swatches — bordered panels, zero gaps
    auto* cssGrid = new wxGridSizer(0, CSS_COLS, 0, 0);
    m_commonSwatches.resize(CSS_COUNT);
    for (int i = 0; i < CSS_COUNT; ++i) {
        const auto& c = CSS_COLORS[i];
        auto* p = makeSwatchPanel(wxColour(c.r, c.g, c.b));
        p->SetToolTip(wxString::Format("%s  #%02X%02X%02X", c.name, c.r, c.g, c.b));
        m_commonSwatches[i] = p;
        cssGrid->Add(p, 0, wxEXPAND);
        p->Bind(wxEVT_LEFT_DOWN, [this, i](wxMouseEvent&) {
            const auto& cc = CSS_COLORS[i];
            SetColorAndSync(xlColor(cc.r, cc.g, cc.b));
        });
    }

    // ── Recent swatches — same CSS_COLS-wide grid so cells match common swatch size
    auto* recentGrid = new wxGridSizer(1, CSS_COLS, 0, 0);
    m_recentSwatches.resize(RECENT_COUNT);
    for (int i = 0; i < RECENT_COUNT; ++i) {
        auto* p = makeSwatchPanel(wxColour(64, 64, 64));
        m_recentSwatches[i] = p;
        recentGrid->Add(p, 0, wxEXPAND);
        p->Bind(wxEVT_LEFT_DOWN, [this, i](wxMouseEvent&) {
            wxColour wc = xlColourData::INSTANCE.GetCustomColour(i);
            if (wc.IsOk())
                SetColorAndSync(wxColourToXlColor(wc));
        });
    }
    // pad remaining columns so the grid row is full width
    for (int i = RECENT_COUNT; i < CSS_COLS; ++i)
        recentGrid->Add(SWATCH_W, SWATCH_H);
    LoadRecentSwatches();

    // ── Bottom bar ────────────────────────────────────────────────────────
    auto* resetBtn  = new wxButton(this, ID_RESET,         "Reset to Default");
    auto* sysBtn    = new wxButton(this, ID_SYSTEM_PICKER, "System Picker...");
    auto* okBtn     = new wxButton(this, ID_OK_BTN,        "OK");
    auto* cancelBtn = new wxButton(this, ID_CANCEL_BTN,    "Cancel");
    okBtn->SetDefault();

    auto* bottomBar = new wxBoxSizer(wxHORIZONTAL);
    bottomBar->Add(resetBtn, 0, wxALIGN_CENTER_VERTICAL);
    bottomBar->Add(sysBtn,   0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
    bottomBar->AddStretchSpacer(1);
    bottomBar->Add(okBtn,     0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
    bottomBar->Add(cancelBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);

    // ── Main layout ───────────────────────────────────────────────────────
    // proportion=1 on topRow and cssGrid lets them share extra vertical space
    auto* main = new wxBoxSizer(wxVERTICAL);
    main->Add(topRow,      1, wxEXPAND | wxALL, 8);
    main->Add(lbl("Common Colors:"), 0, wxLEFT | wxRIGHT, 8);
    main->Add(cssGrid,     1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
    main->Add(lbl("Recent Colors:"), 0, wxLEFT | wxRIGHT, 8);
    main->Add(recentGrid,  0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
    main->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 6);
    main->Add(bottomBar,   0, wxEXPAND | wxALL, 8);
    SetSizer(main);
    main->Fit(this);
    SetMinSize(GetSize());
    SetEscapeId(ID_CANCEL_BTN);

    SyncAllFromColor(true);
}

// ---------------------------------------------------------------------------
wxColour xlColourPickerDialog::GetColour() const
{
    return xlColorToWxColour(m_color);
}

// ---------------------------------------------------------------------------
void xlColourPickerDialog::SyncAllFromColor(bool syncDisc)
{
    if (m_updating) return;
    m_updating = true;
    HSVValue hsv;
    m_color.toHSV(hsv);
    if (syncDisc) {
        m_disc->SetRGB(m_color);
        m_valueSlider->SetHSV(hsv);
    }
    m_hueSpin->SetValue((int)(hsv.hue * 360.0 + 0.5));
    m_satSpin->SetValue((int)(hsv.saturation * 255.0 + 0.5));
    m_valSpin->SetValue((int)(hsv.value * 255.0 + 0.5));
    m_redSpin->SetValue((int)m_color.red);
    m_grnSpin->SetValue((int)m_color.green);
    m_bluSpin->SetValue((int)m_color.blue);
    m_hexField->ChangeValue(wxString::Format("#%02X%02X%02X",
                                              m_color.red, m_color.green, m_color.blue));
    UpdateAfterPanel();
    m_updating = false;
}

void xlColourPickerDialog::UpdateAfterPanel()
{
    m_afterPanel->SetBackgroundColour(xlColorToWxColour(m_color));
    m_afterPanel->Refresh();
}

void xlColourPickerDialog::SetColorAndSync(const xlColor& c)
{
    m_color = c;
    SyncAllFromColor(true);
}

// ---------------------------------------------------------------------------
void xlColourPickerDialog::LoadRecentSwatches()
{
    for (int i = 0; i < RECENT_COUNT; ++i) {
        wxColour wc = xlColourData::INSTANCE.GetCustomColour(i);
        xlColor  c  = wc.IsOk() ? wxColourToXlColor(wc) : xlColor(64, 64, 64);
        m_recentSwatches[i]->SetBackgroundColour(xlColorToWxColour(c));
        m_recentSwatches[i]->SetToolTip(wxString::Format("#%02X%02X%02X", c.red, c.green, c.blue));
        m_recentSwatches[i]->Refresh();   // EVT_PAINT handler redraws with new colour + border
    }
}

void xlColourPickerDialog::PushRecentColor(const xlColor& c)
{
    for (int i = xlColourData::INSTANCE.GetNumCustomColours() - 1; i > 0; --i) {
        xlColourData::INSTANCE.SetCustomColour(i, xlColourData::INSTANCE.GetCustomColour(i - 1));
    }
    xlColourData::INSTANCE.SetCustomColour(0, xlColorToWxColour(c));
    LoadRecentSwatches();
}

// ---------------------------------------------------------------------------
// Event handlers
// ---------------------------------------------------------------------------
void xlColourPickerDialog::OnDiscChanged(wxCommandEvent& /*event*/)
{
    if (m_updating) return;
    m_updating = true;
    m_color = m_disc->GetRGB();
    HSVValue hsv = m_disc->GetHSV();
    m_valueSlider->SetHSV(hsv);
    m_hueSpin->SetValue((int)(hsv.hue * 360.0 + 0.5));
    m_satSpin->SetValue((int)(hsv.saturation * 255.0 + 0.5));
    m_valSpin->SetValue((int)(hsv.value * 255.0 + 0.5));
    m_redSpin->SetValue((int)m_color.red);
    m_grnSpin->SetValue((int)m_color.green);
    m_bluSpin->SetValue((int)m_color.blue);
    m_hexField->ChangeValue(wxString::Format("#%02X%02X%02X",
                                              m_color.red, m_color.green, m_color.blue));
    UpdateAfterPanel();
    m_updating = false;
}

void xlColourPickerDialog::OnSliderChanged(wxCommandEvent& /*event*/)
{
    if (m_updating) return;
    m_updating = true;
    HSVValue discHsv = m_disc->GetHSV();
    discHsv.value    = m_valueSlider->GetHSV().value;
    m_disc->SetHSV(discHsv);
    m_color = discHsv;
    m_hueSpin->SetValue((int)(discHsv.hue * 360.0 + 0.5));
    m_satSpin->SetValue((int)(discHsv.saturation * 255.0 + 0.5));
    m_valSpin->SetValue((int)(discHsv.value * 255.0 + 0.5));
    m_redSpin->SetValue((int)m_color.red);
    m_grnSpin->SetValue((int)m_color.green);
    m_bluSpin->SetValue((int)m_color.blue);
    m_hexField->ChangeValue(wxString::Format("#%02X%02X%02X",
                                              m_color.red, m_color.green, m_color.blue));
    UpdateAfterPanel();
    m_updating = false;
}

void xlColourPickerDialog::OnHSVSpin(wxSpinEvent& /*event*/)
{
    if (m_updating) return;
    HSVValue hsv;
    hsv.hue        = m_hueSpin->GetValue() / 360.0;
    hsv.saturation = m_satSpin->GetValue() / 255.0;
    hsv.value      = m_valSpin->GetValue() / 255.0;
    m_color.fromHSV(hsv);
    m_updating = true;
    m_disc->SetRGB(m_color);
    m_valueSlider->SetHSV(hsv);
    m_redSpin->SetValue((int)m_color.red);
    m_grnSpin->SetValue((int)m_color.green);
    m_bluSpin->SetValue((int)m_color.blue);
    m_hexField->ChangeValue(wxString::Format("#%02X%02X%02X",
                                              m_color.red, m_color.green, m_color.blue));
    UpdateAfterPanel();
    m_updating = false;
}

void xlColourPickerDialog::OnRGBSpin(wxSpinEvent& /*event*/)
{
    if (m_updating) return;
    m_color.Set((uint8_t)m_redSpin->GetValue(),
                (uint8_t)m_grnSpin->GetValue(),
                (uint8_t)m_bluSpin->GetValue());
    m_updating = true;
    HSVValue hsv;
    m_color.toHSV(hsv);
    m_disc->SetRGB(m_color);
    m_valueSlider->SetHSV(hsv);
    m_hueSpin->SetValue((int)(hsv.hue * 360.0 + 0.5));
    m_satSpin->SetValue((int)(hsv.saturation * 255.0 + 0.5));
    m_valSpin->SetValue((int)(hsv.value * 255.0 + 0.5));
    m_hexField->ChangeValue(wxString::Format("#%02X%02X%02X",
                                              m_color.red, m_color.green, m_color.blue));
    UpdateAfterPanel();
    m_updating = false;
}

void xlColourPickerDialog::OnHexText(wxCommandEvent& /*event*/)
{
    if (m_updating) return;
    wxString hex = m_hexField->GetValue().Trim();
    if (hex.StartsWith("#")) hex = hex.Mid(1);
    if (hex.Length() < 6) return;
    char* end;
    long rv = std::strtol(hex.Left(2).c_str(), &end, 16);
    long gv = std::strtol(hex.Mid(2,2).c_str(), &end, 16);
    long bv = std::strtol(hex.Mid(4,2).c_str(), &end, 16);
    m_color.Set((uint8_t)rv, (uint8_t)gv, (uint8_t)bv);
    SyncAllFromColor(true);
}


void xlColourPickerDialog::OnRandomize(wxCommandEvent& /*event*/)
{
    HSVValue hsv;
    hsv.hue        = (std::rand() % 360) / 360.0;
    hsv.saturation = 0.70 + (std::rand() % 30) / 100.0;
    hsv.value      = 0.80 + (std::rand() % 20) / 100.0;
    xlColor c;
    c.fromHSV(hsv);
    SetColorAndSync(c);
}

void xlColourPickerDialog::OnReset(wxCommandEvent& /*event*/)
{
    SetColorAndSync(wxColourToXlColor(m_initialColor));
}

void xlColourPickerDialog::OnSystemPicker(wxCommandEvent& /*event*/)
{
    wxColourData data;
    data.SetColour(xlColorToWxColour(m_color));
    wxColourDialog dlg(this, &data);
    if (dlg.ShowModal() == wxID_OK) {
        SetColorAndSync(wxColourToXlColor(dlg.GetColourData().GetColour()));
    }
}

void xlColourPickerDialog::OnOK(wxCommandEvent& /*event*/)
{
    PushRecentColor(m_color);
    EndModal(wxID_OK);
}

void xlColourPickerDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}
