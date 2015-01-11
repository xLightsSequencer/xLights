#include "XmlConversionDialog.h"

//(*InternalHeaders(XmlConversionDialog)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(XmlConversionDialog)
const long XmlConversionDialog::ID_STATICTEXT1 = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Filename = wxNewId();
const long XmlConversionDialog::ID_CHOICE_Xml_Settings_Filename = wxNewId();
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
const long XmlConversionDialog::ID_STATICTEXT_Xml_Music_Url = wxNewId();
const long XmlConversionDialog::ID_TEXTCTRL_Xml_Music_Url = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Comment = wxNewId();
const long XmlConversionDialog::ID_TEXTCTRL_Xml_Comment = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Extract_Song_Info = wxNewId();
const long XmlConversionDialog::ID_STATICTEXT_Xml_Timing = wxNewId();
const long XmlConversionDialog::ID_CHOICE1 = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Xml_Import_Timing = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Xml_Delete_Timing = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Xml_Convert = wxNewId();
const long XmlConversionDialog::ID_BUTTON_Xml_Settings_Save = wxNewId();
//*)

BEGIN_EVENT_TABLE(XmlConversionDialog,wxDialog)
	//(*EventTable(XmlConversionDialog)
	//*)
END_EVENT_TABLE()

XmlConversionDialog::XmlConversionDialog(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(XmlConversionDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxStaticBoxSizer* StaticBoxSizer_Xml_Header;
	wxGridBagSizer* GridBagSizer1;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticBoxSizer* StaticBoxSizer_Xml_Song_Timings;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	SetClientSize(wxSize(574,398));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("XML Conversion and Settings Editor"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	wxFont StaticText1Font(12,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,_T("Arial"),wxFONTENCODING_DEFAULT);
	StaticText1->SetFont(StaticText1Font);
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText_Xml_Filename = new wxStaticText(this, ID_STATICTEXT_Xml_Filename, _("XML Filename:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Filename"));
	FlexGridSizer2->Add(StaticText_Xml_Filename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Xml_Settings_Filename = new wxChoice(this, ID_CHOICE_Xml_Settings_Filename, wxDefaultPosition, wxSize(238,21), 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE_Xml_Settings_Filename"));
	FlexGridSizer2->Add(Choice_Xml_Settings_Filename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	StaticText_Xml_Music_Url = new wxStaticText(this, ID_STATICTEXT_Xml_Music_Url, _("Music URL:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Music_Url"));
	FlexGridSizer3->Add(StaticText_Xml_Music_Url, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Xml_Music_Url = new wxTextCtrl(this, ID_TEXTCTRL_Xml_Music_Url, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Music_Url"));
	FlexGridSizer3->Add(TextCtrl_Xml_Music_Url, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Xml_Comment = new wxStaticText(this, ID_STATICTEXT_Xml_Comment, _("Comment:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Xml_Comment"));
	FlexGridSizer3->Add(StaticText_Xml_Comment, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Xml_Comment = new wxTextCtrl(this, ID_TEXTCTRL_Xml_Comment, wxEmptyString, wxDefaultPosition, wxSize(250,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Xml_Comment"));
	FlexGridSizer3->Add(TextCtrl_Xml_Comment, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Extract_Song_Info = new wxButton(this, ID_BUTTON_Extract_Song_Info, _("Extract Song Info from File"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Extract_Song_Info"));
	FlexGridSizer3->Add(Button_Extract_Song_Info, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer_Xml_Header->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(StaticBoxSizer_Xml_Header, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	FlexGridSizer1->Add(StaticBoxSizer_Xml_Song_Timings, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Xml_Convert = new wxButton(this, ID_BUTTON_Xml_Convert, _("Convert"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Xml_Convert"));
	FlexGridSizer4->Add(Button_Xml_Convert, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Xml_Settings_Save = new wxButton(this, ID_BUTTON_Xml_Settings_Save, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Xml_Settings_Save"));
	FlexGridSizer4->Add(Button_Xml_Settings_Save, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();
	//*)
}

XmlConversionDialog::~XmlConversionDialog()
{
	//(*Destroy(XmlConversionDialog)
	//*)
}

