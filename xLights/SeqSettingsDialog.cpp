#include <wx/filedlg.h>
#include "SeqSettingsDialog.h"
#include "RenameTextDialog.h"
#include "NewTimingDialog.h"
#include "xLightsXmlFile.h"
#include "DataLayer.h"
#include "FileConverter.h"
#include "LorConvertDialog.h"
#include <wx/treectrl.h>
#include "Images_png.h"

//(*InternalHeaders(SeqSettingsDialog)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(SeqSettingsDialog)
const long SeqSettingsDialog::ID_STATICTEXT_File = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Filename = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_XML_Type_Version = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_XML_Version = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Num_Models_Label = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Num_Models = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Xml_Seq_Type = wxNewId();
const long SeqSettingsDialog::ID_CHOICE_Xml_Seq_Type = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Xml_MediaFile = wxNewId();
const long SeqSettingsDialog::ID_TEXTCTRL_Xml_Media_File = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_Xml_Media_File = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Xml_Total_Length = wxNewId();
const long SeqSettingsDialog::ID_TEXTCTRL_Xml_Seq_Duration = wxNewId();
const long SeqSettingsDialog::ID_CHECKBOX_Overwrite_Tags = wxNewId();
const long SeqSettingsDialog::ID_CHOICE_Xml_Seq_Timing = wxNewId();
const long SeqSettingsDialog::ID_PANEL3 = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Xml_Author = wxNewId();
const long SeqSettingsDialog::ID_TEXTCTRL_Xml_Author = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Xml_Author_Email = wxNewId();
const long SeqSettingsDialog::ID_TEXTCTRL_Xml_Author_Email = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Xml_Website = wxNewId();
const long SeqSettingsDialog::ID_TEXTCTRL_Xml_Website = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Xml_Song = wxNewId();
const long SeqSettingsDialog::ID_TEXTCTRL_Xml_Song = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Xml_Artist = wxNewId();
const long SeqSettingsDialog::ID_TEXTCTRL_Xml_Artist = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Xml_Album = wxNewId();
const long SeqSettingsDialog::ID_TEXTCTRL_Xml_Album = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Xml_Music_Url = wxNewId();
const long SeqSettingsDialog::ID_TEXTCTRL_Xml_Music_Url = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Xml_Comment = wxNewId();
const long SeqSettingsDialog::ID_TEXTCTRL_Xml_Comment = wxNewId();
const long SeqSettingsDialog::ID_PANEL1 = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_Xml_New_Timing = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_Xml_Import_Timing = wxNewId();
const long SeqSettingsDialog::ID_PANEL2 = wxNewId();
const long SeqSettingsDialog::ID_TREECTRL_Data_Layers = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_Layer_Import = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_Layer_Delete = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_Move_Up = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_Move_Down = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_Reimport = wxNewId();
const long SeqSettingsDialog::ID_PANEL4 = wxNewId();
const long SeqSettingsDialog::ID_NOTEBOOK_Seq_Settings = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Warning = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Warn_No_Media = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_Close = wxNewId();
//*)

const long SeqSettingsDialog::ID_GRID_TIMING = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_Wiz_Music = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_Wiz_Anim = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_25ms = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_50ms = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_100ms = wxNewId();
const long SeqSettingsDialog::ID_PANEL_Wizard = wxNewId();

const long SeqSettingsDialog::ID_BITMAPBUTTON_lor = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_vixen = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_gled = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_hls = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_lynx = wxNewId();
const long SeqSettingsDialog::ID_BITMAPBUTTON_xlights = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_skip_import = wxNewId();

wxDEFINE_EVENT(EVT_DELETE_ROW, wxCommandEvent);
wxDEFINE_EVENT(EVT_NAME_CHANGE, wxCommandEvent);

BEGIN_EVENT_TABLE(FlickerFreeBitmapButtonCtrl,wxBitmapButton)
    EVT_ERASE_BACKGROUND(FlickerFreeBitmapButtonCtrl::OnEraseBackGround)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(SeqSettingsDialog,wxDialog)
	//(*EventTable(SeqSettingsDialog)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_NAME_CHANGE, SeqSettingsDialog::OnButton_Xml_Rename_TimingClick)
    EVT_COMMAND(wxID_ANY, EVT_DELETE_ROW, SeqSettingsDialog::OnButton_Xml_Delete_TimingClick)
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

