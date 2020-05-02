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

//(*Headers(SpirographPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class SpirographPanel: public wxPanel
{
	public:

		SpirographPanel(wxWindow* parent);
		virtual ~SpirographPanel();

		//(*Declarations(SpirographPanel)
		BulkEditSlider* Slider_Spirograph_Animate;
		BulkEditSlider* Slider_Spirograph_Length;
		BulkEditSlider* Slider_Spirograph_R;
		BulkEditSlider* Slider_Spirograph_Speed;
		BulkEditSlider* Slider_Spirograph_Width;
		BulkEditSlider* Slider_Spirograph_d;
		BulkEditSlider* Slider_Spirograph_r;
		BulkEditValueCurveButton* BitmapButton_Spirograph_AnimateVC;
		BulkEditValueCurveButton* BitmapButton_Spirograph_LengthVC;
		BulkEditValueCurveButton* BitmapButton_Spirograph_RVC;
		BulkEditValueCurveButton* BitmapButton_Spirograph_SpeedrVC;
		BulkEditValueCurveButton* BitmapButton_Spirograph_WidthVC;
		BulkEditValueCurveButton* BitmapButton_Spirograph_dVC;
		BulkEditValueCurveButton* BitmapButton_Spirograph_rVC;
		wxStaticText* StaticText190;
		wxStaticText* StaticText191;
		wxStaticText* StaticText1;
		wxStaticText* StaticText87;
		wxStaticText* StaticText88;
		wxStaticText* StaticText89;
		wxStaticText* StaticText90;
		wxStaticText* StaticText92;
		xlLockButton* BitmapButton_SpirographAnimate;
		xlLockButton* BitmapButton_SpirographLength;
		xlLockButton* BitmapButton_SpirographR;
		xlLockButton* BitmapButton_SpirographSpeed;
		xlLockButton* BitmapButton_SpirographWidth;
		xlLockButton* BitmapButton_Spirographd;
		xlLockButton* BitmapButton_Spirographr;
		//*)

	protected:

		//(*Identifiers(SpirographPanel)
		static const long ID_STATICTEXT_Spirograph_Speed;
		static const long IDD_SLIDER_Spirograph_Speed;
		static const long ID_VALUECURVE_Spirograph_Speed;
		static const long ID_TEXTCTRL_Spirograph_Speed;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_Speed;
		static const long ID_STATICTEXT_Spirograph_R;
		static const long ID_SLIDER_Spirograph_R;
		static const long ID_VALUECURVE_Spirograph_R;
		static const long IDD_TEXTCTRL_Spirograph_R;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_R;
		static const long ID_STATICTEXT_Spirograph_r;
		static const long ID_SLIDER_Spirograph_r;
		static const long ID_VALUECURVE_Spirograph_r;
		static const long IDD_TEXTCTRL_Spirograph_r;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_r;
		static const long ID_STATICTEXT_Spirograph_d;
		static const long ID_SLIDER_Spirograph_d;
		static const long ID_VALUECURVE_Spirograph_d;
		static const long IDD_TEXTCTRL_Spirograph_d;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_d;
		static const long ID_STATICTEXT_Spirograph_Animate;
		static const long IDD_SLIDER_Spirograph_Animate;
		static const long ID_VALUECURVE_Spirograph_Animate;
		static const long ID_TEXTCTRL_Spirograph_Animate;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_Animate;
		static const long ID_STATICTEXT_Spirograph_Length;
		static const long IDD_SLIDER_Spirograph_Length;
		static const long ID_VALUECURVE_Spirograph_Length;
		static const long ID_TEXTCTRL_Spirograph_Length;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_Length;
		static const long ID_STATICTEXT1;
		static const long IDD_SLIDER_Spirograph_Width;
		static const long ID_VALUECURVE_Spirograph_Width;
		static const long ID_TEXTCTRL_Spirograph_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_Width;
		static const long ID_STATICTEXT91;
		//*)

	public:

		//(*Handlers(SpirographPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};
