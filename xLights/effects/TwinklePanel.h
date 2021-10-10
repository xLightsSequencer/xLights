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

//(*Headers(TwinklePanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class TwinklePanel: public xlEffectPanel
{
	public:

		TwinklePanel(wxWindow* parent);
		virtual ~TwinklePanel();
		virtual void ValidateWindow() override;

		//(*Declarations(TwinklePanel)
		BulkEditCheckBox* CheckBox_Twinkle_ReRandom;
		BulkEditCheckBox* CheckBox_Twinkle_Strobe;
		BulkEditChoice* Choice_Twinkle_Style;
		BulkEditSlider* Slider_Twinkle_Count;
		BulkEditSlider* Slider_Twinkle_Steps;
		BulkEditValueCurveButton* BitmapButton_Twinkle_CountVC;
		BulkEditValueCurveButton* BitmapButton_Twinkle_StepsVC;
		wxStaticText* StaticText104;
		wxStaticText* StaticText1;
		wxStaticText* StaticText85;
		xlLockButton* BitmapButton24;
		xlLockButton* BitmapButton_TwinkleCount;
		xlLockButton* BitmapButton_TwinkleSteps;
		xlLockButton* BitmapButton_TwinkleStrobe;
		//*)

	protected:

		//(*Identifiers(TwinklePanel)
		static const long ID_STATICTEXT_Twinkle_Count;
		static const long ID_SLIDER_Twinkle_Count;
		static const long ID_VALUECURVE_Twinkle_Count;
		static const long IDD_TEXTCTRL_Twinkle_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Twinkle_Count;
		static const long ID_STATICTEXT_Twinkle_Steps;
		static const long ID_SLIDER_Twinkle_Steps;
		static const long ID_VALUECURVE_Twinkle_Steps;
		static const long IDD_TEXTCTRL_Twinkle_Steps;
		static const long ID_BITMAPBUTTON_SLIDER_Twinkle_Steps;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_Twinkle_Style;
		static const long ID_CHECKBOX_Twinkle_Strobe;
		static const long ID_BITMAPBUTTON_CHECKBOX_Twinkle_Strobe;
		static const long ID_CHECKBOX_Twinkle_ReRandom;
		static const long ID_BITMAPBUTTON_CHECKBOX_Twinkle_ReRandom;
		//*)

	public:

		//(*Handlers(TwinklePanel)
		void OnBitmapButton_Twinkle_CountVCClick(wxCommandEvent& event);
		void OnBitmapButton_Twinkle_StepsVCClick(wxCommandEvent& event);
		//*)

    DECLARE_EVENT_TABLE()
};
