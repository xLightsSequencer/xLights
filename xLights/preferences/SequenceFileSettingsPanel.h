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

//(*Headers(SequenceFileSettingsPanel)
#include <wx/panel.h>
class wxButton;
class wxCheckBox;
class wxChoice;
class wxDirPickerCtrl;
class wxFlexGridSizer;
class wxGridBagSizer;
class wxListBox;
class wxStaticBoxSizer;
class wxStaticText;
//*)

#include <wx/filepicker.h>

class xLightsFrame;
class SequenceFileSettingsPanel: public wxPanel
{
	bool ValidateWindow();
    size_t DecodeMaxRenderCache(const std::string& rcs);
    const std::string EncodeMaxRenderCache(size_t rcs);

	public:

		SequenceFileSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SequenceFileSettingsPanel();

		//(*Declarations(SequenceFileSettingsPanel)
		wxButton* AddMediaButton;
		wxButton* RemoveMediaButton;
		wxCheckBox* CheckBox_FSEQ;
		wxCheckBox* CheckBox_LowDefinitionRender;
		wxCheckBox* CheckBox_RenderCache;
		wxCheckBox* FSEQSaveCheckBox;
		wxCheckBox* RenderOnSaveCheckBox;
		wxChoice* AutoSaveIntervalChoice;
		wxChoice* Choice_MaximumRenderCache;
		wxChoice* FSEQVersionChoice;
		wxChoice* ModelBlendDefaultChoice;
		wxChoice* RenderCacheChoice;
		wxChoice* ViewDefaultChoice;
		wxDirPickerCtrl* DirPickerCtrl_FSEQ;
		wxDirPickerCtrl* DirPickerCtrl_RenderCache;
		wxListBox* MediaDirectoryList;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		//*)
        
        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(SequenceFileSettingsPanel)
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX3;
		static const long ID_CHECKBOX2;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE4;
		static const long ID_CHOICE1;
		static const long ID_CHOICE2;
		static const long ID_CHOICE3;
		static const long ID_CHECKBOX6;
		static const long ID_DIRPICKERCTRL3;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE5;
		static const long ID_CHECKBOX5;
		static const long ID_DIRPICKERCTRL2;
		static const long ID_LISTBOX_MEDIA;
		static const long ID_BUTTON_ADDMEDIA;
		static const long ID_BUTTON_REMOVE_MEDIA;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE_VIEW_DEFAULT;
		//*)

	private:
        xLightsFrame *frame;

		//(*Handlers(SequenceFileSettingsPanel)
		void OnRenderOnSaveCheckBoxClick(wxCommandEvent& event);
		void OnFSEQSaveCheckBoxClick(wxCommandEvent& event);
		void OnRenderCacheChoiceSelect(wxCommandEvent& event);
		void OnAutoSaveIntervalChoiceSelect(wxCommandEvent& event);
		void OnFSEQVersionChoiceSelect(wxCommandEvent& event);
		void OnRenderModeChoiceSelect(wxCommandEvent& event);
		void OnCheckBox_RenderCacheClick(wxCommandEvent& event);
		void OnCheckBox_MediaClick(wxCommandEvent& event);
		void OnCheckBox_FSEQClick(wxCommandEvent& event);
		void OnDirPickerCtrl_RenderCacheDirChanged(wxFileDirPickerEvent& event);
		void OnDirPickerCtrl_MediaDirChanged(wxFileDirPickerEvent& event);
		void OnDirPickerCtrl_FSEQDirChanged(wxFileDirPickerEvent& event);
		void OnAddMediaButtonClick(wxCommandEvent& event);
		void OnRemoveMediaButtonClick(wxCommandEvent& event);
		void OnMediaDirectoryListSelect(wxCommandEvent& event);
		void OnModelBlendDefaultChoiceSelect(wxCommandEvent& event);
		void OnViewDefaultChoiceSelect(wxCommandEvent& event);
		void OnCheckBox_LowDefinitionRenderClick(wxCommandEvent& event);
		void OnChoice_MaximumRenderCacheSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
