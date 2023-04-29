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

 //(*Headers(LightningPanel)
 #include <wx/panel.h>
 class wxBitmapButton;
 class wxCheckBox;
 class wxChoice;
 class wxFlexGridSizer;
 class wxSlider;
 class wxStaticText;
 class wxTextCtrl;
 //*)

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class LightningPanel: public xlEffectPanel
{
	public:

		LightningPanel(wxWindow* parent);
		virtual ~LightningPanel();
		virtual void ValidateWindow() override;

		//(*Declarations(LightningPanel)
		BulkEditCheckBox* CheckBox_ForkedLightning;
		BulkEditChoice* CHOICE_Lightning_Direction;
		BulkEditSlider* Slider_Lightning_BOTX;
		BulkEditSlider* Slider_Lightning_WIDTH;
		BulkEditSlider* Slider_Lightning_TopX;
		BulkEditSlider* Slider_Lightning_TopY;
		BulkEditSlider* Slider_Number_Bolts;
		BulkEditSlider* Slider_Number_Segments;
		BulkEditTextCtrl* TextCtrl86;
		BulkEditTextCtrl* TextCtrl88;
		BulkEditTextCtrl* TextCtrl89;
		BulkEditTextCtrl* TextCtrl90;
		BulkEditValueCurveButton* BitmapButton_Lightning_TopXVC;
		BulkEditValueCurveButton* BitmapButton_Lightning_TopYVC;
		BulkEditValueCurveButton* BitmapButton_Number_BoltsVC;
		BulkEditValueCurveButton* BitmapButton_Number_SegmentsVC;
		wxStaticText* StaticText1;
		wxStaticText* StaticText203;
		wxStaticText* StaticText205;
		wxStaticText* StaticText206;
		wxStaticText* StaticText207;
		wxStaticText* StaticText208;
		wxStaticText* StaticText209;
		wxStaticText* StaticText210;
		xlLockButton* BitmapButton_ForkedLightning;
		xlLockButton* BitmapButton_LightningBOTX;
		xlLockButton* BitmapButton_LightningWIDTH;
		xlLockButton* BitmapButton_LightningTopX;
		xlLockButton* BitmapButton_LightningTopY;
		xlLockButton* BitmapButton_NumberBolts;
		xlLockButton* BitmapButton_NumberSegments;
		//*)

	protected:

		//(*Identifiers(LightningPanel)
		static const long ID_STATICTEXT_Number_Bolts;
		static const long ID_SLIDER_Number_Bolts;
		static const long ID_VALUECURVE_Number_Bolts;
		static const long IDD_TEXTCTRL_Number_Bolts;
		static const long ID_BITMAPBUTTON_SLIDER_Number_Bolts;
		static const long ID_STATICTEXT_Number_Segments;
		static const long ID_SLIDER_Number_Segments;
		static const long ID_VALUECURVE_Number_Segments;
		static const long IDD_TEXTCTRL_Number_Segments;
		static const long ID_BITMAPBUTTON_SLIDER_Number_Segments;
		static const long ID_STATICTEXT_ForkedLightning;
		static const long ID_CHECKBOX_ForkedLightning;
		static const long ID_BITMAPBUTTON_CHECKBOX_ForkedLightning;
		static const long ID_STATICTEXT_Lightning_TopX;
		static const long ID_SLIDER_Lightning_TopX;
		static const long ID_VALUECURVE_Lightning_TopX;
		static const long IDD_TEXTCTRL_Lightning_TopX;
		static const long ID_BITMAPBUTTON_SLIDER_Lightning_TopX;
		static const long ID_STATICTEXT_Lightning_TopY;
		static const long ID_SLIDER_Lightning_TopY;
		static const long ID_VALUECURVE_Lightning_TopY;
		static const long IDD_TEXTCTRL_Lightning_TopY;
		static const long ID_BITMAPBUTTON_SLIDER_Lightning_TopY;
		static const long ID_STATICTEXT_Lightning_BOTX;
		static const long ID_SLIDER_Lightning_BOTX;
		static const long IDD_TEXTCTRL_Lightning_BOTX;
		static const long ID_BITMAPBUTTON_SLIDER_Lightning_BOTX;
		static const long ID_STATICTEXT_Lightning_WIDTH;
		static const long ID_SLIDER_Lightning_WIDTH;
		static const long IDD_TEXTCTRL_Lightning_WIDTH;
		static const long ID_BITMAPBUTTON_SLIDER_Lightning_WIDTH;
		static const long ID_STATICTEXT_Lightning_Direction;
		static const long ID_CHOICE_Lightning_Direction;
		//*)

	public:

		//(*Handlers(LightningPanel)
		//*)

		DECLARE_EVENT_TABLE()
};
