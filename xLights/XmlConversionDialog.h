#ifndef XMLCONVERSIONDIALOG_H
#define XMLCONVERSIONDIALOG_H

#include <wx/arrstr.h>
#include "xLightsXmlFile.h"

//(*Headers(XmlConversionDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/gbsizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class XmlConversionDialog: public wxDialog
{
	public:

		XmlConversionDialog(wxWindow* parent, xLightsXmlFile* file_to_handle);
		virtual ~XmlConversionDialog();

        void Clear();

		//(*Declarations(XmlConversionDialog)
		wxStaticText* StaticText_Xml_Author_Email;
		wxStaticText* StaticText_Num_Models_Label;
		wxChoice* Choice_Xml_Song_Timings;
		wxButton* Button_Xml_Settings_Save;
		wxStaticText* StaticText_XML_Version;
		wxTextCtrl* TextCtrl_Xml_Album;
		wxButton* Button_Extract_Song_Info;
		wxStaticText* StaticText_Xml_Artist;
		wxStaticText* StaticText_Xml_Website;
		wxStaticText* StaticText_Xml_Total_Length;
		wxChoice* Choice_Xml_Settings_Filename;
		wxStaticText* StaticText_Xml_Music_Url;
		wxTextCtrl* TextCtrl_Xml_Author_Email;
		wxStaticText* StaticText_Num_Models;
		wxTextCtrl* TextCtrl_Xml_Song;
		wxStaticText* StaticText_Xml_Author;
		wxStaticText* StaticText_Xml_Album;
		wxStaticText* StaticText_XML_Type_Version;
		wxStaticText* StaticText_Xml_Filename;
		wxButton* Button_Xml_Delete_Timing;
		wxTextCtrl* TextCtrl_Xml_Music_Url;
		wxButton* Button_Xml_Import_Timing;
		wxStaticText* StaticText_Xml_Timing;
		wxButton* Button_Xml_Rename_Timing;
		wxTextCtrl* TextCtrl_Xml_Log;
		wxStaticText* StaticText_Work_Dir;
		wxTextCtrl* TextCtrl_Xml_Comment;
		wxStaticText* StaticText_Xml_Comment;
		wxTextCtrl* TextCtrl_Xml_Author;
		wxStaticText* StaticText_Xml_Song;
		wxTextCtrl* TextCtrl_Xml_Total_Length;
		wxTextCtrl* TextCtrl_Xml_Website;
		wxBitmapButton* BitmapButton_Change_Dir;
		wxButton* Button_Xml_Close_Dialog;
		wxTextCtrl* TextCtrl_Xml_Artist;
		wxStaticText* StaticText_XML_Convert_Title;
		wxButton* Button_Xml_New_Timing;
		//*)

	protected:

		//(*Identifiers(XmlConversionDialog)
		static const long ID_STATICTEXT_XML_Convert_Title;
		static const long ID_STATICTEXT_Xml_Filename;
		static const long ID_CHOICE_Xml_Settings_Filename;
		static const long ID_BITMAPBUTTON_Change_Dir;
		static const long ID_STATICTEXT_Work_Dir;
		static const long ID_STATICTEXT_XML_Type_Version;
		static const long ID_STATICTEXT_XML_Version;
		static const long ID_STATICTEXT_Num_Models_Label;
		static const long ID_STATICTEXT_Num_Models;
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
		static const long ID_STATICTEXT_Xml_Album;
		static const long ID_TEXTCTRL_Xml_Album;
		static const long ID_STATICTEXT_Xml_Music_Url;
		static const long ID_TEXTCTRL_Xml_Music_Url;
		static const long ID_STATICTEXT_Xml_Comment;
		static const long ID_TEXTCTRL_Xml_Comment;
		static const long ID_STATICTEXT_Xml_Total_Length;
		static const long ID_TEXTCTRL_Xml_Total_Length;
		static const long ID_BUTTON_Extract_Song_Info;
		static const long ID_BUTTON_Xml_Settings_Save;
		static const long ID_BUTTON_Xml_Close_Dialog;
		static const long ID_STATICTEXT_Xml_Timing;
		static const long ID_CHOICE_Xml_Song_Timings;
		static const long ID_BUTTON_Xml_New_Timing;
		static const long ID_BUTTON_Xml_Import_Timing;
		static const long ID_BUTTON_Xml_Rename_Timing;
		static const long ID_BUTTON_Xml_Delete_Timing;
		static const long ID_TEXTCTRL_Xml_Log;
		//*)

	private:
        wxArrayString xml_file_list;
        xLightsXmlFile* xml_file;
        int current_selection;
        bool fixed_file_mode;

        void PopulateFiles();
        void PopulateSongTimings();
        void SetSelectionToXMLFile();
        void ProcessSelectedFile();
        void SetWindowState(bool value);

		//(*Handlers(XmlConversionDialog)
		void OnChoice_Xml_Settings_FilenameSelect(wxCommandEvent& event);
		void OnButton_Xml_Close_DialogClick(wxCommandEvent& event);
		void OnButton_Xml_ConvertClick(wxCommandEvent& event);
		void OnButton_Xml_Settings_SaveClick(wxCommandEvent& event);
		void OnTextCtrl_Xml_AuthorText(wxCommandEvent& event);
		void OnTextCtrl_Xml_Author_EmailText(wxCommandEvent& event);
		void OnTextCtrl_Xml_WebsiteText(wxCommandEvent& event);
		void OnTextCtrl_Xml_SongText(wxCommandEvent& event);
		void OnTextCtrl_Xml_ArtistText(wxCommandEvent& event);
		void OnTextCtrl_Xml_AlbumText(wxCommandEvent& event);
		void OnTextCtrl_Xml_Music_UrlText(wxCommandEvent& event);
		void OnTextCtrl_Xml_CommentText(wxCommandEvent& event);
		void OnBitmapButton_Change_DirClick(wxCommandEvent& event);
		void OnButton_Xml_Import_TimingClick(wxCommandEvent& event);
		void OnButton_Xml_Delete_TimingClick(wxCommandEvent& event);
		void OnButton_Xml_New_TimingClick(wxCommandEvent& event);
		void OnButton_Xml_Rename_TimingClick(wxCommandEvent& event);
		void OnButton_Extract_Song_InfoClick(wxCommandEvent& event);
		void OnChoice_Xml_Song_TimingsSelect(wxCommandEvent& event);
		void OnCustom1Paint(wxPaintEvent& event);
		void OnClose(wxCloseEvent& event);
		void OnTextCtrl_Xml_Total_LengthText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
