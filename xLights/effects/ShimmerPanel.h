#ifndef SHIMMERPANEL_H
#define SHIMMERPANEL_H

//(*Headers(ShimmerPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

#include "../BulkEditControls.h"

class ShimmerPanel: public wxPanel
{
	public:

		ShimmerPanel(wxWindow* parent);
		virtual ~ShimmerPanel();

		//(*Declarations(ShimmerPanel)
		BulkEditCheckBox* CheckBox_PRE_2017_7;
		wxBitmapButton* BitmapButton_Shimmer_Use_All_Colors;
		wxBitmapButton* BitmapButton5;
		wxStaticText* StaticText65;
		BulkEditValueCurveButton* BitmapButton_Shimmer_CyclesVC;
		BulkEditValueCurveButton* BitmapButton_Shimmer_Duty_FactorVC;
		wxStaticText* StaticText67;
		BulkEditSlider* Slider_Shimmer_Duty_Factor;
		BulkEditCheckBox* CheckBox_Shimmer_Use_All_Colors;
		BulkEditSliderF1* Slider_Shimmer_Cycles;
		wxStaticText* StaticText66;
		wxBitmapButton* BitmapButton_Shimmer_Duty_Factor;
		//*)

	protected:

		//(*Identifiers(ShimmerPanel)
		static const long ID_STATICTEXT_Shimmer_Duty_Factor;
		static const long ID_SLIDER_Shimmer_Duty_Factor;
		static const long ID_VALUECURVE_Shimmer_Duty_Factor;
		static const long IDD_TEXTCTRL_Shimmer_Duty_Factor;
		static const long ID_BITMAPBUTTON_DutyFactor;
		static const long ID_STATICTEXT_Shimmer_Cycles;
		static const long IDD_SLIDER_Shimmer_Cycles;
		static const long ID_VALUECURVE_Shimmer_Cycles;
		static const long ID_TEXTCTRL_Shimmer_Cycles;
		static const long ID_BITMAPBUTTON24;
		static const long ID_STATICTEXT69;
		static const long ID_CHECKBOX_Shimmer_Use_All_Colors;
		static const long ID_BITMAPBUTTON_ShimmerUseAllColors;
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

#endif
