#ifndef ONPANEL_H
#define ONPANEL_H

//(*Headers(OnPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class OnPanel: public wxPanel
{
	public:

		OnPanel(wxWindow* parent);
		virtual ~OnPanel();

		//(*Declarations(OnPanel)
		BulkEditCheckBox* CheckBoxShimmer;
		BulkEditSlider* SliderCycles;
		BulkEditSlider* SliderEnd;
		BulkEditSlider* SliderStart;
		BulkEditSlider* Slider_On_Transparency;
		BulkEditTextCtrl* TextCtrlCycles;
		BulkEditTextCtrl* TextCtrlEnd;
		BulkEditTextCtrl* TextCtrlOnTransparency;
		BulkEditTextCtrl* TextCtrlStart;
		BulkEditValueCurveButton* BitmapButton_On_Transparency;
		wxStaticText* StaticText113;
		wxStaticText* StaticText114;
		wxStaticText* StaticText137;
		wxStaticText* StaticText174;
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