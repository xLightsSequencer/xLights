#ifndef SMSSETTINGSDIALOG_H
#define SMSSETTINGSDIALOG_H

//(*Headers(SMSSettingsDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class SMSDaemonOptions;

class SMSSettingsDialog: public wxDialog
{
    SMSDaemonOptions* _options;

	public:

		SMSSettingsDialog(wxWindow* parent, SMSDaemonOptions* options, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SMSSettingsDialog();

		//(*Declarations(SMSSettingsDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxCheckBox* CheckBox_IgnoreOversized;
		wxCheckBox* CheckBox_OneWordOnly;
		wxCheckBox* CheckBox_RejectProfanity;
		wxCheckBox* CheckBox_UpperCase;
		wxCheckBox* CheckBox_UseLocalBlacklist;
		wxCheckBox* CheckBox_UseLocalWhitelist;
		wxCheckBox* CheckBox_UsePhoneBlacklist;
		wxCheckBox* CheckBox_UsePurgoMalum;
		wxChoice* Choice_SMSService;
		wxSpinCtrl* SpinCtrl_DisplayDuration;
		wxSpinCtrl* SpinCtrl_MaxMessageAge;
		wxSpinCtrl* SpinCtrl_MaximumMessageLength;
		wxSpinCtrl* SpinCtrl_RetrieveInterval;
		wxSpinCtrl* SpinCtrl_TimesToDisplay;
		wxSpinCtrl* SpinCtrl_xSchedulePort;
		wxStaticText* StaticText10;
		wxStaticText* StaticText11;
		wxStaticText* StaticText13;
		wxStaticText* StaticText14;
		wxStaticText* StaticText15;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		wxStaticText* StaticText_SID;
		wxStaticText* StaticText_Token;
		wxStaticText* StaticText_User;
		wxTextCtrl* TextCtrl_DefaultMessage;
		wxTextCtrl* TextCtrl_Phone;
		wxTextCtrl* TextCtrl_RejectMessage;
		wxTextCtrl* TextCtrl_SID;
		wxTextCtrl* TextCtrl_SuccessMessage;
		wxTextCtrl* TextCtrl_TargetMatrix;
		wxTextCtrl* TextCtrl_Token;
		wxTextCtrl* TextCtrl_User;
		wxTextCtrl* TextCtrl_xScheduleIPAddress;
		//*)

	protected:

		//(*Identifiers(SMSSettingsDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT7;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT15;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT12;
		static const long ID_TEXTCTRL7;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT10;
		static const long ID_TEXTCTRL5;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX7;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX8;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL3;
		static const long ID_CHECKBOX5;
		static const long ID_CHECKBOX3;
		static const long ID_CHECKBOX6;
		static const long ID_STATICTEXT8;
		static const long ID_SPINCTRL5;
		static const long ID_STATICTEXT9;
		static const long ID_SPINCTRL6;
		static const long ID_STATICTEXT11;
		static const long ID_TEXTCTRL6;
		static const long ID_STATICTEXT13;
		static const long ID_TEXTCTRL8;
		static const long ID_STATICTEXT14;
		static const long ID_TEXTCTRL9;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(SMSSettingsDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnTextCtrl_xScheduleIPAddressText(wxCommandEvent& event);
		void OnTextCtrl_TargetMatrixText(wxCommandEvent& event);
		void OnTextCtrl_TwilioSIDText(wxCommandEvent& event);
		void OnTextCtrl_TwilioTokenText(wxCommandEvent& event);
		void OnTextCtrl_TwilioPhoneText(wxCommandEvent& event);
		void OnTextCtrl_UserText(wxCommandEvent& event);
		void OnCheckBox_UsePurgoMalumClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

        void ValidateWindow();
};

#endif