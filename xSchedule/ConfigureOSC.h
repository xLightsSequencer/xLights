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

 //(*Headers(ConfigureOSC)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class OSCOptions;

class ConfigureOSC: public wxDialog
{
    OSCOptions* _oscOptions = nullptr;

    void ValidateWindow();
    bool IsValidPath() const;

	public:

		ConfigureOSC(wxWindow* parent, OSCOptions* oscOptions, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ConfigureOSC();

		//(*Declarations(ConfigureOSC)
		wxButton* Button_Cancel;
		wxButton* Button_ok;
		wxCheckBox* CheckBox_ServerBroadcast;
		wxChoice* Choice_Frames;
		wxChoice* Choice_RemotePath;
		wxChoice* Choice_Time;
		wxRadioButton* RadioButton_Frames;
		wxRadioButton* RadioButton_Time;
		wxSpinCtrl* SpinCtrl_RemotePort;
		wxSpinCtrl* SpinCtrl_ServerPort;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxTextCtrl* TextCtrl_MasterPath;
		wxTextCtrl* TextCtrl_ServerIP;
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
