#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
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
#include <wx/listbox.h>
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

		SeqSettingsDialog(wxWindow* parent, xLightsXmlFile* file_to_handle_, const std::list<std::string>& media_dirs, const wxString& warning, const wxString& defaultView, bool wizard_active_ = false, const std::string& media = "", uint32_t durationMS = 0);
		virtual ~SeqSettingsDialog();

        const std::string GetView() const {return selected_view;}

		//(*Declarations(SeqSettingsDialog)
		wxBitmapButton* BitmapButton_ModifyTiming;
		wxBitmapButton* BitmapButton_Xml_Media_File;
		wxButton* ButtonAddSubAudio;
		wxButton* ButtonRemoveSubAudio;
		wxButton* Button_AddMilliseconds;
		wxButton* Button_Cancel;
		wxButton* Button_Close;
		wxButton* Button_Download;
		wxButton* Button_EmailSend;
		wxButton* Button_Layer_Delete;
		wxButton* Button_Layer_Import;
		wxButton* Button_Move_Down;
		wxButton* Button_Move_Up;
		wxButton* Button_MusicOpen;
		wxButton* Button_Reimport;
		wxButton* Button_WebsiteOpen;
		wxButton* Button_Xml_Import_Timing;
		wxButton* Button_Xml_New_Timing;
		wxCheckBox* BlendingCheckBox;
		wxCheckBox* CheckBox_Overwrite_Tags;
		wxChoice* Choice_Xml_Seq_Type;
		wxChoice* RenderModeChoice;
		wxListBox* ListBoxSubAudio;
		wxNotebook* Notebook_Seq_Settings;
		wxPanel* PanelInfo;
		wxPanel* PanelMetaData;
		wxPanel* PanelTimings;
		wxPanel* Panel_DataLayers;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
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
		wxTextCtrl* TextCtrl_Postmiliseconds;
		wxTextCtrl* TextCtrl_Premiliseconds;
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
		static const wxWindowID ID_STATICTEXT_File;
		static const wxWindowID ID_STATICTEXT_Filename;
		static const wxWindowID ID_STATICTEXT_XML_Type_Version;
		static const wxWindowID ID_STATICTEXT_XML_Version;
		static const wxWindowID ID_STATICTEXT_Num_Models_Label;
		static const wxWindowID ID_STATICTEXT_Num_Models;
		static const wxWindowID ID_STATICTEXT_Xml_Seq_Type;
		static const wxWindowID ID_CHOICE_Xml_Seq_Type;
		static const wxWindowID ID_STATICTEXT_Xml_MediaFile;
		static const wxWindowID ID_TEXTCTRL_Xml_Media_File;
		static const wxWindowID ID_BITMAPBUTTON_Xml_Media_File;
		static const wxWindowID ID_STATICTEXT2;
		static const wxWindowID ID_STATICTEXT3;
		static const wxWindowID ID_STATICTEXT1;
		static const wxWindowID ID_TEXTCTRL1;
		static const wxWindowID ID_TEXTCTRL2;
		static const wxWindowID ID_TEXTCTRL3;
		static const wxWindowID ID_BUTTON1;
		static const wxWindowID ID_BUTTON_AddMilliseconds;
		static const wxWindowID ID_STATICTEXT4;
		static const wxWindowID ID_LISTBOX_SUB_AUDIO;
		static const wxWindowID ID_BUTTON_ADD_SUB_AUDIO;
		static const wxWindowID ID_BUTTON_REMOVE_SUB_AUDIO;
		static const wxWindowID ID_STATICTEXT_Xml_Total_Length;
		static const wxWindowID ID_TEXTCTRL_Xml_Seq_Duration;
		static const wxWindowID ID_CHECKBOX_Overwrite_Tags;
		static const wxWindowID ID_TEXTCTRL_SeqTiming;
		static const wxWindowID ID_BITMAPBUTTON__ModifyTiming;
		static const wxWindowID ID_CHECKBOX1;
		static const wxWindowID ID_PANEL3;
		static const wxWindowID ID_STATICTEXT_Xml_Author;
		static const wxWindowID ID_TEXTCTRL_Xml_Author;
		static const wxWindowID ID_STATICTEXT_Xml_Author_Email;
		static const wxWindowID ID_TEXTCTRL_Xml_Author_Email;
		static const wxWindowID ID_BUTTON4;
		static const wxWindowID ID_STATICTEXT_Xml_Website;
		static const wxWindowID ID_TEXTCTRL_Xml_Website;
		static const wxWindowID ID_BUTTON3;
		static const wxWindowID ID_STATICTEXT_Xml_Song;
		static const wxWindowID ID_TEXTCTRL_Xml_Song;
		static const wxWindowID ID_STATICTEXT_Xml_Artist;
		static const wxWindowID ID_TEXTCTRL_Xml_Artist;
		static const wxWindowID ID_STATICTEXT_Xml_Album;
		static const wxWindowID ID_TEXTCTRL_Xml_Album;
		static const wxWindowID ID_STATICTEXT_Xml_Music_Url;
		static const wxWindowID ID_TEXTCTRL_Xml_Music_Url;
		static const wxWindowID ID_BUTTON2;
		static const wxWindowID ID_STATICTEXT_Xml_Comment;
		static const wxWindowID ID_TEXTCTRL_Xml_Comment;
		static const wxWindowID ID_PANEL1;
		static const wxWindowID ID_BUTTON_Xml_New_Timing;
		static const wxWindowID ID_BUTTON_Xml_Import_Timing;
		static const wxWindowID ID_PANEL2;
		static const wxWindowID ID_CHOICE1;
		static const wxWindowID ID_TREECTRL_Data_Layers;
		static const wxWindowID ID_BUTTON_Layer_Import;
		static const wxWindowID ID_BUTTON_Layer_Delete;
		static const wxWindowID ID_BUTTON_Move_Up;
		static const wxWindowID ID_BUTTON_Move_Down;
		static const wxWindowID ID_BUTTON_Reimport;
		static const wxWindowID ID_PANEL4;
		static const wxWindowID ID_NOTEBOOK_Seq_Settings;
		static const wxWindowID ID_STATICTEXT_Warning;
		static const wxWindowID ID_STATICTEXT_Info;
		static const wxWindowID ID_STATICTEXT_Warn_No_Media;
		static const wxWindowID ID_BUTTON_CANCEL;
		static const wxWindowID ID_BUTTON_Close;
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
		void OnButton_EmailSendClick(wxCommandEvent& event);
		void OnButton_WebsiteOpenClick(wxCommandEvent& event);
		void OnButton_MusicOpenClick(wxCommandEvent& event);
		void OnButtonAddSubAudioClick(wxCommandEvent& event);
		void OnButtonRemoveSubAudioClick(wxCommandEvent& event);
		void OnButton_AddMillisecondsClick(wxCommandEvent& event);

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
		void OnTextCtrl_Xml_Seq_DurationLoseFocus(wxFocusEvent& event);

        DECLARE_EVENT_TABLE()

        xLightsXmlFile* xml_file = nullptr;
        const std::list<std::string>& media_directories;
		std::string selected_view;
		xLightsFrame* xLightsParent = nullptr;
        int selected_branch_index = 0;
        wxTreeItemId selected_branch;
        bool needs_render = false;
        bool wizard_active = false;
        wxBitmapBundle musical_seq;
        wxBitmapBundle musical_seq_pressed;
        wxBitmapBundle animation_seq;
        wxBitmapBundle animation_seq_pressed;
        wxBitmapBundle time_25ms;
        wxBitmapBundle time_25ms_pressed;
        wxBitmapBundle time_50ms;
        wxBitmapBundle time_50ms_pressed;
        wxBitmapBundle time_custom;
        wxBitmapBundle time_custom_pressed;
        wxBitmapBundle lightorama;
        wxBitmapBundle vixen;
        wxBitmapBundle glediator;
        wxBitmapBundle hls;
        wxBitmapBundle lynx;
        wxBitmapBundle xlights_logo;
        wxBitmapBundle quick_start;
        wxBitmapBundle quick_start_pressed;

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
		bool UpdateSequenceTiming();
		void ValidateWindow();
};
