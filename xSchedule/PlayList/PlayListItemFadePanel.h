#ifndef PLAYLISTITEMFADEPANEL_H
#define PLAYLISTITEMFADEPANEL_H

//(*Headers(PlayListItemFadePanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class OutputManager;
class PlayListItemFade;

class PlayListItemFadePanel: public wxPanel
{
    OutputManager* _outputManager;
    PlayListItemFade* _Fade;

	public:

		PlayListItemFadePanel(wxWindow* parent, OutputManager* outputManager, PlayListItemFade* Fade, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemFadePanel();
        void ValidateWindow();

		//(*Declarations(PlayListItemFadePanel)
		wxCheckBox* CheckBox_LimitChannels;
		wxCheckBox* CheckBox_RemainderOfStepTime;
		wxChoice* Choice_FadeMode;
		wxSpinCtrl* SpinCtrl_Channels;
		wxSpinCtrl* SpinCtrl_Priority;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText_StartChannel;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_FadeDuration;
		wxTextCtrl* TextCtrl_StartChannel;
		//*)

	protected:

		//(*Identifiers(PlayListItemFadePanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX2;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE1;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT6;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT8;
		static const long ID_STATICTEXT7;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT5;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemFadePanel)
		void OnTextCtrl_FadeDurationText(wxCommandEvent& event);
		void OnCheckBox_LimitChannelsClick(wxCommandEvent& event);
		void OnTextCtrl_StartChannelText(wxCommandEvent& event);
		void OnCheckBox_RemainderOfStepTimeClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
