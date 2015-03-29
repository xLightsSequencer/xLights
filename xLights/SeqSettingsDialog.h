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

// create derived wxBitmapButton class to fix flickering issue
class FlickerFreeBitmapButtonCtrl : public wxBitmapButton {
  public:
         FlickerFreeBitmapButtonCtrl(wxWindow* parent,
                                     wxWindowID id,
                                     const wxBitmap& bitmap,
                                     const wxPoint& pos,
                                     const wxSize& size = wxDefaultSize,
                                     long style = 0,
                                     const wxValidator& validator = wxDefaultValidator,
                                     const wxString& name = "ButtonName" ):
         wxBitmapButton(parent,id,bitmap,pos,size,style,validator,name){};
    void OnEraseBackGround(wxEraseEvent& event) {};
         DECLARE_EVENT_TABLE()
};

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
		wxStaticText* StaticText_XML_Version;
		wxStaticText* StaticText_Xml_MediaFile;
		wxTreeCtrl* TreeCtrl_Data_Layers;
		wxChoice* Choice_Xml_Seq_Timing;
		wxTextCtrl* TextCtrl_Xml_Album;
		wxStaticText* StaticText_Xml_Artist;
		wxStaticText* StaticText_Warning;
		wxStaticText* StaticText_Xml_Website;
		wxNotebook* Notebook_Seq_Settings;
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
		wxButton* Button_Cancel;
		wxTextCtrl* TextCtrl_Xml_Media_File;
		wxStaticText* StaticText_Filename;
		wxTextCtrl* TextCtrl_Xml_Comment;
		wxButton* Button_Reimport;
		wxStaticText* StaticText_Xml_Comment;
		wxPanel* Panel_DataLayers;
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
		wxPanel* Panel_Wizard;
        wxGridBagSizer* GridBagSizerWizard;
        wxGridSizer* GridSizerWizButtons;
		FlickerFreeBitmapButtonCtrl* BitmapButton_Wiz_Music;
		FlickerFreeBitmapButtonCtrl* BitmapButton_Wiz_Anim;
		FlickerFreeBitmapButtonCtrl* BitmapButton_25ms;
		FlickerFreeBitmapButtonCtrl* BitmapButton_50ms;
		FlickerFreeBitmapButtonCtrl* BitmapButton_100ms;
		FlickerFreeBitmapButtonCtrl* BitmapButton_lor;
		FlickerFreeBitmapButtonCtrl* BitmapButton_vixen;
		FlickerFreeBitmapButtonCtrl* BitmapButton_gled;
		FlickerFreeBitmapButtonCtrl* BitmapButton_hls;
		FlickerFreeBitmapButtonCtrl* BitmapButton_lynx;
		FlickerFreeBitmapButtonCtrl* BitmapButton_xlights;
		wxButton* Button_SkipImport;
        wxButton* Button_EditMetadata;
        wxButton* Button_ImportTimings;
        wxButton* Button_WizardDone;

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
		static const long ID_BUTTON_CANCEL;
		static const long ID_BUTTON_Close;
		//*)

		static const long ID_GRID_TIMING;
        static const long ID_BITMAPBUTTON_Wiz_Music;
        static const long ID_BITMAPBUTTON_Wiz_Anim;
        static const long ID_BITMAPBUTTON_25ms;
        static const long ID_BITMAPBUTTON_50ms;
        static const long ID_BITMAPBUTTON_100ms;
        static const long ID_BITMAPBUTTON_lor;
        static const long ID_BITMAPBUTTON_vixen;
        static const long ID_BITMAPBUTTON_gled;
        static const long ID_BITMAPBUTTON_hls;
        static const long ID_BITMAPBUTTON_lynx;
        static const long ID_BITMAPBUTTON_xlights;
        static const long ID_BUTTON_skip_import;
        static const long ID_BUTTON_edit_metadata;
        static const long ID_BUTTON_import_timings;
        static const long ID_BUTTON_wizard_done;
        static const long ID_PANEL_Wizard;

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
		void OnTreeCtrl_Data_LayersBeginDrag(wxTreeEvent& event);		void OnBitmapButton_skip_importClick(wxCommandEvent& event);
		void OnButton_Layer_ImportClick(wxCommandEvent& event);
		void OnButton_Layer_DeleteClick(wxCommandEvent& event);
		void OnButton_Move_UpClick(wxCommandEvent& event);
		void OnButton_Move_DownClick(wxCommandEvent& event);
		void OnTreeCtrl_Data_LayersSelectionChanged(wxTreeEvent& event);
		void OnTreeCtrl_Data_LayersBeginLabelEdit(wxTreeEvent& event);
		void OnTreeCtrl_Data_LayersEndLabelEdit(wxTreeEvent& event);
		void OnButton_ReimportClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		void OnButton_Xml_Rename_TimingClick(wxCommandEvent& event);
        void OnButton_Xml_Delete_TimingClick(wxCommandEvent& event);
		void OnBitmapButton_Wiz_MusicClick(wxCommandEvent& event);
		void OnBitmapButton_Wiz_AnimClick(wxCommandEvent& event);
		void OnBitmapButton_25msClick(wxCommandEvent& event);
		void OnBitmapButton_50msClick(wxCommandEvent& event);
		void OnBitmapButton_100msClick(wxCommandEvent& event);
		void OnBitmapButton_lorClick(wxCommandEvent& event);
		void OnBitmapButton_vixenClick(wxCommandEvent& event);
		void OnBitmapButton_gledClick(wxCommandEvent& event);
		void OnBitmapButton_hlsClick(wxCommandEvent& event);
		void OnBitmapButton_lynxClick(wxCommandEvent& event);
		void OnBitmapButton_xlightsClick(wxCommandEvent& event);
		void OnButton_skip_importClick(wxCommandEvent& event);
		void OnButton_EditMetadataClick(wxCommandEvent& event);
		void OnButton_ImportTimingsClick(wxCommandEvent& event);
		void OnButton_Button_WizardDoneClick(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

        xLightsXmlFile* xml_file;
        wxString& media_directory;
        xLightsFrame* xLightsParent;
        int selected_branch_index;
        wxTreeItemId selected_branch;
        bool wizard_active;
        wxBitmap music_seq;
        wxBitmap music_seq_pressed;
        wxBitmap animation_seq;
        wxBitmap animation_pressed;
        wxBitmap time_div_25ms;
        wxBitmap time_div_25ms_pressed;
        wxBitmap time_div_50ms;
        wxBitmap time_div_50ms_pressed;
        wxBitmap time_div_100ms;
        wxBitmap time_div_100ms_pressed;
        wxBitmap lightorama;
        wxBitmap vixen;
        wxBitmap glediator;
        wxBitmap hls;
        wxBitmap lynx;
        wxBitmap xlights_logo;

        std::vector<wxGridCellButtonRenderer*> mCellRenderers;

        void ProcessSequenceType();
        void PopulateTimingGrid();
        void AddTimingCell(const wxString& name);
        void UpdateDataLayer();
        void MediaChooser();
        void RemoveWizard();
        void WizardPage1();
        void WizardPage2();
        void WizardPage3();
        void WizardPage4();
        bool ImportDataLayer(const wxString& filetypes);
};

#endif
