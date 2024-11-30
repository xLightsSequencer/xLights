/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(SeqSettingsDialog)
#include <wx/bitmap.h>
#include <wx/font.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/artprov.h>
#include <wx/numdlg.h>
#include <wx/filedlg.h>
#include <wx/treectrl.h>
#include <wx/dir.h>

#include <list>

#include "SeqSettingsDialog.h"
#include "NewTimingDialog.h"
#include "xLightsXmlFile.h"
#include "DataLayer.h"
#include "FileConverter.h"
#include "LorConvertDialog.h"
#include "ConvertLogDialog.h"
#include "VAMPPluginDialog.h"
#include "CustomTimingDialog.h"
#include "VendorMusicDialog.h"
#include "xLightsMain.h"
#include "MetronomeLabelDialog.h"
#include "UtilFunctions.h"
#include "ExternalHooks.h"
#include "ConvertDialog.h"


//(*IdInit(SeqSettingsDialog)
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_File = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Filename = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_XML_Type_Version = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_XML_Version = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Num_Models_Label = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Num_Models = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Xml_Seq_Type = wxNewId();
const wxWindowID SeqSettingsDialog::ID_CHOICE_Xml_Seq_Type = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Xml_MediaFile = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TEXTCTRL_Xml_Media_File = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BITMAPBUTTON_Xml_Media_File = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT2 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT3 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT1 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TEXTCTRL1 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TEXTCTRL2 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TEXTCTRL3 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BUTTON1 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BUTTON_AddMilliseconds = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Xml_Total_Length = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TEXTCTRL_Xml_Seq_Duration = wxNewId();
const wxWindowID SeqSettingsDialog::ID_CHECKBOX_Overwrite_Tags = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TEXTCTRL_SeqTiming = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BITMAPBUTTON__ModifyTiming = wxNewId();
const wxWindowID SeqSettingsDialog::ID_CHECKBOX1 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_PANEL3 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Xml_Author = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TEXTCTRL_Xml_Author = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Xml_Author_Email = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TEXTCTRL_Xml_Author_Email = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BUTTON4 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Xml_Website = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TEXTCTRL_Xml_Website = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BUTTON3 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Xml_Song = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TEXTCTRL_Xml_Song = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Xml_Artist = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TEXTCTRL_Xml_Artist = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Xml_Album = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TEXTCTRL_Xml_Album = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Xml_Music_Url = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TEXTCTRL_Xml_Music_Url = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BUTTON2 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Xml_Comment = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TEXTCTRL_Xml_Comment = wxNewId();
const wxWindowID SeqSettingsDialog::ID_PANEL1 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BUTTON_Xml_New_Timing = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BUTTON_Xml_Import_Timing = wxNewId();
const wxWindowID SeqSettingsDialog::ID_PANEL2 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_CHOICE1 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_TREECTRL_Data_Layers = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BUTTON_Layer_Import = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BUTTON_Layer_Delete = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BUTTON_Move_Up = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BUTTON_Move_Down = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BUTTON_Reimport = wxNewId();
const wxWindowID SeqSettingsDialog::ID_PANEL4 = wxNewId();
const wxWindowID SeqSettingsDialog::ID_NOTEBOOK_Seq_Settings = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Warning = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Info = wxNewId();
const wxWindowID SeqSettingsDialog::ID_STATICTEXT_Warn_No_Media = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BUTTON_CANCEL = wxNewId();
const wxWindowID SeqSettingsDialog::ID_BUTTON_Close = wxNewId();
//*)

const long SeqSettingsDialog::ID_GRID_TIMING = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_Wiz_Music = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_Wiz_Anim = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_25ms = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_50ms = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_Custom = wxNewId();
const long SeqSettingsDialog::ID_PANEL_Wizard = wxNewId();

const long SeqSettingsDialog::ID_BITMAPBUTTON_lor = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_vixen = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_gled = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_hls = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_lynx = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_xlights = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_quick_start = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_skip_import = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_edit_metadata = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_import_timings = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_wizard_done = wxNewId();
const long SeqSettingsDialog::ID_CHOICE_Models = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_models_next = wxNewId();


wxDEFINE_EVENT(EVT_GRID_ROW_CLICKED, wxCommandEvent);
wxDEFINE_EVENT(EVT_NAME_CHANGE, wxCommandEvent);

BEGIN_EVENT_TABLE(SeqSettingsDialog,wxDialog)
	//(*EventTable(SeqSettingsDialog)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_NAME_CHANGE, SeqSettingsDialog::OnButton_Xml_Rename_TimingClick)
    EVT_COMMAND(wxID_ANY, EVT_GRID_ROW_CLICKED, SeqSettingsDialog::OnButton_Xml_Delete_TimingClick)
END_EVENT_TABLE()

#define string_format wxString::Format

class LayerTreeItemData : public wxTreeItemData
{
public:
    LayerTreeItemData(DataLayer* layer_) : layer(layer_) { }

    DataLayer* GetLayer() const { return layer; }

private:
    DataLayer* layer;
};

