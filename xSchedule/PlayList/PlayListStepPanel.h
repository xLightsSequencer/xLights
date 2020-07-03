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

 //(*Headers(PlayListStepPanel)
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListStep;

class PlayListStepPanel: public wxPanel
{
    PlayListStep* _step;

	public:

		PlayListStepPanel(wxWindow* parent, PlayListStep* step, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListStepPanel();

		//(*Declarations(PlayListStepPanel)
		wxCheckBox* CheckBox_EveryStep;
		wxCheckBox* CheckBox_ExcludeFromRandom;
		wxStaticText* StaticText1;
		wxTextCtrl* TextCtrl_PlayListStepName;
		//*)

	protected:

		//(*Identifiers(PlayListStepPanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		//*)

	private:

		//(*Handlers(PlayListStepPanel)
		void OnTextCtrl_PlayListStepNameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

