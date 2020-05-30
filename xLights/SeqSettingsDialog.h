#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "xLightsXmlFile.h"
#include "tmGridCell.h"
#include "FlickerFreeBitmapButton.h"

//(*Headers(SeqSettingsDialog)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/gbsizer.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/treectrl.h>
//*)

#define NEEDS_RENDER 9998

class ConvertLogDialog;

class SeqSettingsDialog: public wxDialog
{
    ConvertLogDialog* _plog;
	public:

		SeqSettingsDialog(wxWindow* parent, xLightsXmlFile* file_to_handle_, std::string& media_dir, const wxString& warning, bool wizard_active_ = false);
		virtual ~SeqSettingsDialog();

        const std::string GetView() const {return selected_view;}

		//(*Declarations(SeqSettingsDialog)
		wxBitmapButton* BitmapButton_ModifyTiming;
		wxBitmapButton* BitmapButton_Xml_Media_File;
		wxButton* Button_Cancel;
		wxButton* Button_Close;
		wxButton* Button_Download;
		wxButton* Button_Layer_Delete;
		wxButton* Button_Layer_Import;
		wxButton* Button_Move_Down;
		wxButton* Button_Move_Up;
		wxButton* Button_Reimport;
		wxButton* Button_Xml_Import_Timing;
		wxButton* Button_Xml_New_Timing;
		wxCheckBox* BlendingCheckBox;
		wxCheckBox* CheckBox_Overwrite_Tags;
		wxChoice* Choice_Xml_Seq_Type;
		wxChoice* RenderModeChoice;
		wxNotebook* Notebook_Seq_Settings;
		wxPanel* PanelInfo;
		wxPanel* PanelMetaData;
		wxPanel* PanelTimings;
		wxPanel* Panel_DataLayers;
		wxStaticText* StaticText1;
		wxStaticText* StaticText_File;
		wxStaticText* StaticText_Filename;
		wxStaticText* StaticText_Info;
		wxStaticText* StaticText_Num_Models;
		wxStaticText* StaticText_Num_Models_Label;
		wxStaticText* StaticText_Warn_No_Media;
		wxStaticText* StaticText_Warning;
		wxStaticText* StaticText_XML_Type_Version;
		wxStaticText* StaticText_XML_Version;
		wxStaticText* StaticText_Xml_Album;
		wxStaticText* StaticText_Xml_Artist;
		wxStaticText* StaticText_Xml_Author;
		wxStaticText* StaticText_Xml_Author_Email;
		wxStaticText* StaticText_Xml_Comment;
		wxStaticText* StaticText_Xml_MediaFile;
		wxStaticText* StaticText_Xml_Music_Url;
		wxStaticText* StaticText_Xml_Seq_Type;
		wxStaticText* StaticText_Xml_Song;
		wxStaticText* StaticText_Xml_Total_Length;
		wxStaticText* StaticText_Xml_Website;
		wxTextCtrl* TextCtrl_Hash;
		wxTextCtrl* TextCtrl_SeqTiming;
		wxTextCtrl* TextCtrl_Xml_Album;
		wxTextCtrl* TextCtrl_Xml_Artist;
		wxTextCtrl* TextCtrl_Xml_Author;
		wxTextCtrl* TextCtrl_Xml_Author_Email;
		wxTextCtrl* TextCtrl_Xml_Comment;
		wxTextCtrl* TextCtrl_Xml_Media_File;
		wxTextCtrl* TextCtrl_Xml_Music_Url;
		wxTextCtrl* TextCtrl_Xml_Seq_Duration;
		wxTextCtrl* TextCtrl_Xml_Song;
		wxTextCtrl* TextCtrl_Xml_Website;
		wxTreeCtrl* TreeCtrl_Data_Layers;
		//*)

