#include "XmlConversionDialog.h"
#include <wx/dir.h>
#include "xLightsMain.h"

//(*InternalHeaders(XmlConversionDialog)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(XmlConversionDialog)
const long XmlConversionDialog::ID_STATICTEXT1 = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Filename = wxNewId();
const long XmlConversionDialog::ID_CHOICE_Xml_Settings_Filename = wxNewId();
const long XmlConversionDialog::ID_BITMAPBUTTON_Change_Dir = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Work_Dir = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_XML_Type_Version = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_XML_Version = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Num_Models_Label = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Num_Models = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Author = wxNewId();
const long XmlConversionDialog::ID_TEXTCTRL_Xml_Author = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Author_Email = wxNewId();
const long XmlConversionDialog::ID_TEXTCTRL_Xml_Author_Email = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Website = wxNewId();
const long XmlConversionDialog::ID_TEXTCTRL_Xml_Website = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Song = wxNewId();
const long XmlConversionDialog::ID_TEXTCTRL_Xml_Song = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Artist = wxNewId();
const long XmlConversionDialog::ID_TEXTCTRL_Xml_Artist = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Album = wxNewId();
const long XmlConversionDialog::ID_TEXTCTRL_Xml_Album = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Music_Url = wxNewId();
const long XmlConversionDialog::ID_TEXTCTRL_Xml_Music_Url = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Comment = wxNewId();
const long XmlConversionDialog::ID_TEXTCTRL_Xml_Comment = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Extract_Song_Info = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Xml_Settings_Save = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Xml_Close_Dialog = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Timing = wxNewId();
const long XmlConversionDialog::ID_CHOICE1 = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Xml_Import_Timing = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Xml_Delete_Timing = wxNewId();
const long XmlConversionDialog::ID_TEXTCTRL_Xml_Log = wxNewId();
//*)

BEGIN_EVENT_TABLE(XmlConversionDialog,wxDialog)
	//(*EventTable(XmlConversionDialog)
	//*)
END_EVENT_TABLE()

#define string_format wxString::Format

