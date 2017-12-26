#ifndef FANPANEL_H
#define FANPANEL_H

//(*Headers(FanPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxNotebookEvent;
class wxNotebook;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

#include "../BulkEditControls.h"

class FanPanel: public wxPanel
{
	public:

		FanPanel(wxWindow* parent);
		virtual ~FanPanel();

		//(*Declarations(FanPanel)
		wxBitmapButton* BitmapButton_Fan_Duration;
		wxStaticText* StaticText148;
		wxStaticText* StaticText146;
		BulkEditSlider* Slider_Fan_Blade_Angle;
		BulkEditValueCurveButton* ValueCurve_Fan_Accel;
		wxStaticText* StaticText147;
		BulkEditValueCurveButton* ValueCurve_Fan_Blade_Angle;
		BulkEditSlider* Slider_Fan_Start_Radius;
		BulkEditSlider* Slider_Fan_Duration;
		BulkEditCheckBox* CheckBox_Fan_Reverse;
		wxBitmapButton* BitmapButton_Fan_End_Radius;
		wxBitmapButton* BitmapButton_Fan_Start_Radius;
		wxNotebook* Notebook1;
		BulkEditValueCurveButton* ValueCurve_Fan_Start_Angle;
		BulkEditSlider* Slider_Fan_Accel;
		BulkEditSlider* Slider_Fan_CenterX;
		wxBitmapButton* BitmapButton_Fan_Start_Angle;
		BulkEditValueCurveButton* ValueCurve_Fan_Blade_Width;
		wxBitmapButton* BitmapButton_Fan_CenterX;
		wxBitmapButton* BitmapButton_Fan_Blade_Width;
		BulkEditValueCurveButton* ValueCurve_Fan_CenterX;
		wxStaticText* StaticText151;
		BulkEditSlider* Slider_Fan_Num_Elements;
		wxStaticText* StaticText144;
		wxStaticText* StaticText153;
		wxStaticText* StaticText152;
		BulkEditValueCurveButton* ValueCurve_Fan_End_Radius;
		wxStaticText* StaticText150;
		wxBitmapButton* BitmapButton_Fan_Accel;
		BulkEditSlider* Slider_Fan_Element_Width;
		BulkEditSlider* Slider_Fan_CenterY;
		BulkEditSlider* Slider_Fan_Blade_Width;
		BulkEditValueCurveButton* ValueCurve_Fan_Revolutions;
		wxStaticText* StaticText149;
		BulkEditValueCurveButton* ValueCurve_Fan_Duration;
		wxBitmapButton* BitmapButton_Fan_Num_Blades;
		wxStaticText* StaticText133;
		wxBitmapButton* BitmapButton_Fan_Num_Elements;
		BulkEditSlider* Slider_Fan_Num_Blades;
		wxStaticText* StaticText145;
		BulkEditValueCurveButton* ValueCurve_Fan_Start_Radius;
		BulkEditValueCurveButton* ValueCurve_Fan_Num_Blades;
		wxBitmapButton* BitmapButton_Fan_Blade_Angle;
		BulkEditValueCurveButton* ValueCurve_Fan_Element_Width;
		BulkEditCheckBox* CheckBox_Fan_Blend_Edges;
		BulkEditValueCurveButton* ValueCurve_Fan_Num_Elements;
		wxBitmapButton* BitmapButton_Fan_CenterY;
		BulkEditSliderF360* Slider_Fan_Revolutions;
		wxBitmapButton* BitmapButton_Fan_Revolutions;
		BulkEditValueCurveButton* ValueCurve_Fan_CenterY;
		wxStaticText* StaticText143;
		BulkEditSlider* Slider_Fan_End_Radius;
		wxStaticText* StaticText135;
		BulkEditSlider* Slider_Fan_Start_Angle;
		wxBitmapButton* BitmapButton_Fan_Element_Width;
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
