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

#include "../ValueCurveButton.h"

class ShimmerPanel: public wxPanel
{
	public:

		ShimmerPanel(wxWindow* parent);
		virtual ~ShimmerPanel();

		//(*Declarations(ShimmerPanel)
		wxBitmapButton* BitmapButton_Shimmer_Use_All_Colors;
		wxBitmapButton* BitmapButton5;
		wxStaticText* StaticText65;
		wxSlider* Slider_Shimmer_Duty_Factor;
		wxSlider* Slider_Shimmer_Cycles;
		wxStaticText* StaticText67;
		wxCheckBox* CheckBox_Shimmer_Use_All_Colors;
		ValueCurveButton* BitmapButton_Shimmer_Duty_FactorVC;
		wxCheckBox* CheckBox_PRE_2017_7;
		ValueCurveButton* BitmapButton_Shimmer_CyclesVC;
		wxStaticText* StaticText66;
		wxBitmapButton* BitmapButton_Shimmer_Duty_Factor;
		//*)

	protected:

		//(*Identifiers(ShimmerPanel)
		static const long ID_STATICTEXT68;
		static const long ID_SLIDER_Shimmer_Duty_Factor;
		static const long ID_VALUECURVE_Shimmer_Duty_Factor;
		static const long IDD_TEXTCTRL_Shimmer_Duty_Factor;
		static const long ID_BITMAPBUTTON_DutyFactor;
		static const long ID_STATICTEXT9;
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
    void UpdateLinkedSliderFloat(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    void UpdateLinkedSlider360(wxCommandEvent& event);
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);
    void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
    void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    void UpdateLinkedSliderFloat2(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
