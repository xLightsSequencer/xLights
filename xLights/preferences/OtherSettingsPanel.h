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
		wxCheckBox* CheckBox_RecycleTips;
		wxCheckBox* ExcludeAudioCheckBox;
		wxCheckBox* ExcludePresetsCheckBox;
		wxCheckBox* GPURenderCheckbox;
		wxCheckBox* HardwareVideoDecodingCheckBox;
		wxCheckBox* ShaderCheckbox;
		wxChoice* ChoiceCodec;
		wxChoice* Choice_AliasPromptBehavior;
		wxChoice* Choice_LinkControllerUpload;
		wxChoice* Choice_LinkSave;
		wxChoice* Choice_MinTipLevel;
		wxChoice* HardwareVideoRenderChoice;
		wxSpinCtrlDouble* CtrlPingInterval;
		wxSpinCtrlDouble* SpinCtrlDoubleBitrate;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxTextCtrl* eMailTextControl;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(OtherSettingsPanel)
		static const wxWindowID ID_STATICTEXT1;
		static const wxWindowID ID_CHOICE1;
		static const wxWindowID ID_CHECKBOX1;
		static const wxWindowID ID_CHOICE4;
		static const wxWindowID ID_CHECKBOX7;
		static const wxWindowID ID_STATICTEXT3;
		static const wxWindowID ID_CHOICE_CODEC;
		static const wxWindowID ID_STATICTEXT5;
		static const wxWindowID ID_SPINCTRLDOUBLE_BITRATE;
		static const wxWindowID ID_CHECKBOX2;
		static const wxWindowID ID_CHECKBOX3;
		static const wxWindowID ID_CHECKBOX4;
		static const wxWindowID ID_CHECKBOX6;
		static const wxWindowID ID_CHECKBOX5;
		static const wxWindowID ID_STATICTEXT4;
		static const wxWindowID ID_CHOICE3;
		static const wxWindowID ID_CHECKBOX8;
		static const wxWindowID ID_STATICTEXT2;
		static const wxWindowID ID_CHOICE2;
		static const wxWindowID ID_STATICTEXT6;
		static const wxWindowID ID_CHOICE_ALIASPROMPT;
		static const wxWindowID ID_TEXTCTRL1;
		static const wxWindowID ID_CHECKBOX9;
		static const wxWindowID ID_STATICTEXT7;
		static const wxWindowID ID_CTRLPINGINTERVAL;
		//*)

	private:
        xLightsFrame *frame;

		//(*Handlers(OtherSettingsPanel)
		void OnControlChanged(wxCommandEvent& event);
		void OnSpinCtrlDoubleBitrateChange(wxSpinDoubleEvent& event);
		void OnPaint(wxPaintEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
