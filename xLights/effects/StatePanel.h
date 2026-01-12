#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(StatePanel)
#include <wx/panel.h>
class wxChoice;
class wxFlexGridSizer;
class wxRadioButton;
class wxSlider;
class wxStaticBoxSizer;
class wxStaticText;
class wxTextCtrl;
//*)

class StateEffect;
class Model;

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class StatePanel: public xlEffectPanel
{
    StateEffect* _effect;
    Model* _model;
    void UpdateStateList();

	public:
        void SetEffect(StateEffect* effect, Model* model);
		bool _loadingSettings;

		StatePanel(wxWindow* parent);
		virtual ~StatePanel();
		virtual void ValidateWindow() override;

		//(*Declarations(StatePanel)
		BulkEditChoice* Choice_State_Color;
		BulkEditChoice* Choice_State_Mode;
		BulkEditChoice* Choice_State_State;
		BulkEditChoice* Choice_State_TimingTrack;
		BulkEditSlider* SLIDER_State_Fade_Time;
		BulkEditStateChoice* Choice_StateDefinitonChoice;
		BulkEditTextCtrl* TextCtrl_State_Fade_Time;
		wxRadioButton* RadioButton1;
		wxRadioButton* RadioButton2;
		wxStaticText* StaticText14;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		//*)

	protected:

		//(*Identifiers(StatePanel)
		static const wxWindowID ID_STATICTEXT15;
		static const wxWindowID ID_CHOICE_State_StateDefinition;
		static const wxWindowID IDD_RADIOBUTTON_State_State;
		static const wxWindowID ID_CHOICE_State_State;
		static const wxWindowID IDD_RADIOBUTTON_State_TimingTrack;
		static const wxWindowID ID_CHOICE_State_TimingTrack;
		static const wxWindowID ID_STATICTEXT_State_Mode;
		static const wxWindowID ID_CHOICE_State_Mode;
		static const wxWindowID ID_STATICTEXT_State_Color;
		static const wxWindowID ID_CHOICE_State_Color;
		static const wxWindowID ID_STATICTEXT1;
		static const wxWindowID ID_SLIDER_State_Fade_Time;
		static const wxWindowID IDD_TEXTCTRL_State_Fade_Time;
		//*)

	public:

		//(*Handlers(StatePanel)
		void OnMouthMovementTypeSelected(wxCommandEvent& event);
		void OnState_StateDefinitonChoiceSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
