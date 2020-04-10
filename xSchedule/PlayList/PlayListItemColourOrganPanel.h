#pragma once

//(*Headers(PlayListItemColourOrganPanel)
#include <wx/choice.h>
#include <wx/clrpicker.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class OutputManager;
class PlayListItemColourOrgan;

class PlayListItemColourOrganPanel: public wxPanel
{
    OutputManager* _outputManager;
    PlayListItemColourOrgan* _ColourOrgan;
    void ValidateWindow();

	public:

		PlayListItemColourOrganPanel(wxWindow* parent, OutputManager* outputManager, PlayListItemColourOrgan* ColourOrgan, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemColourOrganPanel();

		//(*Declarations(PlayListItemColourOrganPanel)
		wxChoice* Choice_BlendMode;
		wxChoice* Choice_Mode;
		wxColourPickerCtrl* ColourPickerCtrl1;
		wxSpinCtrl* SpinCtrl_Channels;
		wxSpinCtrl* SpinCtrl_EndNote;
		wxSpinCtrl* SpinCtrl_FadeFrames;
		wxSpinCtrl* SpinCtrl_Priority;
		wxSpinCtrl* SpinCtrl_StartNote;
		wxStaticText* StaticText10;
		wxStaticText* StaticText11;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		wxStaticText* StaticText_StartChannel;
		wxTextCtrl* TextCtrl_Duration;
		wxTextCtrl* TextCtrl_Name;
		wxTextCtrl* TextCtrl_StartChannel;
		//*)

	protected:

		//(*Identifiers(PlayListItemColourOrganPanel)
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
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT8;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT11;
		static const long ID_SPINCTRL3;
		static const long ID_STATICTEXT12;
		static const long ID_SPINCTRL5;
		static const long ID_STATICTEXT7;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemColourOrganPanel)
		void OnTextCtrl_NameText(wxCommandEvent& event);
		void OnChoice_ModeSelect(wxCommandEvent& event);
		void OnTextCtrl_StartChannelText(wxCommandEvent& event);
		void OnSpinCtrl_ChannelsChange(wxSpinEvent& event);
		void OnSpinCtrl_StartNoteChange(wxSpinEvent& event);
		void OnSpinCtrl_EndNoteChange(wxSpinEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
