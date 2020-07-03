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

//(*Headers(ShimmerPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class ShimmerPanel: public wxPanel
{
	public:

		ShimmerPanel(wxWindow* parent);
		virtual ~ShimmerPanel();

		//(*Declarations(ShimmerPanel)
		BulkEditCheckBox* CheckBox_PRE_2017_7;
		BulkEditCheckBox* CheckBox_Shimmer_Use_All_Colors;
		BulkEditSlider* Slider_Shimmer_Duty_Factor;
		BulkEditSliderF1* Slider_Shimmer_Cycles;
		BulkEditValueCurveButton* BitmapButton_Shimmer_CyclesVC;
		BulkEditValueCurveButton* BitmapButton_Shimmer_Duty_FactorVC;
		wxStaticText* StaticText65;
		wxStaticText* StaticText66;
		wxStaticText* StaticText67;
		xlLockButton* BitmapButton_ShimmerCycles;
		xlLockButton* BitmapButton_Shimmer_Duty_Factor;
		xlLockButton* BitmapButton_Shimmer_Use_All_Colors;
		//*)

	protected:

		//(*Identifiers(ShimmerPanel)
		static const long ID_STATICTEXT_Shimmer_Duty_Factor;
		static const long ID_SLIDER_Shimmer_Duty_Factor;
		static const long ID_VALUECURVE_Shimmer_Duty_Factor;
		static const long IDD_TEXTCTRL_Shimmer_Duty_Factor;
		static const long ID_BITMAPBUTTON_SLIDER_Shimmer_Duty_Factor;
		static const long ID_STATICTEXT_Shimmer_Cycles;
		static const long IDD_SLIDER_Shimmer_Cycles;
		static const long ID_VALUECURVE_Shimmer_Cycles;
		static const long ID_TEXTCTRL_Shimmer_Cycles;
		static const long ID_BITMAPBUTTON_SLIDER_Shimmer_Cycles;
		static const long ID_STATICTEXT69;
		static const long ID_CHECKBOX_Shimmer_Use_All_Colors;
		static const long ID_BITMAPBUTTON_CHECKBOX_Shimmer_Use_All_Colors;
		static const long ID_CHECKBOX_PRE_2017_7;
		//*)

	public:

		//(*Handlers(ShimmerPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};