		tmGrid* Grid_Timing;
		wxPanel* Panel_Wizard;
        wxGridBagSizer* GridBagSizerWizard;
        wxGridSizer* GridSizerWizButtons;
		FlickerFreeBitmapButton* BitmapButton_Wiz_Music;
		FlickerFreeBitmapButton* BitmapButton_Wiz_Anim;
		FlickerFreeBitmapButton* BitmapButton_25ms;
		FlickerFreeBitmapButton* BitmapButton_50ms;
		FlickerFreeBitmapButton* BitmapButton_Custom;
		FlickerFreeBitmapButton* BitmapButton_lor;
		FlickerFreeBitmapButton* BitmapButton_vixen;
		FlickerFreeBitmapButton* BitmapButton_gled;
		FlickerFreeBitmapButton* BitmapButton_hls;
		FlickerFreeBitmapButton* BitmapButton_lynx;
		FlickerFreeBitmapButton* BitmapButton_xlights;
		FlickerFreeBitmapButton* BitmapButton_quick_start;
		wxButton* Button_SkipImport;
        wxButton* Button_EditMetadata;
        wxButton* Button_ImportTimings;
        wxButton* Button_WizardDone;
        wxChoice* ModelsChoice;
		wxButton* ModelsChoiceNext;

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
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		static const long ID_STATICTEXT_Xml_Total_Length;
		static const long ID_TEXTCTRL_Xml_Seq_Duration;
		static const long ID_CHECKBOX_Overwrite_Tags;
		static const long ID_TEXTCTRL_SeqTiming;
		static const long ID_BITMAPBUTTON__ModifyTiming;
		static const long ID_CHECKBOX1;
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
		static const long ID_CHOICE1;
		static const long ID_TREECTRL_Data_Layers;
		static const long ID_BUTTON_Layer_Import;
		static const long ID_BUTTON_Layer_Delete;
		static const long ID_BUTTON_Move_Up;
		static const long ID_BUTTON_Move_Down;
		static const long ID_BUTTON_Reimport;
		static const long ID_PANEL4;
		static const long ID_NOTEBOOK_Seq_Settings;
		static const long ID_STATICTEXT_Warning;
		static const long ID_STATICTEXT_Info;
		static const long ID_STATICTEXT_Warn_No_Media;
		static const long ID_BUTTON_CANCEL;
		static const long ID_BUTTON_Close;
		//*)

		static const long ID_GRID_TIMING;
        static const long ID_BITMAPBUTTON_Wiz_Music;
        static const long ID_BITMAPBUTTON_Wiz_Anim;
        static const long ID_BITMAPBUTTON_25ms;
        static const long ID_BITMAPBUTTON_50ms;
        static const long ID_BITMAPBUTTON_Custom;
        static const long ID_BITMAPBUTTON_lor;
        static const long ID_BITMAPBUTTON_vixen;
        static const long ID_BITMAPBUTTON_gled;
        static const long ID_BITMAPBUTTON_hls;
        static const long ID_BITMAPBUTTON_lynx;
        static const long ID_BITMAPBUTTON_xlights;
        static const long ID_BITMAPBUTTON_quick_start;
        static const long ID_BUTTON_skip_import;
        static const long ID_BUTTON_edit_metadata;
        static const long ID_BUTTON_import_timings;
        static const long ID_BUTTON_wizard_done;
        static const long ID_PANEL_Wizard;
        static const long ID_CHOICE_Models;
        static const long ID_BUTTON_models_next;

	private:

		//void OnPanel1Paint(wxPaintEvent& event);
		//void OnButton_SaveClick(wxCommandEvent& event);
		//void OnClose(wxCloseEvent& event);
		//void OnBitmapButton_skip_importClick(wxCommandEvent& event);
		//void OnTextCtrl_SeqTimingText(wxCommandEvent& event);
		//void OnTextCtrl_SeqTimingTextEnter(wxCommandEvent& event);

