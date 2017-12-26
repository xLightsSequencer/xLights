#ifndef COLORWASHPANEL_H
#define COLORWASHPANEL_H

//(*Headers(ColorWashPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

#include "../BulkEditControls.h"

#define COLOURWASH_CYCLES_MIN 0.1f
#define COLOURWASH_CYCLES_MAX 20

class ColorWashPanel: public wxPanel
{
	public:

		ColorWashPanel(wxWindow* parent);
		virtual ~ColorWashPanel();

		//(*Declarations(ColorWashPanel)
		BulkEditCheckBox* CircularPaletteCheckBox;
		BulkEditSliderF1* SliderCycles;
		BulkEditTextCtrlF1* CyclesTextCtrl;
		wxBitmapButton* BitmapButton_ColorWashVFade;
		BulkEditCheckBox* HFadeCheckBox;
		wxBitmapButton* BitmapButton_ColorWashHFade;
		BulkEditCheckBox* VFadeCheckBox;
		wxStaticText* StaticText5;
		BulkEditValueCurveButton* BitmapButton_ColorWash_CyclesVC;
		wxBitmapButton* BitmapButton_ColorWashCount;
		BulkEditCheckBox* ShimmerCheckBox;
		//*)


		//(*Identifiers(ColorWashPanel)
		static const long ID_STATICTEXT_ColorWash_Cycles;
		static const long IDD_SLIDER_ColorWash_Cycles;
		static const long ID_VALUECURVE_ColorWash_Cycles;
		static const long ID_TEXTCTRL_ColorWash_Cycles;
		static const long ID_BITMAPBUTTON_SLIDER_ColorWash_Count;
		static const long ID_CHECKBOX_ColorWash_VFade;
		static const long ID_BITMAPBUTTON_CHECKBOX_ColorWash_VFade;
		static const long ID_CHECKBOX_ColorWash_HFade;
		static const long ID_BITMAPBUTTON_CHECKBOX_ColorWash_HFade;
		static const long ID_CHECKBOX_ColorWash_Shimmer;
		static const long ID_CHECKBOX_ColorWash_CircularPalette;
		//*)

    
		//(*Handlers(ColorWashPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};

#endif
