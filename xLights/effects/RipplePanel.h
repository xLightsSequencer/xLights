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

//(*Headers(RipplePanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxChoice;
class wxFilePickerCtrl;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class RipplePanel: public xlEffectPanel
{
	public:

		RipplePanel(wxWindow* parent);
		virtual ~RipplePanel();
		virtual void ValidateWindow() override;

		//(*Declarations(RipplePanel)
		BulkEditCheckBox* CheckBox_Ripple3D;
		BulkEditChoice* Choice_Ripple_Draw_Style;
		BulkEditChoice* Choice_Ripple_Movement;
		BulkEditChoice* Choice_Ripple_Object_To_Draw;
		BulkEditFilePickerCtrl* FilePickerCtrl_Ripple_SVG;
		BulkEditSlider* Slider_Ripple_Direction;
		BulkEditSlider* Slider_Ripple_Points;
		BulkEditSlider* Slider_Ripple_Rotation;
		BulkEditSlider* Slider_Ripple_Scale;
		BulkEditSlider* Slider_Ripple_Thickness;
		BulkEditSlider* Slider_Ripple_XC;
		BulkEditSlider* Slider_Ripple_YC;
		BulkEditSliderF1* Slider_Ripple_Cycles;
		BulkEditSliderF1* Slider_Ripple_Outline;
		BulkEditSliderF1* Slider_Ripple_Spacing;
		BulkEditSliderF1* Slider_Ripple_Twist;
		BulkEditSliderF1* Slider_Ripple_Velocity;
		BulkEditTextCtrl* TextCtrl_Ripple_Direction;
		BulkEditTextCtrl* TextCtrl_Ripple_Points;
		BulkEditTextCtrl* TextCtrl_Ripple_Rotation;
		BulkEditTextCtrl* TextCtrl_Ripple_Scale;
		BulkEditTextCtrl* TextCtrl_Ripple_XC;
		BulkEditTextCtrl* TextCtrl_Ripple_YC;
		BulkEditTextCtrlF1* TextCtrl_Ripple_Outline;
		BulkEditTextCtrlF1* TextCtrl_Ripple_Spacing;
		BulkEditTextCtrlF1* TextCtrl_Ripple_Twist;
		BulkEditTextCtrlF1* TextCtrl_Ripple_Velocity;
		BulkEditValueCurveButton* BitmapButton_Ripple_CyclesVC;
		BulkEditValueCurveButton* BitmapButton_Ripple_DirectionVC;
		BulkEditValueCurveButton* BitmapButton_Ripple_OutlineVC;
		BulkEditValueCurveButton* BitmapButton_Ripple_RotationVC;
		BulkEditValueCurveButton* BitmapButton_Ripple_ScaleVC;
		BulkEditValueCurveButton* BitmapButton_Ripple_SpacingVC;
		BulkEditValueCurveButton* BitmapButton_Ripple_ThicknessVC;
		BulkEditValueCurveButton* BitmapButton_Ripple_TwistVC;
		BulkEditValueCurveButton* BitmapButton_Ripple_VelocityVC;
		BulkEditValueCurveButton* BitmapButton_Ripple_XCVC;
		BulkEditValueCurveButton* BitmapButton_Ripple_YCVC;
		wxStaticText* StaticText176;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText69;
		wxStaticText* StaticText6;
		wxStaticText* StaticText70;
		wxStaticText* StaticText72;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		wxStaticText* StaticText_Ripple_Rotation;
		xlLockButton* BitmapButton_Ripple3D;
		xlLockButton* BitmapButton_RippleThickness;
		xlLockButton* BitmapButton_Ripple_Direction;
		xlLockButton* BitmapButton_Ripple_Draw_Style;
		xlLockButton* BitmapButton_Ripple_Movement;
		xlLockButton* BitmapButton_Ripple_Object_To_Draw;
		xlLockButton* BitmapButton_Ripple_Outline;
		xlLockButton* BitmapButton_Ripple_Points;
		xlLockButton* BitmapButton_Ripple_Rotation;
		xlLockButton* BitmapButton_Ripple_Scale;
		xlLockButton* BitmapButton_Ripple_Spacing;
		xlLockButton* BitmapButton_Ripple_Twist;
		xlLockButton* BitmapButton_Ripple_Velocity;
		//*)

	protected:

