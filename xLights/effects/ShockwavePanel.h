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

//(*Headers(ShockwavePanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxFlexGridSizer;
class wxNotebook;
class wxNotebookEvent;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class ShockwavePanel: public xlEffectPanel
{
	public:

		ShockwavePanel(wxWindow* parent);
		virtual ~ShockwavePanel();
		virtual void ValidateWindow() override;

		//(*Declarations(ShockwavePanel)
		BulkEditCheckBox* CheckBox_Shockwave_Blend_Edges;
		BulkEditCheckBox* CheckBox_Shockwave_Scale;
		BulkEditSlider* Slider_Shockwave_Accel;
		BulkEditSlider* Slider_Shockwave_CenterX;
		BulkEditSlider* Slider_Shockwave_CenterY;
		BulkEditSlider* Slider_Shockwave_Cycles;
		BulkEditSlider* Slider_Shockwave_End_Radius;
		BulkEditSlider* Slider_Shockwave_End_Width;
		BulkEditSlider* Slider_Shockwave_Start_Radius;
		BulkEditSlider* Slider_Shockwave_Start_Width;
		BulkEditTextCtrl* TextCtrl_Shockwave_Cycles;
		BulkEditValueCurveButton* BitmapButton_Shockwave_CenterX;
		BulkEditValueCurveButton* BitmapButton_Shockwave_CenterY;
		BulkEditValueCurveButton* BitmapButton_Shockwave_End_Radius;
		BulkEditValueCurveButton* BitmapButton_Shockwave_End_Width;
		BulkEditValueCurveButton* BitmapButton_Shockwave_Start_Radius;
		BulkEditValueCurveButton* BitmapButton_Shockwave_Start_Width;
		wxNotebook* Notebook3;
		wxStaticText* StaticText154;
		wxStaticText* StaticText155;
		wxStaticText* StaticText156;
		wxStaticText* StaticText157;
		wxStaticText* StaticText158;
		wxStaticText* StaticText159;
		wxStaticText* StaticText166;
		wxStaticText* StaticText1;
		xlLockButton* BitmapButton_Shockwave_Accel;
		xlLockButton* BitmapButton_Shockwave_Center_X;
		xlLockButton* BitmapButton_Shockwave_Center_Y;
		xlLockButton* BitmapButton_Shockwave_Cycles;
		xlLockButton* BitmapButton_Shockwave_EndRadius;
		xlLockButton* BitmapButton_Shockwave_EndWidth;
		xlLockButton* BitmapButton_Shockwave_StartRadius;
		xlLockButton* BitmapButton_Shockwave_StartWidth;
		//*)

	protected:

		//(*Identifiers(ShockwavePanel)
		static const long ID_STATICTEXT_Shockwave_CenterX;
		static const long ID_SLIDER_Shockwave_CenterX;
		static const long ID_VALUECURVE_Shockwave_CenterX;
		static const long IDD_TEXTCTRL_Shockwave_CenterX;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_CenterX;
		static const long ID_STATICTEXT_Shockwave_CenterY;
		static const long ID_SLIDER_Shockwave_CenterY;
		static const long ID_VALUECURVE_Shockwave_CenterY;
		static const long IDD_TEXTCTRL_Shockwave_CenterY;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_CenterY;
		static const long ID_STATICTEXT_Shockwave_Start_Radius;
		static const long ID_SLIDER_Shockwave_Start_Radius;
		static const long ID_VALUECURVE_Shockwave_Start_Radius;
		static const long IDD_TEXTCTRL_Shockwave_Start_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_Start_Radius;
		static const long ID_STATICTEXT_Shockwave_End_Radius;
		static const long ID_SLIDER_Shockwave_End_Radius;
		static const long ID_VALUECURVE_Shockwave_End_Radius;
		static const long IDD_TEXTCTRL_Shockwave_End_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_End_Radius;
		static const long ID_STATICTEXT_Shockwave_Start_Width;
		static const long ID_SLIDER_Shockwave_Start_Width;
		static const long ID_VALUECURVE_Shockwave_Start_Width;
		static const long IDD_TEXTCTRL_Shockwave_Start_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_Start_Width;
		static const long ID_STATICTEXT_Shockwave_End_Width;
		static const long ID_SLIDER_Shockwave_End_Width;
		static const long ID_VALUECURVE_Shockwave_End_Width;
		static const long IDD_TEXTCTRL_Shockwave_End_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_End_Width;
		static const long ID_PANEL40;
		static const long ID_STATICTEXT_Shockwave_Accel;
		static const long ID_SLIDER_Shockwave_Accel;
		static const long IDD_TEXTCTRL_Shockwave_Accel;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_Accel;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_Shockwave_Cycles;
		static const long IDD_TEXTCTRL_Shockwave_Cycles;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_Cycles;
		static const long ID_CHECKBOX_Shockwave_Blend_Edges;
		static const long ID_CHECKBOX_Shockwave_Scale;
		static const long ID_PANEL44;
		static const long ID_NOTEBOOK_Shockwave;
		//*)

	public:

		//(*Handlers(ShockwavePanel)
		//*)

		DECLARE_EVENT_TABLE()
};
