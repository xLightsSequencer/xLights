#ifndef SHIMMERPANEL_H
#define SHIMMERPANEL_H

//(*Headers(ShimmerPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
//*)

#include "../BulkEditControls.h"

class ShimmerPanel: public wxPanel
{
	public:

		ShimmerPanel(wxWindow* parent);
		virtual ~ShimmerPanel();

		//(*Declarations(ShimmerPanel)
		BulkEditSliderF1* Slider_Shimmer_Cycles;
		BulkEditCheckBox* CheckBox_PRE_2017_7;
		wxBitmapButton* BitmapButton_ShimmerCycles;
		BulkEditSlider* Slider_Shimmer_Duty_Factor;
		BulkEditCheckBox* CheckBox_Shimmer_Use_All_Colors;
		BulkEditValueCurveButton* BitmapButton_Shimmer_CyclesVC;
		wxStaticText* StaticText65;
		wxStaticText* StaticText66;
		wxBitmapButton* BitmapButton_Shimmer_Duty_Factor;
		wxStaticText* StaticText67;
		wxBitmapButton* BitmapButton_Shimmer_Use_All_Colors;
		BulkEditValueCurveButton* BitmapButton_Shimmer_Duty_FactorVC;
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

#endif
