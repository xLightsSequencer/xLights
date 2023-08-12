/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ColorManagerSettingsPanel.h"

//(*InternalHeaders(ColorManagerSettingsPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/colourdata.h>
#include <wx/colordlg.h>

#include <wx/preferences.h>
#include "../xLightsMain.h"
#include "../sequencer/MainSequencer.h"
#include "../UtilFunctions.h"

//(*IdInit(ColorManagerSettingsPanel)
const long ColorManagerSettingsPanel::ID_CHECKBOX1 = wxNewId();
const long ColorManagerSettingsPanel::ID_BUTTON_IMPORT = wxNewId();
const long ColorManagerSettingsPanel::ID_BUTTON_EXPORT = wxNewId();
const long ColorManagerSettingsPanel::ID_BUTTON_RESET = wxNewId();
//*)

BEGIN_EVENT_TABLE(ColorManagerSettingsPanel,wxPanel)
	//(*EventTable(ColorManagerSettingsPanel)
	//*)
END_EVENT_TABLE()

ColorManagerSettingsPanel::ColorManagerSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id,const wxPoint& pos,const wxSize& size) : frame(f)
{
	//(*Initialize(ColorManagerSettingsPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer7;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer2;
	wxStaticBoxSizer* StaticBoxSizer3;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableCol(1);
	FlexGridSizer3->AddGrowableCol(2);
	FlexGridSizer3->AddGrowableRow(0);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Timing Tracks"));
	Sizer_Timing_Tracks = new wxFlexGridSizer(0, 2, 0, 0);
	StaticBoxSizer1->Add(Sizer_Timing_Tracks, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxVERTICAL, this, _("Effect Grid"));
	Sizer_Effect_Grid = new wxFlexGridSizer(0, 4, 0, 0);
	StaticBoxSizer2->Add(Sizer_Effect_Grid, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer3 = new wxStaticBoxSizer(wxVERTICAL, this, _("Layout Tab"));
	Sizer_Layout_Tab = new wxFlexGridSizer(0, 2, 0, 0);
	StaticBoxSizer3->Add(Sizer_Layout_Tab, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(StaticBoxSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	CheckBox_SuppressDarkMode = new wxCheckBox(this, ID_CHECKBOX1, _("Suppress Dark Mode"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_SuppressDarkMode->SetValue(false);
	FlexGridSizer2->Add(CheckBox_SuppressDarkMode, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 5, 0, 0);
	ButtonImport = new wxButton(this, ID_BUTTON_IMPORT, _("Import"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_IMPORT"));
	FlexGridSizer7->Add(ButtonImport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonExport = new wxButton(this, ID_BUTTON_EXPORT, _("Export"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_EXPORT"));
	FlexGridSizer7->Add(ButtonExport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Reset = new wxButton(this, ID_BUTTON_RESET, _("Reset Defaults"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_RESET"));
	FlexGridSizer7->Add(Button_Reset, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_IMPORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerSettingsPanel::OnButtonImportClick);
	Connect(ID_BUTTON_EXPORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerSettingsPanel::OnButtonExportClick);
	Connect(ID_BUTTON_RESET,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerSettingsPanel::OnButton_ResetClick);
	//*)

    #ifndef __WXMSW__
    FlexGridSizer2->Show(false);
    #endif

    #ifdef _MSC_VER
    MSWDisableComposited();
    #endif

    AddButtonsToDialog();
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    UpdateButtonColors();
}

ColorManagerSettingsPanel::~ColorManagerSettingsPanel()
{
	//(*Destroy(ColorManagerSettingsPanel)
	//*)
}

bool ColorManagerSettingsPanel::TransferDataToWindow() {
#ifdef __WXMSW__
    CheckBox_SuppressDarkMode->SetValue(IsSuppressDarkMode());
#endif
    return true;
}

bool ColorManagerSettingsPanel::TransferDataFromWindow() {
#ifdef __WXMSW__
    SetSuppressDarkMode(CheckBox_SuppressDarkMode->IsChecked());
#endif
    return true;
}

void ColorManagerSettingsPanel::UpdateButtonColors()
{
    for (size_t i = 0; i < ColorManager::NUM_COLORS; ++i) {
        wxString name = "ID_BITMAPBUTTON_" + frame->color_mgr.xLights_color[i].name;
        wxBitmapButton* btn = (wxBitmapButton*)FindWindowByName(name);
        if (btn != nullptr) {
            SetButtonColor(btn, frame->color_mgr.GetColor(frame->color_mgr.xLights_color[i].id));
        }
    }
}

void ColorManagerSettingsPanel::AddButtonsToDialog()
{
    for (size_t i = 0; i < ColorManager::NUM_COLORS; ++i) {
        wxString bitmapbutton_id_str = "ID_BITMAPBUTTON_" + frame->color_mgr.xLights_color[i].name;
        wxString static_text_id_str = "ID_STATICTEXT_" + frame->color_mgr.xLights_color[i].name;

        const long static_text_id = wxNewId();
        const long bitmapbutton_id = wxNewId();

        wxStaticText* txt = new wxStaticText(this, static_text_id, frame->color_mgr.xLights_color[i].display_name, wxDefaultPosition, wxDefaultSize, 0, static_text_id_str);
        wxBitmapButton* btn = new wxBitmapButton(this, bitmapbutton_id, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, bitmapbutton_id_str);

        Connect(bitmapbutton_id,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerSettingsPanel::ColorButtonSelected);

        switch (frame->color_mgr.xLights_color[i].category) {
        case ColorManager::ColorCategory::COLOR_CAT_TIMINGS:
            Sizer_Timing_Tracks->Add(txt, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
            Sizer_Timing_Tracks->Add(btn, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
            break;

        case ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID:
            Sizer_Effect_Grid->Add(txt, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
            Sizer_Effect_Grid->Add(btn, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
            break;

        case ColorManager::ColorCategory::COLOR_CAT_LAYOUT_TAB:
            Sizer_Layout_Tab->Add(txt, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
            Sizer_Layout_Tab->Add(btn, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
            break;
        default:
            break;
        }
    }
}

void ColorManagerSettingsPanel::SetButtonColor(wxBitmapButton* btn, const wxColour &c) {
#ifdef __WXOSX__
    SetButtonBackground(btn, c, 1);
#else
    btn->SetBackgroundColour(c);
    btn->SetForegroundColour(c);
#endif
    wxImage image(36,18);
    image.SetRGB(wxRect(0,0,36,18), c.Red(), c.Green(), c.Blue());
    wxBitmap bmp(image);
    btn->SetBitmap(bmp);
}

void ColorManagerSettingsPanel::SetButtonColor(wxBitmapButton* btn, const xlColor &color) {
    wxColour c = (wxColour)(color);
    SetButtonColor(btn, c);
}

void ColorManagerSettingsPanel::RefreshColors()
{
    frame->color_mgr.RefreshColors();
    frame->GetMainSequencer()->PanelEffectGrid->ForceRefresh();
    frame->GetMainSequencer()->PanelRowHeadings->Refresh();
    frame->GetMainSequencer()->PanelTimeLine->Refresh();
    frame->GetMainSequencer()->PanelWaveForm->Refresh();
}

void ColorManagerSettingsPanel::OnButtonImportClick(wxCommandEvent& event) {
    wxFileDialog dlg(this, "Import Theme", wxEmptyString, "Colors", "Theme Files (*.xtheme)|*.xtheme|All Files (*.)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        wxXmlDocument themeXml(dlg.GetPath());
        wxXmlNode* root = themeXml.GetRoot();
        if (root->GetName() == "theme") {
            wxXmlNode* colorNode = root->GetChildren();
            if (colorNode->GetName() == "colors") {
                frame->color_mgr.Load(colorNode);
                UpdateButtonColors();
                RefreshColors();
                frame->color_mgr.SetDirty();
            }
        }
    }
}
void ColorManagerSettingsPanel::OnButtonExportClick(wxCommandEvent& event) {
    wxFileDialog dlg(this, "Export Theme", wxEmptyString, "Colors", "Theme Files (*.xtheme)|*.xtheme|All Files (*.)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        wxXmlDocument themeXml;
        wxXmlNode *root = new wxXmlNode(wxXML_ELEMENT_NODE, "theme");
        themeXml.SetRoot(root);
        frame->color_mgr.Save(&themeXml);
        themeXml.Save(dlg.GetPath());
    }
}
void ColorManagerSettingsPanel::OnButton_ResetClick(wxCommandEvent& event) {
    if (wxMessageBox("Are you sure you want to reset all colors to the defaults?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
        return;
    }
    frame->color_mgr.ResetDefaults();
    UpdateButtonColors();
    RefreshColors();
    frame->color_mgr.SetDirty();
}
void ColorManagerSettingsPanel::ColorButtonSelected(wxCommandEvent& event) {
    wxBitmapButton * button = (wxBitmapButton*)event.GetEventObject();
    wxString name = button->GetName();

    wxColour color = button->GetBackgroundColour();
    wxColourData _colorData;
    _colorData.SetColour(color);
    wxColourDialog dialog(this, &_colorData);
    if (dialog.ShowModal() == wxID_OK) {
        _colorData = dialog.GetColourData();
        color = _colorData.GetColour();
        SetButtonColor(button, color);
        xlColor c(color);
        frame->color_mgr.SetNewColor(name.ToStdString(), c);
        RefreshColors();
        frame->color_mgr.SetDirty();
    }

}

void ColorManagerSettingsPanel::OnCheckBox_SuppressDarkModeClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
