#ifndef FANPANEL_H
#define FANPANEL_H

//(*Headers(FanPanel)
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

class FanPanel: public wxPanel
{
	public:

		FanPanel(wxWindow* parent);
		virtual ~FanPanel();

		//(*Declarations(FanPanel)
		BulkEditCheckBox* CheckBox_Fan_Blend_Edges;
		BulkEditCheckBox* CheckBox_Fan_Reverse;
		BulkEditSlider* Slider_Fan_Accel;
		BulkEditSlider* Slider_Fan_Blade_Angle;
		BulkEditSlider* Slider_Fan_Blade_Width;
		BulkEditSlider* Slider_Fan_CenterX;
		BulkEditSlider* Slider_Fan_CenterY;
		BulkEditSlider* Slider_Fan_Duration;
		BulkEditSlider* Slider_Fan_Element_Width;
		BulkEditSlider* Slider_Fan_End_Radius;
		BulkEditSlider* Slider_Fan_Num_Blades;
		BulkEditSlider* Slider_Fan_Num_Elements;
		BulkEditSlider* Slider_Fan_Start_Angle;
		BulkEditSlider* Slider_Fan_Start_Radius;
		BulkEditSliderF360* Slider_Fan_Revolutions;
		BulkEditValueCurveButton* ValueCurve_Fan_Accel;
		BulkEditValueCurveButton* ValueCurve_Fan_Blade_Angle;
		BulkEditValueCurveButton* ValueCurve_Fan_Blade_Width;
		BulkEditValueCurveButton* ValueCurve_Fan_CenterX;
		BulkEditValueCurveButton* ValueCurve_Fan_CenterY;
		BulkEditValueCurveButton* ValueCurve_Fan_Duration;
		BulkEditValueCurveButton* ValueCurve_Fan_Element_Width;
		BulkEditValueCurveButton* ValueCurve_Fan_End_Radius;
		BulkEditValueCurveButton* ValueCurve_Fan_Num_Blades;
		BulkEditValueCurveButton* ValueCurve_Fan_Num_Elements;
		BulkEditValueCurveButton* ValueCurve_Fan_Revolutions;
		BulkEditValueCurveButton* ValueCurve_Fan_Start_Angle;
		BulkEditValueCurveButton* ValueCurve_Fan_Start_Radius;
		wxNotebook* Notebook1;
		wxStaticText* StaticText133;
		wxStaticText* StaticText135;
		wxStaticText* StaticText143;
		wxStaticText* StaticText144;
		wxStaticText* StaticText145;
		wxStaticText* StaticText146;
		wxStaticText* StaticText147;
		wxStaticText* StaticText148;
		wxStaticText* StaticText149;
		wxStaticText* StaticText150;
		wxStaticText* StaticText151;
		wxStaticText* StaticText152;
		wxStaticText* StaticText153;
		xlLockButton* BitmapButton_Fan_Accel;
		xlLockButton* BitmapButton_Fan_Blade_Angle;
		xlLockButton* BitmapButton_Fan_Blade_Width;
		xlLockButton* BitmapButton_Fan_CenterX;
		xlLockButton* BitmapButton_Fan_CenterY;
		xlLockButton* BitmapButton_Fan_Duration;
		xlLockButton* BitmapButton_Fan_Element_Width;
		xlLockButton* BitmapButton_Fan_End_Radius;
		xlLockButton* BitmapButton_Fan_Num_Blades;
		xlLockButton* BitmapButton_Fan_Num_Elements;
		xlLockButton* BitmapButton_Fan_Revolutions;
		xlLockButton* BitmapButton_Fan_Start_Angle;
		xlLockButton* BitmapButton_Fan_Start_Radius;
		//*)

	protected:

		//(*Identifiers(FanPanel)
		static const long ID_STATICTEXT_Fan_CenterX;
		static const long ID_SLIDER_Fan_CenterX;
		static const long ID_VALUECURVE_Fan_CenterX;
		static const long IDD_TEXTCTRL_Fan_CenterX;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_CenterX;
		static const long ID_STATICTEXT_Fan_CenterY;
		static const long ID_SLIDER_Fan_CenterY;
		static const long ID_VALUECURVE_Fan_CenterY;
		static const long IDD_TEXTCTRL_Fan_CenterY;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_CenterY;
		static const long ID_STATICTEXT_Fan_Start_Radius;
		static const long ID_SLIDER_Fan_Start_Radius;
		static const long ID_VALUECURVE_Fan_Start_Radius;
		static const long IDD_TEXTCTRL_Fan_Start_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Start_Radius;
		static const long ID_STATICTEXT_Fan_Start_Angle;
		static const long ID_SLIDER_Fan_Start_Angle;
		static const long ID_VALUECURVE_Fan_Start_Angle;
		static const long IDD_TEXTCTRL_Fan_Start_Angle;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Start_Angle;
		static const long ID_STATICTEXT_Fan_End_Radius;
		static const long ID_SLIDER_Fan_End_Radius;
		static const long ID_VALUECURVE_Fan_End_Radius;
		static const long IDD_TEXTCTRL_Fan_End_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_End_Radius;
		static const long ID_STATICTEXT_Fan_Revolutions;
		static const long ID_SLIDER_Fan_Revolutions;
		static const long ID_VALUECURVE_Fan_Revolutions;
		static const long IDD_TEXTCTRL_Fan_Revolutions;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Revolutions;
		static const long ID_PANEL38;
		static const long ID_STATICTEXT_Fan_Num_Blades;
		static const long ID_SLIDER_Fan_Num_Blades;
		static const long ID_VALUECURVE_Fan_Num_Blades;
		static const long IDD_TEXTCTRL_Fan_Num_Blades;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Num_Blades;
		static const long ID_STATICTEXT_Fan_Blade_Width;
		static const long ID_SLIDER_Fan_Blade_Width;
		static const long ID_VALUECURVE_Fan_Blade_Width;
		static const long IDD_TEXTCTRL_Fan_Blade_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Blade_Width;
		static const long ID_STATICTEXT_Fan_Blade_Angle;
		static const long ID_SLIDER_Fan_Blade_Angle;
		static const long ID_VALUECURVE_Fan_Blade_Angle;
		static const long IDD_TEXTCTRL_Fan_Blade_Angle;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Blade_Angle;
		static const long ID_STATICTEXT_Fan_Num_Elements;
		static const long ID_SLIDER_Fan_Num_Elements;
		static const long ID_VALUECURVE_Fan_Num_Elements;
		static const long IDD_TEXTCTRL_Fan_Num_Elements;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Num_Elements;
		static const long ID_STATICTEXT_Fan_Element_Width;
		static const long ID_SLIDER_Fan_Element_Width;
		static const long ID_VALUECURVE_Fan_Element_Width;
		static const long IDD_TEXTCTRL_Fan_Element_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Element_Width;
		static const long ID_PANEL42;
		static const long ID_STATICTEXT_Fan_Duration;
		static const long ID_SLIDER_Fan_Duration;
		static const long ID_VALUECURVE_Fan_Duration;
		static const long IDD_TEXTCTRL_Fan_Duration;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Duration;
		static const long ID_STATICTEXT_Fan_Accel;
		static const long ID_SLIDER_Fan_Accel;
		static const long ID_VALUECURVE_Fan_Accel;
		static const long IDD_TEXTCTRL_Fan_Accel;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Accel;
		static const long ID_CHECKBOX_Fan_Reverse;
		static const long ID_CHECKBOX_Fan_Blend_Edges;
		static const long ID_PANEL41;
		static const long ID_NOTEBOOK_Fan;
		//*)

	public:

		//(*Handlers(FanPanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
