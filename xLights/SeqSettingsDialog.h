#ifndef SEQSETTINGSDIALOG_H
#define SEQSETTINGSDIALOG_H

#include "xLightsXmlFile.h"

//(*Headers(SeqSettingsDialog)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/gbsizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class SeqSettingsDialog: public wxDialog
{
	public:

		SeqSettingsDialog(wxWindow* parent, xLightsXmlFile* file_to_handle_);
		virtual ~SeqSettingsDialog();
    
        void SetMediaFilename(const wxString &filename);
    

		//(*Declarations(SeqSettingsDialog)
		wxStaticText* StaticText_Xml_Author_Email;
		wxStaticText* StaticText_Num_Models_Label;
		wxStaticText* StaticText_Xml_Seq_Type;
		wxChoice* Choice_Xml_Song_Timings;
		wxStaticText* StaticText_XML_Version;
		wxStaticText* StaticText_Xml_MediaFile;
		wxTextCtrl* TextCtrl_Xml_Album;
		wxStaticText* StaticText_Xml_Artist;
		wxStaticText* StaticText_Xml_Website;
		wxNotebook* Notebook_Seq_Settings;
		wxButton* Button_Save;
		wxStaticText* StaticText_Xml_Total_Length;
		wxStaticText* StaticText_Xml_Music_Url;
		wxPanel* Panel1;
		wxTextCtrl* TextCtrl_Xml_Author_Email;
		wxStaticText* StaticText_Num_Models;
		wxPanel* Panel3;
		wxTextCtrl* TextCtrl_Xml_Song;
		wxStaticText* StaticText_File;
		wxStaticText* StaticText_Xml_Author;
		wxStaticText* StaticText_Xml_Album;
		wxStaticText* StaticText_XML_Type_Version;
		wxButton* Button_Xml_Delete_Timing;
		wxTextCtrl* TextCtrl_Xml_Music_Url;
		wxButton* Button_Xml_Import_Timing;
		wxStaticText* StaticText_Xml_Timing;
		wxTextCtrl* TextCtrl_Xml_Seq_Duration;
		wxTextCtrl* TextCtrl_Xml_Media_File;
		wxStaticText* StaticText_Filename;
		wxButton* Button_Xml_Rename_Timing;
		wxTextCtrl* TextCtrl_Xml_Comment;
		wxStaticText* StaticText_Xml_Comment;
		wxTextCtrl* TextCtrl_Xml_Author;
		wxStaticText* StaticText_Xml_Song;
		wxPanel* Panel2;
		wxChoice* Choice_Xml_Seq_Type;
		wxTextCtrl* TextCtrl_Xml_Website;
		wxButton* Button_Close;
		wxTextCtrl* TextCtrl_Xml_Artist;
		wxBitmapButton* BitmapButton_Xml_Media_File;
		wxButton* Button_Xml_New_Timing;
		//*)

	protected:

		//(*Identifiers(SeqSettingsDialog)
		static const long ID_STATICTEXT_File;
		static const long ID_STATICTEXT_Filename;
		static const long ID_STATICTEXT_XML_Type_Version;
		static const long ID_STATICTEXT_XML_Version;
		static const long ID_STATICTEXT_Num_Models_Label;
		static const long ID_STATICTEXT_Num_Models;
		static const long ID_STATICTEXT_Xml_Seq_Type;
		static const long ID_CHOICE_Xml_Seq_Type;
		static const long ID_STATICTEXT_Xml_MediaFile;
		static const long ID_TEXTCTRL_Xml_Media_File;
		static const long ID_BITMAPBUTTON_Xml_Media_File;
		static const long ID_STATICTEXT_Xml_Total_Length;
		static const long ID_TEXTCTRL_Xml_Seq_Duration;
		static const long ID_PANEL3;
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
		static const long ID_PANEL1;
		static const long ID_STATICTEXT_Xml_Timing;
		static const long ID_CHOICE_Xml_Song_Timings;
		static const long ID_BUTTON_Xml_New_Timing;
		static const long ID_BUTTON_Xml_Import_Timing;
		static const long ID_BUTTON_Xml_Rename_Timing;
		static const long ID_BUTTON_Xml_Delete_Timing;
		static const long ID_PANEL2;
		static const long ID_NOTEBOOK_Seq_Settings;
		static const long ID_BUTTON_Save;
		static const long ID_BUTTON_Close;
		//*)

	private:

		//(*Handlers(SeqSettingsDialog)
		void OnNotebook_Seq_SettingsPageChanged(wxNotebookEvent& event);
		void OnPanel1Paint(wxPaintEvent& event);
		void OnChoice_Xml_Seq_TypeSelect(wxCommandEvent& event);
		void OnBitmapButton_Xml_Media_FileClick(wxCommandEvent& event);
		void OnTextCtrl_Xml_AuthorText(wxCommandEvent& event);
		void OnTextCtrl_Xml_Author_EmailText(wxCommandEvent& event);
		void OnTextCtrl_Xml_WebsiteText(wxCommandEvent& event);
		void OnTextCtrl_Xml_SongText(wxCommandEvent& event);
		void OnTextCtrl_Xml_ArtistText(wxCommandEvent& event);
		void OnTextCtrl_Xml_AlbumText(wxCommandEvent& event);
		void OnTextCtrl_Xml_Music_UrlText(wxCommandEvent& event);
		void OnTextCtrl_Xml_CommentText(wxCommandEvent& event);
		void OnTextCtrl_Xml_Seq_DurationText(wxCommandEvent& event);
		void OnChoice_Xml_Song_TimingsSelect(wxCommandEvent& event);
		void OnButton_Xml_New_TimingClick(wxCommandEvent& event);
		void OnButton_Xml_Import_TimingClick(wxCommandEvent& event);
		void OnButton_Xml_Rename_TimingClick(wxCommandEvent& event);
		void OnButton_Xml_Delete_TimingClick(wxCommandEvent& event);
		void OnButton_SaveClick(wxCommandEvent& event);
		void OnButton_CloseClick(wxCommandEvent& event);
		void OnClose(wxCloseEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

        xLightsXmlFile* xml_file;

        void ProcessSequenceType();
        void PopulateSongTimings();
        bool ExtractMetaTagsFromMP3(wxString filename);
};

#endif
