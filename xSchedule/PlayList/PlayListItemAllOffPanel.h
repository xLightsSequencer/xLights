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

 //(*Headers(PlayListItemAllOffPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class OutputManager;
class PlayListItemAllOff;

class PlayListItemAllOffPanel: public wxPanel
{
    OutputManager* _outputManager;
    PlayListItemAllOff* _alloff;

	public:

		PlayListItemAllOffPanel(wxWindow* parent, OutputManager* outputManager, PlayListItemAllOff* alloff, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemAllOffPanel();
        void ValidateWindow();

		//(*Declarations(PlayListItemAllOffPanel)
		wxStaticText* StaticText_StartChannel;
		wxChoice* Choice_BlendMode;
		wxSpinCtrl* SpinCtrl_Channels;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_AllOffDuration;
		wxStaticText* StaticText6;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxCheckBox* CheckBox_LimitChannels;
		wxTextCtrl* TextCtrl_Delay;
		wxSpinCtrl* SpinCtrl_Priority;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxTextCtrl* TextCtrl_StartChannel;
		wxStaticText* StaticText4;
		wxSpinCtrl* SpinCtrl_AllSet;
		//*)

	protected:

		//(*Identifiers(PlayListItemAllOffPanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT6;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT8;
		static const long ID_STATICTEXT7;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT5;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemAllOffPanel)
		void OnTextCtrl_AllOffDurationText(wxCommandEvent& event);
		void OnCheckBox_LimitChannelsClick(wxCommandEvent& event);
		void OnTextCtrl_StartChannelText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

