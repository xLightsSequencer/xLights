#ifndef SEQSETTINGSDIALOG_H
#define SEQSETTINGSDIALOG_H

#include "xLightsMain.h"
#include "xLightsXmlFile.h"
#include "tmGridCell.h"

//(*Headers(SeqSettingsDialog)
#include <wx/treectrl.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
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

		SeqSettingsDialog(wxWindow* parent, xLightsXmlFile* file_to_handle_, wxString& media_dir, const wxString& warning, bool wizard_active_ = false);
		virtual ~SeqSettingsDialog();

		//(*Declarations(SeqSettingsDialog)
		wxButton* Button_Layer_Import;
		wxStaticText* StaticText_Xml_Author_Email;
		wxStaticText* StaticText_Num_Models_Label;
		wxStaticText* StaticText_Xml_Seq_Type;
		wxBitmapButton* BitmapButton_Wiz_Anim;
		wxStaticText* StaticText_XML_Version;
		wxStaticText* StaticText_Xml_MediaFile;
		wxBitmapButton* BitmapButton_Wiz_Music;
		wxTreeCtrl* TreeCtrl_Data_Layers;
		wxChoice* Choice_Xml_Seq_Timing;
		wxTextCtrl* TextCtrl_Xml_Album;
		wxStaticText* StaticText_Xml_Artist;
		wxStaticText* StaticText_Warning;
		wxPanel* Panel4;
		wxStaticText* StaticText_Xml_Website;
		wxNotebook* Notebook_Seq_Settings;
		wxPanel* Panel_Wizard;
		wxStaticText* StaticText_Xml_Total_Length;
		wxStaticText* StaticText_Xml_Music_Url;
		wxPanel* Panel1;
		wxTextCtrl* TextCtrl_Xml_Author_Email;
		wxStaticText* StaticText_Num_Models;
		wxCheckBox* CheckBox_Overwrite_Tags;
		wxPanel* Panel3;
		wxTextCtrl* TextCtrl_Xml_Song;
		wxButton* Button_Layer_Delete;
		wxStaticText* StaticText_File;
		wxStaticText* StaticText_Xml_Author;
		wxStaticText* StaticText_Xml_Album;
		wxStaticText* StaticText_XML_Type_Version;
		wxTextCtrl* TextCtrl_Xml_Music_Url;
		wxButton* Button_Xml_Import_Timing;
		wxTextCtrl* TextCtrl_Xml_Seq_Duration;
		wxTextCtrl* TextCtrl_Xml_Media_File;
		wxStaticText* StaticText_Filename;
		wxTextCtrl* TextCtrl_Xml_Comment;
		wxButton* Button_Reimport;
		wxStaticText* StaticText_Xml_Comment;
		wxTextCtrl* TextCtrl_Xml_Author;
		wxStaticText* StaticText_Warn_No_Media;
		wxStaticText* StaticText_Xml_Song;
		wxPanel* Panel2;
		wxChoice* Choice_Xml_Seq_Type;
		wxTextCtrl* TextCtrl_Xml_Website;
		wxButton* Button_Close;
		wxButton* Button_Move_Down;
		wxTextCtrl* TextCtrl_Xml_Artist;
		wxButton* Button_Move_Up;
		wxBitmapButton* BitmapButton_Xml_Media_File;
		wxButton* Button_Xml_New_Timing;
		//*)

		tmGrid* Grid_Timing;

	protected:

		//(*Identifiers(SeqSettingsDialog)
		static const long ID_BITMAPBUTTON_Wiz_Music;
		static const long ID_BITMAPBUTTON_Wiz_Anim;
		static const long ID_PANEL_Wizard;
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
		static const long ID_CHECKBOX_Overwrite_Tags;
		static const long ID_CHOICE_Xml_Seq_Timing;
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
		static const long ID_BUTTON_Xml_New_Timing;
		static const long ID_BUTTON_Xml_Import_Timing;
		static const long ID_PANEL2;
		static const long ID_TREECTRL_Data_Layers;
		static const long ID_BUTTON_Layer_Import;
		static const long ID_BUTTON_Layer_Delete;
		static const long ID_BUTTON_Move_Up;
		static const long ID_BUTTON_Move_Down;
		static const long ID_BUTTON_Reimport;
		static const long ID_PANEL4;
		static const long ID_NOTEBOOK_Seq_Settings;
		static const long ID_STATICTEXT_Warning;
		static const long ID_STATICTEXT_Warn_No_Media;
		static const long ID_BUTTON_Close;
		//*)

		static const long ID_GRID_TIMING;

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
		void OnButton_SaveClick(wxCommandEvent& event);
		void OnButton_CloseClick(wxCommandEvent& event);
		void OnClose(wxCloseEvent& event);
		void OnChoice_Xml_Seq_TimingSelect(wxCommandEvent& event);
		void OnTreeCtrl_Data_LayersBeginDrag(wxTreeEvent& event);
		void OnButton_Layer_ImportClick(wxCommandEvent& event);
		void OnButton_Layer_DeleteClick(wxCommandEvent& event);
		void OnButton_Move_UpClick(wxCommandEvent& event);
		void OnButton_Move_DownClick(wxCommandEvent& event);
		void OnTreeCtrl_Data_LayersSelectionChanged(wxTreeEvent& event);
		void OnTreeCtrl_Data_LayersBeginLabelEdit(wxTreeEvent& event);
		void OnTreeCtrl_Data_LayersEndLabelEdit(wxTreeEvent& event);
		void OnButton_ReimportClick(wxCommandEvent& event);
		void OnBitmapButton_Wiz_MusicClick(wxCommandEvent& event);
		void OnBitmapButton_Wiz_AnimClick(wxCommandEvent& event);
		//*)

		void OnButton_Xml_Rename_TimingClick(wxCommandEvent& event);
        void OnButton_Xml_Delete_TimingClick(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

        xLightsXmlFile* xml_file;
        wxString& media_directory;
        xLightsFrame* xLightsParent;
        int selected_branch_index;
        wxTreeItemId selected_branch;
        bool wizard_active;

        std::vector<wxGridCellButtonRenderer*> mCellRenderers;

        void ProcessSequenceType();
        void PopulateTimingGrid();
        void AddTimingCell(const wxString& name);
        void UpdateDataLayer();
};

#endif