		//(*Handlers(SeqSettingsDialog)
		void OnChoice_Xml_Seq_TypeSelect(wxCommandEvent& event);
		void OnNotebook_Seq_SettingsPageChanged(wxBookCtrlEvent& event);
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
		void OnButton_Xml_New_TimingClick(wxCommandEvent& event);
		void OnButton_Xml_Import_TimingClick(wxCommandEvent& event);
		void OnButton_CloseClick(wxCommandEvent& event);
		void OnTreeCtrl_Data_LayersBeginDrag(wxTreeEvent& event);
		void OnButton_Layer_ImportClick(wxCommandEvent& event);
		void OnButton_Layer_DeleteClick(wxCommandEvent& event);
		void OnButton_Move_UpClick(wxCommandEvent& event);
		void OnButton_Move_DownClick(wxCommandEvent& event);
		void OnTreeCtrl_Data_LayersSelectionChanged(wxTreeEvent& event);
		void OnTreeCtrl_Data_LayersBeginLabelEdit(wxTreeEvent& event);
		void OnTreeCtrl_Data_LayersEndLabelEdit(wxTreeEvent& event);
		void OnButton_ReimportClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnCheckBox1Click(wxCommandEvent& event);
		void OnBitmapButton_ModifyTimingClick(wxCommandEvent& event);
		void OnButton_DownloadClick(wxCommandEvent& event);
		void OnRenderModeChoiceSelect(wxCommandEvent& event);
		//*)

		void OnButton_Xml_Rename_TimingClick(wxCommandEvent& event);
        void OnButton_Xml_Delete_TimingClick(wxCommandEvent& event);
		void OnBitmapButton_Wiz_MusicClick(wxCommandEvent& event);
		void OnBitmapButton_Wiz_AnimClick(wxCommandEvent& event);
		void OnBitmapButton_25msClick(wxCommandEvent& event);
		void OnBitmapButton_50msClick(wxCommandEvent& event);
		void OnBitmapButton_CustomClick(wxCommandEvent& event);
		void OnBitmapButton_lorClick(wxCommandEvent& event);
		void OnBitmapButton_vixenClick(wxCommandEvent& event);
		void OnBitmapButton_gledClick(wxCommandEvent& event);
		void OnBitmapButton_hlsClick(wxCommandEvent& event);
		void OnBitmapButton_lynxClick(wxCommandEvent& event);
		void OnBitmapButton_xlightsClick(wxCommandEvent& event);
		void OnBitmapButton_quick_startClick(wxCommandEvent& event);
		void OnButton_skip_importClick(wxCommandEvent& event);
		void OnButton_EditMetadataClick(wxCommandEvent& event);
		void OnButton_ImportTimingsClick(wxCommandEvent& event);
		void OnButton_Button_WizardDoneClick(wxCommandEvent& event);
		void OnButton_ModelsChoiceNext(wxCommandEvent& event);
		void OnViewSelect(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

        xLightsXmlFile* xml_file = nullptr;
        std::string& media_directory;
		std::string selected_view;
		xLightsFrame* xLightsParent = nullptr;
        int selected_branch_index = 0;
        wxTreeItemId selected_branch;
        bool needs_render = false;
        bool wizard_active = false;
        wxBitmap musical_seq;
        wxBitmap musical_seq_pressed;
        wxBitmap animation_seq;
        wxBitmap animation_seq_pressed;
        wxBitmap time_25ms;
        wxBitmap time_25ms_pressed;
        wxBitmap time_50ms;
        wxBitmap time_50ms_pressed;
        wxBitmap time_custom;
        wxBitmap time_custom_pressed;
        wxBitmap lightorama;
        wxBitmap vixen;
        wxBitmap glediator;
        wxBitmap hls;
        wxBitmap lynx;
        wxBitmap xlights_logo;
        wxBitmap quick_start;
        wxBitmap quick_start_pressed;

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
        void WizardPage5();
        bool ImportDataLayer(const wxString& filetypes, ConvertLogDialog* plog);
        void SetHash();
        void MediaLoad(wxFileName filename);
};
