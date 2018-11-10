#include "ColorManagerDialog.h"
#include "ColorManager.h"
#include "ColorFanImage.h"
#include "sequencer/MainSequencer.h"

#include <wx/colour.h>
#include <wx/colourdata.h>
#include <wx/colordlg.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>

//(*InternalHeaders(ColorManagerDialog)
#include <wx/bitmap.h>
#include <wx/font.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ColorManagerDialog)
const long ColorManagerDialog::ID_STATICTEXT1 = wxNewId();
const long ColorManagerDialog::ID_STATICBITMAP1 = wxNewId();
const long ColorManagerDialog::ID_BUTTON_IMPORT = wxNewId();
const long ColorManagerDialog::ID_BUTTON_EXPORT = wxNewId();
const long ColorManagerDialog::ID_BUTTON_RESET = wxNewId();
const long ColorManagerDialog::ID_BUTTON_Cancel = wxNewId();
const long ColorManagerDialog::ID_BUTTON_Close = wxNewId();
//*)

BEGIN_EVENT_TABLE(ColorManagerDialog,wxDialog)
	//(*EventTable(ColorManagerDialog)
	//*)
END_EVENT_TABLE()

ColorManagerDialog::ColorManagerDialog(wxWindow* parent,ColorManager& color_mgr_,wxWindowID id,const wxPoint& pos,const wxSize& size)
: color_mgr(color_mgr_)
{
    color_fan = wxBITMAP_PNG_FROM_DATA(colorfan);

	//(*Initialize(ColorManagerDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer7;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer2;
	wxStaticBoxSizer* StaticBoxSizer3;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Color Manager"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	wxFont StaticText1Font(16,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText1->SetFont(StaticText1Font);
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBitmap1 = new wxStaticBitmap(this, ID_STATICBITMAP1, color_fan, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, _T("ID_STATICBITMAP1"));
	FlexGridSizer2->Add(StaticBitmap1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	Sizer_Effect_Grid = new wxFlexGridSizer(0, 2, 0, 0);
	StaticBoxSizer2->Add(Sizer_Effect_Grid, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer3 = new wxStaticBoxSizer(wxVERTICAL, this, _("Layout Tab"));
	Sizer_Layout_Tab = new wxFlexGridSizer(0, 2, 0, 0);
	StaticBoxSizer3->Add(Sizer_Layout_Tab, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(StaticBoxSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 5, 0, 0);
	ButtonImport = new wxButton(this, ID_BUTTON_IMPORT, _("Import"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_IMPORT"));
	FlexGridSizer7->Add(ButtonImport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonExport = new wxButton(this, ID_BUTTON_EXPORT, _("Export"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_EXPORT"));
	FlexGridSizer7->Add(ButtonExport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Reset = new wxButton(this, ID_BUTTON_RESET, _("Reset Defaults"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_RESET"));
	FlexGridSizer7->Add(Button_Reset, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON_Cancel, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Cancel"));
	FlexGridSizer7->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Close = new wxButton(this, ID_BUTTON_Close, _("OK"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Close"));
	FlexGridSizer7->Add(Button_Close, 1, wxALL|wxALIGN_RIGHT, 5);
	FlexGridSizer1->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_IMPORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::OnButtonImportClick);
	Connect(ID_BUTTON_EXPORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::OnButtonExportClick);
	Connect(ID_BUTTON_RESET,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::OnButton_Reset_DefaultsClick);
	Connect(ID_BUTTON_Cancel,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::OnButton_CancelClick);
	Connect(ID_BUTTON_Close,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::OnButton_OkClick);
	//*)

	AddButtonsToDialog();
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

    SetEscapeId(Button_Cancel->GetId());

    UpdateButtonColors();
    color_mgr.Snapshot();
}

ColorManagerDialog::~ColorManagerDialog()
{
	//(*Destroy(ColorManagerDialog)
	//*)
}

void ColorManagerDialog::UpdateButtonColors()
{
    for (size_t i = 0; i < ColorManager::NUM_COLORS; ++i)
	{
        wxString name = "ID_BITMAPBUTTON_" + color_mgr.xLights_color[i].name;
        wxBitmapButton* btn = (wxBitmapButton*)FindWindowByName(name);
        if (btn != nullptr) {
            SetButtonColor( btn, color_mgr.GetColor(color_mgr.xLights_color[i].id));
        }
	}
}

void ColorManagerDialog::AddButtonsToDialog()
{
    for (size_t i = 0; i < ColorManager::NUM_COLORS; ++i)
	{
        wxString bitmapbutton_id_str = "ID_BITMAPBUTTON_" + color_mgr.xLights_color[i].name;
        wxString static_text_id_str = "ID_STATICTEXT_" + color_mgr.xLights_color[i].name;

        const long static_text_id = wxNewId();
        const long bitmapbutton_id = wxNewId();

        wxStaticText* txt = new wxStaticText(this, static_text_id, color_mgr.xLights_color[i].display_name, wxDefaultPosition, wxDefaultSize, 0, static_text_id_str);
        wxBitmapButton* btn = new wxBitmapButton(this, bitmapbutton_id, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, bitmapbutton_id_str);

        Connect(bitmapbutton_id,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);

        switch( color_mgr.xLights_color[i].category)
        {
        case ColorManager::COLOR_CAT_TIMINGS:
            Sizer_Timing_Tracks->Add(txt, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
            Sizer_Timing_Tracks->Add(btn, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
            break;

        case ColorManager::COLOR_CAT_EFFECT_GRID:
            Sizer_Effect_Grid->Add(txt, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
            Sizer_Effect_Grid->Add(btn, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
            break;

        case ColorManager::COLOR_CAT_LAYOUT_TAB:
            Sizer_Layout_Tab->Add(txt, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
            Sizer_Layout_Tab->Add(btn, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
            break;
        default:
            break;
        }
	}
}

void ColorManagerDialog::SetMainSequencer(MainSequencer* sequencer)
{
    main_sequencer = sequencer;
}

void ColorManagerDialog::SetButtonColor(wxBitmapButton* btn, const wxColour c)
{
    btn->SetBackgroundColour(c);
    btn->SetForegroundColour(c);

    wxImage image(36,18);
    image.SetRGB(wxRect(0,0,36,18), c.Red(), c.Green(), c.Blue());
    wxBitmap bmp(image);

    btn->SetBitmap(bmp);
}

void ColorManagerDialog::SetButtonColor(wxBitmapButton* btn, const xlColor color)
{
    wxColour c = (wxColour)(color);
    SetButtonColor(btn, c);
}

void ColorManagerDialog::ColorButtonSelected(wxCommandEvent& event)
{
    wxBitmapButton * button = (wxBitmapButton*)event.GetEventObject();
    wxString name = button->GetName();

    wxColour color = button->GetBackgroundColour();
    wxColourData colorData;
    colorData.SetColour(color);
    wxColourDialog dialog(this, &colorData);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxColourData retData = dialog.GetColourData();
        color = retData.GetColour();
        SetButtonColor(button, color);
        xlColor c(color);
        color_mgr.SetNewColor(name.ToStdString(), c);
        RefreshColors();
        color_mgr.SetDirty();
    }
}

void ColorManagerDialog::OnButton_Reset_DefaultsClick(wxCommandEvent& event)
{
    if (wxMessageBox("Are you sure you want to reset all colors to the defaults?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO)
    {
        return;
    }

    color_mgr.ResetDefaults();
    UpdateButtonColors();
    RefreshColors();
    color_mgr.SetDirty();
}

void ColorManagerDialog::RefreshColors()
{
    main_sequencer->PanelEffectGrid->ForceRefresh();
    main_sequencer->PanelRowHeadings->Refresh();
    color_mgr.RefreshColors();
}

void ColorManagerDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}

void ColorManagerDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    color_mgr.RestoreSnapshot();
    RefreshColors();
    EndModal(wxID_CANCEL);
}


void ColorManagerDialog::OnButtonImportClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, "Import Theme", wxEmptyString, "Colors", "Theme Files (*.xtheme)|*.xtheme|All Files (*.)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        wxXmlDocument themeXml(dlg.GetPath());
        wxXmlNode* root = themeXml.GetRoot();
        if (root->GetName() == "theme")
        {
            wxXmlNode* colorNode = root->GetChildren();
            if (colorNode->GetName() == "colors")
            {
                color_mgr.Load(colorNode);
                UpdateButtonColors();
                RefreshColors();
                color_mgr.SetDirty();
            }
        }
    }
}

void ColorManagerDialog::OnButtonExportClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, "Export Theme", wxEmptyString, "Colors", "Theme Files (*.xtheme)|*.xtheme|All Files (*.)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxXmlDocument themeXml;
        wxXmlNode *root = new wxXmlNode(wxXML_ELEMENT_NODE, "theme");
        themeXml.SetRoot(root);
        color_mgr.Save(&themeXml);
        themeXml.Save(dlg.GetPath());
    }
}
