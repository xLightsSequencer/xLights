#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

 //(*Headers(GalaxyPanel)
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

class GalaxyPanel: public xlEffectPanel
{
	public:

		GalaxyPanel(wxWindow* parent);
		virtual ~GalaxyPanel();
		virtual void ValidateWindow() override;

		//(*Declarations(GalaxyPanel)
		BulkEditCheckBox* CheckBox_Galaxy_Blend_Edges;
		BulkEditCheckBox* CheckBox_Galaxy_Inward;
		BulkEditCheckBox* CheckBox_Galaxy_Reverse;
		BulkEditCheckBox* CheckBox_Galaxy_Scale;
		BulkEditSlider* Slider_Galaxy_Accel;
		BulkEditSlider* Slider_Galaxy_CenterX;
		BulkEditSlider* Slider_Galaxy_CenterY;
		BulkEditSlider* Slider_Galaxy_Duration;
		BulkEditSlider* Slider_Galaxy_End_Radius;
		BulkEditSlider* Slider_Galaxy_End_Width;
		BulkEditSlider* Slider_Galaxy_Start_Angle;
		BulkEditSlider* Slider_Galaxy_Start_Radius;
		BulkEditSlider* Slider_Galaxy_Start_Width;
		BulkEditSliderF360* Slider_Galaxy_Revolutions;
		BulkEditValueCurveButton* ValueCurve_Galaxy_Accel;
		BulkEditValueCurveButton* ValueCurve_Galaxy_CenterX;
		BulkEditValueCurveButton* ValueCurve_Galaxy_CenterY;
		BulkEditValueCurveButton* ValueCurve_Galaxy_Duration;
		BulkEditValueCurveButton* ValueCurve_Galaxy_End_Radius;
		BulkEditValueCurveButton* ValueCurve_Galaxy_End_Width;
		BulkEditValueCurveButton* ValueCurve_Galaxy_Revolutions;
		BulkEditValueCurveButton* ValueCurve_Galaxy_Start_Angle;
		BulkEditValueCurveButton* ValueCurve_Galaxy_Start_Radius;
		BulkEditValueCurveButton* ValueCurve_Galaxy_Start_Width;
		wxNotebook* Notebook_Galaxy;
		wxStaticText* StaticText125;
		wxStaticText* StaticText126;
		wxStaticText* StaticText127;
		wxStaticText* StaticText129;
		wxStaticText* StaticText131;
		wxStaticText* StaticText138;
		wxStaticText* StaticText139;
		wxStaticText* StaticText140;
		wxStaticText* StaticText141;
		wxStaticText* StaticText142;
		xlLockButton* BitmapButton_Galaxy_Accel;
		xlLockButton* BitmapButton_Galaxy_CenterX;
		xlLockButton* BitmapButton_Galaxy_CenterY;
		xlLockButton* BitmapButton_Galaxy_Duration;
		xlLockButton* BitmapButton_Galaxy_End_Radius;
		xlLockButton* BitmapButton_Galaxy_End_Width;
		xlLockButton* BitmapButton_Galaxy_Revolutions;
		xlLockButton* BitmapButton_Galaxy_Start_Angle;
		xlLockButton* BitmapButton_Galaxy_Start_Radius;
		xlLockButton* BitmapButton_Galaxy_Start_Width;
		//*)

	protected:

		//(*Identifiers(GalaxyPanel)
		static const wxWindowID ID_STATICTEXT_Galaxy_CenterX;
		static const wxWindowID ID_SLIDER_Galaxy_CenterX;
		static const wxWindowID ID_VALUECURVE_Galaxy_CenterX;
		static const wxWindowID IDD_TEXTCTRL_Galaxy_CenterX;
		static const wxWindowID ID_BITMAPBUTTON_SLIDER_Galaxy_CenterX;
		static const wxWindowID ID_STATICTEXT_Galaxy_CenterY;
		static const wxWindowID ID_SLIDER_Galaxy_CenterY;
		static const wxWindowID ID_VALUECURVE_Galaxy_CenterY;
		static const wxWindowID IDD_TEXTCTRL_Galaxy_CenterY;
		static const wxWindowID ID_BITMAPBUTTON_SLIDER_Galaxy_CenterY;
		static const wxWindowID ID_STATICTEXT_Galaxy_Start_Radius;
		static const wxWindowID ID_SLIDER_Galaxy_Start_Radius;
		static const wxWindowID ID_VALUECURVE_Galaxy_Start_Radius;
		static const wxWindowID IDD_TEXTCTRL_Galaxy_Start_Radius;
		static const wxWindowID ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Radius;
		static const wxWindowID ID_STATICTEXT_Galaxy_Start_Width;
		static const wxWindowID ID_SLIDER_Galaxy_Start_Width;
		static const wxWindowID ID_VALUECURVE_Galaxy_Start_Width;
		static const wxWindowID IDD_TEXTCTRL_Galaxy_Start_Width;
		static const wxWindowID ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Width;
		static const wxWindowID ID_STATICTEXT_Galaxy_Start_Angle;
		static const wxWindowID ID_SLIDER_Galaxy_Start_Angle;
		static const wxWindowID ID_VALUECURVE_Galaxy_Start_Angle;
		static const wxWindowID IDD_TEXTCTRL_Galaxy_Start_Angle;
		static const wxWindowID ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Angle;
		static const wxWindowID ID_PANEL29;
		static const wxWindowID ID_STATICTEXT_Galaxy_End_Radius;
		static const wxWindowID ID_SLIDER_Galaxy_End_Radius;
		static const wxWindowID ID_VALUECURVE_Galaxy_End_Radius;
		static const wxWindowID IDD_TEXTCTRL_Galaxy_End_Radius;
		static const wxWindowID ID_BITMAPBUTTON_SLIDER_Galaxy_End_Radius;
		static const wxWindowID ID_STATICTEXT_Galaxy_End_Width;
		static const wxWindowID ID_SLIDER_Galaxy_End_Width;
		static const wxWindowID ID_VALUECURVE_Galaxy_End_Width;
		static const wxWindowID IDD_TEXTCTRL_Galaxy_End_Width;
		static const wxWindowID ID_BITMAPBUTTON_SLIDER_Galaxy_End_Width;
		static const wxWindowID ID_STATICTEXT_Galaxy_Revolutions;
		static const wxWindowID ID_SLIDER_Galaxy_Revolutions;
		static const wxWindowID ID_VALUECURVE_Galaxy_Revolutions;
		static const wxWindowID IDD_TEXTCTRL_Galaxy_Revolutions;
		static const wxWindowID ID_BITMAPBUTTON_SLIDER_Galaxy_Revolutions;
		static const wxWindowID ID_PANEL31;
		static const wxWindowID ID_STATICTEXT_Galaxy_Duration;
		static const wxWindowID ID_SLIDER_Galaxy_Duration;
		static const wxWindowID ID_VALUECURVE_Galaxy_Duration;
		static const wxWindowID IDD_TEXTCTRL_Galaxy_Duration;
		static const wxWindowID ID_BITMAPBUTTON_SLIDER_Galaxy_Duration;
		static const wxWindowID ID_STATICTEXT_Galaxy_Accel;
		static const wxWindowID ID_SLIDER_Galaxy_Accel;
		static const wxWindowID ID_VALUECURVE_Galaxy_Accel;
		static const wxWindowID IDD_TEXTCTRL_Galaxy_Accel;
		static const wxWindowID ID_BITMAPBUTTON_SLIDER_Galaxy_Accel;
		static const wxWindowID ID_CHECKBOX_Galaxy_Reverse;
		static const wxWindowID ID_CHECKBOX_Galaxy_Blend_Edges;
		static const wxWindowID ID_CHECKBOX_Galaxy_Inward;
		static const wxWindowID ID_CHECKBOX_Galaxy_Scale;
		static const wxWindowID ID_PANEL32;
		static const wxWindowID ID_NOTEBOOK_Galaxy;
		//*)

	public:

		//(*Handlers(GalaxyPanel)
		//*)

		DECLARE_EVENT_TABLE()
};