XmlConversionDialog::XmlConversionDialog(wxWindow* parent,wxWindowID id)
:   current_selection(-1)
{
	//(*Initialize(XmlConversionDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxStaticBoxSizer* StaticBoxSizer_File_Info;
	wxStaticBoxSizer* StaticBoxSizer_Xml_Header;
	wxGridBagSizer* GridBagSizer1;
	wxFlexGridSizer* FlexGridSizer3;
	wxGridBagSizer* GridBagSizer2;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer7;
	wxStaticBoxSizer* StaticBoxSizer_Xml_Song_Timings;
	wxFlexGridSizer* FlexGridSizer6;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	SetClientSize(wxSize(574,398));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("XML Conversion and Settings Editor"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	wxFont StaticText1Font(12,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,_T("Arial"),wxFONTENCODING_DEFAULT);
	StaticText1->SetFont(StaticText1Font);
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
	GridBagSizer2 = new wxGridBagSizer(0, 0);
	StaticText_Xml_Filename = new wxStaticText(this, ID_STATICTEXT_Xml_Filename, _("XML Filename:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Filename"));
	GridBagSizer2->Add(StaticText_Xml_Filename, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Xml_Settings_Filename = new wxChoice(this, ID_CHOICE_Xml_Settings_Filename, wxDefaultPosition, wxSize(238,21), 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE_Xml_Settings_Filename"));
	GridBagSizer2->Add(Choice_Xml_Settings_Filename, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Change_Dir = new wxBitmapButton(this, ID_BITMAPBUTTON_Change_Dir, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FOLDER_OPEN")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Change_Dir"));
	GridBagSizer2->Add(BitmapButton_Change_Dir, wxGBPosition(0, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Work_Dir = new wxStaticText(this, ID_STATICTEXT_Work_Dir, _("Directory:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Work_Dir"));
	GridBagSizer2->Add(StaticText_Work_Dir, wxGBPosition(1, 0), wxGBSpan(1, 3), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6->Add(GridBagSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer_File_Info = new wxStaticBoxSizer(wxHORIZONTAL, this, _("File Information:"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 4, 0, 0);
	StaticText_XML_Type_Version = new wxStaticText(this, ID_STATICTEXT_XML_Type_Version, _("XML Version:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_XML_Type_Version"));
	FlexGridSizer5->Add(StaticText_XML_Type_Version, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_XML_Version = new wxStaticText(this, ID_STATICTEXT_XML_Version, wxEmptyString, wxDefaultPosition, wxSize(70,-1), 0, _T("ID_STATICTEXT_XML_Version"));
	FlexGridSizer5->Add(StaticText_XML_Version, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Num_Models_Label = new wxStaticText(this, ID_STATICTEXT_Num_Models_Label, _("# Models:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Num_Models_Label"));
	FlexGridSizer5->Add(StaticText_Num_Models_Label, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Num_Models = new wxStaticText(this, ID_STATICTEXT_Num_Models, wxEmptyString, wxDefaultPosition, wxSize(70,-1), 0, _T("ID_STATICTEXT_Num_Models"));
	FlexGridSizer5->Add(StaticText_Num_Models, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer_File_Info->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6->Add(StaticBoxSizer_File_Info, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer_Xml_Header = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Header"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText_Xml_Author = new wxStaticText(this, ID_STATICTEXT_Xml_Author, _("Author:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Author"));
	FlexGridSizer3->Add(StaticText_Xml_Author, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Xml_Author = new wxTextCtrl(this, ID_TEXTCTRL_Xml_Author, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Author"));
	FlexGridSizer3->Add(TextCtrl_Xml_Author, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Xml_Author_Email = new wxStaticText(this, ID_STATICTEXT_Xml_Author_Email, _("Email:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Author_Email"));
	FlexGridSizer3->Add(StaticText_Xml_Author_Email, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Xml_Author_Email = new wxTextCtrl(this, ID_TEXTCTRL_Xml_Author_Email, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Author_Email"));
	FlexGridSizer3->Add(TextCtrl_Xml_Author_Email, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Xml_Website = new wxStaticText(this, ID_STATICTEXT_Xml_Website, _("Website:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Website"));
	FlexGridSizer3->Add(StaticText_Xml_Website, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Xml_Website = new wxTextCtrl(this, ID_TEXTCTRL_Xml_Website, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Website"));
	FlexGridSizer3->Add(TextCtrl_Xml_Website, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Xml_Song = new wxStaticText(this, ID_STATICTEXT_Xml_Song, _("Song:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Song"));
	FlexGridSizer3->Add(StaticText_Xml_Song, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Xml_Song = new wxTextCtrl(this, ID_TEXTCTRL_Xml_Song, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Song"));
	FlexGridSizer3->Add(TextCtrl_Xml_Song, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Xml_Artist = new wxStaticText(this, ID_STATICTEXT_Xml_Artist, _("Artist:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Artist"));
	FlexGridSizer3->Add(StaticText_Xml_Artist, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Xml_Artist = new wxTextCtrl(this, ID_TEXTCTRL_Xml_Artist, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Artist"));
	FlexGridSizer3->Add(TextCtrl_Xml_Artist, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Xml_Album = new wxStaticText(this, ID_STATICTEXT_Xml_Album, _("Album:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Album"));
	FlexGridSizer3->Add(StaticText_Xml_Album, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Xml_Album = new wxTextCtrl(this, ID_TEXTCTRL_Xml_Album, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Album"));
	FlexGridSizer3->Add(TextCtrl_Xml_Album, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Xml_Music_Url = new wxStaticText(this, ID_STATICTEXT_Xml_Music_Url, _("Music URL:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Music_Url"));
	FlexGridSizer3->Add(StaticText_Xml_Music_Url, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Xml_Music_Url = new wxTextCtrl(this, ID_TEXTCTRL_Xml_Music_Url, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Music_Url"));
	FlexGridSizer3->Add(TextCtrl_Xml_Music_Url, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Xml_Comment = new wxStaticText(this, ID_STATICTEXT_Xml_Comment, _("Comment:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Comment"));
	FlexGridSizer3->Add(StaticText_Xml_Comment, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Xml_Comment = new wxTextCtrl(this, ID_TEXTCTRL_Xml_Comment, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Comment"));
	FlexGridSizer3->Add(TextCtrl_Xml_Comment, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Extract_Song_Info = new wxButton(this, ID_BUTTON_Extract_Song_Info, _("Extract Song Info from File"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Extract_Song_Info"));
	FlexGridSizer3->Add(Button_Extract_Song_Info, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer_Xml_Header->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6->Add(StaticBoxSizer_Xml_Header, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Xml_Settings_Save = new wxButton(this, ID_BUTTON_Xml_Settings_Save, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Xml_Settings_Save"));
	Button_Xml_Settings_Save->Disable();
	FlexGridSizer4->Add(Button_Xml_Settings_Save, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Xml_Close_Dialog = new wxButton(this, ID_BUTTON_Xml_Close_Dialog, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Xml_Close_Dialog"));
	FlexGridSizer4->Add(Button_Xml_Close_Dialog, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer_Xml_Song_Timings = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Song Timings"));
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	StaticText_Xml_Timing = new wxStaticText(this, ID_STATICTEXT_Xml_Timing, _("Timing:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Timing"));
	GridBagSizer1->Add(StaticText_Xml_Timing, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice1 = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxSize(238,21), 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE1"));
	GridBagSizer1->Add(Choice1, wxGBPosition(0, 1), wxGBSpan(1, 2), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Xml_Import_Timing = new wxButton(this, ID_BUTTON_Xml_Import_Timing, _("Import"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Xml_Import_Timing"));
	GridBagSizer1->Add(Button_Xml_Import_Timing, wxGBPosition(1, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Xml_Delete_Timing = new wxButton(this, ID_BUTTON_Xml_Delete_Timing, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Xml_Delete_Timing"));
	GridBagSizer1->Add(Button_Xml_Delete_Timing, wxGBPosition(1, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer_Xml_Song_Timings->Add(GridBagSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(StaticBoxSizer_Xml_Song_Timings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Message Log:"));
	TextCtrl_Xml_Log = new wxTextCtrl(this, ID_TEXTCTRL_Xml_Log, wxEmptyString, wxDefaultPosition, wxSize(293,238), wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Log"));
	StaticBoxSizer1->Add(TextCtrl_Xml_Log, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(StaticBoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(ID_CHOICE_Xml_Settings_Filename,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&XmlConversionDialog::OnChoice_Xml_Settings_FilenameSelect);
	Connect(ID_BITMAPBUTTON_Change_Dir,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&XmlConversionDialog::OnBitmapButton_Change_DirClick);
	Connect(ID_TEXTCTRL_Xml_Author,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_AuthorText);
	Connect(ID_TEXTCTRL_Xml_Author_Email,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_Author_EmailText);
	Connect(ID_TEXTCTRL_Xml_Website,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_WebsiteText);
	Connect(ID_TEXTCTRL_Xml_Song,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_SongText);
	Connect(ID_TEXTCTRL_Xml_Artist,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_ArtistText);
	Connect(ID_TEXTCTRL_Xml_Album,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_AlbumText);
	Connect(ID_TEXTCTRL_Xml_Music_Url,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_Music_UrlText);
	Connect(ID_TEXTCTRL_Xml_Comment,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_CommentText);
	Connect(ID_BUTTON_Xml_Settings_Save,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&XmlConversionDialog::OnButton_Xml_Settings_SaveClick);
	Connect(ID_BUTTON_Xml_Close_Dialog,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&XmlConversionDialog::OnButton_Xml_Close_DialogClick);
	//*)

    PopulateFiles();
    Clear();
}

XmlConversionDialog::~XmlConversionDialog()
{
	//(*Destroy(XmlConversionDialog)
	//*)

}

void XmlConversionDialog::PopulateFiles()
{
    wxString filename;
    wxFileName oName;
    wxDir dir(xml_file.GetPath());
    Choice_Xml_Settings_Filename->Clear();
    xml_file_list.Clear();
    bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
    while ( cont )
    {
        oName.SetFullName(filename);
        if (oName.GetExt() == _("xml"))
        {
            xml_file_list.Add(oName.GetFullPath());
        }
        cont = dir.GetNext(&filename);
    }
    Choice_Xml_Settings_Filename->Set(xml_file_list);
    StaticText_Work_Dir->SetLabelText(_("Directory: " + xml_file.GetPath()));
}

void XmlConversionDialog::SetSelectionToXMLFile()
{
    wxString current_file = xml_file.GetFullName();
    for(int i = 0; i < xml_file_list.GetCount(); ++i)
    {
        if( xml_file_list[i] == current_file )
        {
            Choice_Xml_Settings_Filename->SetSelection(i);
            current_selection = i;
            break;
        }
    }
}
void XmlConversionDialog::Clear()
{
    StaticText_XML_Version->SetLabelText(_(""));
    StaticText_Num_Models->SetLabelText(_(""));
    TextCtrl_Xml_Author->SetValue(_(""));
    TextCtrl_Xml_Author_Email->SetValue(_(""));
    TextCtrl_Xml_Website->SetValue(_(""));
    TextCtrl_Xml_Song->SetValue(_(""));
    TextCtrl_Xml_Artist->SetValue(_(""));
    TextCtrl_Xml_Album->SetValue(_(""));
    TextCtrl_Xml_Music_Url->SetValue(_(""));
    TextCtrl_Xml_Comment->SetValue(_(""));
    Button_Xml_Settings_Save->Enable(false);
    Button_Xml_Settings_Save->SetLabel(_("Save"));
    xml_file.Clear();
}

void XmlConversionDialog::OnChoice_Xml_Settings_FilenameSelect(wxCommandEvent& event)
{
    int selection = Choice_Xml_Settings_Filename->GetSelection();
    if( selection != current_selection )
    {
        xml_file.Clear();
        xml_file.SetFullName(xml_file_list[selection]);
        xml_file.Load();
        if( xml_file.IsLoaded() )
        {
            StaticText_XML_Version->SetLabelText(xml_file.GetVersion());
            StaticText_Num_Models->SetLabelText(string_format("%d",xml_file.GetNumModels()));
            if( xml_file.NeedsConversion() )
            {
                Button_Xml_Settings_Save->Enable(true);
                Button_Xml_Settings_Save->SetLabel(_("Convert"));
            }
            TextCtrl_Xml_Author->SetValue(xml_file.GetHeaderInfo(xLightsXmlFile::AUTHOR));
            TextCtrl_Xml_Author_Email->SetValue(xml_file.GetHeaderInfo(xLightsXmlFile::AUTHOR_EMAIL));
            TextCtrl_Xml_Website->SetValue(xml_file.GetHeaderInfo(xLightsXmlFile::WEBSITE));
            TextCtrl_Xml_Song->SetValue(xml_file.GetHeaderInfo(xLightsXmlFile::SONG));
            TextCtrl_Xml_Artist->SetValue(xml_file.GetHeaderInfo(xLightsXmlFile::ARTIST));
            TextCtrl_Xml_Album->SetValue(xml_file.GetHeaderInfo(xLightsXmlFile::ALBUM));
            TextCtrl_Xml_Music_Url->SetValue(xml_file.GetHeaderInfo(xLightsXmlFile::URL));
            TextCtrl_Xml_Comment->SetValue(xml_file.GetHeaderInfo(xLightsXmlFile::COMMENT));
        }
    }
    if( xml_file.IsLoaded() )
    {
        if( xml_file.NeedsConversion() )
        {
            Button_Xml_Settings_Save->Enable(true);
            Button_Xml_Settings_Save->SetLabel(_("Convert"));
        }
        else
        {
            Button_Xml_Settings_Save->Enable(false);
        }
    }
    current_selection = selection;
}

void XmlConversionDialog::OnButton_Xml_Close_DialogClick(wxCommandEvent& event)
{
    xml_file.Clear();
    Close();
}

void XmlConversionDialog::OnButton_Xml_Settings_SaveClick(wxCommandEvent& event)
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
    xml_file.SetHeaderInfo(info);
    bool reload = xml_file.NeedsConversion();
    xml_file.Save(TextCtrl_Xml_Log);
    StaticText_XML_Version->SetLabelText(xml_file.GetVersion());
    if( reload )
    {
        PopulateFiles();
        xml_file.Clear();
        xml_file.Load();
    }
    Button_Xml_Settings_Save->Enable(false);
    Button_Xml_Settings_Save->SetLabel(_("Save"));
    SetSelectionToXMLFile();
}

void XmlConversionDialog::OnTextCtrl_Xml_AuthorText(wxCommandEvent& event)
{
    if( xml_file.IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_Author_EmailText(wxCommandEvent& event)
{
    if( xml_file.IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_WebsiteText(wxCommandEvent& event)
{
    if( xml_file.IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_SongText(wxCommandEvent& event)
{
    if( xml_file.IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_ArtistText(wxCommandEvent& event)
{
    if( xml_file.IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_AlbumText(wxCommandEvent& event)
{
    if( xml_file.IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_Music_UrlText(wxCommandEvent& event)
{
    if( xml_file.IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_CommentText(wxCommandEvent& event)
{
    if( xml_file.IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnBitmapButton_Change_DirClick(wxCommandEvent& event)
{
    wxDirDialog* dlg = new wxDirDialog(this, _("Select directory"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));

    wxString newdir;
    if (dlg->ShowModal() == wxID_OK)
    {
        newdir=dlg->GetPath();
        if (newdir == xml_file.GetPath()) return;
        xml_file.SetPath(newdir);
        PopulateFiles();
        Clear();
    }
}
