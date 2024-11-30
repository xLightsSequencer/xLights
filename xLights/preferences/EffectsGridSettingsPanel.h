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

//(*Headers(EffectsGridSettingsPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxChoice;
class wxGridBagSizer;
class wxStaticText;
//*)

class xLightsFrame;
class EffectsGridSettingsPanel: public wxPanel
{
	public:

		EffectsGridSettingsPanel(wxWindow* parent,xLightsFrame *f,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EffectsGridSettingsPanel();

		//(*Declarations(EffectsGridSettingsPanel)
		wxCheckBox* ColorUpdateWarnCheckBox;
		wxCheckBox* IconBackgroundsCheckBox;
		wxCheckBox* NodeValuesCheckBox;
		wxCheckBox* SmallWaveformCheckBox;
		wxCheckBox* SnapToTimingCheckBox;
		wxCheckBox* TransistionMarksCheckBox;
		wxChoice* DoubleClickChoice;
		wxChoice* GridSpacingChoice;
		wxStaticText* StaticText1;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(EffectsGridSettingsPanel)
		static const long ID_CHOICE1;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		static const long ID_STATICTEXT1;
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX6;
		static const long ID_CHECKBOX5;
		static const long ID_CHOICE2;
		//*)

	private:
        xLightsFrame *frame;


		//(*Handlers(EffectsGridSettingsPanel)
		void OnIconBackgroundsCheckBoxClick(wxCommandEvent& event);
		void OnNodeValuesCheckBoxClick(wxCommandEvent& event);
		void OnSnapToTimingCheckBoxClick(wxCommandEvent& event);
		void OnSmallWaveformCheckBoxClick(wxCommandEvent& event);
		void OnGridSpacingChoiceSelect(wxCommandEvent& event);
		void OnTransistionMarksCheckBoxClick(wxCommandEvent& event);
		void OnDoubleClickChoiceSelect(wxCommandEvent& event);
		void OnColorUpdateWarnCheckBoxClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
