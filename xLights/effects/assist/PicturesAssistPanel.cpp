/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(PicturesAssistPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/artprov.h>
#include <wx/odcombo.h>
#include <wx/regex.h>
#include <wx/stdpaths.h>
#include <wx/txtstrm.h>

#include "PicturesAssistPanel.h"
#include "../../xlColorPicker.h"
#include "../../xlColorPickerFields.h"
#include "../../../include/PaintToolIcons.h"
#include "../../models/Model.h"
#include "../../xLightsMain.h"
#include "../../xLightsXmlFile.h"
#include "ExternalHooks.h"
#include "ColorPanel.h"

#include "./utils/spdlog_macros.h"

#define ZERO 0
#define PALETTE_SIZE 8

//(*IdInit(PicturesAssistPanel)
const wxWindowID PicturesAssistPanel::ID_BUTTON_NewImage = wxNewId();
const wxWindowID PicturesAssistPanel::ID_BUTTON_LoadImage = wxNewId();
const wxWindowID PicturesAssistPanel::ID_BUTTON_SaveImage = wxNewId();
const wxWindowID PicturesAssistPanel::ID_BUTTON_SaveAs = wxNewId();
const wxWindowID PicturesAssistPanel::ID_BUTTON1 = wxNewId();
const wxWindowID PicturesAssistPanel::ID_STATICTEXT_CurrentImage = wxNewId();
const wxWindowID PicturesAssistPanel::ID_STATICTEXT_ImageSize = wxNewId();
const wxWindowID PicturesAssistPanel::ID_STATICTEXT_ModelSize = wxNewId();
const wxWindowID PicturesAssistPanel::ID_PANEL_RightSide = wxNewId();
const wxWindowID PicturesAssistPanel::ID_SCROLLED_EffectAssist = wxNewId();
const wxWindowID PicturesAssistPanel::ID_PANEL1 = wxNewId();
//*)

#define SWATCH_WIDTH 11
class PAColourList : public wxOwnerDrawnComboBox
{
public:
    PAColourList(wxWindow* parent, wxWindowID id,
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

    virtual void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const
    {
        if (item == wxNOT_FOUND)
            return;

        wxString s = GetString(item);

        if (s == "(Load)") {
            if (rect.GetHeight() != 18) {
                wxFontMetrics fm = dc.GetFontMetrics();
                dc.SetTextForeground(*wxBLACK);
                dc.DrawText(s, rect.GetLeft() + 2, rect.GetTop() + (rect.GetHeight() - fm.height) / 2);
            }
            GetVListBoxComboPopup()->UnsetToolTip();
        } else {
            wxArrayString as = wxSplit(s, ',');

            int i = 0;

            for (auto it = as.begin(); it != as.end() && i < PALETTE_SIZE; ++it) {
                xlColor c;
                c.SetFromString(it->ToStdString());
                wxPen p(c.asWxColor());
                wxBrush b(c.asWxColor());
                dc.SetPen(p);
                dc.SetBrush(b);
                dc.DrawRectangle(i * SWATCH_WIDTH, rect.GetTop(), SWATCH_WIDTH - 1, rect.GetHeight() - 1);
                i++;
            }

            if (flags & wxODCB_PAINTING_SELECTED) {
                wxString file = as.back();
                GetVListBoxComboPopup()->SetToolTip(file);
            }
        }
    }
};


const long PicturesAssistPanel::ID_BITMAPBUTTON_Paint_Pencil = wxNewId();
const long PicturesAssistPanel::ID_BITMAPBUTTON_Paint_Eraser = wxNewId();
const long PicturesAssistPanel::ID_BITMAPBUTTON_Paint_Eyedropper = wxNewId();
const long PicturesAssistPanel::ID_BITMAPBUTTON_Paint_Selectcopy = wxNewId();
const long PicturesAssistPanel::ID_BITMAPBUTTON_COLOR_SWATCHES = wxNewId();
const long ID_BITMAPBUTTON_SavePalette = wxNewId();
const long ID_BITMAPBUTTON_DeletePalette = wxNewId();

wxDEFINE_EVENT(EVT_PAINT_COLOR, wxCommandEvent);
wxDEFINE_EVENT(EVT_IMAGE_FILE_SELECTED, wxCommandEvent);
wxDEFINE_EVENT(EVT_IMAGE_SIZE, wxCommandEvent);
wxDEFINE_EVENT(EVT_EYEDROPPER_COLOR, wxCommandEvent);

BEGIN_EVENT_TABLE(PicturesAssistPanel,wxPanel)
	//(*EventTable(PicturesAssistPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_PAINT_COLOR, PicturesAssistPanel::OnColorChange)
    EVT_COMMAND(wxID_ANY, EVT_IMAGE_FILE_SELECTED, PicturesAssistPanel::OnImageFileSelected)
    EVT_COMMAND(wxID_ANY, EVT_IMAGE_SIZE, PicturesAssistPanel::OnImageSize)
    EVT_COMMAND(wxID_ANY, EVT_EYEDROPPER_COLOR, PicturesAssistPanel::OnEyedropperColor)
END_EVENT_TABLE()

PicturesAssistPanel::PicturesAssistPanel(wxWindow* parent, wxWindowID id,const wxPoint& pos,const wxSize& size)
: mPaintMode(xlGridCanvasPictures::PAINT_PENCIL), mPaintColor(xlRED)
{
    paint_pencil = wxBITMAP_PNG_FROM_DATA(pencil);
    paint_pencil_selected = wxBITMAP_PNG_FROM_DATA(pencil_sel);
    paint_eraser = wxBITMAP_PNG_FROM_DATA(eraser);
    paint_eraser_selected = wxBITMAP_PNG_FROM_DATA(eraser_sel);
    paint_eyedropper = wxBITMAP_PNG_FROM_DATA(eyedropper);
    paint_eyedropper_selected = wxBITMAP_PNG_FROM_DATA(eyedropper_sel);
    paint_selectcopy = wxBITMAP_PNG_FROM_DATA(selectcopy);
    paint_selectcopy_selected = wxBITMAP_PNG_FROM_DATA(selectcopy_sel);

	//(*Initialize(PicturesAssistPanel)
	wxFlexGridSizer* ColorPickerSizer;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* PaintFuntionsSizer;
	wxFlexGridSizer* PaintToolsSizer;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxWANTS_CHARS, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	Panel_Sizer = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxWANTS_CHARS, _T("ID_PANEL1"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	ScrolledWindowEffectAssist = new wxScrolledWindow(Panel_Sizer, ID_SCROLLED_EffectAssist, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS|wxVSCROLL|wxHSCROLL, _T("ID_SCROLLED_EffectAssist"));
	FlexGridSizer_Container = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer_Container->AddGrowableCol(0);
	FlexGridSizer_Container->AddGrowableRow(0);
	Panel_RightSide = new wxPanel(ScrolledWindowEffectAssist, ID_PANEL_RightSide, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_RightSide"));
	FlexGridSizer_RightSide = new wxFlexGridSizer(0, 1, 0, 0);
	ColorPickerSizer = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer_RightSide->Add(ColorPickerSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	PaintToolsSizer = new wxFlexGridSizer(0, 6, 0, 10);
	FlexGridSizer_RightSide->Add(PaintToolsSizer, 1, wxALL, 0);
	PaintFuntionsSizer = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 6, 0, 0);
	Button_NewImage = new wxButton(Panel_RightSide, ID_BUTTON_NewImage, _("New\nImage"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_NewImage"));
	FlexGridSizer3->Add(Button_NewImage, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Button_LoadImage = new wxButton(Panel_RightSide, ID_BUTTON_LoadImage, _("Load\nImage"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LoadImage"));
	FlexGridSizer3->Add(Button_LoadImage, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Button_SaveImage = new wxButton(Panel_RightSide, ID_BUTTON_SaveImage, _("Save\nImage"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SaveImage"));
	FlexGridSizer3->Add(Button_SaveImage, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Button_SaveAs = new wxButton(Panel_RightSide, ID_BUTTON_SaveAs, _("Save As\nImage"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SaveAs"));
	FlexGridSizer3->Add(Button_SaveAs, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Resize = new wxButton(Panel_RightSide, ID_BUTTON1, _("Resize\nImage"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button_Resize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	PaintFuntionsSizer->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText_CurrentImage = new wxStaticText(Panel_RightSide, ID_STATICTEXT_CurrentImage, _("Current Image:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_CurrentImage"));
	PaintFuntionsSizer->Add(StaticText_CurrentImage, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_ImageSize = new wxStaticText(Panel_RightSide, ID_STATICTEXT_ImageSize, _("Image Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ImageSize"));
	PaintFuntionsSizer->Add(StaticText_ImageSize, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_ModelSize = new wxStaticText(Panel_RightSide, ID_STATICTEXT_ModelSize, _("Model Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ModelSize"));
	PaintFuntionsSizer->Add(StaticText_ModelSize, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer_RightSide->Add(PaintFuntionsSizer, 1, wxALL, 0);
	Panel_RightSide->SetSizer(FlexGridSizer_RightSide);
	FlexGridSizer_Container->Add(Panel_RightSide, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	ScrolledWindowEffectAssist->SetSizer(FlexGridSizer_Container);
	FlexGridSizer2->Add(ScrolledWindowEffectAssist, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	Panel_Sizer->SetSizer(FlexGridSizer2);
	FlexGridSizer1->Add(Panel_Sizer, 1, wxALL, 0);
	SetSizer(FlexGridSizer1);

	Connect(ID_BUTTON_NewImage, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&PicturesAssistPanel::OnButton_NewImageClick);
	Connect(ID_BUTTON_LoadImage, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&PicturesAssistPanel::OnButton_LoadImageClick);
	Connect(ID_BUTTON_SaveImage, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&PicturesAssistPanel::OnButton_SaveImageClick);
	Connect(ID_BUTTON_SaveAs, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&PicturesAssistPanel::OnButton_SaveAsClick);
	Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&PicturesAssistPanel::OnButton_ResizeClick);
	//*)

    CreatePaintTools(Panel_RightSide, PaintToolsSizer);
    Connect(wxID_ANY, wxEVT_COMBOBOX, (wxObjectEventFunction)&PicturesAssistPanel::OnColourChoiceSelect, 0, this);

    mColorPicker = new xlColorPickerFields(Panel_RightSide, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_ColorPicker"));
	ColorPickerSizer->Add(mColorPicker, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);

	FlexGridSizer_RightSide->Fit(Panel_RightSide);
	FlexGridSizer_RightSide->SetSizeHints(Panel_RightSide);
	FlexGridSizer_Container->Fit(ScrolledWindowEffectAssist);
	FlexGridSizer_Container->SetSizeHints(ScrolledWindowEffectAssist);
	FlexGridSizer2->Fit(Panel_Sizer);
	FlexGridSizer2->SetSizeHints(Panel_Sizer);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

    for (int i=wxEVT_SCROLLWIN_TOP; i<=wxEVT_SCROLLWIN_THUMBRELEASE; i++) {
        ScrolledWindowEffectAssist->Connect(wxID_ANY, i, wxScrollWinEventHandler(PicturesAssistPanel::OnWindowScrolled), NULL, this);
    }
    SetDefaultPalette();
}

void PicturesAssistPanel::LoadPalettes(wxDir& directory, bool subdirs)
{
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
                                                    wxAtoi(comp[0]),
                                                    wxAtoi(comp[1]),
                                                    wxAtoi(comp[2]));
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
    for (auto& filename : files) {
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
}

void PicturesAssistPanel::LoadAllPalettes()
{
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

    if (BitmapButton_ColourChoice->GetCount() != 0) {
        BitmapButton_ColourChoice->Clear();
    }
    BitmapButton_ColourChoice->AppendString("(Load)");
    for (auto it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it) {
        BitmapButton_ColourChoice->AppendString(*it);
    }
}

void PicturesAssistPanel::SetSwatchColor(const std::string& cstr, const int i)
{
    xlColor c;
    c.SetFromString(cstr);
    mColorPicker->SetButtonColor(i, c);
    mColorPicker->SetColor(c);
    ValidateWindow();
}

void PicturesAssistPanel::SetDefaultPalette()
{
    SetSwatchColor("#FFFFFF", 1);
    SetSwatchColor("#FF0000", 2);
    SetSwatchColor("#00FF00", 3);
    SetSwatchColor("#0000FF", 4);
    SetSwatchColor("#FFFF00", 5);
    SetSwatchColor("#000000", 6);
    SetSwatchColor("#00FFFF", 7);
    SetSwatchColor("#FF00FF", 8);

    mColorPicker->ResetPanel();

    xlColor c;
    c.SetFromString(wxString("#FFFFFF"));
    mColorPicker->SetButtonColor(1, c);
    mColorPicker->SetColor(c);
    if (BitmapButton_ColourChoice->GetCount() < 2) {
        LoadAllPalettes();
    }
    ValidateWindow();
}

void PicturesAssistPanel::OnColourChoiceDropDown(wxCommandEvent& WXUNUSED(event))
{
    if (_lastShowDir != xLightsFrame::CurrentDir) {
        _lastShowDir = xLightsFrame::CurrentDir;
        LoadAllPalettes();
        ValidateWindow();
    }
}

void PicturesAssistPanel::OnColourChoiceSelect(wxCommandEvent& event)
{
    long sel = event.GetInt();
    wxString s = BitmapButton_ColourChoice->GetString(sel);
    int activeButton = mColorPicker->GetActiveButton();
    if (s != "(Load)") {
        wxArrayString as = wxSplit(s, ',');
        for (size_t i = 0; i < std::min(as.size(), (size_t)PALETTE_SIZE); i++) {
            xlColor c;
            c.SetFromString(as[i].ToStdString());
            mColorPicker->SetButtonColor(i+1, c);
            if (activeButton == i+1) {
                mColorPicker->SetColor(c);
            }            
        }
    }
    BitmapButton_ColourChoice->SetSelection(0); // set dropdown back to 0
    ValidateWindow();
}

void PicturesAssistPanel::ValidateWindow()
{
    // only enable save if this palette was not loaded from disk or has been saved to disk
    wxString pal = wxString(GetCurrentPalette()).BeforeLast(',');
    for (auto it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it) {
        wxString ss(it->c_str());
        if (ss.BeforeLast(',') == pal) {
            BitmapButton_SavePalette->Disable();
            if (FindPaletteFile(ss.AfterLast(','), pal + ",") != "") {
                BitmapButton_DeletePalette->Enable();
            } else {
                BitmapButton_DeletePalette->Disable();
            }
            return;
        }
    }
    BitmapButton_SavePalette->Enable();
    BitmapButton_DeletePalette->Disable();
}

PicturesAssistPanel::~PicturesAssistPanel()
{
	//(*Destroy(PicturesAssistPanel)
	//*)
}

void PicturesAssistPanel::ForceRefresh()
{
    ScrolledWindowEffectAssist->Refresh();
}

void PicturesAssistPanel::CreatePaintTools(wxWindow* parent, wxFlexGridSizer* container)
{
    BitmapButton_Paint_Pencil = new FlickerFreeBitmapButton(parent, ID_BITMAPBUTTON_Paint_Pencil, paint_pencil_selected, wxDefaultPosition, wxSize(64,64), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Paint_Pencil"));
    BitmapButton_Paint_Pencil->SetBitmapSelected(paint_pencil_selected);
    BitmapButton_Paint_Pencil->SetBitmapFocus(paint_pencil_selected);
    BitmapButton_Paint_Pencil->SetBitmapCurrent(paint_pencil_selected);
    container->Add(BitmapButton_Paint_Pencil, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Paint_Eraser = new FlickerFreeBitmapButton(parent, ID_BITMAPBUTTON_Paint_Eraser, paint_eraser, wxDefaultPosition, wxSize(64,64), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Paint_Eraser"));
    BitmapButton_Paint_Eraser->SetBitmapSelected(paint_eraser_selected);
    BitmapButton_Paint_Eraser->SetBitmapFocus(paint_eraser_selected);
    BitmapButton_Paint_Eraser->SetBitmapCurrent(paint_eraser_selected);
    container->Add(BitmapButton_Paint_Eraser, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Paint_Eyedropper = new FlickerFreeBitmapButton(parent, ID_BITMAPBUTTON_Paint_Eyedropper, paint_eyedropper, wxDefaultPosition, wxSize(64,64), wxBU_AUTODRAW, wxDefaultValidator, _T("BitmapButton_Paint_Eyedropper"));
    BitmapButton_Paint_Eyedropper->SetBitmapSelected(paint_eyedropper_selected);
    BitmapButton_Paint_Eyedropper->SetBitmapFocus(paint_eyedropper_selected);
    BitmapButton_Paint_Eyedropper->SetBitmapCurrent(paint_eyedropper_selected);
    container->Add(BitmapButton_Paint_Eyedropper, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Paint_Selectcopy = new FlickerFreeBitmapButton(parent, ID_BITMAPBUTTON_Paint_Selectcopy, paint_selectcopy, wxDefaultPosition, wxSize(64,64), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Paint_Selectcopy"));
    BitmapButton_Paint_Selectcopy->SetBitmapSelected(paint_selectcopy_selected);
    BitmapButton_Paint_Selectcopy->SetBitmapFocus(paint_selectcopy_selected);
    BitmapButton_Paint_Selectcopy->SetBitmapCurrent(paint_selectcopy_selected);
    container->Add(BitmapButton_Paint_Selectcopy, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

	BitmapButton_ColourChoice = new PAColourList(Panel_RightSide, ID_BITMAPBUTTON_COLOR_SWATCHES, wxDefaultPosition, wxDefaultSize, ZERO, wxDefaultValidator, _T("ID_BITMAPBUTTON_COLOR_SWATCHES"));
    container->Add(BitmapButton_ColourChoice, 1, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);

    FlexGridSizer_Palette = new wxFlexGridSizer(0, 1, 0, 0);
    BoxSizer_PaletteButtons = new wxBoxSizer(wxVERTICAL);
    BitmapButton_SavePalette = new xlSizedBitmapButton(Panel_RightSide, ID_BITMAPBUTTON_SavePalette, wxArtProvider::GetBitmapBundle("xlART_colorpanel_save_xpm", wxART_BUTTON), wxDefaultPosition, wxSize(24, 24), wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SavePalette"));
    BoxSizer_PaletteButtons->Add(BitmapButton_SavePalette, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_DeletePalette = new xlSizedBitmapButton(Panel_RightSide, ID_BITMAPBUTTON_DeletePalette, wxArtProvider::GetBitmapBundle("xlART_colorpanel_delete_xpm", wxART_BUTTON), wxDefaultPosition, wxSize(24, 24), wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_DeletePalette"));
    BoxSizer_PaletteButtons->Add(BitmapButton_DeletePalette, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer_Palette->Add(BoxSizer_PaletteButtons, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    container->Add(FlexGridSizer_Palette, 1, wxALL | wxALIGN_LEFT, 5);

    Connect(ID_BITMAPBUTTON_Paint_Pencil,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PicturesAssistPanel::OnBitmapButton_Paint_PencilClick);
    Connect(ID_BITMAPBUTTON_Paint_Eraser,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PicturesAssistPanel::OnBitmapButton_Paint_EraserClick);
    Connect(ID_BITMAPBUTTON_Paint_Eyedropper,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PicturesAssistPanel::OnBitmapButton_Paint_EyedropperClick);
    Connect(ID_BITMAPBUTTON_Paint_Selectcopy,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PicturesAssistPanel::OnBitmapButton_Paint_SelectcopyClick);
    Connect(ID_BITMAPBUTTON_SavePalette, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&PicturesAssistPanel::OnBitmapButton_SavePaletteClick);
    Connect(ID_BITMAPBUTTON_DeletePalette, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&PicturesAssistPanel::OnBitmapButton_DeletePaletteClick);
}

void PicturesAssistPanel::OnBitmapButton_Paint_PencilClick(wxCommandEvent& event)
{
    BitmapButton_Paint_Pencil->SetBitmap(paint_pencil_selected);
    BitmapButton_Paint_Eraser->SetBitmap(paint_eraser);
    BitmapButton_Paint_Eyedropper->SetBitmap(paint_eyedropper);
    BitmapButton_Paint_Selectcopy->SetBitmap(paint_selectcopy);
    mGridCanvas->SetPaintMode(xlGridCanvasPictures::PAINT_PENCIL);
}

void PicturesAssistPanel::OnBitmapButton_Paint_EraserClick(wxCommandEvent& event)
{
    BitmapButton_Paint_Pencil->SetBitmap(paint_pencil);
    BitmapButton_Paint_Eraser->SetBitmap(paint_eraser_selected);
    BitmapButton_Paint_Eyedropper->SetBitmap(paint_eyedropper);
    BitmapButton_Paint_Selectcopy->SetBitmap(paint_selectcopy);
    mGridCanvas->SetPaintMode(xlGridCanvasPictures::PAINT_ERASER);
}

void PicturesAssistPanel::OnBitmapButton_Paint_EyedropperClick(wxCommandEvent& event)
{
    BitmapButton_Paint_Pencil->SetBitmap(paint_pencil);
    BitmapButton_Paint_Eraser->SetBitmap(paint_eraser);
    BitmapButton_Paint_Eyedropper->SetBitmap(paint_eyedropper_selected);
    BitmapButton_Paint_Selectcopy->SetBitmap(paint_selectcopy);
    mGridCanvas->SetPaintMode(xlGridCanvasPictures::PAINT_EYEDROPPER);
}

void PicturesAssistPanel::OnBitmapButton_Paint_SelectcopyClick(wxCommandEvent& event)
{
    BitmapButton_Paint_Pencil->SetBitmap(paint_pencil);
    BitmapButton_Paint_Eraser->SetBitmap(paint_eraser);
    BitmapButton_Paint_Eyedropper->SetBitmap(paint_eyedropper);
    BitmapButton_Paint_Selectcopy->SetBitmap(paint_selectcopy_selected);
    mGridCanvas->SetPaintMode(xlGridCanvasPictures::PAINT_SELECTCOPY);
}

void PicturesAssistPanel::OnColorChange(wxCommandEvent& event)
{
    xlColor* color = (xlColor*)event.GetClientData();
    mGridCanvas->SetPaintColor(*color);
    ValidateWindow();
}

void PicturesAssistPanel::OnEyedropperColor(wxCommandEvent& event)
{
    xlColor color(event.GetInt(), true);
    mGridCanvas->SetPaintColor(color);
    mColorPicker->SetColor(color);
}

void PicturesAssistPanel::OnImageFileSelected(wxCommandEvent& event)
{
   wxString* name = (wxString*)event.GetClientData();
   StaticText_CurrentImage->SetLabelText( "Current Image: " + *name );
}

void PicturesAssistPanel::OnImageSize(wxCommandEvent& event)
{
    wxString image_size = event.GetString();
    StaticText_ImageSize->SetLabelText( image_size );
    int bw, bh;
    mGridCanvas->GetModel()->GetBufferSize(mGridCanvas->GetEffect()->GetSettings().Get("T_CHOICE_BufferStyle", "Default"),
                                           mGridCanvas->GetEffect()->GetSettings().Get("T_CHOICE_PerPreviewCamera", "2D"),
                                           mGridCanvas->GetEffect()->GetSettings().Get("T_CHOICE_BufferTransform", "None"),
                                           bw, bh, mGridCanvas->GetEffect()->GetSettings().GetInt("B_SPINCTRL_BufferStagger", 0));
    StaticText_ModelSize->SetLabelText(wxString::Format("Model Size: %d x %d", bw, bh));
}

void PicturesAssistPanel::OnButton_SaveImageClick(wxCommandEvent& event)
{
    mGridCanvas->SaveImage();
}

void PicturesAssistPanel::OnButton_NewImageClick(wxCommandEvent& event)
{
    wxString image_dir = mxLightsParent->CurrentSeqXmlFile->GetImageDir(mxLightsParent);
    mGridCanvas->CreateNewImage(image_dir);
}

void PicturesAssistPanel::OnButton_LoadImageClick(wxCommandEvent& event)
{
    mGridCanvas->LoadImage();
}

void PicturesAssistPanel::OnButton_SaveAsClick(wxCommandEvent& event)
{
    mGridCanvas->SaveAsImage();
}

void PicturesAssistPanel::OnButton_ResizeClick(wxCommandEvent& event)
{
    mGridCanvas->ResizeImage();
}

void PicturesAssistPanel::OnWindowScrolled(wxScrollWinEvent &event)
{
    ForceRefresh();
}

void PicturesAssistPanel::OnBitmapButton_SavePaletteClick(wxCommandEvent& event)
{
    // Double check that this has not been saved before
    if (BitmapButton_ColourChoice->GetCount() == 1) {
        LoadAllPalettes();
        ValidateWindow();
        if (!BitmapButton_SavePalette->IsEnabled()) {
            
            LOG_ERROR("Already saved xpalette ... skipped.");
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
        
        LOG_ERROR("Unable to create file %s.", (const char*)fn.c_str());
    }

    LoadAllPalettes();

    ValidateWindow();
}

wxString PicturesAssistPanel::FindPaletteFile(const wxString& filename, const wxString& palette) const
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

wxColour PicturesAssistPanel::GetPaletteColor(int idx) const
{
    if (idx < PALETTE_SIZE)
        return mColorPicker->GetButtonColor(idx + 1);
    return *wxBLACK;
}

std::string PicturesAssistPanel::GetCurrentPalette() const
{
    std::string res;
    for (size_t i = 0; i < PALETTE_SIZE; i++) {
        wxColour c = GetPaletteColor(i);
        res += c.GetAsString(wxC2S_HTML_SYNTAX).ToStdString() + ",";
    }
    return res;
}

void PicturesAssistPanel::OnBitmapButton_DeletePaletteClick(wxCommandEvent& event)
{
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