SeqSettingsDialog::SeqSettingsDialog(wxWindow* parent, xLightsXmlFile* file_to_handle_, wxString& media_dir, const wxString& warning, bool wizard_active_)
:   xml_file(file_to_handle_),
    media_directory(media_dir),
    xLightsParent((xLightsFrame*)parent),
    selected_branch_index(-1),
    wizard_active(wizard_active_)
{
    music_seq = wxBITMAP_PNG_FROM_DATA(music);
    music_seq_pressed = wxBITMAP_PNG_FROM_DATA(music_pressed);
    animation_seq = wxBITMAP_PNG_FROM_DATA(animation);
    animation_pressed = wxBITMAP_PNG_FROM_DATA(animation_pressed);
    time_div_25ms = wxBITMAP_PNG_FROM_DATA(time_25ms);
    time_div_25ms_pressed = wxBITMAP_PNG_FROM_DATA(time_25ms_pressed);
    time_div_50ms = wxBITMAP_PNG_FROM_DATA(time_50ms);
    time_div_50ms_pressed = wxBITMAP_PNG_FROM_DATA(time_50ms_pressed);
    time_div_100ms = wxBITMAP_PNG_FROM_DATA(time_100ms);
    time_div_100ms_pressed = wxBITMAP_PNG_FROM_DATA(time_100ms_pressed);
    lightorama = wxBITMAP_PNG_FROM_DATA(lightorama);
    vixen = wxBITMAP_PNG_FROM_DATA(vixen);
    glediator = wxBITMAP_PNG_FROM_DATA(glediator);
    hls = wxBITMAP_PNG_FROM_DATA(hls);
    lynx = wxBITMAP_PNG_FROM_DATA(lynx);
    xlights_logo = wxBITMAP_PNG_FROM_DATA(xlights_logo);

    //(*Initialize(SeqSettingsDialog)
    wxFlexGridSizer* FlexGridSizer4;
    wxGridBagSizer* GridBagSizer1;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer3;
    wxStaticText* StaticText_Xml_Seq_Timing;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer11;
    wxFlexGridSizer* FlexGridSizer_Timing_Grid;
    wxFlexGridSizer* FlexGridSizer_Timing_Page;

    Create(parent, wxID_ANY, _("Sequence Settings"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    Notebook_Seq_Settings = new wxNotebook(this, ID_NOTEBOOK_Seq_Settings, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_Seq_Settings"));
    Panel3 = new wxPanel(Notebook_Seq_Settings, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
    GridBagSizer1 = new wxGridBagSizer(0, 0);
    StaticText_File = new wxStaticText(Panel3, ID_STATICTEXT_File, _("Filename:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_File"));
    GridBagSizer1->Add(StaticText_File, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Filename = new wxStaticText(Panel3, ID_STATICTEXT_Filename, wxEmptyString, wxDefaultPosition, wxSize(331,13), 0, _T("ID_STATICTEXT_Filename"));
    GridBagSizer1->Add(StaticText_Filename, wxGBPosition(0, 1), wxGBSpan(1, 4), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_XML_Type_Version = new wxStaticText(Panel3, ID_STATICTEXT_XML_Type_Version, _("XML Version:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_XML_Type_Version"));
    GridBagSizer1->Add(StaticText_XML_Type_Version, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_XML_Version = new wxStaticText(Panel3, ID_STATICTEXT_XML_Version, wxEmptyString, wxDefaultPosition, wxSize(70,-1), 0, _T("ID_STATICTEXT_XML_Version"));
    GridBagSizer1->Add(StaticText_XML_Version, wxGBPosition(1, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridBagSizer1->Add(40,20,1, wxALL|wxFIXED_MINSIZE|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Num_Models_Label = new wxStaticText(Panel3, ID_STATICTEXT_Num_Models_Label, _("# Models:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Num_Models_Label"));
    GridBagSizer1->Add(StaticText_Num_Models_Label, wxGBPosition(1, 3), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Num_Models = new wxStaticText(Panel3, ID_STATICTEXT_Num_Models, wxEmptyString, wxDefaultPosition, wxSize(70,-1), 0, _T("ID_STATICTEXT_Num_Models"));
    GridBagSizer1->Add(StaticText_Num_Models, wxGBPosition(1, 4), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(GridBagSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText_Xml_Seq_Type = new wxStaticText(Panel3, ID_STATICTEXT_Xml_Seq_Type, _("Sequence Type:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Seq_Type"));
    FlexGridSizer5->Add(StaticText_Xml_Seq_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Xml_Seq_Type = new wxChoice(Panel3, ID_CHOICE_Xml_Seq_Type, wxDefaultPosition, wxSize(125,21), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Xml_Seq_Type"));
    Choice_Xml_Seq_Type->Append(_("Media"));
    Choice_Xml_Seq_Type->Append(_("Animation"));
    FlexGridSizer5->Add(Choice_Xml_Seq_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer10 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText_Xml_MediaFile = new wxStaticText(Panel3, ID_STATICTEXT_Xml_MediaFile, _("Media:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_MediaFile"));
    FlexGridSizer10->Add(StaticText_Xml_MediaFile, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Media_File = new wxTextCtrl(Panel3, ID_TEXTCTRL_Xml_Media_File, wxEmptyString, wxDefaultPosition, wxSize(326,21), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Media_File"));
    FlexGridSizer10->Add(TextCtrl_Xml_Media_File, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Xml_Media_File = new wxBitmapButton(Panel3, ID_BITMAPBUTTON_Xml_Media_File, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_CDROM")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Xml_Media_File"));
    BitmapButton_Xml_Media_File->Disable();
    FlexGridSizer10->Add(BitmapButton_Xml_Media_File, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6 = new wxFlexGridSizer(0, 4, 0, 0);
    StaticText_Xml_Total_Length = new wxStaticText(Panel3, ID_STATICTEXT_Xml_Total_Length, _("Sequence Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Total_Length"));
    FlexGridSizer6->Add(StaticText_Xml_Total_Length, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Seq_Duration = new wxTextCtrl(Panel3, ID_TEXTCTRL_Xml_Seq_Duration, wxEmptyString, wxDefaultPosition, wxSize(81,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Seq_Duration"));
    FlexGridSizer6->Add(TextCtrl_Xml_Seq_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6->Add(72,20,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Overwrite_Tags = new wxCheckBox(Panel3, ID_CHECKBOX_Overwrite_Tags, _("Overwrite Media Tags"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Overwrite_Tags"));
    CheckBox_Overwrite_Tags->SetValue(false);
    FlexGridSizer6->Add(CheckBox_Overwrite_Tags, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText_Xml_Seq_Timing = new wxStaticText(Panel3, wxID_ANY, _("Sequence Timing:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer3->Add(StaticText_Xml_Seq_Timing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Xml_Seq_Timing = new wxChoice(Panel3, ID_CHOICE_Xml_Seq_Timing, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Xml_Seq_Timing"));
    Choice_Xml_Seq_Timing->Append(_("25 ms"));
    Choice_Xml_Seq_Timing->SetSelection( Choice_Xml_Seq_Timing->Append(_("50 ms")) );
    Choice_Xml_Seq_Timing->Append(_("100 ms"));
    FlexGridSizer3->Add(Choice_Xml_Seq_Timing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Panel3->SetSizer(FlexGridSizer4);
    FlexGridSizer4->Fit(Panel3);
    FlexGridSizer4->SetSizeHints(Panel3);
    Panel1 = new wxPanel(Notebook_Seq_Settings, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer_Timing_Page = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText_Xml_Author = new wxStaticText(Panel1, ID_STATICTEXT_Xml_Author, _("Author:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Author"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Author, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Author = new wxTextCtrl(Panel1, ID_TEXTCTRL_Xml_Author, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Author"));
    FlexGridSizer_Timing_Page->Add(TextCtrl_Xml_Author, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Xml_Author_Email = new wxStaticText(Panel1, ID_STATICTEXT_Xml_Author_Email, _("Email:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Author_Email"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Author_Email, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Author_Email = new wxTextCtrl(Panel1, ID_TEXTCTRL_Xml_Author_Email, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Author_Email"));
    FlexGridSizer_Timing_Page->Add(TextCtrl_Xml_Author_Email, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Xml_Website = new wxStaticText(Panel1, ID_STATICTEXT_Xml_Website, _("Website:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Website"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Website, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Website = new wxTextCtrl(Panel1, ID_TEXTCTRL_Xml_Website, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Website"));
    FlexGridSizer_Timing_Page->Add(TextCtrl_Xml_Website, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Xml_Song = new wxStaticText(Panel1, ID_STATICTEXT_Xml_Song, _("Song:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Song"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Song, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Song = new wxTextCtrl(Panel1, ID_TEXTCTRL_Xml_Song, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Song"));
    FlexGridSizer_Timing_Page->Add(TextCtrl_Xml_Song, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Xml_Artist = new wxStaticText(Panel1, ID_STATICTEXT_Xml_Artist, _("Artist:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Artist"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Artist, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Artist = new wxTextCtrl(Panel1, ID_TEXTCTRL_Xml_Artist, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Artist"));
    FlexGridSizer_Timing_Page->Add(TextCtrl_Xml_Artist, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Xml_Album = new wxStaticText(Panel1, ID_STATICTEXT_Xml_Album, _("Album:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Album"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Album, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Album = new wxTextCtrl(Panel1, ID_TEXTCTRL_Xml_Album, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Album"));
    FlexGridSizer_Timing_Page->Add(TextCtrl_Xml_Album, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Xml_Music_Url = new wxStaticText(Panel1, ID_STATICTEXT_Xml_Music_Url, _("Music URL:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Music_Url"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Music_Url, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Music_Url = new wxTextCtrl(Panel1, ID_TEXTCTRL_Xml_Music_Url, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Music_Url"));
    FlexGridSizer_Timing_Page->Add(TextCtrl_Xml_Music_Url, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Xml_Comment = new wxStaticText(Panel1, ID_STATICTEXT_Xml_Comment, _("Comment:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Comment"));
    FlexGridSizer_Timing_Page->Add(StaticText_Xml_Comment, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Xml_Comment = new wxTextCtrl(Panel1, ID_TEXTCTRL_Xml_Comment, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Comment"));
    FlexGridSizer_Timing_Page->Add(TextCtrl_Xml_Comment, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1->SetSizer(FlexGridSizer_Timing_Page);
    FlexGridSizer_Timing_Page->Fit(Panel1);
    FlexGridSizer_Timing_Page->SetSizeHints(Panel1);
    Panel2 = new wxPanel(Notebook_Seq_Settings, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer_Timing_Grid = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer8->Add(FlexGridSizer_Timing_Grid, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    Button_Xml_New_Timing = new wxButton(Panel2, ID_BUTTON_Xml_New_Timing, _("New"), wxDefaultPosition, wxSize(60,23), 0, wxDefaultValidator, _T("ID_BUTTON_Xml_New_Timing"));
    FlexGridSizer2->Add(Button_Xml_New_Timing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Xml_Import_Timing = new wxButton(Panel2, ID_BUTTON_Xml_Import_Timing, _("Import"), wxDefaultPosition, wxSize(60,23), 0, wxDefaultValidator, _T("ID_BUTTON_Xml_Import_Timing"));
    FlexGridSizer2->Add(Button_Xml_Import_Timing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(412,20,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel2->SetSizer(FlexGridSizer8);
    FlexGridSizer8->Fit(Panel2);
    FlexGridSizer8->SetSizeHints(Panel2);
    Panel4 = new wxPanel(Notebook_Seq_Settings, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
    TreeCtrl_Data_Layers = new wxTreeCtrl(Panel4, ID_TREECTRL_Data_Layers, wxDefaultPosition, wxSize(413,158), wxTR_EDIT_LABELS|wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL_Data_Layers"));
    wxTreeItemId TreeCtrl_Data_Layers_Item1 = TreeCtrl_Data_Layers->AddRoot(_T("Layers to Render"));
    FlexGridSizer9->Add(TreeCtrl_Data_Layers, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer11 = new wxFlexGridSizer(0, 4, 0, 0);
    Button_Layer_Import = new wxButton(Panel4, ID_BUTTON_Layer_Import, _("Import"), wxDefaultPosition, wxSize(60,23), 0, wxDefaultValidator, _T("ID_BUTTON_Layer_Import"));
    FlexGridSizer11->Add(Button_Layer_Import, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Layer_Delete = new wxButton(Panel4, ID_BUTTON_Layer_Delete, _("Delete"), wxDefaultPosition, wxSize(60,23), 0, wxDefaultValidator, _T("ID_BUTTON_Layer_Delete"));
    Button_Layer_Delete->Disable();
    FlexGridSizer11->Add(Button_Layer_Delete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Move_Up = new wxButton(Panel4, ID_BUTTON_Move_Up, _("Move Up"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Move_Up"));
    Button_Move_Up->Disable();
    FlexGridSizer11->Add(Button_Move_Up, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Move_Down = new wxButton(Panel4, ID_BUTTON_Move_Down, _("Move Down"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Move_Down"));
    Button_Move_Down->Disable();
    FlexGridSizer11->Add(Button_Move_Down, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Reimport = new wxButton(Panel4, ID_BUTTON_Reimport, _("Re-Import"), wxDefaultPosition, wxSize(60,23), 0, wxDefaultValidator, _T("ID_BUTTON_Reimport"));
    Button_Reimport->Disable();
    FlexGridSizer11->Add(Button_Reimport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(FlexGridSizer11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel4->SetSizer(FlexGridSizer9);
    FlexGridSizer9->Fit(Panel4);
    FlexGridSizer9->SetSizeHints(Panel4);
    Notebook_Seq_Settings->AddPage(Panel3, _("Info / Media"), false);
    Notebook_Seq_Settings->AddPage(Panel1, _("Meta Data"), false);
    Notebook_Seq_Settings->AddPage(Panel2, _("Timings"), false);
    Notebook_Seq_Settings->AddPage(Panel4, _("Data Layers"), false);
    FlexGridSizer1->Add(Notebook_Seq_Settings, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Warning = new wxStaticText(this, ID_STATICTEXT_Warning, _("Show Warning Here"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Warning"));
    StaticText_Warning->Hide();
    StaticText_Warning->SetForegroundColour(wxColour(255,0,0));
    wxFont StaticText_WarningFont(wxDEFAULT,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText_Warning->SetFont(StaticText_WarningFont);
    FlexGridSizer1->Add(StaticText_Warning, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Warn_No_Media = new wxStaticText(this, ID_STATICTEXT_Warn_No_Media, _("Media File must be selected or change to animation!"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Warn_No_Media"));
    StaticText_Warn_No_Media->Hide();
    StaticText_Warn_No_Media->SetForegroundColour(wxColour(255,0,0));
    wxFont StaticText_Warn_No_MediaFont(wxDEFAULT,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText_Warn_No_Media->SetFont(StaticText_Warn_No_MediaFont);
    FlexGridSizer1->Add(StaticText_Warn_No_Media, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
    Button_Close = new wxButton(this, ID_BUTTON_Close, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Close"));
    FlexGridSizer7->Add(Button_Close, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer7, 1, wxLEFT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHOICE_Xml_Seq_Type,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SeqSettingsDialog::OnChoice_Xml_Seq_TypeSelect);
    Connect(ID_BITMAPBUTTON_Xml_Media_File,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_Xml_Media_FileClick);
    Connect(ID_TEXTCTRL_Xml_Seq_Duration,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_Seq_DurationText);
    Connect(ID_CHOICE_Xml_Seq_Timing,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SeqSettingsDialog::OnChoice_Xml_Seq_TimingSelect);
    Connect(ID_TEXTCTRL_Xml_Author,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_AuthorText);
    Connect(ID_TEXTCTRL_Xml_Author_Email,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_Author_EmailText);
    Connect(ID_TEXTCTRL_Xml_Website,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_WebsiteText);
    Connect(ID_TEXTCTRL_Xml_Song,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_SongText);
    Connect(ID_TEXTCTRL_Xml_Artist,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_ArtistText);
    Connect(ID_TEXTCTRL_Xml_Album,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_AlbumText);
    Connect(ID_TEXTCTRL_Xml_Music_Url,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_Music_UrlText);
    Connect(ID_TEXTCTRL_Xml_Comment,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SeqSettingsDialog::OnTextCtrl_Xml_CommentText);
    Connect(ID_BUTTON_Xml_New_Timing,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_Xml_New_TimingClick);
    Connect(ID_BUTTON_Xml_Import_Timing,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_Xml_Import_TimingClick);
    Connect(ID_TREECTRL_Data_Layers,wxEVT_COMMAND_TREE_BEGIN_DRAG,(wxObjectEventFunction)&SeqSettingsDialog::OnTreeCtrl_Data_LayersBeginDrag);
    Connect(ID_TREECTRL_Data_Layers,wxEVT_COMMAND_TREE_BEGIN_LABEL_EDIT,(wxObjectEventFunction)&SeqSettingsDialog::OnTreeCtrl_Data_LayersBeginLabelEdit);
    Connect(ID_TREECTRL_Data_Layers,wxEVT_COMMAND_TREE_END_LABEL_EDIT,(wxObjectEventFunction)&SeqSettingsDialog::OnTreeCtrl_Data_LayersEndLabelEdit);
    Connect(ID_TREECTRL_Data_Layers,wxEVT_COMMAND_TREE_SEL_CHANGED,(wxObjectEventFunction)&SeqSettingsDialog::OnTreeCtrl_Data_LayersSelectionChanged);
    Connect(ID_BUTTON_Layer_Import,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_Layer_ImportClick);
    Connect(ID_BUTTON_Layer_Delete,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_Layer_DeleteClick);
    Connect(ID_BUTTON_Move_Up,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_Move_UpClick);
    Connect(ID_BUTTON_Move_Down,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_Move_DownClick);
    Connect(ID_BUTTON_Reimport,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_ReimportClick);
    Connect(ID_BUTTON_Close,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_CloseClick);
    //*)

    if( wizard_active )
    {
        WizardPage1();
    }

	if( warning != "" )
    {
        StaticText_Warning->SetLabelText(warning);
        StaticText_Warning->Show();
    }

    xml_file->AcknowledgeConversion();

	StaticText_Filename->SetLabelText(xml_file->GetFullPath());
	ProcessSequenceType();

	// Setup Grid
	Grid_Timing = new tmGrid(Panel2, ID_GRID_TIMING, wxDefaultPosition, wxSize(390,150), wxBORDER_SIMPLE, _T("ID_GRID_TIMING"));
	FlexGridSizer_Timing_Grid->Add(Grid_Timing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 15);

	FlexGridSizer8->Fit(Panel2);
	FlexGridSizer8->SetSizeHints(Panel2);
	Grid_Timing->DisableDragGridSize();
	Grid_Timing->DisableDragRowSize();
	Grid_Timing->DisableDragColSize();
    Grid_Timing->CreateGrid(0, 2);
	int total_width = Grid_Timing->GetSize().GetWidth();
	Grid_Timing->HideRowLabels();
	Grid_Timing->SetDefaultRowSize(25, true);

	Grid_Timing->SetColumnWidth(0, 342);
	Grid_Timing->SetColumnWidth(1, 25);
	//wxGridCellButtonRenderer* btn1 = new wxGridCellButtonRenderer("");
	//Grid_Timing->SetCellRenderer(0,0, btn1);
	Grid_Timing->SetColLabelValue(0, "Timing Grids");
	Grid_Timing->SetColLabelValue(1, "");
	PopulateTimingGrid();

    StaticText_XML_Version->SetLabelText(xml_file->GetVersion());
    StaticText_Num_Models->SetLabelText(string_format("%d",xml_file->GetNumModels()));
    TextCtrl_Xml_Author->SetValue(xml_file->GetHeaderInfo(xLightsXmlFile::AUTHOR));
    TextCtrl_Xml_Author_Email->SetValue(xml_file->GetHeaderInfo(xLightsXmlFile::AUTHOR_EMAIL));
    TextCtrl_Xml_Website->SetValue(xml_file->GetHeaderInfo(xLightsXmlFile::WEBSITE));
    TextCtrl_Xml_Song->SetValue(xml_file->GetHeaderInfo(xLightsXmlFile::SONG));
    TextCtrl_Xml_Artist->SetValue(xml_file->GetHeaderInfo(xLightsXmlFile::ARTIST));
    TextCtrl_Xml_Album->SetValue(xml_file->GetHeaderInfo(xLightsXmlFile::ALBUM));
    TextCtrl_Xml_Music_Url->SetValue(xml_file->GetHeaderInfo(xLightsXmlFile::URL));
    TextCtrl_Xml_Comment->SetValue(xml_file->GetHeaderInfo(xLightsXmlFile::COMMENT));
    Choice_Xml_Seq_Type->SetSelection(Choice_Xml_Seq_Type->FindString(xml_file->GetSequenceType()));
    Choice_Xml_Seq_Timing->SetSelection(Choice_Xml_Seq_Timing->FindString(xml_file->GetSequenceTiming()));
    TextCtrl_Xml_Media_File->SetValue(xml_file->GetMediaFile());
    TextCtrl_Xml_Seq_Duration->SetValue(xml_file->GetSequenceDurationString());

    DataLayerSet& data_layers = xml_file->GetDataLayers();
    wxTreeItemId root = TreeCtrl_Data_Layers->GetRootItem();

    for( int i = 0; i < data_layers.GetNumLayers(); ++i )
    {
        DataLayer* layer = data_layers.GetDataLayer(i);
        wxTreeItemId branch = TreeCtrl_Data_Layers->AppendItem(root, layer->GetName(), -1, -1, new LayerTreeItemData(layer));
        TreeCtrl_Data_Layers->AppendItem(branch, "Source: " + layer->GetSource());
        TreeCtrl_Data_Layers->AppendItem(branch, "Data: " + layer->GetDataSource());
        TreeCtrl_Data_Layers->AppendItem(branch, wxString::Format("Number of Channels: %d", layer->GetNumChannels()));
        TreeCtrl_Data_Layers->AppendItem(branch, wxString::Format("Channel Offset: %d", layer->GetChannelOffset()));
    }
    TreeCtrl_Data_Layers->Expand(root);

    UpdateDataLayer();
}

SeqSettingsDialog::~SeqSettingsDialog()
{
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
    Panel_Wizard = new wxPanel(Notebook_Seq_Settings, ID_PANEL_Wizard, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Wizard"));
    Notebook_Seq_Settings->InsertPage(0, Panel_Wizard, _("Wizard"), true);
    GridBagSizerWizard = new wxGridBagSizer(0, 1);
    GridBagSizerWizard->Add(493,16,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridSizerWizButtons = new wxGridSizer(0, 1, 10, 0);
    BitmapButton_Wiz_Music = new FlickerFreeBitmapButtonCtrl(Panel_Wizard, ID_BITMAPBUTTON_Wiz_Music, music_seq, wxDefaultPosition, wxSize(268,90), wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Wiz_Music"));
    BitmapButton_Wiz_Music->SetBitmapSelected(music_seq_pressed);
    GridSizerWizButtons->Add(BitmapButton_Wiz_Music, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Wiz_Anim = new FlickerFreeBitmapButtonCtrl(Panel_Wizard, ID_BITMAPBUTTON_Wiz_Anim, animation_seq, wxDefaultPosition, wxSize(268,90), wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Wiz_Anim"));
    BitmapButton_Wiz_Anim->SetBitmapSelected(animation_pressed);
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
    GridBagSizerWizard->Clear(true);
    GridBagSizerWizard->Add(493,16,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridSizerWizButtons = new wxGridSizer(0, 1, 10, 0);
    BitmapButton_25ms = new FlickerFreeBitmapButtonCtrl(Panel_Wizard, ID_BITMAPBUTTON_25ms, time_div_25ms, wxDefaultPosition, wxSize(185,50), wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_25ms"));
    BitmapButton_25ms->SetBitmapSelected(time_div_25ms_pressed);
    GridSizerWizButtons->Add(BitmapButton_25ms, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_50ms = new FlickerFreeBitmapButtonCtrl(Panel_Wizard, ID_BITMAPBUTTON_50ms, time_div_50ms, wxDefaultPosition, wxSize(185,50), wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_50ms"));
    BitmapButton_50ms->SetBitmapSelected(time_div_50ms_pressed);
    GridSizerWizButtons->Add(BitmapButton_50ms, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_100ms = new FlickerFreeBitmapButtonCtrl(Panel_Wizard, ID_BITMAPBUTTON_100ms, time_div_100ms, wxDefaultPosition, wxSize(185,50), wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_100ms"));
    BitmapButton_100ms->SetBitmapSelected(time_div_100ms_pressed);
    GridSizerWizButtons->Add(BitmapButton_100ms, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridBagSizerWizard->Add(GridSizerWizButtons, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel_Wizard->SetSizer(GridBagSizerWizard);
    GridBagSizerWizard->Fit(Panel_Wizard);
    GridBagSizerWizard->SetSizeHints(Panel_Wizard);

    Connect(ID_BITMAPBUTTON_25ms,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_25msClick);
    Connect(ID_BITMAPBUTTON_50ms,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_50msClick);
    Connect(ID_BITMAPBUTTON_100ms,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnBitmapButton_100msClick);
}

void SeqSettingsDialog::WizardPage3()
{
    GridBagSizerWizard->Clear(true);
    GridBagSizerWizard->Add(493,1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    wxStaticText* StaticText_Page3Optional = new wxStaticText(Panel_Wizard, wxID_ANY, _("Import Data (Optional):"), wxDefaultPosition, wxDefaultSize, 0, _T(""));
    wxFont Page3OptionalFont(12,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText_Page3Optional->SetFont(Page3OptionalFont);
    GridSizerWizButtons = new wxGridSizer(0, 2, 5, 10);
    GridSizerWizButtons->Add(StaticText_Page3Optional, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    GridSizerWizButtons->Add(50,1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    BitmapButton_lor = new FlickerFreeBitmapButtonCtrl(Panel_Wizard, ID_BITMAPBUTTON_lor, lightorama, wxDefaultPosition, wxSize(185,50), wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_lor"));
    GridSizerWizButtons->Add(BitmapButton_lor, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_hls = new FlickerFreeBitmapButtonCtrl(Panel_Wizard, ID_BITMAPBUTTON_hls, hls, wxDefaultPosition, wxSize(185,50), wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_hls"));
    GridSizerWizButtons->Add(BitmapButton_hls, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_xlights = new FlickerFreeBitmapButtonCtrl(Panel_Wizard, ID_BITMAPBUTTON_xlights, xlights_logo, wxDefaultPosition, wxSize(185,50), wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_xlights"));
    GridSizerWizButtons->Add(BitmapButton_xlights, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_vixen = new FlickerFreeBitmapButtonCtrl(Panel_Wizard, ID_BITMAPBUTTON_vixen, vixen, wxDefaultPosition, wxSize(185,50), wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_vixen"));
    GridSizerWizButtons->Add(BitmapButton_vixen, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_gled = new FlickerFreeBitmapButtonCtrl(Panel_Wizard, ID_BITMAPBUTTON_gled, glediator, wxDefaultPosition, wxSize(185,50), wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_gled"));
    GridSizerWizButtons->Add(BitmapButton_gled, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_lynx = new FlickerFreeBitmapButtonCtrl(Panel_Wizard, ID_BITMAPBUTTON_lynx, lynx, wxDefaultPosition, wxSize(185,50), wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_lynx"));
    GridSizerWizButtons->Add(BitmapButton_lynx, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridSizerWizButtons->Add(185,30,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_SkipImport = new wxButton(Panel_Wizard, ID_BUTTON_skip_import, _("Skip >>"), wxDefaultPosition, wxSize(185,30), 0, wxDefaultValidator, _T("ID_BUTTON_skip_import"));
    GridSizerWizButtons->Add(Button_SkipImport, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    wxFont SkipImportFont(16,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    Button_SkipImport->SetFont(SkipImportFont);
    GridBagSizerWizard->Add(GridSizerWizButtons, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel_Wizard->SetSizer(GridBagSizerWizard);
    GridBagSizerWizard->Fit(Panel_Wizard);
    GridBagSizerWizard->SetSizeHints(Panel_Wizard);
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
}

void SeqSettingsDialog::OnChoice_Xml_Seq_TypeSelect(wxCommandEvent& event)
{
    int selection = Choice_Xml_Seq_Type->GetSelection();
    wxString type = Choice_Xml_Seq_Type->GetString(selection);
    if( type != xml_file->GetSequenceType() )
    {
        xml_file->SetSequenceType(type);
        ProcessSequenceType();
    }
}

void SeqSettingsDialog::OnBitmapButton_Xml_Media_FileClick(wxCommandEvent& event)
{
    MediaChooser();
}

void SeqSettingsDialog::OnTextCtrl_Xml_AuthorText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(xLightsXmlFile::AUTHOR, TextCtrl_Xml_Author->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_Author_EmailText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(xLightsXmlFile::AUTHOR_EMAIL, TextCtrl_Xml_Author_Email->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_WebsiteText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(xLightsXmlFile::WEBSITE, TextCtrl_Xml_Website->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_SongText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(xLightsXmlFile::SONG, TextCtrl_Xml_Song->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_ArtistText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(xLightsXmlFile::ARTIST, TextCtrl_Xml_Artist->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_AlbumText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(xLightsXmlFile::ALBUM, TextCtrl_Xml_Album->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_Music_UrlText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(xLightsXmlFile::URL, TextCtrl_Xml_Music_Url->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_CommentText(wxCommandEvent& event)
{
    xml_file->SetHeaderInfo(xLightsXmlFile::COMMENT, TextCtrl_Xml_Comment->GetValue());
}

void SeqSettingsDialog::OnTextCtrl_Xml_Seq_DurationText(wxCommandEvent& event)
{
    xml_file->SetSequenceDuration(TextCtrl_Xml_Seq_Duration->GetValue());
}

void SeqSettingsDialog::PopulateTimingGrid()
{
    wxArrayString timings = xml_file->GetTimingList();
    for(int i = 0; i < timings.GetCount(); ++i)
    {
        AddTimingCell(timings[i]);
    }
}

void SeqSettingsDialog::OnButton_Xml_New_TimingClick(wxCommandEvent& event)
{
    NewTimingDialog dialog(this);
    dialog.Fit();
    if (dialog.ShowModal() == wxID_OK)
    {
        wxString selected_timing = dialog.GetTiming();
        xml_file->AddFixedTimingSection(selected_timing);
        AddTimingCell(selected_timing);
    }
    dialog.Destroy();
}

void SeqSettingsDialog::OnButton_Xml_Import_TimingClick(wxCommandEvent& event)
{
    wxFileDialog* OpenDialog = new wxFileDialog( this, "Choose Audacity timing file(s)", wxEmptyString, wxEmptyString, "Text files (*.txt)|*.txt", wxFD_OPEN | wxFD_MULTIPLE, wxDefaultPosition);
    wxString fDir;
    if (OpenDialog->ShowModal() == wxID_OK)
    {
        fDir =	OpenDialog->GetDirectory();
        wxArrayString filenames;
        OpenDialog->GetFilenames(filenames);
        xml_file->ProcessAudacityTimingFiles(fDir, filenames);
        for(int i = 0; i < filenames.GetCount(); ++i)
        {
            AddTimingCell(filenames[i]);
        }
    }

    OpenDialog->Destroy();
}

void SeqSettingsDialog::OnButton_Xml_Rename_TimingClick(wxCommandEvent& event)
{
    int selection = event.GetId();
    wxArrayString timing_list = xml_file->GetTimingList();
    xml_file->SetTimingSectionName(timing_list[selection], Grid_Timing->GetCellValue(selection, 0));
}

void SeqSettingsDialog::OnButton_Xml_Delete_TimingClick(wxCommandEvent& event)
{
    if( Grid_Timing->GetGridCursorCol() == 1 )
    {
        int row = Grid_Timing->GetGridCursorRow();
        wxArrayString timing_list = xml_file->GetTimingList();
        xml_file->DeleteTimingSection(timing_list[row]);
        Grid_Timing->DeleteRows(row);
        Refresh();
    }
}

void SeqSettingsDialog::OnButton_Layer_ImportClick(wxCommandEvent& event)
{
    wxFileDialog* ImportDialog = new wxFileDialog( this, "Choose file to import as data layer", wxEmptyString, wxEmptyString, strSupportedFileTypes, wxFD_OPEN, wxDefaultPosition);
    wxString fDir;
    Button_Close->Enable(false);
    Button_Layer_Import->Enable(false);
    if (ImportDialog->ShowModal() == wxID_OK)
    {
        fDir =	ImportDialog->GetDirectory();
        wxString filename = ImportDialog->GetFilename();
        wxFileName full_name(filename);
        full_name.SetPath(fDir);
        wxFileName data_file(full_name);
        data_file.SetExt("iseq");
        data_file.SetPath(xLightsParent->GetFseqDirectory());
        DataLayerSet& data_layers = xml_file->GetDataLayers();
        DataLayer* new_data_layer = data_layers.AddDataLayer(full_name.GetName(), full_name.GetFullPath(), data_file.GetFullPath() );
        wxTreeItemId root = TreeCtrl_Data_Layers->GetRootItem();
        wxTreeItemId branch1 = TreeCtrl_Data_Layers->AppendItem(root, filename, -1, -1, new LayerTreeItemData(new_data_layer) );
        TreeCtrl_Data_Layers->AppendItem(branch1, "Source: " + full_name.GetFullPath());
        wxTreeItemId branch_data = TreeCtrl_Data_Layers->AppendItem(branch1, "Data: <waiting for file conversion>");
        wxTreeItemId branch_num_channels = TreeCtrl_Data_Layers->AppendItem(branch1, "Number of Channels: <waiting for file conversion>");
        TreeCtrl_Data_Layers->AppendItem(branch1, "Channel Offset: 0");
        TreeCtrl_Data_Layers->Expand(branch1);

        wxString media_filename;
        ConvertParameters conv_params(full_name.GetFullPath(),                                  // input filename
                                      new_data_layer->GetSequenceData(),                        // sequence data object
                                      xLightsParent->GetNetInfo(),                              // global network info
                                      ConvertParameters::READ_MODE_NORMAL,                      // file read mode
                                      xLightsParent,                                            // xLights main frame
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
            new_data_layer->SetLORConvertParams( conv_params.channels_off_at_end | (conv_params.map_empty_channels << 1) | (conv_params.map_no_network_channels << 2) );
            FileConverter::ReadLorFile(conv_params);
            FileConverter::WriteFalconPiFile( conv_params );
        }
        else if( full_name.GetExt() == "xseq" )
        {
            conv_params.channels_off_at_end = (wxYES == wxMessageBox("Turn off all channels at the end?", "Conversion Options", wxICON_QUESTION | wxYES_NO));
            new_data_layer->SetLORConvertParams( conv_params.channels_off_at_end );
            FileConverter::ReadXlightsFile(conv_params);
            FileConverter::WriteFalconPiFile( conv_params );
        }
        else if( full_name.GetExt() == "hlsIdata" )
        {
            conv_params.channels_off_at_end = (wxYES == wxMessageBox("Turn off all channels at the end?", "Conversion Options", wxICON_QUESTION | wxYES_NO));
            new_data_layer->SetLORConvertParams( conv_params.channels_off_at_end );
            FileConverter::ReadHLSFile(conv_params);
            FileConverter::WriteFalconPiFile( conv_params );
        }
        else if( full_name.GetExt() == "vix" )
        {
            conv_params.channels_off_at_end = (wxYES == wxMessageBox("Turn off all channels at the end?", "Conversion Options", wxICON_QUESTION | wxYES_NO));
            new_data_layer->SetLORConvertParams( conv_params.channels_off_at_end );
            FileConverter::ReadVixFile(conv_params);
            FileConverter::WriteFalconPiFile( conv_params );
        }
        else if( full_name.GetExt() == "gled" )
        {
            conv_params.channels_off_at_end = (wxYES == wxMessageBox("Turn off all channels at the end?", "Conversion Options", wxICON_QUESTION | wxYES_NO));
            new_data_layer->SetLORConvertParams( conv_params.channels_off_at_end );
            FileConverter::ReadGlediatorFile(conv_params);
            FileConverter::WriteFalconPiFile( conv_params );
        }
        else if( full_name.GetExt() == "seq" )
        {
            conv_params.channels_off_at_end = (wxYES == wxMessageBox("Turn off all channels at the end?", "Conversion Options", wxICON_QUESTION | wxYES_NO));
            new_data_layer->SetLORConvertParams( conv_params.channels_off_at_end );
            FileConverter::ReadConductorFile(conv_params);
            FileConverter::WriteFalconPiFile( conv_params );
        }
        else if( full_name.GetExt() == "iseq" || full_name.GetExt() == "fseq")
        {
            // we read only the header to fill in the channel count info
            conv_params.read_mode = ConvertParameters::READ_MODE_HEADER_ONLY;
            conv_params.media_filename = nullptr;
            FileConverter::ReadFalconFile(conv_params);
        }
        TreeCtrl_Data_Layers->SetItemText(branch_data, "Data: " + data_file.GetFullPath());
        TreeCtrl_Data_Layers->SetItemText(branch_num_channels, wxString::Format("Number of Channels: %d", new_data_layer->GetNumChannels()));
        UpdateDataLayer();
    }

    ImportDialog->Destroy();
    Button_Close->Enable(true);
    Button_Layer_Import->Enable(true);
}

void SeqSettingsDialog::OnButton_ReimportClick(wxCommandEvent& event)
{
    LayerTreeItemData* data = (LayerTreeItemData*)TreeCtrl_Data_Layers->GetItemData(selected_branch);
    DataLayer* layer = data->GetLayer();
    wxString media_filename;
    wxFileName full_name(layer->GetSource());
    if( full_name.GetExt() == "lms" )
    {
        ConvertParameters conv_params(layer->GetSource(),                                       // input filename
                                      layer->GetSequenceData(),                                 // sequence data object
                                      xLightsParent->GetNetInfo(),                              // global network info
                                      ConvertParameters::READ_MODE_NORMAL,                      // file read mode
                                      xLightsParent,                                            // xLights main frame
                                      &media_filename,                                          // media filename
                                      layer,                                                    // data layer to fill in header info
                                      layer->GetDataSource(),                                   // output filename
                                      atoi(xml_file->GetSequenceTiming().c_str()),              // sequence timing
                                      layer->GetLORConvertParams() & 0x1,                       // turn off all channels at end
                                      (layer->GetLORConvertParams() >> 1) & 0x1,                // map empty channels (mainly LOR)
                                      (layer->GetLORConvertParams() >> 2) & 0x1 );              // map no network channels (mainly LOR)
        FileConverter::ReadLorFile(conv_params);
        FileConverter::WriteFalconPiFile( conv_params );
    }
    else if( full_name.GetExt() == "iseq" || full_name.GetExt() == "fseq")
    {
        // we read only the header to fill in the channel count info
        ConvertParameters conv_params(layer->GetSource(),                                       // input filename
                                      layer->GetSequenceData(),                                 // sequence data object
                                      xLightsParent->GetNetInfo(),                              // global network info
                                      ConvertParameters::READ_MODE_HEADER_ONLY,                 // file read mode
                                      xLightsParent,                                            // xLights main frame
                                      nullptr,                                                  // media filename
                                      layer );                                                  // data layer to fill in header info
        FileConverter::ReadFalconFile(conv_params);
    }
    // TreeCtrl_Data_Layers->SetItemText(branch_num_channels, wxString::Format("Number of Channels: %d", new_data_layer->GetNumChannels()));  FIXME update in case channel number changes
}

void SeqSettingsDialog::UpdateDataLayer()
{
    // update buttons
    Button_Layer_Delete->Enable(false);
    Button_Reimport->Enable(false);
    Button_Move_Up->Enable(false);
    Button_Move_Down->Enable(false);
    wxTreeItemId root = TreeCtrl_Data_Layers->GetRootItem();
    int num_layers = TreeCtrl_Data_Layers->GetChildrenCount(root, false);
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
void SeqSettingsDialog::OnChoice_Xml_Seq_TimingSelect(wxCommandEvent& event)
{
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
}

void SeqSettingsDialog::OnTreeCtrl_Data_LayersSelectionChanged(wxTreeEvent& event)
{
    UpdateDataLayer();
}

void SeqSettingsDialog::OnButton_CloseClick(wxCommandEvent& event)
{
    Close();
}

void SeqSettingsDialog::OnTreeCtrl_Data_LayersBeginLabelEdit(wxTreeEvent& event)
{
    wxTreeItemId itemId = event.GetItem();
    wxString item_text = TreeCtrl_Data_Layers->GetItemText(itemId);

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

void SeqSettingsDialog::MediaChooser()
{
    wxFileDialog* OpenDialog = new wxFileDialog( this, "Choose Audio file", wxEmptyString, wxEmptyString, "MP3 files (*.mp3)|*.mp3", wxFD_OPEN, wxDefaultPosition);
    wxString fDir;
    OpenDialog->SetDirectory(media_directory);
    if (OpenDialog->ShowModal() == wxID_OK)
    {
        fDir = OpenDialog->GetDirectory();
        wxString filename = OpenDialog->GetFilename();
        wxFileName name_and_path(filename);
        name_and_path.SetPath(fDir);
        xml_file->SetMediaFile(name_and_path.GetFullPath(), CheckBox_Overwrite_Tags->IsChecked());
        TextCtrl_Xml_Media_File->SetValue(name_and_path.GetFullPath());
        TextCtrl_Xml_Song->SetValue(xml_file->GetHeaderInfo(xLightsXmlFile::SONG));
        TextCtrl_Xml_Album->SetValue(xml_file->GetHeaderInfo(xLightsXmlFile::ALBUM));
        TextCtrl_Xml_Artist->SetValue(xml_file->GetHeaderInfo(xLightsXmlFile::ARTIST));
        int length_ms = Waveform::GetLengthOfMusicFileInMS(name_and_path.GetFullPath());
        double length = length_ms / 1000.0f;
        xml_file->SetSequenceDuration(length);
        TextCtrl_Xml_Seq_Duration->SetValue(string_format("%.3f", length));
        StaticText_Warning->Hide();
        ProcessSequenceType();
    }

    OpenDialog->Destroy();
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
}

void SeqSettingsDialog::OnBitmapButton_Wiz_AnimClick(wxCommandEvent& event)
{
    Choice_Xml_Seq_Type->SetSelection(1);
    xml_file->SetSequenceType("Animation");
    TextCtrl_Xml_Seq_Duration->SetValue("30.0");
    xml_file->SetSequenceDuration(30.0);
    ProcessSequenceType();
    WizardPage2();
}

void SeqSettingsDialog::OnBitmapButton_25msClick(wxCommandEvent& event)
{
    Choice_Xml_Seq_Timing->SetSelection(0);
    WizardPage3();
}

void SeqSettingsDialog::OnBitmapButton_50msClick(wxCommandEvent& event)
{
    Choice_Xml_Seq_Timing->SetSelection(1);
    WizardPage3();
}

void SeqSettingsDialog::OnBitmapButton_100msClick(wxCommandEvent& event)
{
    Choice_Xml_Seq_Timing->SetSelection(2);
    WizardPage3();
}
