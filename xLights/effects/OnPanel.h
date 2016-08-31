#ifndef ONPANEL_H
#define ONPANEL_H


//(*Headers(OnPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

#include "../ValueCurveButton.h"


class OnPanel: public wxPanel
{
	public:

		OnPanel(wxWindow* parent);
		virtual ~OnPanel();

		//(*Declarations(OnPanel)
		wxCheckBox* CheckBoxShimmer;
		wxTextCtrl* TextCtrlEnd;
		wxSlider* Slider_On_Transparency;
		wxSlider* SliderStart;
		wxSlider* SliderEnd;
		ValueCurveButton* BitmapButton_On_Transparency;
		wxTextCtrl* TextCtrlCycles;
		wxTextCtrl* TextCtrlStart;
		wxSlider* SliderCycles;
		wxTextCtrl* TextCtrlOnTransparency;
		//*)

	protected:

		//(*Identifiers(OnPanel)
		static const long IDD_SLIDER_Eff_On_Start;
		static const long ID_TEXTCTRL_Eff_On_Start;
		static const long IDD_SLIDER_Eff_On_End;
		static const long ID_TEXTCTRL_Eff_On_End;
		static const long IDD_SLIDER_On_Transparency;
		static const long ID_VALUECURVE_On_Transparency;
		static const long ID_TEXTCTRL_On_Transparency;
		static const long IDD_SLIDER_On_Cycles;
		static const long ID_TEXTCTRL_On_Cycles;
		static const long ID_CHECKBOX_On_Shimmer;
		//*)

	private:

		//(*Handlers(OnPanel)
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
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
