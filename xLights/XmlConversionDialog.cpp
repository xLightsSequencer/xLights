#include "XmlConversionDialog.h"
#include <wx/dir.h>
#include "xLightsMain.h"
#include "RenameTextDialog.h"
#include "NewTimingDialog.h"
#include <wx/collpane.h>

//(*InternalHeaders(XmlConversionDialog)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(XmlConversionDialog)
const long XmlConversionDialog::ID_STATICTEXT_XML_Convert_Title = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Filename = wxNewId();
const long XmlConversionDialog::ID_CHOICE_Xml_Settings_Filename = wxNewId();
const long XmlConversionDialog::ID_BITMAPBUTTON_Change_Dir = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Work_Dir = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Seq_Type = wxNewId();
const long XmlConversionDialog::ID_CHOICE_Xml_Seq_Type = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_MediaFile = wxNewId();
const long XmlConversionDialog::ID_TEXTCTRL_Xml_Media_File = wxNewId();
const long XmlConversionDialog::ID_BITMAPBUTTON_Xml_Media_File = wxNewId();
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
const long XmlConversionDialog::ID_STATICTEXT_Xml_Total_Length = wxNewId();
const long XmlConversionDialog::ID_TEXTCTRL_Xml_Seq_Duration = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Xml_Settings_Save = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Xml_Close_Dialog = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Timing = wxNewId();
const long XmlConversionDialog::ID_CHOICE_Xml_Song_Timings = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Xml_New_Timing = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Xml_Import_Timing = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Xml_Rename_Timing = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Xml_Delete_Timing = wxNewId();
const long XmlConversionDialog::ID_TEXTCTRL_Xml_Log = wxNewId();
//*)

BEGIN_EVENT_TABLE(XmlConversionDialog,wxDialog)
	//(*EventTable(XmlConversionDialog)
	//*)
END_EVENT_TABLE()

#define string_format wxString::Format

