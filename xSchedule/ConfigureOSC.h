#ifndef CONFIGUREOSC_H
#define CONFIGUREOSC_H

//(*Headers(ConfigureOSC)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/radiobut.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class OSCOptions;

class ConfigureOSC: public wxDialog
{
    OSCOptions* _oscOptions;

    void ValidateWindow();
    bool IsValidPath() const;

	public:

		ConfigureOSC(wxWindow* parent, OSCOptions* oscOptions, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ConfigureOSC();

		//(*Declarations(ConfigureOSC)
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_ServerIP;
		wxStaticText* StaticText6;
		wxStaticText* StaticText8;
		wxChoice* Choice_Frames;
		wxChoice* Choice_RemotePath;
		wxButton* Button_ok;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxSpinCtrl* SpinCtrl_RemotePort;
		wxSpinCtrl* SpinCtrl_ServerPort;
		wxRadioButton* RadioButton_Time;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxRadioButton* RadioButton_Frames;
		wxButton* Button_Cancel;
		wxCheckBox* CheckBox_ServerBroadcast;
		wxTextCtrl* TextCtrl_MasterPath;
		wxStaticText* StaticText4;
		wxChoice* Choice_Time;
		//*)

	protected:

		//(*Identifiers(ConfigureOSC)
		static const long ID_STATICTEXT4;
		static const long ID_RADIOBUTTON1;
		static const long ID_CHOICE1;
		static const long ID_RADIOBUTTON2;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL2;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE3;
		static const long ID_STATICTEXT7;
		static const long ID_SPINCTRL2;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ConfigureOSC)
		void OnTextCtrl_PathText(wxCommandEvent& event);
		void OnRadioButton_TimeSelect(wxCommandEvent& event);
		void OnRadioButton_FramesSelect(wxCommandEvent& event);
		void OnCheckBox_ServerBroadcastClick(wxCommandEvent& event);
		void OnTextCtrl_ServerIPText(wxCommandEvent& event);
		void OnButton_okClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
