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

 //(*Headers(ColorWashPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
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
		BulkEditCheckBox* HFadeCheckBox;
		BulkEditCheckBox* ShimmerCheckBox;
		BulkEditCheckBox* VFadeCheckBox;
		BulkEditSliderF1* SliderCycles;
		BulkEditTextCtrlF1* CyclesTextCtrl;
		BulkEditValueCurveButton* BitmapButton_ColorWash_CyclesVC;
		wxStaticText* StaticText5;
		xlLockButton* BitmapButton_ColorWashCount;
		xlLockButton* BitmapButton_ColorWashHFade;
		xlLockButton* BitmapButton_ColorWashVFade;
		//*)


		//(*Identifiers(ColorWashPanel)
		static const long ID_STATICTEXT_ColorWash_Cycles;
		static const long IDD_SLIDER_ColorWash_Cycles;
		static const long ID_VALUECURVE_ColorWash_Cycles;
		static const long ID_TEXTCTRL_ColorWash_Cycles;
		static const long ID_BITMAPBUTTON_SLIDER_ColorWash_Cycles;
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
