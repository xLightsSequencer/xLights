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

//(*Headers(SelectPanel)
#include <wx/button.h>
#include <wx/clrpicker.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class MainSequencer;
class SequenceElements;
class Effect;
class Element;

class SelectPanel: public wxPanel
{
	public:

		SelectPanel(SequenceElements* elements, MainSequencer* sequencer, wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~SelectPanel();

		void ClearData();

		//(*Declarations(SelectPanel)
		wxButton* Button_Select_All_Color;
		wxButton* Button_Select_All_Time;
		wxButton* Button_Select_Effect_All;
		wxButton* Button_Select_Model_All;
		wxButton* Button_Select_Refresh;
		wxColourPickerCtrl* ColourPickerCtrlSelect;
		wxComboBox* ComboBox_Select_Effect;
		wxListBox* ListBox_Select_Models;
		wxListCtrl* ListCtrl_Select_Effects;
		wxSlider* SliderColorSensitivity;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxTextCtrl* TextCtrl_Select_EndTime;
		wxTextCtrl* TextCtrl_Select_StartTime;
		//*)

	protected:

		//(*Identifiers(SelectPanel)
		static const long ID_STATICTEXT1;
		static const long ID_COMBOBOX_SELECT_EFFECT;
		static const long ID_STATICTEXT2;
		static const long ID_LISTBOX_SELECT_MODELS;
		static const long ID_BUTTON_SELECT_MODEL_ALL;
		static const long ID_STATICTEXT6;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL_SELECT_STARTTIME;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL_SELECT_ENDTIME;
		static const long ID_BUTTON_SELECT_ALL_TIME;
		static const long ID_STATICTEXT7;
		static const long ID_COLOURPICKERCTRL_SELECT;
		static const long ID_SLIDER_COLOR_SENSITIVITY;
		static const long ID_BUTTON_SELECT_ALL_COLOR;
		static const long ID_STATICTEXT4;
		static const long ID_LISTCTRL_Select_Effects;
		static const long ID_BUTTON_SELECT_EFFECT_ALL;
		static const long ID_BUTTON_SELECT_REFRESH;
		//*)

	private:

        SequenceElements * mSequenceElements;
        MainSequencer* mainSequencer;

        void populateModelsList(const std::string& effectType);
        void populateEffectsList();
        void SelectEffects();

		void GetEffectTypes();

		bool ContainsColor(Effect* eff) const;

        std::pair< int, int > GetStartAndEndTime();

		//(*Handlers(SelectPanel)
		void OnListBox_Select_EffectsSelect(wxCommandEvent& event);
		void OnButton_Select_RefreshClick(wxCommandEvent& event);
		void OnComboBox_Select_ModelsTextEnter(wxCommandEvent& event);
		void OnListBox_Select_ModelsSelect(wxCommandEvent& event);
		void OnButton_Select_Model_AllClick(wxCommandEvent& event);
		void OnButton_Select_Effect_AllClick(wxCommandEvent& event);
		void OnTextCtrl_Select_StartTimeText(wxCommandEvent& event);
		void OnTextCtrl_Select_EndTimeText(wxCommandEvent& event);
		void OnButton_Select_All_TimeClick(wxCommandEvent& event);
		void OnComboBox_Select_EffectDropdown(wxCommandEvent& event);
		void OnComboBox_Select_EffectSelected(wxCommandEvent& event);
		void OnColourPickerCtrlSelectColourChanged(wxColourPickerEvent& event);
		void OnSliderColorSensitivityCmdSliderUpdated(wxScrollEvent& event);
		void OnButton_Select_All_ColorClick(wxCommandEvent& event);
		void OnListCtrl_Select_EffectsItemSelect(wxListEvent& event);
		void OnListCtrl_Select_EffectsItemDeselect(wxListEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