SeqSettingsDialog::SeqSettingsDialog(wxWindow* parent, xLightsXmlFile* file_to_handle_, const std::list<std::string>& media_dirs, const wxString& warning, const wxString& defaultView, bool wizard_active_, const std::string& media, uint32_t durationMS) :
    xml_file(file_to_handle_),
    media_directories(media_dirs),
    xLightsParent((xLightsFrame*)parent),
    selected_branch_index(-1),
    selected_view("All Models"),
    wizard_active(wizard_active_)
{
    _plog = nullptr;
    Button_WizardDone = nullptr;
    BitmapButton_quick_start = nullptr;

    musical_seq = wxArtProvider::GetBitmapBundle("xlART_musical_seq", wxART_BUTTON);
    musical_seq_pressed = wxArtProvider::GetBitmapBundle("xlART_musical_seq_pressed", wxART_BUTTON);
    animation_seq = wxArtProvider::GetBitmapBundle("xlART_animation_seq", wxART_BUTTON);
    animation_seq_pressed = wxArtProvider::GetBitmapBundle("xlART_animation_seq_pressed", wxART_BUTTON);
    time_25ms = wxArtProvider::GetBitmapBundle("xlART_time_25ms", wxART_BUTTON);
    time_25ms_pressed = wxArtProvider::GetBitmapBundle("xlART_time_25ms_pressed", wxART_BUTTON);
    time_50ms = wxArtProvider::GetBitmapBundle("xlART_time_50ms", wxART_BUTTON);
    time_50ms_pressed = wxArtProvider::GetBitmapBundle("xlART_time_50ms_pressed", wxART_BUTTON);
    time_custom = wxArtProvider::GetBitmapBundle("xlART_time_custom", wxART_BUTTON);
    time_custom_pressed = wxArtProvider::GetBitmapBundle("xlART_time_custom_pressed", wxART_BUTTON);
    lightorama = wxArtProvider::GetBitmapBundle("xlART_lightorama", wxART_BUTTON);
    vixen = wxArtProvider::GetBitmapBundle("xlART_vixen", wxART_BUTTON);
    glediator = wxArtProvider::GetBitmapBundle("xlART_glediator", wxART_BUTTON);
    hls = wxArtProvider::GetBitmapBundle("xlART_hls", wxART_BUTTON);
    lynx = wxArtProvider::GetBitmapBundle("xlART_lynx", wxART_BUTTON);
    xlights_logo = wxArtProvider::GetBitmapBundle("xlART_xlights_logo", wxART_BUTTON);
    quick_start = wxArtProvider::GetBitmapBundle("xlART_quick_start", wxART_BUTTON);
    quick_start_pressed = wxArtProvider::GetBitmapBundle("xlART_quick_start_pressed", wxART_BUTTON);

    //(*Initialize(SeqSettingsDialog)
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer11;
    wxFlexGridSizer* FlexGridSizer12;
    wxFlexGridSizer* FlexGridSizer13;
    wxFlexGridSizer* FlexGridSizer14;
    wxFlexGridSizer* FlexGridSizer15;
    wxFlexGridSizer* FlexGridSizer16;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizer_Timing_Grid;
    wxFlexGridSizer* FlexGridSizer_Timing_Page;
    wxGridBagSizer* GridBagSizer1;
    wxStaticText* StaticText2;
    wxStaticText* StaticText_Xml_Seq_Timing;

    Create(parent, wxID_ANY, _("Sequence Settings"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxSYSTEM_MENU|wxBORDER_STATIC, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableRow(0);
    Notebook_Seq_Settings = new wxNotebook(this, ID_NOTEBOOK_Seq_Settings, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_Seq_Settings"));
    PanelInfo = new wxPanel(Notebook_Seq_Settings, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    GridBagSizer1 = new wxGridBagSizer(0, 0);
    StaticText_File = new wxStaticText(PanelInfo, ID_STATICTEXT_File, _("Filename:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_File"));
    GridBagSizer1->Add(StaticText_File, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Filename = new wxStaticText(PanelInfo, ID_STATICTEXT_Filename, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Filename"));
    GridBagSizer1->Add(StaticText_Filename, wxGBPosition(0, 1), wxGBSpan(1, 4), wxALL|wxEXPAND, 5);
    StaticText_XML_Type_Version = new wxStaticText(PanelInfo, ID_STATICTEXT_XML_Type_Version, _("XML Version:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_XML_Type_Version"));
    GridBagSizer1->Add(StaticText_XML_Type_Version, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_XML_Version = new wxStaticText(PanelInfo, ID_STATICTEXT_XML_Version, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(PanelInfo,wxSize(50,-1)), 0, _T("ID_STATICTEXT_XML_Version"));
    GridBagSizer1->Add(StaticText_XML_Version, wxGBPosition(1, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    wxSize __SpacerSize_1 = wxDLG_UNIT(PanelInfo,wxSize(25,-1));
    GridBagSizer1->Add(__SpacerSize_1.GetWidth(),__SpacerSize_1.GetHeight(),1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
    StaticText_Num_Models_Label = new wxStaticText(PanelInfo, ID_STATICTEXT_Num_Models_Label, _("# Models:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Num_Models_Label"));
    GridBagSizer1->Add(StaticText_Num_Models_Label, wxGBPosition(1, 3), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Num_Models = new wxStaticText(PanelInfo, ID_STATICTEXT_Num_Models, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(PanelInfo,wxSize(50,-1)), 0, _T("ID_STATICTEXT_Num_Models"));
    GridBagSizer1->Add(StaticText_Num_Models, wxGBPosition(1, 4), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(GridBagSizer1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText_Xml_Seq_Type = new wxStaticText(PanelInfo, ID_STATICTEXT_Xml_Seq_Type, _("Sequence Type:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Seq_Type"));
    FlexGridSizer5->Add(StaticText_Xml_Seq_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Xml_Seq_Type = new wxChoice(PanelInfo, ID_CHOICE_Xml_Seq_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Xml_Seq_Type"));
    Choice_Xml_Seq_Type->Append(_("Media"));
    Choice_Xml_Seq_Type->Append(_("Animation"));
    FlexGridSizer5->Add(Choice_Xml_Seq_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer10 = new wxFlexGridSizer(0, 5, 0, 0);
    FlexGridSizer10->AddGrowableCol(1);
    StaticText_Xml_MediaFile = new wxStaticText(PanelInfo, ID_STATICTEXT_Xml_MediaFile, _("Media:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_MediaFile"));
    FlexGridSizer10->Add(StaticText_Xml_MediaFile, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Media_File = new wxTextCtrl(PanelInfo, ID_TEXTCTRL_Xml_Media_File, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Media_File"));
    FlexGridSizer10->Add(TextCtrl_Xml_Media_File, 1, wxALL|wxEXPAND, 5);
    BitmapButton_Xml_Media_File = new wxBitmapButton(PanelInfo, ID_BITMAPBUTTON_Xml_Media_File, wxArtProvider::GetBitmapBundle("wxART_CDROM",wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Xml_Media_File"));
    BitmapButton_Xml_Media_File->Disable();
    FlexGridSizer10->Add(BitmapButton_Xml_Media_File, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(PanelInfo, ID_STATICTEXT2, _("Pre"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer10->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(PanelInfo, ID_STATICTEXT3, _("Post"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer10->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText1 = new wxStaticText(PanelInfo, ID_STATICTEXT1, _("Hash:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer10->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Hash = new wxTextCtrl(PanelInfo, ID_TEXTCTRL1, _("N/A"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer10->Add(TextCtrl_Hash, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer10->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Premilliseconds = new wxTextCtrl(PanelInfo, ID_TEXTCTRL2, _T("0"), wxDefaultPosition, wxSize(50,25), 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    TextCtrl_Premilliseconds->SetMaxLength(5);
    TextCtrl_Premilliseconds->SetToolTip(_("Milliseconds to add to the begining of the sequence"));
    FlexGridSizer10->Add(TextCtrl_Premilliseconds, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Postmilliseconds = new wxTextCtrl(PanelInfo, ID_TEXTCTRL3, _T("0"), wxDefaultPosition, wxSize(50,25), 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    TextCtrl_Postmilliseconds->SetMaxLength(5);
    TextCtrl_Postmilliseconds->SetToolTip(_("Milliseconds to add to the ending of the sequence"));
    FlexGridSizer10->Add(TextCtrl_Postmilliseconds, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer10->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Download = new wxButton(PanelInfo, ID_BUTTON1, _("Download Sequence and Lyrics"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer10->Add(Button_Download, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer10->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer16 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_AddMilliseconds = new wxButton(PanelInfo, ID_BUTTON_AddMilliseconds, _("Add Milliseconds"), wxDefaultPosition, wxSize(143,23), 0, wxDefaultValidator, _T("ID_BUTTON_AddMilliseconds"));
    FlexGridSizer16->Add(Button_AddMilliseconds, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer10->Add(FlexGridSizer16, 1, wxLEFT, 5);
    FlexGridSizer4->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer6 = new wxFlexGridSizer(0, 4, 0, 0);
    StaticText_Xml_Total_Length = new wxStaticText(PanelInfo, ID_STATICTEXT_Xml_Total_Length, _("Sequence Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Total_Length"));
    FlexGridSizer6->Add(StaticText_Xml_Total_Length, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Seq_Duration = new wxTextCtrl(PanelInfo, ID_TEXTCTRL_Xml_Seq_Duration, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Seq_Duration"));
    TextCtrl_Xml_Seq_Duration->SetMinSize(wxDLG_UNIT(PanelInfo,wxSize(50,-1)));
    FlexGridSizer6->Add(TextCtrl_Xml_Seq_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    wxSize __SpacerSize_2 = wxDLG_UNIT(PanelInfo,wxSize(25,-1));
    FlexGridSizer6->Add(__SpacerSize_2.GetWidth(),__SpacerSize_2.GetHeight(),1, wxALL|wxEXPAND, 5);
    CheckBox_Overwrite_Tags = new wxCheckBox(PanelInfo, ID_CHECKBOX_Overwrite_Tags, _("Overwrite Media Tags"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Overwrite_Tags"));
    CheckBox_Overwrite_Tags->SetValue(false);
    FlexGridSizer6->Add(CheckBox_Overwrite_Tags, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
    StaticText_Xml_Seq_Timing = new wxStaticText(PanelInfo, wxID_ANY, _("Sequence Timing:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer3->Add(StaticText_Xml_Seq_Timing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_SeqTiming = new wxTextCtrl(PanelInfo, ID_TEXTCTRL_SeqTiming, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_SeqTiming"));
    TextCtrl_SeqTiming->SetMinSize(wxDLG_UNIT(PanelInfo,wxSize(50,-1)));
    FlexGridSizer3->Add(TextCtrl_SeqTiming, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_ModifyTiming = new wxBitmapButton(PanelInfo, ID_BITMAPBUTTON__ModifyTiming, wxArtProvider::GetBitmapBundle("wxART_INFORMATION",wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON__ModifyTiming"));
    FlexGridSizer3->Add(BitmapButton_ModifyTiming, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BlendingCheckBox = new wxCheckBox(PanelInfo, ID_CHECKBOX1, _("Allow Blending Between Models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    BlendingCheckBox->SetValue(false);
    FlexGridSizer3->Add(BlendingCheckBox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
    PanelInfo->SetSizer(FlexGridSizer4);
    PanelMetaData = new wxPanel(Notebook_Seq_Settings, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer_Timing_Page = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer_Timing_Page->AddGrowableCol(1);
    FlexGridSizer_Timing_Page->AddGrowableRow(7);
    StaticText_Xml_Author = new wxStaticText(PanelMetaData, ID_STATICTEXT_Xml_Author, _("Author:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Author"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Author, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Author = new wxTextCtrl(PanelMetaData, ID_TEXTCTRL_Xml_Author, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(PanelMetaData,wxSize(150,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Author"));
    FlexGridSizer_Timing_Page->Add(TextCtrl_Xml_Author, 1, wxALL|wxEXPAND, 5);
    StaticText_Xml_Author_Email = new wxStaticText(PanelMetaData, ID_STATICTEXT_Xml_Author_Email, _("Email:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Author_Email"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Author_Email, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer13 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer13->AddGrowableCol(0);
    TextCtrl_Xml_Author_Email = new wxTextCtrl(PanelMetaData, ID_TEXTCTRL_Xml_Author_Email, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(PanelMetaData,wxSize(150,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Author_Email"));
    FlexGridSizer13->Add(TextCtrl_Xml_Author_Email, 1, wxALL|wxEXPAND, 5);
    Button_EmailSend = new wxButton(PanelMetaData, ID_BUTTON4, _("Send"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer13->Add(Button_EmailSend, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer_Timing_Page->Add(FlexGridSizer13, 1, wxALL|wxEXPAND, 5);
    StaticText_Xml_Website = new wxStaticText(PanelMetaData, ID_STATICTEXT_Xml_Website, _("Website:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Website"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Website, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer14 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer14->AddGrowableCol(0);
    TextCtrl_Xml_Website = new wxTextCtrl(PanelMetaData, ID_TEXTCTRL_Xml_Website, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(PanelMetaData,wxSize(150,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Website"));
    FlexGridSizer14->Add(TextCtrl_Xml_Website, 1, wxALL|wxEXPAND, 5);
    Button_WebsiteOpen = new wxButton(PanelMetaData, ID_BUTTON3, _("Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer14->Add(Button_WebsiteOpen, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer_Timing_Page->Add(FlexGridSizer14, 1, wxALL|wxEXPAND, 5);
    StaticText_Xml_Song = new wxStaticText(PanelMetaData, ID_STATICTEXT_Xml_Song, _("Song:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Song"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Song, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Song = new wxTextCtrl(PanelMetaData, ID_TEXTCTRL_Xml_Song, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(PanelMetaData,wxSize(150,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Song"));
    FlexGridSizer_Timing_Page->Add(TextCtrl_Xml_Song, 1, wxALL|wxEXPAND, 5);
    StaticText_Xml_Artist = new wxStaticText(PanelMetaData, ID_STATICTEXT_Xml_Artist, _("Artist:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Artist"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Artist, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Artist = new wxTextCtrl(PanelMetaData, ID_TEXTCTRL_Xml_Artist, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(PanelMetaData,wxSize(150,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Artist"));
    FlexGridSizer_Timing_Page->Add(TextCtrl_Xml_Artist, 1, wxALL|wxEXPAND, 5);
    StaticText_Xml_Album = new wxStaticText(PanelMetaData, ID_STATICTEXT_Xml_Album, _("Album:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Album"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Album, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Album = new wxTextCtrl(PanelMetaData, ID_TEXTCTRL_Xml_Album, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(PanelMetaData,wxSize(150,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Album"));
    FlexGridSizer_Timing_Page->Add(TextCtrl_Xml_Album, 1, wxALL|wxEXPAND, 5);
    StaticText_Xml_Music_Url = new wxStaticText(PanelMetaData, ID_STATICTEXT_Xml_Music_Url, _("Music URL:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Music_Url"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Music_Url, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer15 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer15->AddGrowableCol(0);
    TextCtrl_Xml_Music_Url = new wxTextCtrl(PanelMetaData, ID_TEXTCTRL_Xml_Music_Url, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(PanelMetaData,wxSize(150,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Music_Url"));
    FlexGridSizer15->Add(TextCtrl_Xml_Music_Url, 1, wxALL|wxEXPAND, 5);
    Button_MusicOpen = new wxButton(PanelMetaData, ID_BUTTON2, _("Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer15->Add(Button_MusicOpen, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer_Timing_Page->Add(FlexGridSizer15, 1, wxALL|wxEXPAND, 5);
    StaticText_Xml_Comment = new wxStaticText(PanelMetaData, ID_STATICTEXT_Xml_Comment, _("Comment:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Comment"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Comment, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Comment = new wxTextCtrl(PanelMetaData, ID_TEXTCTRL_Xml_Comment, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(PanelMetaData,wxSize(150,-1)), wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Comment"));
    FlexGridSizer_Timing_Page->Add(TextCtrl_Xml_Comment, 1, wxALL|wxEXPAND, 5);
    PanelMetaData->SetSizer(FlexGridSizer_Timing_Page);
    PanelTimings = new wxPanel(Notebook_Seq_Settings, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer8->AddGrowableCol(0);
    FlexGridSizer8->AddGrowableRow(0);
    FlexGridSizer_Timing_Grid = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer_Timing_Grid->AddGrowableCol(0);
    FlexGridSizer_Timing_Grid->AddGrowableRow(0);
    FlexGridSizer8->Add(FlexGridSizer_Timing_Grid, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    Button_Xml_New_Timing = new wxButton(PanelTimings, ID_BUTTON_Xml_New_Timing, _("New"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator, _T("ID_BUTTON_Xml_New_Timing"));
    FlexGridSizer2->Add(Button_Xml_New_Timing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Xml_Import_Timing = new wxButton(PanelTimings, ID_BUTTON_Xml_Import_Timing, _("Import"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator, _T("ID_BUTTON_Xml_Import_Timing"));
    FlexGridSizer2->Add(Button_Xml_Import_Timing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    wxSize __SpacerSize_3 = wxDLG_UNIT(PanelTimings,wxSize(-1,12));
    FlexGridSizer8->Add(__SpacerSize_3.GetWidth(),__SpacerSize_3.GetHeight(),1, wxALL|wxEXPAND, 5);
    PanelTimings->SetSizer(FlexGridSizer8);
    Panel_DataLayers = new wxPanel(Notebook_Seq_Settings, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer9->AddGrowableCol(0);
    FlexGridSizer9->AddGrowableRow(1);
    FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText2 = new wxStaticText(Panel_DataLayers, wxID_ANY, _("Render Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer12->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RenderModeChoice = new wxChoice(Panel_DataLayers, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    RenderModeChoice->SetSelection( RenderModeChoice->Append(_("Erase")) );
    RenderModeChoice->Append(_("Canvas"));
    FlexGridSizer12->Add(RenderModeChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(FlexGridSizer12, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TreeCtrl_Data_Layers = new wxTreeCtrl(Panel_DataLayers, ID_TREECTRL_Data_Layers, wxDefaultPosition, wxDLG_UNIT(Panel_DataLayers,wxSize(300,100)), wxTR_EDIT_LABELS|wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL_Data_Layers"));
    FlexGridSizer9->Add(TreeCtrl_Data_Layers, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer11 = new wxFlexGridSizer(0, 5, 0, 0);
    Button_Layer_Import = new wxButton(Panel_DataLayers, ID_BUTTON_Layer_Import, _("Import"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Layer_Import"));
    FlexGridSizer11->Add(Button_Layer_Import, 1, wxALL|wxEXPAND, 5);
    Button_Layer_Delete = new wxButton(Panel_DataLayers, ID_BUTTON_Layer_Delete, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Layer_Delete"));
    Button_Layer_Delete->Disable();
    FlexGridSizer11->Add(Button_Layer_Delete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Move_Up = new wxButton(Panel_DataLayers, ID_BUTTON_Move_Up, _("Move Up"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Move_Up"));
    Button_Move_Up->Disable();
    FlexGridSizer11->Add(Button_Move_Up, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Move_Down = new wxButton(Panel_DataLayers, ID_BUTTON_Move_Down, _("Move Down"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Move_Down"));
    Button_Move_Down->Disable();
    FlexGridSizer11->Add(Button_Move_Down, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Reimport = new wxButton(Panel_DataLayers, ID_BUTTON_Reimport, _("Re-Import"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Reimport"));
    Button_Reimport->Disable();
    FlexGridSizer11->Add(Button_Reimport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(FlexGridSizer11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
    Panel_DataLayers->SetSizer(FlexGridSizer9);
    Notebook_Seq_Settings->AddPage(PanelInfo, _("Info / Media"), false);
    Notebook_Seq_Settings->AddPage(PanelMetaData, _("Meta Data"), false);
    Notebook_Seq_Settings->AddPage(PanelTimings, _("Timings"), false);
    Notebook_Seq_Settings->AddPage(Panel_DataLayers, _("Data Layers"), false);
    FlexGridSizer1->Add(Notebook_Seq_Settings, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 5);
    StaticText_Warning = new wxStaticText(this, ID_STATICTEXT_Warning, _("Show Warning Here"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Warning"));
    StaticText_Warning->Hide();
    StaticText_Warning->SetForegroundColour(wxColour(255,0,0));
    wxFont StaticText_WarningFont(wxDEFAULT,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText_Warning->SetFont(StaticText_WarningFont);
    FlexGridSizer1->Add(StaticText_Warning, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Info = new wxStaticText(this, ID_STATICTEXT_Info, _("Show Info Here"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Info"));
    StaticText_Info->Hide();
    StaticText_Info->SetForegroundColour(wxColour(43,149,213));
    wxFont StaticText_InfoFont(wxDEFAULT,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText_Info->SetFont(StaticText_InfoFont);
    FlexGridSizer1->Add(StaticText_Info, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Warn_No_Media = new wxStaticText(this, ID_STATICTEXT_Warn_No_Media, _("Media File must be selected or change to animation!"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Warn_No_Media"));
    StaticText_Warn_No_Media->Hide();
    StaticText_Warn_No_Media->SetForegroundColour(wxColour(255,0,0));
    wxFont StaticText_Warn_No_MediaFont(20,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText_Warn_No_Media->SetFont(StaticText_Warn_No_MediaFont);
    FlexGridSizer1->Add(StaticText_Warn_No_Media, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
    Button_Cancel = new wxButton(this, ID_BUTTON_CANCEL, _("Don\'t Create Sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CANCEL"));
    Button_Cancel->Hide();
    FlexGridSizer7->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Close = new wxButton(this, ID_BUTTON_Close, _("Done"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Close"));
    FlexGridSizer7->Add(Button_Close, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer7, 1, wxLEFT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHOICE_Xml_Seq_Type, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&SeqSettingsDialog::OnChoice_Xml_Seq_TypeSelect);
    Connect(ID_BITMAPBUTTON_Xml_Media_File, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_Xml_Media_FileClick);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnButton_DownloadClick);
    Connect(ID_BUTTON_AddMilliseconds, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnButton_AddMillisecondsClick);
    Connect(ID_TEXTCTRL_Xml_Seq_Duration, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_Seq_DurationText);
    Connect(ID_BITMAPBUTTON__ModifyTiming, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_ModifyTimingClick);
    Connect(ID_CHECKBOX1, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnCheckBox1Click);
    Connect(ID_TEXTCTRL_Xml_Author, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_AuthorText);
    Connect(ID_TEXTCTRL_Xml_Author_Email, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_Author_EmailText);
    Connect(ID_BUTTON4, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnButton_EmailSendClick);
    Connect(ID_TEXTCTRL_Xml_Website, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_WebsiteText);
    Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnButton_WebsiteOpenClick);
    Connect(ID_TEXTCTRL_Xml_Song, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_SongText);
    Connect(ID_TEXTCTRL_Xml_Artist, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_ArtistText);
    Connect(ID_TEXTCTRL_Xml_Album, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_AlbumText);
    Connect(ID_TEXTCTRL_Xml_Music_Url, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_Music_UrlText);
    Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnButton_MusicOpenClick);
    Connect(ID_TEXTCTRL_Xml_Comment, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_CommentText);
    Connect(ID_BUTTON_Xml_New_Timing, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnButton_Xml_New_TimingClick);
    Connect(ID_BUTTON_Xml_Import_Timing, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnButton_Xml_Import_TimingClick);
    Connect(ID_CHOICE1, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&SeqSettingsDialog::OnRenderModeChoiceSelect);
    Connect(ID_TREECTRL_Data_Layers, wxEVT_COMMAND_TREE_BEGIN_DRAG, (wxObjectEventFunction)&SeqSettingsDialog::OnTreeCtrl_Data_LayersBeginDrag);
    Connect(ID_TREECTRL_Data_Layers, wxEVT_COMMAND_TREE_BEGIN_LABEL_EDIT, (wxObjectEventFunction)&SeqSettingsDialog::OnTreeCtrl_Data_LayersBeginLabelEdit);
    Connect(ID_TREECTRL_Data_Layers, wxEVT_COMMAND_TREE_END_LABEL_EDIT, (wxObjectEventFunction)&SeqSettingsDialog::OnTreeCtrl_Data_LayersEndLabelEdit);
    Connect(ID_TREECTRL_Data_Layers, wxEVT_COMMAND_TREE_SEL_CHANGED, (wxObjectEventFunction)&SeqSettingsDialog::OnTreeCtrl_Data_LayersSelectionChanged);
    Connect(ID_BUTTON_Layer_Import, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnButton_Layer_ImportClick);
    Connect(ID_BUTTON_Layer_Delete, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnButton_Layer_DeleteClick);
    Connect(ID_BUTTON_Move_Up, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnButton_Move_UpClick);
    Connect(ID_BUTTON_Move_Down, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnButton_Move_DownClick);
    Connect(ID_BUTTON_Reimport, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnButton_ReimportClick);
    Connect(ID_BUTTON_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnButton_CancelClick);
    Connect(ID_BUTTON_Close, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SeqSettingsDialog::OnButton_CloseClick);
    //*)

    TextCtrl_Xml_Seq_Duration->Connect(wxEVT_KILL_FOCUS, (wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_Seq_DurationLoseFocus, nullptr, this);

    TreeCtrl_Data_Layers->AddRoot("Layers to Render");
    Button_Close->SetDefault();

    if (wizard_active) {
        WizardPage1();
        Button_Cancel->Show();
    }

    if (warning != "") {
        StaticText_Warning->SetLabelText(warning);
        StaticText_Warning->Show();
    }

    xml_file->AcknowledgeConversion();

    StaticText_Filename->SetLabelText(xml_file->GetFullPath());
    ProcessSequenceType();

    // Setup Grid
    Grid_Timing = new tmGrid(PanelTimings, ID_GRID_TIMING, wxDefaultPosition, wxDLG_UNIT(PanelTimings, wxSize(300, 100)), wxBORDER_SIMPLE, _T("ID_GRID_TIMING"));
    FlexGridSizer_Timing_Grid->Add(Grid_Timing, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 15);

    FlexGridSizer8->Fit(PanelTimings);
    FlexGridSizer8->SetSizeHints(PanelTimings);
    Grid_Timing->DisableDragGridSize();
    Grid_Timing->DisableDragRowSize();
    Grid_Timing->DisableDragColSize();
    Grid_Timing->CreateGrid(0, 2);
    Grid_Timing->GetSize().GetWidth();
    Grid_Timing->HideRowLabels();

    Grid_Timing->SetColSize(0, 342);
    Grid_Timing->SetColSize(1, 25);
    //wxGridCellButtonRenderer* btn1 = new wxGridCellButtonRenderer("");
    //Grid_Timing->SetCellRenderer(0,0, btn1);
    Grid_Timing->SetColLabelValue(0, "Timing Grids");
    Grid_Timing->SetColLabelValue(1, "");
    PopulateTimingGrid();

    StaticText_XML_Version->SetLabelText(xml_file->GetVersion());
    StaticText_Num_Models->SetLabelText(string_format("%d", xml_file->GetNumModels()));
    TextCtrl_Xml_Author->SetValue(xml_file->GetHeaderInfo(HEADER_INFO_TYPES::AUTHOR));
    TextCtrl_Xml_Author_Email->SetValue(xml_file->GetHeaderInfo(HEADER_INFO_TYPES::AUTHOR_EMAIL));
    TextCtrl_Xml_Website->SetValue(xml_file->GetHeaderInfo(HEADER_INFO_TYPES::WEBSITE));
    TextCtrl_Xml_Song->SetValue(xml_file->GetHeaderInfo(HEADER_INFO_TYPES::SONG));
    TextCtrl_Xml_Artist->SetValue(xml_file->GetHeaderInfo(HEADER_INFO_TYPES::ARTIST));
    TextCtrl_Xml_Album->SetValue(xml_file->GetHeaderInfo(HEADER_INFO_TYPES::ALBUM));
    TextCtrl_Xml_Music_Url->SetValue(xml_file->GetHeaderInfo(HEADER_INFO_TYPES::URL));
    TextCtrl_Xml_Comment->SetValue(xml_file->GetHeaderInfo(HEADER_INFO_TYPES::COMMENT));
    Choice_Xml_Seq_Type->SetSelection(Choice_Xml_Seq_Type->FindString(xml_file->GetSequenceType()));
    TextCtrl_SeqTiming->SetValue(xml_file->GetSequenceTiming());
    if (xml_file->GetMedia() == nullptr) {
        TextCtrl_Xml_Media_File->SetValue("");
    } else {
        TextCtrl_Xml_Media_File->SetValue(xml_file->GetMedia()->FileName());
    }
    SetHash();
    TextCtrl_Xml_Seq_Duration->ChangeValue(xml_file->GetSequenceDurationString());
    BlendingCheckBox->SetValue(xml_file->supportsModelBlending());

    DataLayerSet& data_layers = xml_file->GetDataLayers();
    wxTreeItemId root = TreeCtrl_Data_Layers->GetRootItem();

    for (int i = 0; i < data_layers.GetNumLayers(); ++i) {
        DataLayer* layer = data_layers.GetDataLayer(i);
        wxTreeItemId branch = TreeCtrl_Data_Layers->AppendItem(root, layer->GetName(), -1, -1, new LayerTreeItemData(layer));
        TreeCtrl_Data_Layers->AppendItem(branch, "Source: " + layer->GetSource());
        TreeCtrl_Data_Layers->AppendItem(branch, "Data: " + layer->GetDataSource());
        TreeCtrl_Data_Layers->AppendItem(branch, wxString::Format("Number of Channels: %d", layer->GetNumChannels()));
        TreeCtrl_Data_Layers->AppendItem(branch, wxString::Format("Channel Offset: %d", layer->GetChannelOffset()));
    }
    TreeCtrl_Data_Layers->Expand(root);

    RenderModeChoice->SetStringSelection(xml_file->GetRenderMode());

    if (!defaultView.IsEmpty()) {
        if (xLightsParent->GetViewsManager()->GetViewIndex(defaultView) != -1) {
            selected_view = defaultView;
        }
    }

    UpdateDataLayer();
    needs_render = false;

    int x, y, w, h;
    GetPosition(&x, &y);
    GetSize(&w, &h);
    x += w;
    _plog = new ConvertLogDialog(this, -1, wxPoint(x, y));
    _plog->Show(false);
    SetEscapeId(Button_Cancel->GetId());
    ValidateWindow();

    // this is handles automation
    if (media != "") {
        Choice_Xml_Seq_Type->SetSelection(0);
        xml_file->SetSequenceType("Media");
        ProcessSequenceType();
        ObtainAccessToURL(media);

        wxFileName name_and_path(media);
        MediaLoad(name_and_path);
        EndModal(wxID_OK);

    } else if (durationMS != 0) {
        float d = (float)(durationMS) / 1000.0f;
        TextCtrl_Xml_Seq_Duration->SetValue(wxString::Format("%f", d));
        UpdateSequenceTiming();
        EndModal(wxID_OK);
    }
}

void SeqSettingsDialog::OnTextCtrl_Xml_Seq_DurationLoseFocus(wxFocusEvent& event)
{
    UpdateSequenceTiming();
    event.Skip();
}

SeqSettingsDialog::~SeqSettingsDialog()
{
    if (_plog != nullptr)
    {
        _plog->Show(false);
        _plog->Close();
        delete _plog;
        _plog = nullptr;
    }
	//(*Destroy(SeqSettingsDialog)
	//*)
}

void SeqSettingsDialog::RemoveWizard()
{
    /*Disconnect(ID_BITMAPBUTTON_Wiz_Music,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_Wiz_MusicClick);
    Disconnect(ID_BITMAPBUTTON_Wiz_Anim,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_Wiz_AnimClick);
    Disconnect(ID_BITMAPBUTTON_25ms,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_25msClick);
    Disconnect(ID_BITMAPBUTTON_50ms,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_50msClick);
    Disconnect(ID_BITMAPBUTTON_100ms,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_100msClick);*/
    Notebook_Seq_Settings->RemovePage(0);
    Fit();
}

void SeqSettingsDialog::WizardPage1()
{
    BitmapButton_quick_start = nullptr;
    Panel_Wizard = new wxPanel(Notebook_Seq_Settings, ID_PANEL_Wizard, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Wizard"));
    Notebook_Seq_Settings->InsertPage(0, Panel_Wizard, _("Wizard"), true);
    GridBagSizerWizard = new wxGridBagSizer(0, 1);
    GridBagSizerWizard->Add(493,16,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridSizerWizButtons = new wxGridSizer(0, 1, 10, 0);
    BitmapButton_Wiz_Music = new FlickerFreeBitmapButton(Panel_Wizard, ID_BITMAPBUTTON_Wiz_Music, musical_seq, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Wiz_Music"));
    BitmapButton_Wiz_Music->SetBitmapPressed(musical_seq_pressed);
    GridSizerWizButtons->Add(BitmapButton_Wiz_Music, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Wiz_Anim = new FlickerFreeBitmapButton(Panel_Wizard, ID_BITMAPBUTTON_Wiz_Anim, animation_seq, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Wiz_Anim"));
    BitmapButton_Wiz_Anim->SetBitmapPressed(animation_seq_pressed);
    GridSizerWizButtons->Add(BitmapButton_Wiz_Anim, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridBagSizerWizard->Add(GridSizerWizButtons, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel_Wizard->SetSizer(GridBagSizerWizard);
    GridBagSizerWizard->Fit(Panel_Wizard);
    GridBagSizerWizard->SetSizeHints(Panel_Wizard);
    Connect(ID_BITMAPBUTTON_Wiz_Music,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_Wiz_MusicClick);
    Connect(ID_BITMAPBUTTON_Wiz_Anim,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_Wiz_AnimClick);
}

void SeqSettingsDialog::WizardPage2()
{
    BitmapButton_quick_start = nullptr;
    GridBagSizerWizard->Clear(true);
    GridBagSizerWizard->Add(493,16,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridSizerWizButtons = new wxGridSizer(0, 1, 10, 0);
    BitmapButton_25ms = new FlickerFreeBitmapButton(Panel_Wizard, ID_BITMAPBUTTON_25ms, time_25ms, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_25ms"));
    BitmapButton_25ms->SetBitmapPressed(time_25ms_pressed);
    GridSizerWizButtons->Add(BitmapButton_25ms, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_50ms = new FlickerFreeBitmapButton(Panel_Wizard, ID_BITMAPBUTTON_50ms, time_50ms, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_50ms"));
    BitmapButton_50ms->SetBitmapPressed(time_50ms_pressed);
    GridSizerWizButtons->Add(BitmapButton_50ms, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Custom = new FlickerFreeBitmapButton(Panel_Wizard, ID_BITMAPBUTTON_Custom, time_custom, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Custom"));
    BitmapButton_Custom->SetBitmapPressed(time_custom_pressed);
    GridSizerWizButtons->Add(BitmapButton_Custom, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridBagSizerWizard->Add(GridSizerWizButtons, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel_Wizard->SetSizer(GridBagSizerWizard);
    GridBagSizerWizard->Fit(Panel_Wizard);
    GridBagSizerWizard->SetSizeHints(Panel_Wizard);

    Connect(ID_BITMAPBUTTON_25ms,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_25msClick);
    Connect(ID_BITMAPBUTTON_50ms,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_50msClick);
    Connect(ID_BITMAPBUTTON_Custom,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_CustomClick);

    StaticText_Info->SetLabelText("This option defines the smallest division in the sequence. \nHigher FPS options result in larger file sizes.  If you're not sure choose 20fps.");
    StaticText_Info->Show();
    Fit();
    Refresh();
}

void SeqSettingsDialog::WizardPage3()
{
    BitmapButton_quick_start = nullptr;
    GridBagSizerWizard->Clear(true);
    GridBagSizerWizard->Add(493,16,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridSizerWizButtons = new wxGridSizer(0, 1, 5, 10);
    wxStaticText* StaticText_Page3Optional = new wxStaticText(Panel_Wizard, wxID_ANY, _("Select a View:"), wxDefaultPosition, wxDefaultSize, 0, _T(""));
    wxFont Page3OptionalFont(14,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD);
    StaticText_Page3Optional->SetFont(Page3OptionalFont);
    GridSizerWizButtons->Add(StaticText_Page3Optional, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	ModelsChoice = new wxChoice(Panel_Wizard, ID_CHOICE_Models, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Models"));
	ModelsChoice->Append(_("All Models"));
	ModelsChoice->Append(_("Empty"));
	auto views = xLightsParent->GetViewsManager()->GetViews();
    for(auto it = views.begin(); it != views.end(); ++it)
    {
        // dont add the master view
        if ((*it)->GetName() != "Master View")
        {
            ModelsChoice->Append((*it)->GetName());
        }
    }
	ModelsChoice->SetSelection(0);
    if (selected_view != "All Models") {
        ModelsChoice->SetStringSelection(selected_view);
    }
	GridSizerWizButtons->Add(ModelsChoice, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_quick_start = new FlickerFreeBitmapButton(Panel_Wizard, ID_BITMAPBUTTON_quick_start, quick_start, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_quick_start"));
    BitmapButton_quick_start->SetBitmapPressed(quick_start_pressed);
    GridSizerWizButtons->Add(BitmapButton_quick_start, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ModelsChoiceNext = new wxButton(Panel_Wizard, ID_BUTTON_models_next, _("More Options >>"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_models_next"));
    GridSizerWizButtons->Add(ModelsChoiceNext, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    wxFont SkipImportFont(16,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD);
    ModelsChoiceNext->SetFont(SkipImportFont);
    GridBagSizerWizard->Add(GridSizerWizButtons, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel_Wizard->SetSizer(GridBagSizerWizard);
    GridBagSizerWizard->Fit(Panel_Wizard);
    GridBagSizerWizard->SetSizeHints(Panel_Wizard);

    Connect(ID_BUTTON_models_next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_ModelsChoiceNext);
    Connect(ID_BITMAPBUTTON_quick_start,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_quick_startClick);
    ModelsChoice->Connect(wxEVT_CHOICE, (wxObjectEventFunction)&SeqSettingsDialog::OnViewSelect, NULL, this);

    StaticText_Info->SetLabelText("This option is used to select which models will populate the master view. \nPress Quick Start to begin sequencing and skip option steps.");
    StaticText_Info->Show();
    Fit();
    Refresh();
}

void SeqSettingsDialog::WizardPage4()
{
    BitmapButton_quick_start = nullptr;
    GridBagSizerWizard->Clear(true);
    GridBagSizerWizard->Add(493,1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    wxStaticText* StaticText_Page3Optional = new wxStaticText(Panel_Wizard, wxID_ANY, _("Import Data (Optional):"), wxDefaultPosition, wxDefaultSize, 0, _T(""));
    wxFont Page3OptionalFont(12,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD);
    StaticText_Page3Optional->SetFont(Page3OptionalFont);
    GridSizerWizButtons = new wxGridSizer(0, 2, 5, 10);
    GridSizerWizButtons->Add(StaticText_Page3Optional, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    GridSizerWizButtons->Add(50,1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    BitmapButton_lor = new FlickerFreeBitmapButton(Panel_Wizard, ID_BITMAPBUTTON_lor, lightorama, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_lor"));
    GridSizerWizButtons->Add(BitmapButton_lor, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_hls = new FlickerFreeBitmapButton(Panel_Wizard, ID_BITMAPBUTTON_hls, hls, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_hls"));
    GridSizerWizButtons->Add(BitmapButton_hls, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_xlights = new FlickerFreeBitmapButton(Panel_Wizard, ID_BITMAPBUTTON_xlights, xlights_logo, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_xlights"));
    GridSizerWizButtons->Add(BitmapButton_xlights, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_vixen = new FlickerFreeBitmapButton(Panel_Wizard, ID_BITMAPBUTTON_vixen, vixen, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_vixen"));
    GridSizerWizButtons->Add(BitmapButton_vixen, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_gled = new FlickerFreeBitmapButton(Panel_Wizard, ID_BITMAPBUTTON_gled, glediator, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_gled"));
    GridSizerWizButtons->Add(BitmapButton_gled, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_lynx = new FlickerFreeBitmapButton(Panel_Wizard, ID_BITMAPBUTTON_lynx, lynx, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_lynx"));
    GridSizerWizButtons->Add(BitmapButton_lynx, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridSizerWizButtons->Add(185,30,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_SkipImport = new wxButton(Panel_Wizard, ID_BUTTON_skip_import, _("Skip >>"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_skip_import"));
    GridSizerWizButtons->Add(Button_SkipImport, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    wxFont SkipImportFont(16,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD);
    Button_SkipImport->SetFont(SkipImportFont);
    GridBagSizerWizard->Add(GridSizerWizButtons, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel_Wizard->SetSizer(GridBagSizerWizard);
    GridBagSizerWizard->Fit(Panel_Wizard);
    GridBagSizerWizard->SetSizeHints(Panel_Wizard);
    Connect(ID_BITMAPBUTTON_lor,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_lorClick);
    Connect(ID_BITMAPBUTTON_vixen,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_vixenClick);
    Connect(ID_BITMAPBUTTON_gled,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_gledClick);
    Connect(ID_BITMAPBUTTON_hls,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_hlsClick);
    Connect(ID_BITMAPBUTTON_lynx,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_lynxClick);
    Connect(ID_BITMAPBUTTON_xlights,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_xlightsClick);
    Connect(ID_BUTTON_skip_import,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_skip_importClick);
    StaticText_Info->Hide();
    Fit();
    Refresh();
}

void SeqSettingsDialog::WizardPage5()
{
    BitmapButton_quick_start = nullptr;
    GridBagSizerWizard->Clear(true);
    GridBagSizerWizard->Add(493,1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    wxStaticText* StaticText_Page3Optional = new wxStaticText(Panel_Wizard, wxID_ANY, _("Other Optional Tasks:"), wxDefaultPosition, wxDefaultSize, 0, _T(""));
    wxFont Page3OptionalFont(12,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD);
    StaticText_Page3Optional->SetFont(Page3OptionalFont);
    GridSizerWizButtons = new wxGridSizer(0, 2, 5, 10);
    GridSizerWizButtons->Add(StaticText_Page3Optional, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    GridSizerWizButtons->Add(50,1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    Button_EditMetadata = new wxButton(Panel_Wizard, ID_BUTTON_edit_metadata, _("Edit Metadata"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_edit_metadata"));
    GridSizerWizButtons->Add(Button_EditMetadata, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridSizerWizButtons->Add(30,1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    Button_ImportTimings = new wxButton(Panel_Wizard, ID_BUTTON_import_timings, _("Import Timings"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_import_timings"));
    GridSizerWizButtons->Add(Button_ImportTimings, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridSizerWizButtons->Add(30,1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    GridSizerWizButtons->Add(30,1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    GridSizerWizButtons->Add(30,1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    GridSizerWizButtons->Add(30,1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    Button_WizardDone = new wxButton(Panel_Wizard, ID_BUTTON_wizard_done, _("Done >>"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_wizard_done"));
    GridSizerWizButtons->Add(Button_WizardDone, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    wxFont LargerFont(16,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD);
    Button_EditMetadata->SetFont(LargerFont);
    Button_ImportTimings->SetFont(LargerFont);
    Button_WizardDone->SetFont(LargerFont);
    GridBagSizerWizard->Add(GridSizerWizButtons, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel_Wizard->SetSizer(GridBagSizerWizard);
    GridBagSizerWizard->Fit(Panel_Wizard);
    GridBagSizerWizard->SetSizeHints(Panel_Wizard);
    Connect(ID_BUTTON_edit_metadata,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_EditMetadataClick);
    Connect(ID_BUTTON_import_timings,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_ImportTimingsClick);
    Connect(ID_BUTTON_wizard_done,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_Button_WizardDoneClick);
    Fit();
    Refresh();
}

void SeqSettingsDialog::AddTimingCell(const wxString& name)
{
	wxGridCellButtonRenderer* new_renderer = new wxGridCellButtonRenderer("");
	mCellRenderers.push_back(new_renderer);
	int num_rows = Grid_Timing->GetNumberRows();
	Grid_Timing->AppendRows();
	Grid_Timing->SetCellRenderer(num_rows,1, new_renderer);
    Grid_Timing->SetCellValue(num_rows, 0, name);
}

void SeqSettingsDialog::ProcessSequenceType()
{
    wxString type = xml_file->GetSequenceType();
    BitmapButton_Xml_Media_File->Enable((type == "Media"));
    TextCtrl_Xml_Media_File->Enable((type == "Media"));
    CheckBox_Overwrite_Tags->Enable((type == "Media"));
    if( !wizard_active && type == "Media" && !xml_file->HasAudioMedia() )
    {
        StaticText_Warn_No_Media->Show();
    }
    else
    {
        StaticText_Warn_No_Media->Hide();
    }
    Fit();
    ValidateWindow();
}

void SeqSettingsDialog::SetHash()
{
    if (Notebook_Seq_Settings->GetPageText(Notebook_Seq_Settings->GetSelection()) == "Info / Media")
    {
        if (xml_file->GetMedia() != nullptr)
        {
            TextCtrl_Hash->SetValue(xml_file->GetMedia()->Hash());
            Button_Download->Enable();
        }
        else
        {
            TextCtrl_Hash->SetValue("N/A");
            Button_Download->Disable();
        }
    }
}

void SeqSettingsDialog::OnNotebook_Seq_SettingsPageChanged(wxBookCtrlEvent& event)
{
    SetHash();
}

void SeqSettingsDialog::OnChoice_Xml_Seq_TypeSelect(wxCommandEvent& event)
{
    int selection = Choice_Xml_Seq_Type->GetSelection();
    wxString type = Choice_Xml_Seq_Type->GetString(selection);
    if (type != xml_file->GetSequenceType())
    {
        xml_file->SetSequenceType(type);
        ProcessSequenceType();
    }

    wxString path = "";
    if (type != "Animation")
    {
        path = TextCtrl_Xml_Media_File->GetValue();
        if (FileExists(path))
        {
            wxFileName name_and_path(path);
            MediaLoad(path);
        }
    }
    SetHash();
    xLightsFrame *pFrame = dynamic_cast<xLightsFrame *>(GetParent());
    wxASSERT(pFrame != nullptr);
    if (pFrame != nullptr)
        pFrame->UpdateSequenceVideoPanel(path);
    ValidateWindow();
}

void SeqSettingsDialog::OnBitmapButton_Xml_Media_FileClick(wxCommandEvent& event)
{
    MediaChooser();
    ValidateWindow();
}

void SeqSettingsDialog::OnTextCtrl_Xml_AuthorText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(HEADER_INFO_TYPES::AUTHOR, TextCtrl_Xml_Author->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_Author_EmailText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(HEADER_INFO_TYPES::AUTHOR_EMAIL, TextCtrl_Xml_Author_Email->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_WebsiteText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(HEADER_INFO_TYPES::WEBSITE, TextCtrl_Xml_Website->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_SongText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(HEADER_INFO_TYPES::SONG, TextCtrl_Xml_Song->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_ArtistText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(HEADER_INFO_TYPES::ARTIST, TextCtrl_Xml_Artist->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_AlbumText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(HEADER_INFO_TYPES::ALBUM, TextCtrl_Xml_Album->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_Music_UrlText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(HEADER_INFO_TYPES::URL, TextCtrl_Xml_Music_Url->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_CommentText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(HEADER_INFO_TYPES::COMMENT, TextCtrl_Xml_Comment->GetValue());
}

bool SeqSettingsDialog::UpdateSequenceTiming()
{
    double duration = wxAtof(TextCtrl_Xml_Seq_Duration->GetValue());
    if (duration < 0.05) duration = 0.05;

    bool cont = true;
    if ((long)(duration * 1000.0) != xml_file->GetSequenceDurationMS()) {
        if (duration > 3600) {
            if (wxMessageBox("Are you sure you want a sequence longer than an hour. This will consume a large amount of memory and is likely to crash xLights.", "Excessively long sequence detected.", wxYES_NO, this) == wxNO) {
                cont = false;
            }
        }
        if (cont) {
            xml_file->SetSequenceDuration(TextCtrl_Xml_Seq_Duration->GetValue());
            xLightsParent->UpdateSequenceLength();
            xLightsParent->SetSequenceEnd(xml_file->GetSequenceDurationMS());
        }
    }

    return cont;
}

void SeqSettingsDialog::OnTextCtrl_Xml_Seq_DurationText(wxCommandEvent& event)
{
    ValidateWindow();
}

void SeqSettingsDialog::PopulateTimingGrid()
{
    wxArrayString timings;
    if( xml_file->GetSequenceLoaded() )
    {
        timings = xml_file->GetTimingList(xLightsParent->GetSequenceElements());
    }
    else
    {
        timings = xml_file->GetTimingList();
    }

    for(size_t i = 0; i < timings.GetCount(); ++i)
    {
        AddTimingCell(timings[i]);
    }
}

void SeqSettingsDialog::OnButton_Xml_New_TimingClick(wxCommandEvent& event)
{
    NewTimingDialog dialog(this);
    dialog.Fit();
    if(xml_file->GetFrequency() < 40)
    {
        dialog.RemoveChoice("25ms");
    }
    if(xml_file->GetFrequency() < 20)
    {
        dialog.RemoveChoice("50ms");
    }


    VAMPPluginDialog vamp(this);
    std::list<std::string> plugins;
    if (xml_file->HasAudioMedia())
	{
        plugins = xml_file->GetMedia()->GetVamp()->GetAvailablePlugins(xml_file->GetMedia());
        if (plugins.size() == 0)
        {
            dialog.Choice_New_Fixed_Timing->Append("Download Queen Mary Vamp plugins for audio analysis");
        }
        else
        {
            for (const auto& it : plugins)
            {
                dialog.Choice_New_Fixed_Timing->Append(it);
            }
        }
    }

    dialog.Fit();

    if (dialog.ShowModal() == wxID_OK)
    {
        std::string selected_timing = dialog.GetTiming().ToStdString();
        selected_timing = RemoveUnsafeXmlChars(selected_timing);

        if (selected_timing == "Download Queen Mary Vamp plugins for audio analysis")
        {
            DownloadVamp();
        }
        else
        {
            if (std::find(plugins.begin(), plugins.end(), selected_timing) != plugins.end())
            {
                wxString name = vamp.ProcessPlugin(xml_file, xLightsParent, selected_timing, xml_file->GetMedia());
                if (name != "") {
                    AddTimingCell(name);
                }
            }
            else if (selected_timing == "Empty") {
                bool first = true;
                wxTextEntryDialog te(this, "Enter a name for the timing track", wxGetTextFromUserPromptStr, selected_timing);

                OptimiseDialogPosition(&te);
                while (first || xml_file->TimingAlreadyExists(selected_timing, xLightsParent) || selected_timing == "") {
                    first = false;

                    auto base = selected_timing;

                    int suffix = 2;
                    while (xml_file->TimingAlreadyExists(selected_timing, xLightsParent)) {
                        selected_timing = wxString::Format("%s_%d", base, suffix++);
                    }

                    te.SetValue(selected_timing);
                    if (te.ShowModal() == wxID_OK) {
                        selected_timing = te.GetValue();
                        selected_timing = RemoveUnsafeXmlChars(selected_timing);
                    }
                    else {
                        selected_timing = "";
                        break;
                    }
                }

                if (selected_timing != "") {
                    xml_file->AddFixedTimingSection(selected_timing, xLightsParent);
                    AddTimingCell(selected_timing);
                }
            }
            else if (!xml_file->TimingAlreadyExists(selected_timing, xLightsParent))
            {
                if (selected_timing == "Metronome")
                {
                    int base_timing = xml_file->GetFrameMS();
                    wxNumberEntryDialog dlg(this, "Enter metronome timing", "Milliseconds", "Metronome timing", base_timing, base_timing, 60000);
                    if (dlg.ShowModal() == wxID_OK)
                    {
                        int ms = (dlg.GetValue() + base_timing / 2) / base_timing * base_timing;

                        if (ms != dlg.GetValue())
                        {
                            wxString msg = wxString::Format("Timing adjusted to match sequence timing %dms -> %dms", dlg.GetValue(), ms);
                            wxMessageBox(msg);
                        }
                        wxString ttn = wxString::Format("%dms Metronome", ms);
                        if (!xml_file->TimingAlreadyExists(ttn.ToStdString(), xLightsParent))
                        {
                            xml_file->AddFixedTimingSection(ttn.ToStdString(), xLightsParent);
                            AddTimingCell(ttn);
                        }
                    }
                }
                else if (selected_timing == "Metronome w/ Tags")
                {
                    int base_timing = xml_file->GetFrameMS();
                    MetronomeLabelDialog dlg(base_timing, this);
                    if (dlg.ShowModal() == wxID_OK)
                    {
                        int ms = (dlg.GetTiming() + base_timing / 2) / base_timing * base_timing;

                        if (ms != dlg.GetTiming())
                        {
                            wxString msg = wxString::Format("Timing adjusted to match sequence timing %dms -> %dms", dlg.GetTiming(), ms);
                            wxMessageBox(msg);
                        }
                        wxString ttn = wxString::Format("%s%dms Metronome %d Tag", dlg.IsRandomTiming() || dlg.IsRandomTags() ? "Random " : "", ms, dlg.GetTagCount());
                        //Handle new random tag names
                        if( (dlg.IsRandomTiming() || dlg.IsRandomTags()) && xml_file->TimingAlreadyExists(ttn.ToStdString(), xLightsParent) ) {
                            int copyNum = 1;
                            wxString new_ttn = ttn;
                            do {
                                wxString copyString =  wxString::Format(" (%d)", copyNum);
                                new_ttn = ttn + copyString;
                                copyNum++;
                            } while (xml_file->TimingAlreadyExists(new_ttn.ToStdString(), xLightsParent));
                            ttn = new_ttn;
                        }
                        if (!xml_file->TimingAlreadyExists(ttn.ToStdString(), xLightsParent))
                        {
                            xml_file->AddMetronomeLabelTimingSection(ttn.ToStdString(), ms, dlg.GetTagCount(), xLightsParent, dlg.GetMinRandomTiming(), dlg.IsRandomTags());
                            AddTimingCell(ttn);
                        }
                    }
                }
                else
                {
                    xml_file->AddFixedTimingSection(selected_timing, xLightsParent);
                    AddTimingCell(selected_timing);
                }
            }
            else
            {
                DisplayError(string_format("Fixed Timing section %s already exists!", selected_timing), this);
            }
        }
    }
    dialog.Destroy();
}

void SeqSettingsDialog::OnButton_Xml_Import_TimingClick(wxCommandEvent& event)
{
    xLightsParent->ImportTimingElement();
    int num_rows = Grid_Timing->GetNumberRows();
    if( num_rows > 0 ) {
        Grid_Timing->DeleteRows(0, num_rows);
    }
    PopulateTimingGrid();
}

void SeqSettingsDialog::OnButton_Xml_Rename_TimingClick(wxCommandEvent& event)
{
    int selection = event.GetId();
    std::string new_name = Grid_Timing->GetCellValue(selection, 0).ToStdString();
    if( xml_file->TimingAlreadyExists(new_name, xLightsParent) )
    {
        DisplayError(string_format("Timing section %s already exists!", new_name), this);
        new_name += "_1";
        Grid_Timing->SetCellValue(selection, 0, new_name);
    }
    wxArrayString timing_list;
    if( xml_file->GetSequenceLoaded() )
    {
        timing_list = xml_file->GetTimingList(xLightsParent->GetSequenceElements());
    }
    else
    {
        timing_list = xml_file->GetTimingList();
    }
    xLightsParent->RenameTimingElement(timing_list[selection].ToStdString(), new_name);
}

void SeqSettingsDialog::OnButton_Xml_Delete_TimingClick(wxCommandEvent& event)
{
    if( Grid_Timing->GetGridCursorCol() == 1 )
    {
        int row = Grid_Timing->GetGridCursorRow();
        wxArrayString timing_list;
        if( xml_file->GetSequenceLoaded() )
        {
            timing_list = xml_file->GetTimingList(xLightsParent->GetSequenceElements());
            if (timing_list.size() > row)
                xLightsParent->DeleteTimingElement(timing_list[row].ToStdString());
        }
        else
        {
            timing_list = xml_file->GetTimingList();
        }

        if (timing_list.size() > row)
        {
            xml_file->DeleteTimingSection(timing_list[row].ToStdString());
            Grid_Timing->DeleteRows(row);
        }
        Refresh();
    }
}

void SeqSettingsDialog::OnButton_Layer_ImportClick(wxCommandEvent& event)
{
    bool modified = ImportDataLayer(strSupportedFileTypes, _plog);
    if( modified )
    {
        needs_render = true;
    }
    _plog->Done();
}

bool SeqSettingsDialog::ImportDataLayer(const wxString& filetypes, ConvertLogDialog* plog)
{
    bool return_val = false;
    wxFileDialog* ImportDialog = new wxFileDialog( this, "Choose file to import as data layer", wxEmptyString, wxEmptyString, filetypes, wxFD_OPEN, wxDefaultPosition);
    Button_Close->Enable(false);
    Button_Layer_Import->Enable(false);
    if (ImportDialog->ShowModal() == wxID_OK)
    {
        _plog->Show(true);
        int x, y, w, h;
        GetPosition(&x, &y);
        GetSize(&w, &h);
        x += w;
        _plog->SetPosition(wxPoint(x, y));
        wxString fDir = ImportDialog->GetDirectory();
        wxString filename = ImportDialog->GetFilename();
        ObtainAccessToURL(fDir.ToStdString());
        ObtainAccessToURL(filename.ToStdString());

        wxFileName full_name(filename);
        full_name.SetPath(fDir);
        wxFileName data_file(full_name);
        data_file.SetExt("iseq");
        if( full_name.GetExt() != "iseq" ) {
            data_file.SetPath(xLightsParent->GetShowDirectory());
        }
        DataLayerSet& data_layers = xml_file->GetDataLayers();
        DataLayer* new_data_layer = data_layers.AddDataLayer(full_name.GetName(), full_name.GetFullPath(), data_file.GetFullPath() );
        wxTreeItemId root = TreeCtrl_Data_Layers->GetRootItem();
        wxTreeItemId branch1 = TreeCtrl_Data_Layers->AppendItem(root, filename, -1, -1, new LayerTreeItemData(new_data_layer) );
        TreeCtrl_Data_Layers->AppendItem(branch1, "Source: " + full_name.GetFullPath());
        wxTreeItemId branch_data = TreeCtrl_Data_Layers->AppendItem(branch1, "Data: <waiting for file conversion>");
        wxTreeItemId branch_num_channels = TreeCtrl_Data_Layers->AppendItem(branch1, "Number of Channels: <waiting for file conversion>");
        TreeCtrl_Data_Layers->AppendItem(branch1, "Channel Offset: 0");
        TreeCtrl_Data_Layers->Expand(branch1);


        {

        }

        std::string media_filename;
        ConvertParameters conv_params(full_name.GetFullPath(),                                  // input filename
                                      new_data_layer->GetSequenceData(),                        // sequence data object
                                      xLightsParent->GetOutputManager(),                        // global network info
                                      ConvertParameters::READ_MODE_IMPORT,                      // file read mode
                                      xLightsParent,                                            // xLights main frame
                                      nullptr,
                                      _plog,
                                      &media_filename,                                          // media filename
                                      new_data_layer,                                           // data layer to fill in header info
                                      data_file.GetFullPath(),                                  // output filename
                                      atoi(xml_file->GetSequenceTiming().c_str()),              // sequence timing
                                      false,                                                    // turn off all channels at end
                                      false,                                                    // map empty channels (mainly LOR)
                                      false );                                                  // map no network channels (mainly LOR)

        if( full_name.GetExt() == "lms" || full_name.GetExt() == "las" )
        {
            LorConvertDialog* lor_dialog = new LorConvertDialog(this);
            lor_dialog->ShowModal();
            conv_params.channels_off_at_end = lor_dialog->CheckBoxOffAtEnd->IsChecked();
            conv_params.map_empty_channels = lor_dialog->CheckBoxMapEmptyChannels->IsChecked();
            conv_params.map_no_network_channels = lor_dialog->MapLORChannelsWithNoNetwork->IsChecked();
            new_data_layer->SetLORConvertParams( (int)conv_params.channels_off_at_end | ((int)conv_params.map_empty_channels << 1) | ((int)conv_params.map_no_network_channels << 2) );
            FileConverter::ReadLorFile(conv_params);
        }
        else if( full_name.GetExt() == "hlsIdata" )
        {
            conv_params.channels_off_at_end = (wxYES == wxMessageBox("Turn off all channels at the end?", "Conversion Options", wxICON_QUESTION | wxYES_NO));
            new_data_layer->SetLORConvertParams((int)conv_params.channels_off_at_end);
            FileConverter::ReadHLSFile(conv_params);
        }
        else if( full_name.GetExt() == "vix" )
        {
            conv_params.channels_off_at_end = (wxYES == wxMessageBox("Turn off all channels at the end?", "Conversion Options", wxICON_QUESTION | wxYES_NO));
            new_data_layer->SetLORConvertParams((int)conv_params.channels_off_at_end);
            FileConverter::ReadVixFile(conv_params);
        }
        else if( full_name.GetExt() == "gled" )
        {
            conv_params.channels_off_at_end = (wxYES == wxMessageBox("Turn off all channels at the end?", "Conversion Options", wxICON_QUESTION | wxYES_NO));
            new_data_layer->SetLORConvertParams((int)conv_params.channels_off_at_end);
            FileConverter::ReadGlediatorFile(conv_params);
        }
        else if( full_name.GetExt() == "seq" )
        {
            conv_params.channels_off_at_end = (wxYES == wxMessageBox("Turn off all channels at the end?", "Conversion Options", wxICON_QUESTION | wxYES_NO));
            new_data_layer->SetLORConvertParams((int)conv_params.channels_off_at_end);
            FileConverter::ReadConductorFile(conv_params);
        }
        else if( full_name.GetExt() == "fseq")
        {
            FileConverter::ReadFalconFile(conv_params);
        }
        else if( full_name.GetExt() == "iseq" )
        {
            // we read only the header to fill in the channel count info
            conv_params.read_mode = ConvertParameters::READ_MODE_HEADER_ONLY;
            conv_params.media_filename = nullptr;
            FileConverter::ReadFalconFile(conv_params);
        }
        if( full_name.GetExt() != "iseq" )
        {
            FileConverter::WriteFalconPiFile(conv_params);
        }
        TreeCtrl_Data_Layers->SetItemText(branch_data, "Data: " + data_file.GetFullPath());
        TreeCtrl_Data_Layers->SetItemText(branch_num_channels, wxString::Format("Number of Channels: %d", new_data_layer->GetNumChannels()));
        UpdateDataLayer();
        return_val = true;
    }

    ImportDialog->Destroy();
    Button_Close->Enable(true);
    Button_Layer_Import->Enable(true);
    return return_val;
}

void SeqSettingsDialog::OnButton_ReimportClick(wxCommandEvent& event)
{
    _plog->Show(true);
    LayerTreeItemData* data = (LayerTreeItemData*)TreeCtrl_Data_Layers->GetItemData(selected_branch);
    DataLayer* layer = data->GetLayer();
    Button_Close->Enable(false);
    std::string media_filename;
    wxFileName full_name(layer->GetSource());
    ConvertParameters conv_params(layer->GetSource(),                                       // input filename
                                  layer->GetSequenceData(),                                 // sequence data object
                                  xLightsParent->GetOutputManager(),                              // global network info
                                  ConvertParameters::READ_MODE_IMPORT,                      // file read mode
                                  xLightsParent,                                            // xLights main frame
                                  nullptr,
                                  _plog,
                                  &media_filename,                                          // media filename
                                  layer,                                                    // data layer to fill in header info
                                  layer->GetDataSource(),                                   // output filename
                                  atoi(xml_file->GetSequenceTiming().c_str()),              // sequence timing
                                  layer->GetLORConvertParams() & 0x1,                       // turn off all channels at end
                                  (layer->GetLORConvertParams() >> 1) & 0x1,                // map empty channels (mainly LOR)
                                  (layer->GetLORConvertParams() >> 2) & 0x1 );              // map no network channels (mainly LOR)
    if( full_name.GetExt() == "lms" || full_name.GetExt() == "las")
    {
        FileConverter::ReadLorFile(conv_params);
    }
    else if( full_name.GetExt() == "hlsIdata" )
    {
        FileConverter::ReadHLSFile(conv_params);
    }
    else if( full_name.GetExt() == "vix" )
    {
        FileConverter::ReadVixFile(conv_params);
    }
    else if( full_name.GetExt() == "gled" )
    {
        FileConverter::ReadGlediatorFile(conv_params);
    }
    else if( full_name.GetExt() == "seq" )
    {
        FileConverter::ReadConductorFile(conv_params);
    }
    else if( full_name.GetExt() == "fseq")
    {
        FileConverter::ReadFalconFile(conv_params);
    }
    else if( full_name.GetExt() == "iseq" )
    {
        // we read only the header to fill in the channel count info
        conv_params.read_mode = ConvertParameters::READ_MODE_HEADER_ONLY;
        conv_params.media_filename = nullptr;
        FileConverter::ReadFalconFile(conv_params);
    }
    if( full_name.GetExt() != "iseq" )
    {
        FileConverter::WriteFalconPiFile( conv_params );
    }
    //TreeCtrl_Data_Layers->SetItemText(branch_num_channels, wxString::Format("Number of Channels: %d", new_data_layer->GetNumChannels()));  FIXME update in case channel number changes
    Button_Close->Enable(true);
    _plog->Done();
}

void SeqSettingsDialog::UpdateDataLayer()
{
    // update buttons
    Button_Layer_Delete->Enable(false);
    Button_Reimport->Enable(false);
    Button_Move_Up->Enable(false);
    Button_Move_Down->Enable(false);
    wxTreeItemId root = TreeCtrl_Data_Layers->GetRootItem();
    selected_branch = TreeCtrl_Data_Layers->GetFocusedItem();
    if( !selected_branch.IsOk() ) return;
    wxTreeItemIdValue cookie;
    wxString selected_branch_name = TreeCtrl_Data_Layers->GetItemText(selected_branch);
    bool valid_branch = false;
    int num_branches = 0;
    for(wxTreeItemId branch = TreeCtrl_Data_Layers->GetFirstChild(root, cookie); branch.IsOk(); branch = TreeCtrl_Data_Layers->GetNextChild(root, cookie) )
    {
        num_branches++;
        if( branch == selected_branch )
        {
            valid_branch = true;
            selected_branch_index = num_branches-1;
        }
    }

    if( valid_branch )
    {
        if( selected_branch_name != "Nutcracker" )
        {
            Button_Layer_Delete->Enable(true);
            Button_Reimport->Enable(true);
        }
        if( selected_branch_index > 0 )
        {
            Button_Move_Up->Enable(true);
        }
        else if( selected_branch_index < num_branches-1 )
        {
            Button_Move_Down->Enable(true);
        }
    }
}

void SeqSettingsDialog::OnTreeCtrl_Data_LayersBeginDrag(wxTreeEvent& event)
{
}

void SeqSettingsDialog::OnButton_Layer_DeleteClick(wxCommandEvent& event)
{
    DataLayerSet& data_layers = xml_file->GetDataLayers();
    data_layers.RemoveDataLayer(selected_branch_index);
    TreeCtrl_Data_Layers->Delete(selected_branch);
    UpdateDataLayer();
    needs_render = true;
}

void SeqSettingsDialog::OnButton_Move_UpClick(wxCommandEvent& event)
{
    LayerTreeItemData* data = (LayerTreeItemData*)TreeCtrl_Data_Layers->GetItemData(selected_branch);
    DataLayer* selected_layer = data->GetLayer();
    wxTreeItemId prev_item = TreeCtrl_Data_Layers->GetPrevSibling(selected_branch);
    bool prev_expanded = TreeCtrl_Data_Layers->IsExpanded(prev_item);
    wxTreeItemId root = TreeCtrl_Data_Layers->GetRootItem();
    wxTreeItemId new_branch = TreeCtrl_Data_Layers->InsertItem(root, selected_branch, TreeCtrl_Data_Layers->GetItemText(prev_item), -1, -1, new LayerTreeItemData(selected_layer));
    wxTreeItemIdValue cookie;
    for( wxTreeItemId node = TreeCtrl_Data_Layers->GetFirstChild(prev_item, cookie); node.IsOk(); node = TreeCtrl_Data_Layers->GetNextChild(prev_item, cookie))
    {
        TreeCtrl_Data_Layers->AppendItem(new_branch, TreeCtrl_Data_Layers->GetItemText(node));
    }
    TreeCtrl_Data_Layers->Delete(prev_item);
    if( prev_expanded ) TreeCtrl_Data_Layers->Expand(new_branch);
    DataLayerSet& data_layers = xml_file->GetDataLayers();
    data_layers.MoveLayerUp(selected_branch_index);
    UpdateDataLayer();
    needs_render = true;
}

void SeqSettingsDialog::OnButton_Move_DownClick(wxCommandEvent& event)
{
    wxTreeItemId next_item = TreeCtrl_Data_Layers->GetNextSibling(selected_branch);
    LayerTreeItemData* data = (LayerTreeItemData*)TreeCtrl_Data_Layers->GetItemData(next_item);
    DataLayer* next_item_layer = data->GetLayer();
    bool sel_expanded = TreeCtrl_Data_Layers->IsExpanded(selected_branch);
    wxTreeItemId root = TreeCtrl_Data_Layers->GetRootItem();
    wxTreeItemId new_branch = TreeCtrl_Data_Layers->InsertItem(root, next_item, TreeCtrl_Data_Layers->GetItemText(selected_branch), -1, -1, new LayerTreeItemData(next_item_layer));
    wxTreeItemIdValue cookie;
    for( wxTreeItemId node = TreeCtrl_Data_Layers->GetFirstChild(selected_branch, cookie); node.IsOk(); node = TreeCtrl_Data_Layers->GetNextChild(selected_branch, cookie))
    {
        TreeCtrl_Data_Layers->AppendItem(new_branch, TreeCtrl_Data_Layers->GetItemText(node));
    }
    TreeCtrl_Data_Layers->Delete(selected_branch);
    if( sel_expanded ) TreeCtrl_Data_Layers->Expand(new_branch);
    DataLayerSet& data_layers = xml_file->GetDataLayers();
    data_layers.MoveLayerDown(selected_branch_index);
    UpdateDataLayer();
    needs_render = true;
}

void SeqSettingsDialog::OnTreeCtrl_Data_LayersSelectionChanged(wxTreeEvent& event)
{
    UpdateDataLayer();
}

void SeqSettingsDialog::OnButton_CloseClick(wxCommandEvent& event)
{
    if (UpdateSequenceTiming()) {
        if (needs_render) {
            if (!xLightsParent->IsSequenceDataValid()) {
                EndModal(NEEDS_RENDER);
            }
            else {
                EndModal(wxID_OK);
                xLightsParent->RenderAll();
            }
        }
        else {
            EndModal(wxID_OK);
        }
    }
    else {
        event.Skip();
    }
}

void SeqSettingsDialog::OnTreeCtrl_Data_LayersBeginLabelEdit(wxTreeEvent& event)
{
    wxTreeItemId itemId = event.GetItem();
    wxString item_text = TreeCtrl_Data_Layers->GetItemText(itemId);

    if( item_text.Contains("Data:") )
    {
        wxTreeItemId parent = TreeCtrl_Data_Layers->GetItemParent(itemId);
        LayerTreeItemData* data = (LayerTreeItemData*)TreeCtrl_Data_Layers->GetItemData(parent);
        DataLayer* layer = data->GetLayer();
        if( layer->GetName() == "Nutcracker" )
        {
            if( xml_file->GetRenderMode() == xLightsXmlFile::CANVAS_MODE )
            {
                xml_file->SetRenderMode(xLightsXmlFile::ERASE_MODE);
            }
            else
            {
                xml_file->SetRenderMode(xLightsXmlFile::CANVAS_MODE);
            }
            TreeCtrl_Data_Layers->SetItemText(itemId, wxString::Format("Data: %s", xml_file->GetRenderMode()));
        }
    }

    if( !item_text.Contains("Channel Offset:") )
    {
        event.Veto();
    }
}

void SeqSettingsDialog::OnTreeCtrl_Data_LayersEndLabelEdit(wxTreeEvent& event)
{
    wxTreeItemId itemId = event.GetItem();
    wxString item_text = event.GetLabel();
    wxString val = item_text.AfterLast(' ');

    try
    {
        int channel_offset = atoi(val.c_str());
        wxTreeItemId parent = TreeCtrl_Data_Layers->GetItemParent(itemId);
        LayerTreeItemData* data = (LayerTreeItemData*)TreeCtrl_Data_Layers->GetItemData(parent);
        DataLayer* layer = data->GetLayer();
        layer->SetChannelOffset(channel_offset);
        TreeCtrl_Data_Layers->SetItemText(itemId, wxString::Format("Channel Offset: %d", channel_offset));
        event.Veto();  // text was overwritten with original change if not vetoed
    }
    catch(...)
    {
        event.Veto();
    }
}

void SeqSettingsDialog::MediaLoad(wxFileName name_and_path)
{
    xml_file->SetMediaFile(xLightsParent->GetShowDirectory(), name_and_path.GetFullPath(), CheckBox_Overwrite_Tags->IsChecked());
    TextCtrl_Xml_Media_File->SetValue(name_and_path.GetFullPath());
    TextCtrl_Xml_Song->SetValue(xml_file->GetHeaderInfo(HEADER_INFO_TYPES::SONG));
    TextCtrl_Xml_Album->SetValue(xml_file->GetHeaderInfo(HEADER_INFO_TYPES::ALBUM));
    TextCtrl_Xml_Artist->SetValue(xml_file->GetHeaderInfo(HEADER_INFO_TYPES::ARTIST));
    int length_ms = 0;
    if (xml_file->GetMedia() != nullptr) length_ms = xml_file->GetMedia()->LengthMS(); // shouldnt happen but maybe if media load failed
    double length = length_ms / 1000.0f;
    xml_file->SetSequenceDuration(length);
    TextCtrl_Xml_Seq_Duration->ChangeValue(string_format("%.3f", length));
    if (xml_file->GetSequenceLoaded())
    {
        xLightsParent->LoadAudioData(*xml_file);
    }
    xLightsParent->SetSequenceEnd(xml_file->GetSequenceDurationMS());
    StaticText_Warning->Hide();
    ProcessSequenceType();
    xLightsParent->UpdateSequenceLength();
    SetHash();
    ValidateWindow();
}

void SeqSettingsDialog::MediaChooser()
{
	wxFileDialog OpenDialog(this, "Choose Audio file", wxEmptyString, wxEmptyString, "FPP Audio Files|*.mp3;*.ogg;*.m4p;*.mp4;*.m4a;*.aac;*.wav;*.flac;*.wma;*.au;*.mkv;*.mov|xLights Audio Files|*.mp3;*.ogg;*.m4p;*.mp4;*.avi;*.wma;*.au;*.wav;*.m4a;*.mid;*.mkv;*.mov;*.mpg;*.asf;*.flv;*.mpeg;*.wmv;*.flac", wxFD_OPEN | wxFD_FILE_MUST_EXIST, wxDefaultPosition);

    std::string media_directory = media_directories.empty() ? "" : media_directories.front();

    if (wxDir::Exists(media_directory))
    {
        OpenDialog.SetDirectory(media_directory);
    }
	if (!xml_file->GetMediaFile().empty())
	{
		OpenDialog.SetFilename(wxFileName(xml_file->GetMediaFile()).GetFullName());
	}
	if (!TextCtrl_Xml_Media_File->GetValue().empty())
	{
		OpenDialog.SetPath(TextCtrl_Xml_Media_File->GetValue());
	}
    if (OpenDialog.ShowModal() == wxID_OK)
    {
        wxString fDir = OpenDialog.GetDirectory();
        wxString filename = OpenDialog.GetFilename();

        ObtainAccessToURL(fDir.ToStdString());
        ObtainAccessToURL(filename.ToStdString());

        wxFileName name_and_path(filename);
        name_and_path.SetPath(fDir);

        SetCursor(wxCURSOR_WAIT);
        MediaLoad(name_and_path);
        SetCursor(wxCURSOR_DEFAULT);
    }
}

void SeqSettingsDialog::OnBitmapButton_Wiz_MusicClick(wxCommandEvent& event)
{
    Choice_Xml_Seq_Type->SetSelection(0);
    xml_file->SetSequenceType("Media");
    ProcessSequenceType();
    BitmapButton_Wiz_Music->Hide();
    BitmapButton_Wiz_Anim->Hide();
    CheckBox_Overwrite_Tags->SetValue(true);
    MediaChooser();
    WizardPage2();
    ValidateWindow();
}

void SeqSettingsDialog::OnBitmapButton_Wiz_AnimClick(wxCommandEvent& event)
{
    Choice_Xml_Seq_Type->SetSelection(1);
    xml_file->SetSequenceType("Animation");
    TextCtrl_Xml_Seq_Duration->ChangeValue("30.0");
    xml_file->SetSequenceDuration(30.0);
    xLightsParent->SetSequenceEnd(xml_file->GetSequenceDurationMS());
    ProcessSequenceType();
    WizardPage2();
    ValidateWindow();
}

void SeqSettingsDialog::OnBitmapButton_25msClick(wxCommandEvent& event)
{
    xml_file->SetSequenceTiming("25 ms");
    if (xml_file->HasAudioMedia()) {
        xml_file->GetMedia()->SetFrameInterval(25);
    }
    TextCtrl_SeqTiming->SetValue("25 ms");
    WizardPage3();
}

void SeqSettingsDialog::OnBitmapButton_50msClick(wxCommandEvent& event)
{
    xml_file->SetSequenceTiming("50 ms");
    if (xml_file->HasAudioMedia()) {
        xml_file->GetMedia()->SetFrameInterval(50);
    }
    TextCtrl_SeqTiming->SetValue("50 ms");
    WizardPage3();
}

void SeqSettingsDialog::OnBitmapButton_CustomClick(wxCommandEvent& event)
{
    CustomTimingDialog dialog(this);

    dialog.ShowModal();

    xml_file->SetSequenceTiming(dialog.GetTiming());
    if (xml_file->HasAudioMedia()) {
        xml_file->GetMedia()->SetFrameInterval(wxAtoi(dialog.GetTiming()));
    }

    TextCtrl_SeqTiming->SetValue(dialog.GetTiming());
    WizardPage3();
}

void SeqSettingsDialog::OnBitmapButton_lorClick(wxCommandEvent& event)
{
    Notebook_Seq_Settings->SetSelection(4);
    const wxString strFileTypes = "LOR Sequences (*.lms,*.las)|*.lms;*.las";
    if (ImportDataLayer(strFileTypes, _plog)) { _plog->Done();  WizardPage5(); }
    Notebook_Seq_Settings->SetSelection(0);
}

void SeqSettingsDialog::OnBitmapButton_vixenClick(wxCommandEvent& event)
{
    Notebook_Seq_Settings->SetSelection(4);
    const wxString strFileTypes = "Vixen Sequences (*.vix)|*.vix";
    if (ImportDataLayer(strFileTypes, _plog)) { _plog->Done(); WizardPage5(); }
    Notebook_Seq_Settings->SetSelection(0);
}

void SeqSettingsDialog::OnBitmapButton_gledClick(wxCommandEvent& event)
{
    Notebook_Seq_Settings->SetSelection(4);
    const wxString strFileTypes = "Glediator Record File (*.gled)|*.gled";
    if( ImportDataLayer(strFileTypes, _plog) ) { WizardPage5(); }
    Notebook_Seq_Settings->SetSelection(0);
}

void SeqSettingsDialog::OnBitmapButton_hlsClick(wxCommandEvent& event)
{
    Notebook_Seq_Settings->SetSelection(4);
    const wxString strFileTypes = "HLS hlsIdata Sequences(*.hlsIdata)|*.hlsIdata";
    if (ImportDataLayer(strFileTypes, _plog)) { _plog->Done(); WizardPage5(); }
    Notebook_Seq_Settings->SetSelection(0);
}

void SeqSettingsDialog::OnBitmapButton_lynxClick(wxCommandEvent& event)
{
    Notebook_Seq_Settings->SetSelection(4);
    const wxString strFileTypes = "Lynx Conductor Sequences (*.seq)|*.seq";
    if (ImportDataLayer(strFileTypes, _plog)) { _plog->Done(); WizardPage5(); }
    Notebook_Seq_Settings->SetSelection(0);
}

void SeqSettingsDialog::OnBitmapButton_xlightsClick(wxCommandEvent& event)
{
    Notebook_Seq_Settings->SetSelection(4);
    const wxString strFileTypes = "xLights/FPP Sequences(*.iseq, *.fseq)|*.iseq;*.fseq";
    if (ImportDataLayer(strFileTypes, _plog)) { _plog->Done(); WizardPage5(); }
    Notebook_Seq_Settings->SetSelection(0);
}

void SeqSettingsDialog::OnBitmapButton_quick_startClick(wxCommandEvent& event)
{
    if (UpdateSequenceTiming()) {
        EndModal(wxID_OK);
    }
}

void SeqSettingsDialog::OnButton_skip_importClick(wxCommandEvent& event)
{
    WizardPage5();
}

void SeqSettingsDialog::OnButton_ModelsChoiceNext(wxCommandEvent& event)
{
    WizardPage4();
}

void SeqSettingsDialog::OnButton_EditMetadataClick(wxCommandEvent& event)
{
    Notebook_Seq_Settings->SetSelection(2);
}

void SeqSettingsDialog::OnButton_ImportTimingsClick(wxCommandEvent& event)
{
    Notebook_Seq_Settings->SetSelection(3);
}

void SeqSettingsDialog::OnButton_Button_WizardDoneClick(wxCommandEvent& event)
{
    if (UpdateSequenceTiming()) {
        EndModal(wxID_OK);
    }
}

void SeqSettingsDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

void SeqSettingsDialog::OnViewSelect(wxCommandEvent& event)
{
    selected_view = (ModelsChoice->GetString(ModelsChoice->GetSelection())).ToStdString();
}

void SeqSettingsDialog::OnCheckBox1Click(wxCommandEvent& event)
{
    xml_file->setSupportsModelBlending(BlendingCheckBox->GetValue());
    xLightsParent->GetSequenceElements().SetSupportsModelBlending(BlendingCheckBox->GetValue());
}

void SeqSettingsDialog::OnBitmapButton_ModifyTimingClick(wxCommandEvent& event)
{
    if (wxMessageBox("Are you sure?  Modifying the timing interval will cause the sequence to be Saved, Closed, then Re-Opened.  Effects will be moved to the nearest interval", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO)
    {
        return;
    }

    CustomTimingDialog dialog(this);
    int start_ms = xml_file->GetFrameMS();
    dialog.SetTiming(start_ms);

    if( dialog.ShowModal() == wxID_OK )
    {
        if( start_ms < dialog.GetValue() )
        {
            if (wxMessageBox("Are you sure? Moving to a larger timing interval may truncate small effects?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO)
            {
                return;
            }
        }

        TextCtrl_SeqTiming->SetValue(dialog.GetTiming());
        xml_file->SetSequenceTiming(dialog.GetTiming());
        xLightsParent->SetSequenceTiming(wxAtoi(dialog.GetTiming()));
        xLightsParent->SaveSequence();
        wxString name = xml_file->GetFullPath();
        xLightsParent->CloseSequence();
        xLightsParent->OpenSequence( name, nullptr );
        xml_file = xLightsParent->CurrentSeqXmlFile;
    }
}

void SeqSettingsDialog::OnButton_DownloadClick(wxCommandEvent& event)
{
    if (xLightsParent->CurrentDir == "") {
        wxMessageBox("Show folder invalid. Download aborted.");
    }

    wxString downloadDir = xLightsParent->CurrentDir + wxFileName::GetPathSeparator() + "Downloads";

    if (!wxDirExists(downloadDir))
    {
        wxMkdir(downloadDir);
    }

    VendorMusicDialog dlg(this);
    if (dlg.DlgInit(TextCtrl_Hash->GetValue().ToStdString(), downloadDir))
    {
        dlg.ShowModal();
    }
    else
    {
        DisplayError("Nothing available for this song.", this);
    }
}

void SeqSettingsDialog::OnRenderModeChoiceSelect(wxCommandEvent& event)
{
    xml_file->SetRenderMode(RenderModeChoice->GetStringSelection());
    UpdateDataLayer();
}

void SeqSettingsDialog::ValidateWindow()
{
    bool ok = true;
    double duration = wxAtof(TextCtrl_Xml_Seq_Duration->GetValue());
    if (duration <= 0.0) {
        ok = false;
        TextCtrl_Xml_Seq_Duration->SetBackgroundColour(*wxRED);
        TextCtrl_Xml_Seq_Duration->SetHelpText("Sequence length is invalid.");
    }
    else if (duration > 3600.0) {
        TextCtrl_Xml_Seq_Duration->SetBackgroundColour(xlORANGE.asWxColor());
        TextCtrl_Xml_Seq_Duration->SetHelpText("Sequence is excessively long and may crash xLights.");
    }
    else {
        TextCtrl_Xml_Seq_Duration->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        TextCtrl_Xml_Seq_Duration->SetHelpText("");
    }

    if (Button_WizardDone != nullptr) Button_WizardDone->Enable(ok);
    Button_Close->Enable(ok);
    if (BitmapButton_quick_start != nullptr) BitmapButton_quick_start->Enable(ok);
}

void SeqSettingsDialog::OnButton_EmailSendClick(wxCommandEvent& event)
{
    auto link = TextCtrl_Xml_Author_Email->GetValue().Trim();
    if (!link.Lower().StartsWith("mailto")) {
        link = "mailto://" + link;
    }
    ::wxLaunchDefaultBrowser(link);
}

void SeqSettingsDialog::OnButton_WebsiteOpenClick(wxCommandEvent& event)
{
    auto link = TextCtrl_Xml_Website->GetValue().Trim();
    if (!link.Lower().StartsWith("http")) {
        link = "http://" + link;
    }
    ::wxLaunchDefaultBrowser(link);
}

void SeqSettingsDialog::OnButton_MusicOpenClick(wxCommandEvent& event)
{
    auto link = TextCtrl_Xml_Music_Url->GetValue().Trim();
    if (!link.Lower().StartsWith("http"))
    {
        link = "http://" + link;
    }
    ::wxLaunchDefaultBrowser(link);
}

void SeqSettingsDialog::OnButton_AddMillisecondsClick(wxCommandEvent& event) {
    const std::string inputFile = TextCtrl_Xml_Media_File->GetValue();
    wxFileName inFile(inputFile);
    const std::string mp3dur = TextCtrl_Xml_Seq_Duration->GetValue();
    const std::string pre = TextCtrl_Premilliseconds->GetValue().Trim(true).Trim(false);
    const std::string post = TextCtrl_Postmilliseconds->GetValue().Trim(true).Trim(false);

    if (pre.empty() || post.empty()) {
        wxMessageBox("Invalid Pre/Post value(s). Neither can be blank, 0 is okay.");
        return;
    }
    auto const f_pre = std::stof(pre);
    auto const f_post = std::stof(post);
    auto const f_mp3dur = std::stof(mp3dur);
    if (fp_equal(0.0F, f_post) && fp_equal(0.0F, f_pre)) {
        wxMessageBox("Pre and Post value(s) are Zero. Nothing to Do.");
        return;
    }
    if (fp_equal(0.0F, f_mp3dur)) {
        wxMessageBox("Audio Duration is Invalid.");
        return;
    }
    if ((0.0F > f_post) || (0.0F > f_pre)) {
        wxMessageBox("Pre and Post value(s) can't be less than Zero.");
        return;
    }

    std::string outputFile;
    size_t const lastDot = inputFile.find_last_of('.');
    // If there is an extension, insert "silence" before the extension, else after
    if (lastDot != std::string::npos) {
        outputFile = inputFile.substr(0, lastDot) + "_" + pre + "_" + post + inputFile.substr(lastDot);
    } else {
        outputFile = inputFile + "_" + pre + "_" + post;
    }
    wxFileName targetFile(outputFile);

    struct musicEdit {
        std::string file;
        double start;
        double length;
        double sourceoffset;
        double fadein;
        double fadeout;
        double volume;
        bool crossfadein;
        bool crossfadeout;
        musicEdit(const std::string& f, double s, double l, double so, double fi, double fo, double v, bool cfi, bool cfo) :
            file(f), start(s), length(l), sourceoffset(so), fadein(fi), fadeout(fo), volume(v), crossfadein(cfi), crossfadeout(cfo) {
        }
    };
    std::list<musicEdit> edits;
    AudioManager* firstAudio = nullptr;
    edits.push_back(musicEdit(inFile.GetFullName(), 0, f_pre / 1000, 0, 0, 0, 0, false, false));
    edits.push_back(musicEdit(inFile.GetFullName(), f_pre / 1000, f_mp3dur, 0, 0, 0, 1, false, false));
    edits.push_back(musicEdit(inFile.GetFullName(), f_pre / 1000 + f_mp3dur, f_post / 1000, 0, 0, 0, 0, false, false));

    wxString music = inputFile;

    std::map<std::string, AudioManager*> sourceSongs;
    double outputLength{ 0.0 };
    long sampleRate{ -1 };
    for (const auto& it : edits) {
        outputLength = std::max(outputLength, it.start + it.length);
        sourceSongs[it.file] = new AudioManager(music);
        if (firstAudio == nullptr) {
            firstAudio = sourceSongs[it.file];
        }
    }
    for (const auto& it : sourceSongs) {
        sampleRate = it.second->GetRate();
    }

    long const totalSamples = sampleRate * outputLength;
    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    for (const auto& it : edits) {
        auto audio = sourceSongs[it.file];
        if (audio != nullptr) {
            if (audio->GetFrameInterval() < 0) {
                audio->SetFrameInterval(20);
            }
            audio->GetRawLeftData(audio->GetTrackSize() - 1);

            long const startOutput = sampleRate * it.start;
            long const outputSamples = sampleRate * it.length;
            wxASSERT(startOutput + outputSamples - 1 <= totalSamples);
            long const startSample = audio->GetRate() * it.sourceoffset;
            long const inputSamples = audio->GetRate() * it.length;
            wxASSERT(startSample + inputSamples - 1 < audio->GetTrackSize());
            wxASSERT(inputSamples == outputSamples);

            float* lsource = audio->GetRawLeftDataPtr(startSample);
            float* rsource = audio->GetRawRightDataPtr(startSample);
            long const fadeinsamples = it.fadein * audio->GetRate();
            long const fadeoutsamples = it.fadeout * audio->GetRate();
            long const fadeoutstart = inputSamples - fadeoutsamples;

            for (long i = 0; i < inputSamples; i++) {
                float l = lsource[i] * it.volume;
                float r;
                if (rsource != nullptr) {
                    r = rsource[i] * it.volume;
                } else {
                    r = l;
                }
                if (i < fadeinsamples) {
                    if (it.crossfadein) {
                        double f = log10((double)i / fadeinsamples + 0.1) * 10.0 / 11.0;
                        if (f < 0) {
                            f = 0.0;
                        }
                        if (f > 1) {
                            f = 1.0;
                        }
                        l *= f;
                        r *= f;
                    } else {
                        double f = pow(10.0, ((double)i / fadeinsamples - 1.0) - 0.1) * 1.1;
                        if (f < 0) {
                            f = 0.0;
                        }
                        if (f > 1) {
                            f = 1.0;
                        }
                        l *= f;
                        r *= f;
                    }
                }
                if (i > fadeoutstart) {
                    if (it.crossfadeout) {
                        double f = 1.0 - log10((double)(inputSamples - i) / fadeinsamples + 0.1) * 10.0 / 11.0;
                        if (f < 0) {
                            f = 0.0;
                        }
                        if (f > 1) {
                            f = 1.0;
                        }
                        l *= f;
                        r *= f;
                    } else {
                        double f = 1.0 - pow(10.0, ((double)(inputSamples - i) / fadeinsamples - 1.0) - 0.1) * 1.1;
                        if (f < 0) {
                            f = 0.0;
                        }
                        if (f > 1) {
                            f = 1.0;
                        }
                        l *= f;
                        r *= f;
                    }
                }
                left[startOutput + i] += l;
                right[startOutput + i] += r;
            }
        }
    }

    // Clip it
    for (auto& it : left) {
        if (it > 1.0) {
            it = 1.0;
        }
    }
    for (auto& it : right) {
        if (it > 1.0) {
            it = 1.0;
        }
    }

#ifdef __WXOSX__
    // Cannot generate MP3's, change to AAC/m4a (which has better quality anyway)
    wxFileName fn = targetFile;
    if (fn.GetExt() == "mp3") {
        fn.SetExt("m4a");
        targetFile = fn.GetFullPath();
    }
#endif

    if (!AudioManager::EncodeAudio(left,
                                    right,
                                    sampleRate,
                                    targetFile.GetFullPath(),
                                    firstAudio)) {
        wxMessageBox("Error creating audio file. See log for details.");
    } else {
        wxMessageBox("Audio file created successfully, " + targetFile.GetFullPath() + " Please choose the new media file for your sequence.");
    }
}
