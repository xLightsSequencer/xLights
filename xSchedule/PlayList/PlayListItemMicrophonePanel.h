#ifndef PLAYLISTITEMMICROPHONEPANEL_H
#define PLAYLISTITEMMICROPHONEPANEL_H

//(*Headers(PlayListItemMicrophonePanel)
#include <wx/choice.h>
#include <wx/clrpicker.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class OutputManager;
class PlayListItemMicrophone;

class PlayListItemMicrophonePanel: public wxPanel
{
    OutputManager* _outputManager;
    PlayListItemMicrophone* _microphone;
    void ValidateWindow();

	public:

		PlayListItemMicrophonePanel(wxWindow* parent, OutputManager* outputManager, PlayListItemMicrophone* microphone, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemMicrophonePanel();

		//(*Declarations(PlayListItemMicrophonePanel)
		wxChoice* Choice_BlendMode;
		wxChoice* Choice_Mode;
		wxColourPickerCtrl* ColourPickerCtrl1;
		wxSpinCtrl* SpinCtrl_Channels;
		wxStaticText* StaticText10;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText_StartChannel;
		wxTextCtrl* TextCtrl_Duration;
		wxTextCtrl* TextCtrl_Name;
		wxTextCtrl* TextCtrl_StartChannel;
		//*)

	protected:

		//(*Identifiers(PlayListItemMicrophonePanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT3;
		static const long ID_COLOURPICKERCTRL1;
		static const long ID_STATICTEXT10;
		static const long ID_CHOICE4;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT9;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT7;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemMicrophonePanel)
		void OnTextCtrl_NameText(wxCommandEvent& event);
		void OnChoice_ModeSelect(wxCommandEvent& event);
		void OnTextCtrl_StartChannelText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
