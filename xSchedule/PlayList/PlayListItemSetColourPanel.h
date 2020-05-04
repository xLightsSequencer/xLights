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

 //(*Headers(PlayListItemSetColourPanel)
 #include <wx/checkbox.h>
 #include <wx/choice.h>
 #include <wx/clrpicker.h>
 #include <wx/panel.h>
 #include <wx/sizer.h>
 #include <wx/spinctrl.h>
 #include <wx/stattext.h>
 #include <wx/textctrl.h>
 //*)

class OutputManager;
class PlayListItemSetColour;

class PlayListItemSetColourPanel: public wxPanel
{
    OutputManager* _outputManager;
    PlayListItemSetColour* _SetColour;

	public:

		PlayListItemSetColourPanel(wxWindow* parent, OutputManager* outputManager, PlayListItemSetColour* SetColour, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemSetColourPanel();
        void ValidateWindow();

		//(*Declarations(PlayListItemSetColourPanel)
		wxCheckBox* CheckBox_FadeToBlack;
		wxCheckBox* CheckBox_LimitNodes;
		wxChoice* Choice_BlendMode;
		wxColourPickerCtrl* ColourPickerCtrl1;
		wxSpinCtrl* SpinCtrl_Nodes;
		wxSpinCtrl* SpinCtrl_Priority;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText_StartChannel;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_SetColourDuration;
		wxTextCtrl* TextCtrl_StartChannel;
		//*)

	protected:

		//(*Identifiers(PlayListItemSetColourPanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_COLOURPICKERCTRL1;
		static const long ID_CHECKBOX2;
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

		//(*Handlers(PlayListItemSetColourPanel)
		void OnTextCtrl_SetColourDurationText(wxCommandEvent& event);
		void OnCheckBox_LimitChannelsClick(wxCommandEvent& event);
		void OnTextCtrl_StartChannelText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

