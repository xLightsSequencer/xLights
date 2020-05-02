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

//(*Headers(StatePanel)
#include <wx/panel.h>
class wxChoice;
class wxFlexGridSizer;
class wxRadioButton;
class wxStaticBoxSizer;
class wxStaticText;
//*)

class StateEffect;
class Model;

#include "../BulkEditControls.h"

class StatePanel: public wxPanel
{
    StateEffect* _effect;
    Model* _model;
    void UpdateStateList();

	public:
        void SetEffect(StateEffect* effect, Model* model);

		StatePanel(wxWindow* parent);
		virtual ~StatePanel();

		//(*Declarations(StatePanel)
		BulkEditChoice* Choice_State_Color;
		BulkEditChoice* Choice_State_Mode;
		BulkEditChoice* Choice_State_TimingTrack;
		wxChoice* Choice_StateDefinitonChoice;
		wxChoice* Choice_State_State;
		wxRadioButton* RadioButton1;
		wxRadioButton* RadioButton2;
		wxStaticText* StaticText14;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		//*)

	protected:

		//(*Identifiers(StatePanel)
		static const long ID_STATICTEXT15;
		static const long ID_CHOICE_State_StateDefinition;
		static const long IDD_RADIOBUTTON_State_State;
		static const long ID_CHOICE_State_State;
		static const long IDD_RADIOBUTTON_State_TimingTrack;
		static const long ID_CHOICE_State_TimingTrack;
		static const long ID_STATICTEXT_State_Mode;
		static const long ID_CHOICE_State_Mode;
		static const long ID_STATICTEXT_State_Color;
		static const long ID_CHOICE_State_Color;
		//*)

	public:

		//(*Handlers(StatePanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnMouthMovementTypeSelected(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnState_StateDefinitonChoiceSelect(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};
