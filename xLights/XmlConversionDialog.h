#ifndef XMLCONVERSIONDIALOG_H
#define XMLCONVERSIONDIALOG_H

//(*Headers(XmlConversionDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class XmlConversionDialog: public wxDialog
{
	public:

		XmlConversionDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~XmlConversionDialog();

		//(*Declarations(XmlConversionDialog)
		wxStaticText* StaticText_Xml_Author_Email;
		wxButton* Button_Xml_Settings_Save;
		wxButton* Button_Extract_Song_Info;
		wxStaticText* StaticText_Xml_Artist;
		wxStaticText* StaticText_Xml_Website;
		wxChoice* Choice_Xml_Settings_Filename;
		wxStaticText* StaticText_Xml_Music_Url;
		wxStaticText* StaticText1;
		wxTextCtrl* TextCtrl_Xml_Author_Email;
		wxTextCtrl* TextCtrl_Xml_Song;
		wxStaticText* StaticText_Xml_Author;
		wxStaticText* StaticText_Xml_Filename;
		wxButton* Button_Xml_Delete_Timing;
		wxTextCtrl* TextCtrl_Xml_Music_Url;
		wxButton* Button_Xml_Import_Timing;
		wxStaticText* StaticText_Xml_Timing;
		wxTextCtrl* TextCtrl_Xml_Comment;
		wxStaticText* StaticText_Xml_Comment;
		wxTextCtrl* TextCtrl_Xml_Author;
		wxButton* Button_Xml_Convert;
		wxStaticText* StaticText_Xml_Song;
		wxTextCtrl* TextCtrl_Xml_Website;
		wxTextCtrl* TextCtrl_Xml_Artist;
		wxChoice* Choice1;
		//*)

	protected:

		//(*Identifiers(XmlConversionDialog)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT_Xml_Filename;
		static const long ID_CHOICE_Xml_Settings_Filename;
		static const long ID_STATICTEXT_Xml_Author;
		static const long ID_TEXTCTRL_Xml_Author;
		static const long ID_STATICTEXT_Xml_Author_Email;
		static const long ID_TEXTCTRL_Xml_Author_Email;
		static const long ID_STATICTEXT_Xml_Website;
		static const long ID_TEXTCTRL_Xml_Website;
		static const long ID_STATICTEXT_Xml_Song;
		static const long ID_TEXTCTRL_Xml_Song;
		static const long ID_STATICTEXT_Xml_Artist;
		static const long ID_TEXTCTRL_Xml_Artist;
		static const long ID_STATICTEXT_Xml_Music_Url;
		static const long ID_TEXTCTRL_Xml_Music_Url;
		static const long ID_STATICTEXT_Xml_Comment;
		static const long ID_TEXTCTRL_Xml_Comment;
		static const long ID_BUTTON_Extract_Song_Info;
		static const long ID_STATICTEXT_Xml_Timing;
		static const long ID_CHOICE1;
		static const long ID_BUTTON_Xml_Import_Timing;
		static const long ID_BUTTON_Xml_Delete_Timing;
		static const long ID_BUTTON_Xml_Convert;
		static const long ID_BUTTON_Xml_Settings_Save;
		//*)

	private:

		//(*Handlers(XmlConversionDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