		//(*Identifiers(RipplePanel)
		static const long ID_STATICTEXT_Ripple_Draw_Style;
		static const long ID_CHOICE_Ripple_Draw_Style;
		static const long ID_BITMAPBUTTON_CHOICE_Ripple_Draw_Style;
		static const long ID_STATICTEXT_Ripple_Object_To_Draw;
		static const long ID_CHOICE_Ripple_Object_To_Draw;
		static const long ID_BITMAPBUTTON_CHOICE_Ripple_Object_To_Draw;
		static const long ID_FILEPICKERCTRL_Ripple_SVG;
		static const long ID_STATICTEXT_Ripple_Movement;
		static const long ID_CHOICE_Ripple_Movement;
		static const long ID_BITMAPBUTTON_CHOICE_Ripple_Movement;
		static const long ID_STATICTEXT6;
		static const long ID_SLIDER_Ripple_Scale;
		static const long ID_VALUECURVE_Ripple_Scale;
		static const long IDD_TEXTCTRL_Ripple_Scale;
		static const long ID_BITMAPBUTTON_SLIDER_Ripple_Scale;
		static const long IDD_SLIDER_Ripple_Outline;
		static const long ID_VALUECURVE_Ripple_Outline;
		static const long ID_TEXTCTRL_Ripple_Outline;
		static const long ID_BITMAPBUTTON_SLIDER_Ripple_Outline;
		static const long ID_STATICTEXT_Ripple_Thickness;
		static const long ID_SLIDER_Ripple_Thickness;
		static const long ID_VALUECURVE_Ripple_Thickness;
		static const long IDD_TEXTCTRL_Ripple_Thickness;
		static const long ID_BITMAPBUTTON_SLIDER_Ripple_Thickness;
		static const long ID_STATICTEXT4;
		static const long IDD_SLIDER_Ripple_Spacing;
		static const long ID_VALUECURVE_Ripple_Spacing;
		static const long ID_TEXTCTRL_Ripple_Spacing;
		static const long ID_BITMAPBUTTON_SLIDER_Ripple_Spacing;
		static const long ID_STATICTEXT_Ripple_Cycles;
		static const long IDD_SLIDER_Ripple_Cycles;
		static const long ID_VALUECURVE_Ripple_Cycles;
		static const long ID_TEXTCTRL_Ripple_Cycles;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_RIPPLE_POINTS;
		static const long IDD_TEXTCTRL_RIPPLE_POINTS;
		static const long ID_BITMAPBUTTON_SLIDER_RIPPLE_POINTS;
		static const long ID_STATICTEXT_Ripple_Rotation;
		static const long ID_SLIDER_Ripple_Rotation;
		static const long ID_VALUECURVE_Ripple_Rotation;
		static const long IDD_TEXTCTRL_Ripple_Rotation;
		static const long ID_BITMAPBUTTON_SLIDER_Ripple_Rotation;
		static const long ID_STATICTEXT5;
		static const long IDD_SLIDER_Ripple_Twist;
		static const long ID_VALUECURVE_Ripple_Twist;
		static const long ID_TEXTCTRL_Ripple_Twist;
		static const long ID_BITMAPBUTTON_SLIDER_Ripple_Twist;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_Ripple_XC;
		static const long ID_VALUECURVE_Ripple_XC;
		static const long IDD_TEXTCTRL_Ripple_XC;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER_Ripple_YC;
		static const long ID_VALUECURVE_Ripple_YC;
		static const long IDD_TEXTCTRL_Ripple_YC;
		static const long ID_STATICTEXT7;
		static const long IDD_SLIDER_Ripple_Velocity;
		static const long ID_VALUECURVE_Ripple_Velocity;
		static const long ID_TEXTCTRL_Ripple_Velocity;
		static const long ID_BITMAPBUTTON_SLIDER_Ripple_Velocity;
		static const long ID_STATICTEXT8;
		static const long ID_SLIDER_Ripple_Direction;
		static const long ID_VALUECURVE_Ripple_Direction;
		static const long IDD_TEXTCTRL_Ripple_Direction;
		static const long ID_BITMAPBUTTON_SLIDER_Ripple_Direction;
		static const long ID_CHECKBOX_Ripple3D;
		static const long ID_BITMAPBUTTON_CHECKBOX_Ripple3D;
		//*)

	public:

		//(*Handlers(RipplePanel)
		void OnChoice_Ripple_Object_To_DrawSelect(wxCommandEvent& event);
		void OnChoice_Ripple_Draw_StyleSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
