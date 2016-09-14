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

#include "../ValueCurveButton.h"

class FanPanel: public wxPanel
{
	public:

		FanPanel(wxWindow* parent);
		virtual ~FanPanel();

		//(*Declarations(FanPanel)
		wxBitmapButton* BitmapButton_Fan_Duration;
		ValueCurveButton* ValueCurve_Fan_End_Radius;
		wxStaticText* StaticText148;
		wxStaticText* StaticText146;
		ValueCurveButton* ValueCurve_Fan_CenterX;
		wxStaticText* StaticText147;
		ValueCurveButton* ValueCurve_Fan_Accel;
		wxBitmapButton* BitmapButton_Fan_End_Radius;
		wxBitmapButton* BitmapButton_Fan_Start_Radius;
		wxNotebook* Notebook1;
		wxBitmapButton* BitmapButton_Fan_Start_Angle;
		ValueCurveButton* ValueCurve_Fan_Revolutions;
		ValueCurveButton* ValueCurve_Fan_Element_Width;
		wxBitmapButton* BitmapButton_Fan_CenterX;
		wxBitmapButton* BitmapButton_Fan_Blade_Width;
		wxStaticText* StaticText151;
		wxStaticText* StaticText144;
		wxStaticText* StaticText153;
		ValueCurveButton* ValueCurve_Fan_Num_Elements;
		wxStaticText* StaticText152;
		wxStaticText* StaticText150;
		wxBitmapButton* BitmapButton_Fan_Accel;
		wxCheckBox* CheckBox_Fan_Reverse;
		ValueCurveButton* ValueCurve_Fan_Num_Blades;
		wxStaticText* StaticText149;
		ValueCurveButton* ValueCurve_Fan_Duration;
		wxBitmapButton* BitmapButton_Fan_Num_Blades;
		wxBitmapButton* BitmapButton_Fan_Num_Elements;
		wxStaticText* StaticText145;
		wxBitmapButton* BitmapButton_Fan_Blade_Angle;
		ValueCurveButton* ValueCurve_Fan_Start_Angle;
		ValueCurveButton* ValueCurve_Fan_Blade_Angle;
		ValueCurveButton* ValueCurve_Fan_CenterY;
		wxBitmapButton* BitmapButton_Fan_CenterY;
		wxBitmapButton* BitmapButton_Fan_Revolutions;
		wxCheckBox* CheckBox_Fan_Blend_Edges;
		ValueCurveButton* ValueCurve_Fan_Blade_Width;
		wxStaticText* StaticText143;
		wxStaticText* StaticText135;
		ValueCurveButton* ValueCurve_Fan_Start_Radius;
		wxBitmapButton* BitmapButton_Fan_Element_Width;
		//*)

	protected:

		//(*Identifiers(FanPanel)
		static const long ID_SLIDER_Fan_CenterX;
		static const long ID_VALUECURVE_Fan_CenterX;
		static const long IDD_TEXTCTRL_Fan_CenterX;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_CenterX;
		static const long ID_STATICTEXT128;
		static const long ID_SLIDER_Fan_CenterY;
		static const long ID_VALUECURVE_Fan_CenterY;
		static const long IDD_TEXTCTRL_Fan_CenterY;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_CenterY;
		static const long ID_STATICTEXT136;
		static const long ID_SLIDER_Fan_Start_Radius;
		static const long ID_VALUECURVE_Fan_Start_Radius;
		static const long IDD_TEXTCTRL_Fan_Start_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Start_Radius;
		static const long ID_STATICTEXT142;
		static const long ID_SLIDER_Fan_Start_Angle;
		static const long ID_VALUECURVE_Fan_Start_Angle;
		static const long IDD_TEXTCTRL_Fan_Start_Angle;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Start_Angle;
		static const long ID_STATICTEXT141;
		static const long ID_SLIDER_Fan_End_Radius;
		static const long ID_VALUECURVE_Fan_End_Radius;
		static const long IDD_TEXTCTRL_Fan_End_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_End_Radius;
		static const long ID_STATICTEXT145;
		static const long ID_SLIDER_Fan_Revolutions;
		static const long ID_VALUECURVE_Fan_Revolutions;
		static const long IDD_TEXTCTRL_Fan_Revolutions;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Revolutions;
		static const long ID_PANEL38;
		static const long ID_STATICTEXT148;
		static const long ID_SLIDER_Fan_Num_Blades;
		static const long ID_VALUECURVE_Fan_Num_Blades;
		static const long IDD_TEXTCTRL_Fan_Num_Blades;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Num_Blades;
		static const long ID_STATICTEXT149;
		static const long ID_SLIDER_Fan_Blade_Width;
		static const long ID_VALUECURVE_Fan_Blade_Width;
		static const long IDD_TEXTCTRL_Fan_Blade_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Blade_Width;
		static const long ID_STATICTEXT150;
		static const long ID_SLIDER_Fan_Blade_Angle;
		static const long ID_VALUECURVE_Fan_Blade_Angle;
		static const long IDD_TEXTCTRL_Fan_Blade_Angle;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Blade_Angle;
		static const long ID_STATICTEXT143;
		static const long ID_SLIDER_Fan_Num_Elements;
		static const long ID_VALUECURVE_Fan_Num_Elements;
		static const long IDD_TEXTCTRL_Fan_Num_Elements;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Num_Elements;
		static const long ID_STATICTEXT144;
		static const long ID_SLIDER_Fan_Element_Width;
		static const long ID_VALUECURVE_Fan_Element_Width;
		static const long IDD_TEXTCTRL_Fan_Element_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Element_Width;
		static const long ID_PANEL42;
		static const long ID_STATICTEXT146;
		static const long ID_SLIDER_Fan_Duration;
		static const long ID_VALUECURVE_Fan_Duration;
		static const long IDD_TEXTCTRL_Fan_Duration;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Duration;
		static const long ID_STATICTEXT147;
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
    void UpdateLinkedSliderFloat(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    void UpdateLinkedSlider360(wxCommandEvent& event);
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);
    void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
    void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
