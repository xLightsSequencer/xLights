#include <wx/filedlg.h>
#include "SeqSettingsDialog.h"
#include "RenameTextDialog.h"
#include "NewTimingDialog.h"
#include "xLightsMain.h"

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
const long SeqSettingsDialog::ID_NOTEBOOK_Seq_Settings = wxNewId();
const long SeqSettingsDialog::ID_STATICTEXT_Warning = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_Save = wxNewId();
const long SeqSettingsDialog::ID_BUTTON_Close = wxNewId();
//*)

const long SeqSettingsDialog::ID_GRID_TIMING = wxNewId();

wxDEFINE_EVENT(EVT_DELETE_ROW, wxCommandEvent);
wxDEFINE_EVENT(EVT_NAME_CHANGE, wxCommandEvent);

BEGIN_EVENT_TABLE(SeqSettingsDialog,wxDialog)
	//(*EventTable(SeqSettingsDialog)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_NAME_CHANGE, SeqSettingsDialog::OnButton_Xml_Rename_TimingClick)
    EVT_COMMAND(wxID_ANY, EVT_DELETE_ROW, SeqSettingsDialog::OnButton_Xml_Delete_TimingClick)
END_EVENT_TABLE()

#define string_format wxString::Format

SeqSettingsDialog::SeqSettingsDialog(wxWindow* parent, xLightsXmlFile* file_to_handle_, wxString& media_dir, const wxString& warning, bool search_for_media)
:   xml_file(file_to_handle_),
    media_directory(media_dir)
{
	//(*Initialize(SeqSettingsDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxGridBagSizer* GridBagSizer1;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer3;
	wxStaticText* StaticText_Xml_Seq_Timing;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;
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
	TextCtrl_Xml_Media_File = new wxTextCtrl(Panel3, ID_TEXTCTRL_Xml_Media_File, wxEmptyString, wxDefaultPosition, wxSize(326,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Media_File"));
	TextCtrl_Xml_Media_File->Disable();
	FlexGridSizer10->Add(TextCtrl_Xml_Media_File, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Xml_Media_File = new wxBitmapButton(Panel3, ID_BITMAPBUTTON_Xml_Media_File, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_CDROM")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Xml_Media_File"));
	BitmapButton_Xml_Media_File->Disable();
	FlexGridSizer10->Add(BitmapButton_Xml_Media_File, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText_Xml_Total_Length = new wxStaticText(Panel3, ID_STATICTEXT_Xml_Total_Length, _("Sequence Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Total_Length"));
	FlexGridSizer6->Add(StaticText_Xml_Total_Length, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Xml_Seq_Duration = new wxTextCtrl(Panel3, ID_TEXTCTRL_Xml_Seq_Duration, wxEmptyString, wxDefaultPosition, wxSize(81,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Seq_Duration"));
	FlexGridSizer6->Add(TextCtrl_Xml_Seq_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	Notebook_Seq_Settings->AddPage(Panel3, _("Info / Media"), false);
	Notebook_Seq_Settings->AddPage(Panel1, _("Meta Data"), false);
	Notebook_Seq_Settings->AddPage(Panel2, _("Timings"), false);
	FlexGridSizer1->Add(Notebook_Seq_Settings, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Warning = new wxStaticText(this, ID_STATICTEXT_Warning, _("Show Warning Here"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Warning"));
	StaticText_Warning->Hide();
	StaticText_Warning->SetForegroundColour(wxColour(255,0,0));
	wxFont StaticText_WarningFont(wxDEFAULT,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText_Warning->SetFont(StaticText_WarningFont);
	FlexGridSizer1->Add(StaticText_Warning, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Save = new wxButton(this, ID_BUTTON_Save, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Save"));
	Button_Save->Disable();
	FlexGridSizer7->Add(Button_Save, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	Connect(ID_BUTTON_Save,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_SaveClick);
	Connect(ID_BUTTON_Close,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SeqSettingsDialog::OnButton_CloseClick);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&SeqSettingsDialog::OnClose);
	//*)

	if( !xml_file->IsLoaded() ) xml_file->Load();
	if( warning != "" )
    {
        StaticText_Warning->SetLabelText(warning);
        StaticText_Warning->Show();
    }

	StaticText_Filename->SetLabelText(xml_file->GetFullPath());
    if( xml_file->GetSequenceType() != "Media" && xml_file->GetSequenceType() != "Animation"  )
    {
        xml_file->SetSequenceType("Media");
        xml_file->Save();
    }
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
    Button_Save->Enable(false);
}

SeqSettingsDialog::~SeqSettingsDialog()
{
	//(*Destroy(SeqSettingsDialog)
	//*)
}

void SeqSettingsDialog::SetMediaFilename(const wxString &filename, bool overwrite_tags) {
    if( overwrite_tags) {
        ExtractMetaTagsFromMP3(filename);
    }
    BitmapButton_Xml_Media_File->Enable(true);
    TextCtrl_Xml_Media_File->SetValue(filename);
    Choice_Xml_Seq_Type->SetStringSelection("Media");
    if( "Media" != xml_file->GetSequenceType() )
    {
        xml_file->SetSequenceType("Media");
    }
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
}

void SeqSettingsDialog::OnChoice_Xml_Seq_TypeSelect(wxCommandEvent& event)
{
    int selection = Choice_Xml_Seq_Type->GetSelection();
    wxString type = Choice_Xml_Seq_Type->GetString(selection);
    if( type != xml_file->GetSequenceType() )
    {
        xml_file->SetSequenceType(type);
        ProcessSequenceType();
        Button_Save->Enable(true);
    }
}

void SeqSettingsDialog::OnBitmapButton_Xml_Media_FileClick(wxCommandEvent& event)
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
        TextCtrl_Xml_Media_File->SetValue(name_and_path.GetFullPath());
        ExtractMetaTagsFromMP3(name_and_path.GetFullPath());
        int length_ms = Waveform::GetLengthOfMusicFileInMS(name_and_path.GetFullPath());
        double length = length_ms / 1000.0f;
        TextCtrl_Xml_Seq_Duration->SetValue(string_format("%.3f", length));
        StaticText_Warning->Hide();
        Fit();
        Button_Save->Enable(true);
    }

    OpenDialog->Destroy();
}

void SeqSettingsDialog::OnTextCtrl_Xml_AuthorText(wxCommandEvent& event)
{
    Button_Save->Enable(true);
}

void SeqSettingsDialog::OnTextCtrl_Xml_Author_EmailText(wxCommandEvent& event)
{
    Button_Save->Enable(true);
}

void SeqSettingsDialog::OnTextCtrl_Xml_WebsiteText(wxCommandEvent& event)
{
    Button_Save->Enable(true);
}

void SeqSettingsDialog::OnTextCtrl_Xml_SongText(wxCommandEvent& event)
{
    Button_Save->Enable(true);
}

void SeqSettingsDialog::OnTextCtrl_Xml_ArtistText(wxCommandEvent& event)
{
    Button_Save->Enable(true);
}

void SeqSettingsDialog::OnTextCtrl_Xml_AlbumText(wxCommandEvent& event)
{
    Button_Save->Enable(true);
}

void SeqSettingsDialog::OnTextCtrl_Xml_Music_UrlText(wxCommandEvent& event)
{
    Button_Save->Enable(true);
}

void SeqSettingsDialog::OnTextCtrl_Xml_CommentText(wxCommandEvent& event)
{
    Button_Save->Enable(true);
}

void SeqSettingsDialog::OnTextCtrl_Xml_Seq_DurationText(wxCommandEvent& event)
{
    Button_Save->Enable(true);
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
        //PopulateSongTimings();
        Button_Save->Enable(true);
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
    Button_Save->Enable(true);
}

void SeqSettingsDialog::OnButton_Xml_Rename_TimingClick(wxCommandEvent& event)
{
    int selection = event.GetId();
    wxArrayString timing_list = xml_file->GetTimingList();
    xml_file->SetTimingSectionName(timing_list[selection], Grid_Timing->GetCellValue(selection, 0));
    Button_Save->Enable(true);
}

void SeqSettingsDialog::OnButton_Xml_Delete_TimingClick(wxCommandEvent& event)
{
    if( Grid_Timing->GetGridCursorCol() == 1 )
    {
        int row = Grid_Timing->GetGridCursorRow();
        wxArrayString timing_list = xml_file->GetTimingList();
        xml_file->DeleteTimingSection(timing_list[row]);
        Grid_Timing->DeleteRows(row);
        Button_Save->Enable(true);
        Refresh();
    }
}

bool SeqSettingsDialog::ExtractMetaTagsFromMP3(wxString filename)
{
    bool modified = false;
    mpg123_handle *mh;
    mpg123_id3v1 *v1;
    mpg123_id3v2 *v2;
    int err;

    mpg123_init();
    mh = mpg123_new(NULL, &err);

    mpg123_open(mh, filename);

    if( err == MPG123_OK )
    {
        // get meta tags
        mpg123_scan(mh);
        int meta = mpg123_meta_check(mh);

        if( meta == MPG123_ID3 && mpg123_id3(mh, &v1, &v2) == MPG123_OK )
        {
            wxString title = "";
            wxString artist = "";
            wxString album = "";

            if( v2 != NULL ) // "ID3V2 tag found"
            {
                title = v2->title == NULL ? "" : v2->title->p;
                artist = v2->artist == NULL ? "" : v2->artist->p;
                album = v2->album == NULL ? "" : v2->album->p;
                modified = true;
            }

            else if( v1 != NULL ) // "ID3V1 tag found"
            {
                title = v1->title[0];
                artist = v1->artist[0];
                album = v1->album[0];
                modified = true;
            }

            if( title != "" )
            {
                TextCtrl_Xml_Song->SetValue(title);
            }
            if( artist != "" )
            {
                TextCtrl_Xml_Artist->SetValue(artist);
            }
            if( album != ""
                )
            {
                TextCtrl_Xml_Album->SetValue(album);
            }
        }
    }

    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    return modified;
}

void SeqSettingsDialog::SaveAll()
{
    wxArrayString info;
    info.push_back(TextCtrl_Xml_Author->GetValue());
    info.push_back(TextCtrl_Xml_Author_Email->GetValue());
    info.push_back(TextCtrl_Xml_Website->GetValue());
    info.push_back(TextCtrl_Xml_Song->GetValue());
    info.push_back(TextCtrl_Xml_Artist->GetValue());
    info.push_back(TextCtrl_Xml_Album->GetValue());
    info.push_back(TextCtrl_Xml_Music_Url->GetValue());
    info.push_back(TextCtrl_Xml_Comment->GetValue());
    xml_file->SetSequenceType(Choice_Xml_Seq_Type->GetString(Choice_Xml_Seq_Type->GetSelection()));
    xml_file->SetMediaFile(TextCtrl_Xml_Media_File->GetValue());
    xml_file->SetSequenceDuration(TextCtrl_Xml_Seq_Duration->GetValue());
    xml_file->SetSequenceTiming(Choice_Xml_Seq_Timing->GetString(Choice_Xml_Seq_Timing->GetSelection()));
    xml_file->SetHeaderInfo(info);
    xml_file->Save();
    Button_Save->Enable(false);
}

void SeqSettingsDialog::OnButton_SaveClick(wxCommandEvent& event)
{
    SaveAll();
    StaticText_Warning->Hide();
    Fit();
}

void SeqSettingsDialog::OnButton_CloseClick(wxCommandEvent& event)
{
    Close();
}

void SeqSettingsDialog::OnClose(wxCloseEvent& event)
{
    if( Button_Save->IsEnabled() && event.CanVeto())
    {
        int answer = wxMessageBox("Close without saving XML?", "Confirm", wxYES_NO, this);
        if (answer != wxYES ) {
            event.Veto();
            return;
        }
    }
    EndModal(Button_Save->IsEnabled() ? wxCANCEL : wxOK);
    Destroy();
}

void SeqSettingsDialog::OnChoice_Xml_Seq_TimingSelect(wxCommandEvent& event)
{
}