XmlConversionDialog::XmlConversionDialog(wxWindow* parent, xLightsXmlFile* file_to_handle_)
:   current_selection(-1),
    xml_file(file_to_handle_),
    fixed_file_mode(true)
{
	//(*Initialize(XmlConversionDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxStaticBoxSizer* StaticBoxSizer_File_Info;
	wxStaticBoxSizer* StaticBoxSizer_Xml_Header;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer3;
	wxGridBagSizer* GridBagSizer2;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxStaticBoxSizer* StaticBoxSizer_Xml_Song_Timings;
	wxFlexGridSizer* FlexGridSizer6;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	SetClientSize(wxSize(574,398));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText_XML_Convert_Title = new wxStaticText(this, ID_STATICTEXT_XML_Convert_Title, _("XML Conversion and Settings Editor"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_XML_Convert_Title"));
	wxFont StaticText_XML_Convert_TitleFont(12,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,_T("Arial"),wxFONTENCODING_DEFAULT);
	StaticText_XML_Convert_Title->SetFont(StaticText_XML_Convert_TitleFont);
	FlexGridSizer1->Add(StaticText_XML_Convert_Title, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
	GridBagSizer2 = new wxGridBagSizer(0, 0);
	StaticText_Xml_Filename = new wxStaticText(this, ID_STATICTEXT_Xml_Filename, _("XML Filename:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Filename"));
	GridBagSizer2->Add(StaticText_Xml_Filename, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Xml_Settings_Filename = new wxChoice(this, ID_CHOICE_Xml_Settings_Filename, wxDefaultPosition, wxSize(238,21), 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE_Xml_Settings_Filename"));
	GridBagSizer2->Add(Choice_Xml_Settings_Filename, wxGBPosition(0, 1), wxGBSpan(1, 2), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Change_Dir = new wxBitmapButton(this, ID_BITMAPBUTTON_Change_Dir, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FOLDER_OPEN")),wxART_MENU), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Change_Dir"));
	GridBagSizer2->Add(BitmapButton_Change_Dir, wxGBPosition(0, 3), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Work_Dir = new wxStaticText(this, ID_STATICTEXT_Work_Dir, _("Directory:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Work_Dir"));
	GridBagSizer2->Add(StaticText_Work_Dir, wxGBPosition(1, 0), wxGBSpan(1, 3), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Xml_Seq_Type = new wxStaticText(this, ID_STATICTEXT_Xml_Seq_Type, _("Sequence Type:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Seq_Type"));
	GridBagSizer2->Add(StaticText_Xml_Seq_Type, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Xml_Seq_Type = new wxChoice(this, ID_CHOICE_Xml_Seq_Type, wxDefaultPosition, wxSize(125,21), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Xml_Seq_Type"));
	Choice_Xml_Seq_Type->Append(_("Media"));
	Choice_Xml_Seq_Type->Append(_("Animation"));
	Choice_Xml_Seq_Type->Disable();
	GridBagSizer2->Add(Choice_Xml_Seq_Type, wxGBPosition(2, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer10 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText_Xml_MediaFile = new wxStaticText(this, ID_STATICTEXT_Xml_MediaFile, _("Media:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_MediaFile"));
	FlexGridSizer10->Add(StaticText_Xml_MediaFile, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Xml_Media_File = new wxTextCtrl(this, ID_TEXTCTRL_Xml_Media_File, wxEmptyString, wxDefaultPosition, wxSize(258,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Media_File"));
	TextCtrl_Xml_Media_File->Disable();
	FlexGridSizer10->Add(TextCtrl_Xml_Media_File, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Xml_Media_File = new wxBitmapButton(this, ID_BITMAPBUTTON_Xml_Media_File, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_CDROM")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Xml_Media_File"));
	BitmapButton_Xml_Media_File->Disable();
	FlexGridSizer10->Add(BitmapButton_Xml_Media_File, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizer2->Add(FlexGridSizer10, wxGBPosition(3, 0), wxGBSpan(1, 4), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6->Add(GridBagSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer_File_Info = new wxStaticBoxSizer(wxHORIZONTAL, this, _("File Information:"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 5, 0, 0);
	StaticText_XML_Type_Version = new wxStaticText(this, ID_STATICTEXT_XML_Type_Version, _("XML Version:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_XML_Type_Version"));
	FlexGridSizer5->Add(StaticText_XML_Type_Version, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_XML_Version = new wxStaticText(this, ID_STATICTEXT_XML_Version, wxEmptyString, wxDefaultPosition, wxSize(70,-1), 0, _T("ID_STATICTEXT_XML_Version"));
	FlexGridSizer5->Add(StaticText_XML_Version, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(40,20,1, wxALL|wxFIXED_MINSIZE|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	StaticText_Xml_Total_Length = new wxStaticText(this, ID_STATICTEXT_Xml_Total_Length, _("Seq. Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Total_Length"));
	FlexGridSizer3->Add(StaticText_Xml_Total_Length, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Xml_Seq_Duration = new wxTextCtrl(this, ID_TEXTCTRL_Xml_Seq_Duration, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Seq_Duration"));
	FlexGridSizer3->Add(TextCtrl_Xml_Seq_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer9 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText_Xml_Timing = new wxStaticText(this, ID_STATICTEXT_Xml_Timing, _("Timing:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Timing"));
	FlexGridSizer9->Add(StaticText_Xml_Timing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Xml_Song_Timings = new wxChoice(this, ID_CHOICE_Xml_Song_Timings, wxDefaultPosition, wxSize(238,21), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Xml_Song_Timings"));
	FlexGridSizer9->Add(Choice_Xml_Song_Timings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8->Add(FlexGridSizer9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	Button_Xml_New_Timing = new wxButton(this, ID_BUTTON_Xml_New_Timing, _("New"), wxDefaultPosition, wxSize(60,23), 0, wxDefaultValidator, _T("ID_BUTTON_Xml_New_Timing"));
	Button_Xml_New_Timing->Disable();
	FlexGridSizer2->Add(Button_Xml_New_Timing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Xml_Import_Timing = new wxButton(this, ID_BUTTON_Xml_Import_Timing, _("Import"), wxDefaultPosition, wxSize(60,23), 0, wxDefaultValidator, _T("ID_BUTTON_Xml_Import_Timing"));
	Button_Xml_Import_Timing->Disable();
	FlexGridSizer2->Add(Button_Xml_Import_Timing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Xml_Rename_Timing = new wxButton(this, ID_BUTTON_Xml_Rename_Timing, _("Rename"), wxDefaultPosition, wxSize(60,23), 0, wxDefaultValidator, _T("ID_BUTTON_Xml_Rename_Timing"));
	Button_Xml_Rename_Timing->Disable();
	FlexGridSizer2->Add(Button_Xml_Rename_Timing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Xml_Delete_Timing = new wxButton(this, ID_BUTTON_Xml_Delete_Timing, _("Delete"), wxDefaultPosition, wxSize(60,23), 0, wxDefaultValidator, _T("ID_BUTTON_Xml_Delete_Timing"));
	Button_Xml_Delete_Timing->Disable();
	FlexGridSizer2->Add(Button_Xml_Delete_Timing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer_Xml_Song_Timings->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(StaticBoxSizer_Xml_Song_Timings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Message Log:"));
	TextCtrl_Xml_Log = new wxTextCtrl(this, ID_TEXTCTRL_Xml_Log, wxEmptyString, wxDefaultPosition, wxSize(293,320), wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Log"));
	StaticBoxSizer1->Add(TextCtrl_Xml_Log, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(StaticBoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer7, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(ID_CHOICE_Xml_Settings_Filename,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&XmlConversionDialog::OnChoice_Xml_Settings_FilenameSelect);
	Connect(ID_BITMAPBUTTON_Change_Dir,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&XmlConversionDialog::OnBitmapButton_Change_DirClick);
	Connect(ID_CHOICE_Xml_Seq_Type,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&XmlConversionDialog::OnChoice_Xml_Seq_TypeSelect);
	Connect(ID_BITMAPBUTTON_Xml_Media_File,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&XmlConversionDialog::OnBitmapButton_Xml_Media_FileClick);
	Connect(ID_TEXTCTRL_Xml_Author,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_AuthorText);
	Connect(ID_TEXTCTRL_Xml_Author_Email,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_Author_EmailText);
	Connect(ID_TEXTCTRL_Xml_Website,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_WebsiteText);
	Connect(ID_TEXTCTRL_Xml_Song,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_SongText);
	Connect(ID_TEXTCTRL_Xml_Artist,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_ArtistText);
	Connect(ID_TEXTCTRL_Xml_Album,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_AlbumText);
	Connect(ID_TEXTCTRL_Xml_Music_Url,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_Music_UrlText);
	Connect(ID_TEXTCTRL_Xml_Comment,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_CommentText);
	Connect(ID_TEXTCTRL_Xml_Seq_Duration,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&XmlConversionDialog::OnTextCtrl_Xml_Seq_DurationText);
	Connect(ID_BUTTON_Xml_Settings_Save,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&XmlConversionDialog::OnButton_Xml_Settings_SaveClick);
	Connect(ID_BUTTON_Xml_Close_Dialog,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&XmlConversionDialog::OnButton_Xml_Close_DialogClick);
	Connect(ID_CHOICE_Xml_Song_Timings,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&XmlConversionDialog::OnChoice_Xml_Song_TimingsSelect);
	Connect(ID_BUTTON_Xml_New_Timing,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&XmlConversionDialog::OnButton_Xml_New_TimingClick);
	Connect(ID_BUTTON_Xml_Import_Timing,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&XmlConversionDialog::OnButton_Xml_Import_TimingClick);
	Connect(ID_BUTTON_Xml_Rename_Timing,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&XmlConversionDialog::OnButton_Xml_Rename_TimingClick);
	Connect(ID_BUTTON_Xml_Delete_Timing,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&XmlConversionDialog::OnButton_Xml_Delete_TimingClick);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&XmlConversionDialog::OnClose);
	//*)

    //wxCollapsiblePane *collpane = new wxCollapsiblePane(this, wxID_ANY, "Details:");

    if( xml_file == NULL )
    {
        xml_file = new xLightsXmlFile();
        PopulateFiles();
        fixed_file_mode = false;
    }
    else
    {
        PopulateFiles();
        SetSelectionToXMLFile();
        ProcessSelectedFile();
        Choice_Xml_Settings_Filename->Enable(false);
        BitmapButton_Change_Dir->Enable(false);
    }
}

XmlConversionDialog::~XmlConversionDialog()
{
	//(*Destroy(XmlConversionDialog)
	//*)
    if( !fixed_file_mode ) xml_file->FreeMemory();
}


void XmlConversionDialog::SetMP3File(wxString mp3_file)
{
    ExtractMetaTagsFromMP3(mp3_file);
    TextCtrl_Xml_Media_File->SetValue(mp3_file);
}

void XmlConversionDialog::PopulateFiles()
{
    wxString filename;
    wxFileName oName;
    wxDir dir(xml_file->GetPath());
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
    StaticText_Work_Dir->SetLabelText(_("Directory: " + xml_file->GetPath()));
}

void XmlConversionDialog::PopulateSongTimings()
{
    Choice_Xml_Song_Timings->Clear();
    Choice_Xml_Song_Timings->Set(xml_file->GetTimingList());
    Button_Xml_Import_Timing->Enable(true);
    Button_Xml_Delete_Timing->Enable(true);
    Button_Xml_Rename_Timing->Enable(true);
    Choice_Xml_Song_Timings->SetSelection(0);
}

void XmlConversionDialog::SetSelectionToXMLFile()
{
    wxString current_file = xml_file->GetFullName();
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
    TextCtrl_Xml_Seq_Duration->SetValue(_(""));
    Button_Xml_Settings_Save->Enable(false);
    Button_Xml_Settings_Save->SetLabel(_("Save"));
}

void XmlConversionDialog::SetWindowState(bool value)
{
    TextCtrl_Xml_Author->Enable(value);
    TextCtrl_Xml_Author_Email->Enable(value);
    TextCtrl_Xml_Website->Enable(value);
    TextCtrl_Xml_Song->Enable(value);
    TextCtrl_Xml_Artist->Enable(value);
    TextCtrl_Xml_Album->Enable(value);
    TextCtrl_Xml_Music_Url->Enable(value);
    TextCtrl_Xml_Comment->Enable(value);
    TextCtrl_Xml_Seq_Duration->Enable(value);
    Button_Xml_New_Timing->Enable(value);
    Button_Xml_Import_Timing->Enable(value);
    Button_Xml_Delete_Timing->Enable(value);
    Button_Xml_Rename_Timing->Enable(value);
    Choice_Xml_Song_Timings->Enable(value);
    Choice_Xml_Seq_Type->Enable(value);
}

void XmlConversionDialog::ProcessSelectedFile()
{
    if( xml_file->IsLoaded() )
    {
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
        Choice_Xml_Seq_Type->SetSelection(Choice_Xml_Seq_Type->FindString(xml_file->GetHeaderInfo(xLightsXmlFile::SEQ_TYPE)));
        TextCtrl_Xml_Media_File->SetValue(xml_file->GetHeaderInfo(xLightsXmlFile::MEDIA_FILE));
        TextCtrl_Xml_Seq_Duration->SetValue(xml_file->GetHeaderInfo(xLightsXmlFile::SEQ_DURATION));
        PopulateSongTimings();
    }
    if( xml_file->IsLoaded() )
    {
        if( xml_file->NeedsConversion() )
        {
            SetWindowState(false);
            Button_Xml_Settings_Save->Enable(true);
            Button_Xml_Settings_Save->SetLabel(_("Convert"));
            Choice_Xml_Seq_Type->SetSelection(0);
        }
        else
        {
            SetWindowState(true);
            ProcessSequenceType();
            Button_Xml_Settings_Save->Enable(false);
        }
    }
}

void XmlConversionDialog::OnChoice_Xml_Settings_FilenameSelect(wxCommandEvent& event)
{
    int selection = Choice_Xml_Settings_Filename->GetSelection();
    if( selection != current_selection )
    {
        xml_file->SetFullName(xml_file_list[selection]);
        xml_file->Load();
        ProcessSelectedFile();
    }
    current_selection = selection;
}

void XmlConversionDialog::OnButton_Xml_Close_DialogClick(wxCommandEvent& event)
{
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
    info.push_back(Choice_Xml_Seq_Type->GetString(Choice_Xml_Seq_Type->GetSelection()));
    info.push_back(TextCtrl_Xml_Media_File->GetValue());
    info.push_back(TextCtrl_Xml_Seq_Duration->GetValue());

    xml_file->SetHeaderInfo(info);
    bool reload = xml_file->NeedsConversion();
    xml_file->Save();
    StaticText_XML_Version->SetLabelText(xml_file->GetVersion());
    if( reload )
    {
        PopulateFiles();
        xml_file->Load();
    }
    TextCtrl_Xml_Seq_Duration->SetValue(xml_file->GetHeaderInfo(xLightsXmlFile::SEQ_DURATION));
    PopulateSongTimings();
    SetWindowState(true);
    ProcessSequenceType();
    Button_Xml_Settings_Save->Enable(false);
    Button_Xml_Settings_Save->SetLabel(_("Save"));
    SetSelectionToXMLFile();
}

void XmlConversionDialog::OnTextCtrl_Xml_AuthorText(wxCommandEvent& event)
{
    if( xml_file->IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_Author_EmailText(wxCommandEvent& event)
{
    if( xml_file->IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_WebsiteText(wxCommandEvent& event)
{
    if( xml_file->IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_SongText(wxCommandEvent& event)
{
    if( xml_file->IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_ArtistText(wxCommandEvent& event)
{
    if( xml_file->IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_AlbumText(wxCommandEvent& event)
{
    if( xml_file->IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_Music_UrlText(wxCommandEvent& event)
{
    if( xml_file->IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_CommentText(wxCommandEvent& event)
{
    if( xml_file->IsLoaded() )
    {
        Button_Xml_Settings_Save->Enable(true);
    }
}

void XmlConversionDialog::OnTextCtrl_Xml_Seq_DurationText(wxCommandEvent& event)
{
    if( xml_file->IsLoaded() )
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
        if (newdir == xml_file->GetPath()) return;
        xml_file->SetPath(newdir);
        PopulateFiles();
        Clear();
    }
    dlg->Destroy();
}

void XmlConversionDialog::OnButton_Xml_Import_TimingClick(wxCommandEvent& event)
{
    wxFileDialog* OpenDialog = new wxFileDialog( this, _("Choose Audacity timing file(s)"), wxEmptyString, wxEmptyString, _("Text files (*.txt)|*.txt"), wxFD_OPEN | wxFD_MULTIPLE, wxDefaultPosition);
    wxString fDir;
    if (OpenDialog->ShowModal() == wxID_OK)
    {
        fDir =	OpenDialog->GetDirectory();
        wxArrayString filenames;
        OpenDialog->GetFilenames(filenames);
        xml_file->ProcessAudacityTimingFiles(fDir, filenames);
    }

    OpenDialog->Destroy();
    PopulateSongTimings();
    Button_Xml_Settings_Save->Enable(true);
}

void XmlConversionDialog::OnButton_Xml_Delete_TimingClick(wxCommandEvent& event)
{
    int selection = Choice_Xml_Song_Timings->GetSelection();
    wxArrayString timing_list = xml_file->GetTimingList();
    xml_file->DeleteTimingSection(timing_list[selection]);
    PopulateSongTimings();
    if( timing_list.Count() <= 1 )
    {
        Button_Xml_Delete_Timing->Enable(false);
        Button_Xml_Rename_Timing->Enable(false);
    }
    Button_Xml_Settings_Save->Enable(true);
}

void XmlConversionDialog::OnButton_Xml_New_TimingClick(wxCommandEvent& event)
{
    NewTimingDialog dialog(this);
    dialog.Fit();
    if (dialog.ShowModal() == wxID_OK)
    {
        wxString selected_timing = dialog.GetTiming();
        xml_file->AddFixedTimingSection(selected_timing);
        PopulateSongTimings();
        Button_Xml_Settings_Save->Enable(true);
    }
    dialog.Destroy();
}

void XmlConversionDialog::OnButton_Xml_Rename_TimingClick(wxCommandEvent& event)
{
    int selection = Choice_Xml_Song_Timings->GetSelection();
    wxArrayString timing_list = xml_file->GetTimingList();
    RenameTextDialog dialog(this);
    dialog.SetRenameText(timing_list[selection]);
    dialog.Fit();
    if (dialog.ShowModal() == wxID_OK)
    {
        xml_file->SetTimingSectionName(timing_list[selection], dialog.GetRenameText());
        PopulateSongTimings();
        Button_Xml_Settings_Save->Enable(true);
    }
    dialog.Destroy();
}

void XmlConversionDialog::OnChoice_Xml_Song_TimingsSelect(wxCommandEvent& event)
{
    if( Choice_Xml_Song_Timings->GetString(Choice_Xml_Song_Timings->GetSelection()) == _("Song Timing"))
    {
        Button_Xml_Delete_Timing->Enable(false);
        Button_Xml_Rename_Timing->Enable(false);
    }
    else
    {
        Button_Xml_Delete_Timing->Enable(true);
        Button_Xml_Rename_Timing->Enable(true);
    }
}

void XmlConversionDialog::OnClose(wxCloseEvent& event)
{
    if( Button_Xml_Settings_Save->IsEnabled() && event.CanVeto())
    {
        int answer = wxMessageBox("Close without saving XML?", "Confirm", wxYES_NO, this);
        if (answer != wxYES ) {
            event.Veto();
            return;
        }
    }
    EndModal(Button_Xml_Settings_Save->IsEnabled() ? wxCANCEL : wxOK);
    Destroy();
}

bool XmlConversionDialog::ExtractMetaTagsFromMP3(wxString filename)
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
            wxString title = wxT("");
            wxString artist = wxT("");
            wxString album = wxT("");

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

            if( title != wxT("") )
            {
                TextCtrl_Xml_Song->SetValue(title);
            }
            if( artist != wxT("") )
            {
                TextCtrl_Xml_Artist->SetValue(artist);
            }
            if( album != wxT("") )
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

void XmlConversionDialog::OnBitmapButton_Xml_Media_FileClick(wxCommandEvent& event)
{
    wxFileDialog* OpenDialog = new wxFileDialog( this, _("Choose Audio file"), wxEmptyString, wxEmptyString, _("MP3 files (*.mp3)|*.mp3"), wxFD_OPEN, wxDefaultPosition);
    wxString fDir;
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
        Button_Xml_Settings_Save->Enable(true);
    }

    OpenDialog->Destroy();
}

void XmlConversionDialog::ProcessSequenceType()
{
    wxString type = xml_file->GetSequenceType();
    BitmapButton_Xml_Media_File->Enable((type == wxT("Media")));
}

void XmlConversionDialog::OnChoice_Xml_Seq_TypeSelect(wxCommandEvent& event)
{
    int selection = Choice_Xml_Seq_Type->GetSelection();
    wxString type = Choice_Xml_Seq_Type->GetString(selection);
    xml_file->SetSequenceType(type);
    ProcessSequenceType();
}
