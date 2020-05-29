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
class wxCheckBox;
class wxChoice;
class wxDirPickerCtrl;
class wxGridBagSizer;
class wxStaticBoxSizer;
class wxStaticText;
//*)

#include <wx/filepicker.h>

class xLightsFrame;
class SequenceFileSettingsPanel: public wxPanel
{
	bool ValidateWindow();

	public:

		SequenceFileSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SequenceFileSettingsPanel();

		//(*Declarations(SequenceFileSettingsPanel)
		wxCheckBox* CheckBox_FSEQ;
		wxCheckBox* CheckBox_Media;
		wxCheckBox* CheckBox_RenderCache;
		wxCheckBox* FSEQSaveCheckBox;
		wxCheckBox* ModelBlendCheckBox;
		wxCheckBox* RenderOnSaveCheckBox;
		wxChoice* AutoSaveIntervalChoice;
		wxChoice* FSEQVersionChoice;
		wxChoice* RenderCacheChoice;
		wxDirPickerCtrl* DirPickerCtrl_FSEQ;
		wxDirPickerCtrl* DirPickerCtrl_Media;
		wxDirPickerCtrl* DirPickerCtrl_RenderCache;
		//*)
        
        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(SequenceFileSettingsPanel)
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		static const long ID_CHOICE1;
		static const long ID_CHOICE2;
		static const long ID_CHOICE3;
		static const long ID_CHECKBOX6;
		static const long ID_DIRPICKERCTRL3;
		static const long ID_CHECKBOX5;
		static const long ID_DIRPICKERCTRL2;
		static const long ID_CHECKBOX4;
		static const long ID_DIRPICKERCTRL1;
		//*)

	private:
        xLightsFrame *frame;

		//(*Handlers(SequenceFileSettingsPanel)
		void OnRenderOnSaveCheckBoxClick(wxCommandEvent& event);
		void OnFSEQSaveCheckBoxClick(wxCommandEvent& event);
		void OnModelBlendCheckBoxClick(wxCommandEvent& event);
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
		//*)

		DECLARE_EVENT_TABLE()
};
