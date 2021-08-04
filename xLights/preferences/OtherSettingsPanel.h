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

//(*Headers(OtherSettingsPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxChoice;
class wxGridBagSizer;
class wxStaticBoxSizer;
class wxStaticText;
class wxTextCtrl;
//*)

class xLightsFrame;
class OtherSettingsPanel: public wxPanel
{
	public:

		OtherSettingsPanel(wxWindow* parent,xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~OtherSettingsPanel();

		//(*Declarations(OtherSettingsPanel)
		wxCheckBox* CheckBox_BatchRenderPromptIssues;
		wxCheckBox* CheckBox_WarnGroupIssues;
		wxCheckBox* ExcludeAudioCheckBox;
		wxCheckBox* ExcludePresetsCheckBox;
		wxCheckBox* HardwareVideoDecodingCheckBox;
		wxChoice* Choice_LinkControllerUpload;
		wxChoice* Choice_LinkSave;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxTextCtrl* eMailTextControl;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(OtherSettingsPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		static const long ID_CHOICE2;
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX_WARN_GROUP_ISSUES;
		//*)

	private:
        xLightsFrame *frame;

		//(*Handlers(OtherSettingsPanel)
		void OnExcludeAudioCheckBoxClick(wxCommandEvent& event);
		void OnExcludePresetsCheckBoxClick(wxCommandEvent& event);
		void OnHardwareVideoDecodingCheckBoxClick(wxCommandEvent& event);
		void OneMailTextControlTextEnter(wxCommandEvent& event);
		void OnChoice_LinkSaveSelect(wxCommandEvent& event);
		void OnChoice_LinkControllerUploadSelect(wxCommandEvent& event);
		void OnCheckBox_WarnGroupIssuesClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
