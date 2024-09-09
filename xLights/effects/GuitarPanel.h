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

//(*Headers(GuitarPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include <wx/progdlg.h>
#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class MidiFile;

class GuitarPanel: public xlEffectPanel
{
	public:

		GuitarPanel(wxWindow* parent);
		virtual ~GuitarPanel();
		virtual void ValidateWindow() override;
		void SetTimingTracks(wxCommandEvent& event);

		//(*Declarations(GuitarPanel)
		BulkEditCheckBox* CheckBox_Collapse;
		BulkEditCheckBox* CheckBox_Fade;
		BulkEditCheckBox* CheckBox_ShowStrings;
		BulkEditCheckBox* CheckBox_VaryWaveLengthOnFret;
		BulkEditChoice* Choice_Guitar_MIDITrack_APPLYLAST;
		BulkEditChoice* Choice_Guitar_Type;
		BulkEditSlider* Slider_MaxFrets;
		BulkEditSliderF1* Slider_BaseWaveFactor;
		BulkEditSliderF1* Slider_StringWaveFactor;
		BulkEditTextCtrl* TextCtrl_MaxFrets;
		BulkEditTextCtrlF1* TextCtrl_BaseWaveFactor;
		BulkEditTextCtrlF1* TextCtrl_StringWaveFactor;
		wxChoice* Choice_StringAppearance;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		//*)

	protected:

		//(*Identifiers(GuitarPanel)
		static const wxWindowID ID_STATICTEXT_Guitar_Type;
		static const wxWindowID ID_CHOICE_Guitar_Type;
		static const wxWindowID ID_STATICTEXT_Guitar_MIDITrack_APPLYLAST;
		static const wxWindowID ID_CHOICE_Guitar_MIDITrack_APPLYLAST;
		static const wxWindowID ID_STATICTEXT1;
		static const wxWindowID ID_CHOICE_StringAppearance;
		static const wxWindowID ID_STATICTEXT_Piano_Scale;
		static const wxWindowID ID_SLIDER_MaxFrets;
		static const wxWindowID IDD_TEXTCTRL_MaxFrets;
		static const wxWindowID ID_STATICTEXT2;
		static const wxWindowID ID_SLIDER_BaseWaveFactor;
		static const wxWindowID IDD_TEXTCTRL_BaseWaveFactor;
		static const wxWindowID ID_STATICTEXT3;
		static const wxWindowID ID_SLIDER_StringWaveFactor;
		static const wxWindowID IDD_TEXTCTRL_StringWaveFactor;
		static const wxWindowID ID_CHECKBOX_Fade;
		static const wxWindowID ID_CHECKBOX_Collapse;
		static const wxWindowID ID_CHECKBOX_ShowStrings;
		static const wxWindowID ID_CHECKBOX_VaryWaveLengthOnFret;
		//*)

	public:

		//(*Handlers(GuitarPanel)
		void OnChoice_StringAppearanceSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
