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
class wxFlexGridSizer;
class wxGridBagSizer;
class wxSpinCtrlDouble;
class wxSpinEvent;
class wxStaticBoxSizer;
class wxStaticText;
class wxTextCtrl;
//*)
class wxSpinDoubleEvent;

class xLightsFrame;
class OtherSettingsPanel: public wxPanel
{
	public:

		OtherSettingsPanel(wxWindow* parent,xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~OtherSettingsPanel();

		//(*Declarations(OtherSettingsPanel)
		wxCheckBox* CheckBox_BatchRenderPromptIssues;
		wxCheckBox* CheckBox_IgnoreVendorModelRecommendations;
		wxCheckBox* CheckBox_PurgeDownloadCache;
		wxCheckBox* ExcludeAudioCheckBox;
		wxCheckBox* ExcludePresetsCheckBox;
		wxCheckBox* HardwareVideoDecodingCheckBox;
		wxCheckBox* ShaderCheckbox;
		wxChoice* ChoiceCodec;
		wxChoice* Choice_LinkControllerUpload;
		wxChoice* Choice_LinkSave;
		wxSpinCtrlDouble* SpinCtrlDoubleBitrate;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText6;
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
		static const long ID_CHECKBOX7;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE_CODEC;
		static const long ID_STATICTEXT5;
		static const long ID_SPINCTRLDOUBLE_BITRATE;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		static const long ID_CHOICE2;
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX6;
		static const long ID_CHECKBOX5;
		//*)

	private:
        xLightsFrame *frame;

		//(*Handlers(OtherSettingsPanel)
		void OnControlChanged(wxCommandEvent& event);
		void OnSpinCtrlDoubleBitrateChange(wxSpinDoubleEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
