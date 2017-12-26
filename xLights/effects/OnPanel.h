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

#include "../BulkEditControls.h"

class OnPanel: public wxPanel
{
	public:

		OnPanel(wxWindow* parent);
		virtual ~OnPanel();

		//(*Declarations(OnPanel)
		BulkEditSlider* SliderEnd;
		wxStaticText* StaticText137;
		BulkEditSlider* SliderStart;
		BulkEditTextCtrl* TextCtrlStart;
		BulkEditSlider* Slider_On_Transparency;
		wxStaticText* StaticText174;
		wxStaticText* StaticText114;
		BulkEditTextCtrl* TextCtrlEnd;
		BulkEditCheckBox* CheckBoxShimmer;
		wxStaticText* StaticText113;
		BulkEditTextCtrl* TextCtrlOnTransparency;
		BulkEditTextCtrl* TextCtrlCycles;
		BulkEditSlider* SliderCycles;
		BulkEditValueCurveButton* BitmapButton_On_Transparency;
		//*)

	protected:

		//(*Identifiers(OnPanel)
		static const long ID_STATICTEXT_Eff_On_Start;
		static const long IDD_SLIDER_Eff_On_Start;
		static const long ID_TEXTCTRL_Eff_On_Start;
		static const long ID_STATICTEXT_Eff_On_End;
		static const long IDD_SLIDER_Eff_On_End;
		static const long ID_TEXTCTRL_Eff_On_End;
		static const long ID_STATICTEXT_On_Transparency;
		static const long IDD_SLIDER_On_Transparency;
		static const long ID_VALUECURVE_On_Transparency;
		static const long ID_TEXTCTRL_On_Transparency;
		static const long ID_STATICTEXT_On_Cycles;
		static const long IDD_SLIDER_On_Cycles;
		static const long ID_TEXTCTRL_On_Cycles;
		static const long ID_CHECKBOX_On_Shimmer;
		//*)

	private:

	    //(*Handlers(OnPanel)
        void OnLockButtonClick(wxCommandEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif