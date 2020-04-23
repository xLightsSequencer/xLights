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

class GalaxyPanel: public wxPanel
{
	public:

		GalaxyPanel(wxWindow* parent);
		virtual ~GalaxyPanel();

		//(*Declarations(GalaxyPanel)
		BulkEditCheckBox* CheckBox_Galaxy_Blend_Edges;
		BulkEditCheckBox* CheckBox_Galaxy_Inward;
		BulkEditCheckBox* CheckBox_Galaxy_Reverse;
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
		static const long ID_STATICTEXT_Galaxy_CenterX;
		static const long ID_SLIDER_Galaxy_CenterX;
		static const long ID_VALUECURVE_Galaxy_CenterX;
		static const long IDD_TEXTCTRL_Galaxy_CenterX;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_CenterX;
		static const long ID_STATICTEXT_Galaxy_CenterY;
		static const long ID_SLIDER_Galaxy_CenterY;
		static const long ID_VALUECURVE_Galaxy_CenterY;
		static const long IDD_TEXTCTRL_Galaxy_CenterY;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_CenterY;
		static const long ID_STATICTEXT_Galaxy_Start_Radius;
		static const long ID_SLIDER_Galaxy_Start_Radius;
		static const long ID_VALUECURVE_Galaxy_Start_Radius;
		static const long IDD_TEXTCTRL_Galaxy_Start_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Radius;
		static const long ID_STATICTEXT_Galaxy_Start_Width;
		static const long ID_SLIDER_Galaxy_Start_Width;
		static const long ID_VALUECURVE_Galaxy_Start_Width;
		static const long IDD_TEXTCTRL_Galaxy_Start_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Width;
		static const long ID_STATICTEXT_Galaxy_Start_Angle;
		static const long ID_SLIDER_Galaxy_Start_Angle;
		static const long ID_VALUECURVE_Galaxy_Start_Angle;
		static const long IDD_TEXTCTRL_Galaxy_Start_Angle;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Angle;
		static const long ID_PANEL29;
		static const long ID_STATICTEXT_Galaxy_End_Radius;
		static const long ID_SLIDER_Galaxy_End_Radius;
		static const long ID_VALUECURVE_Galaxy_End_Radius;
		static const long IDD_TEXTCTRL_Galaxy_End_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_End_Radius;
		static const long ID_STATICTEXT_Galaxy_End_Width;
		static const long ID_SLIDER_Galaxy_End_Width;
		static const long ID_VALUECURVE_Galaxy_End_Width;
		static const long IDD_TEXTCTRL_Galaxy_End_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_End_Width;
		static const long ID_STATICTEXT_Galaxy_Revolutions;
		static const long ID_SLIDER_Galaxy_Revolutions;
		static const long ID_VALUECURVE_Galaxy_Revolutions;
		static const long IDD_TEXTCTRL_Galaxy_Revolutions;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Revolutions;
		static const long ID_PANEL31;
		static const long ID_STATICTEXT_Galaxy_Duration;
		static const long ID_SLIDER_Galaxy_Duration;
		static const long ID_VALUECURVE_Galaxy_Duration;
		static const long IDD_TEXTCTRL_Galaxy_Duration;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Duration;
		static const long ID_STATICTEXT_Galaxy_Accel;
		static const long ID_SLIDER_Galaxy_Accel;
		static const long ID_VALUECURVE_Galaxy_Accel;
		static const long IDD_TEXTCTRL_Galaxy_Accel;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Accel;
		static const long ID_CHECKBOX_Galaxy_Reverse;
		static const long ID_CHECKBOX_Galaxy_Blend_Edges;
		static const long ID_CHECKBOX_Galaxy_Inward;
		static const long ID_PANEL32;
		static const long ID_NOTEBOOK_Galaxy;
		//*)

	public:

		//(*Handlers(GalaxyPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};
