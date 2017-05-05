#ifndef GALAXYPANEL_H
#define GALAXYPANEL_H

//(*Headers(GalaxyPanel)
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

class GalaxyPanel: public wxPanel
{
	public:

		GalaxyPanel(wxWindow* parent);
		virtual ~GalaxyPanel();

		//(*Declarations(GalaxyPanel)
		wxCheckBox* CheckBox_Galaxy_Reverse;
		wxBitmapButton* BitmapButton_Galaxy_Accel;
		ValueCurveButton* ValueCurve_Galaxy_End_Radius;
		wxStaticText* StaticText131;
		ValueCurveButton* ValueCurve_Galaxy_End_Width;
		wxSlider* Slider_Galaxy_Accel;
		wxBitmapButton* BitmapButton_Galaxy_Start_Radius;
		wxStaticText* StaticText129;
		wxStaticText* StaticText140;
		ValueCurveButton* ValueCurve_Galaxy_CenterY;
		wxSlider* Slider_Galaxy_Start_Width;
		wxStaticText* StaticText141;
		wxBitmapButton* BitmapButton_Galaxy_Revolutions;
		ValueCurveButton* ValueCurve_Galaxy_Start_Radius;
		wxBitmapButton* BitmapButton_Galaxy_Start_Width;
		wxSlider* Slider_Galaxy_Start_Radius;
		wxSlider* Slider_Galaxy_End_Width;
		ValueCurveButton* ValueCurve_Galaxy_Start_Width;
		wxSlider* Slider_Galaxy_CenterX;
		wxSlider* Slider_Galaxy_CenterY;
		ValueCurveButton* ValueCurve_Galaxy_Duration;
		wxBitmapButton* BitmapButton_Galaxy_End_Radius;
		wxNotebook* Notebook_Galaxy;
		wxStaticText* StaticText127;
		wxSlider* Slider_Galaxy_End_Radius;
		wxCheckBox* CheckBox_Galaxy_Inward;
		wxCheckBox* CheckBox_Galaxy_Blend_Edges;
		ValueCurveButton* ValueCurve_Galaxy_Start_Angle;
		wxSlider* Slider_Galaxy_Revolutions;
		ValueCurveButton* ValueCurve_Galaxy_Accel;
		wxBitmapButton* BitmapButton_Galaxy_CenterX;
		wxStaticText* StaticText126;
		wxBitmapButton* BitmapButton_Galaxy_Duration;
		wxSlider* Slider_Galaxy_Duration;
		ValueCurveButton* ValueCurve_Galaxy_CenterX;
		wxBitmapButton* BitmapButton_Galaxy_End_Width;
		wxBitmapButton* BitmapButton_Galaxy_Start_Angle;
		ValueCurveButton* ValueCurve_Galaxy_Revolutions;
		wxStaticText* StaticText139;
		wxStaticText* StaticText138;
		wxBitmapButton* BitmapButton_Galaxy_CenterY;
		wxSlider* Slider_Galaxy_Start_Angle;
		wxStaticText* StaticText142;
		//*)

	protected:

		//(*Identifiers(GalaxyPanel)
		static const long ID_SLIDER_Galaxy_CenterX;
		static const long ID_VALUECURVE_Galaxy_CenterX;
		static const long IDD_TEXTCTRL_Galaxy_CenterX;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_CenterX;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_Galaxy_CenterY;
		static const long ID_VALUECURVE_Galaxy_CenterY;
		static const long IDD_TEXTCTRL_Galaxy_CenterY;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_CenterY;
		static const long ID_STATICTEXT125;
		static const long ID_SLIDER_Galaxy_Start_Radius;
		static const long ID_VALUECURVE_Galaxy_Start_Radius;
		static const long IDD_TEXTCTRL_Galaxy_Start_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Radius;
		static const long ID_STATICTEXT127;
		static const long ID_SLIDER_Galaxy_Start_Width;
		static const long ID_VALUECURVE_Galaxy_Start_Width;
		static const long IDD_TEXTCTRL_Galaxy_Start_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Width;
		static const long ID_STATICTEXT126;
		static const long ID_SLIDER_Galaxy_Start_Angle;
		static const long ID_VALUECURVE_Galaxy_Start_Angle;
		static const long IDD_TEXTCTRL_Galaxy_Start_Angle;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Angle;
		static const long ID_PANEL29;
		static const long ID_STATICTEXT130;
		static const long ID_SLIDER_Galaxy_End_Radius;
		static const long ID_VALUECURVE_Galaxy_End_Radius;
		static const long IDD_TEXTCTRL_Galaxy_End_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_End_Radius;
		static const long ID_STATICTEXT132;
		static const long ID_SLIDER_Galaxy_End_Width;
		static const long ID_VALUECURVE_Galaxy_End_Width;
		static const long IDD_TEXTCTRL_Galaxy_End_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_End_Width;
		static const long ID_STATICTEXT134;
		static const long ID_SLIDER_Galaxy_Revolutions;
		static const long ID_VALUECURVE_Galaxy_Revolutions;
		static const long IDD_TEXTCTRL_Galaxy_Revolutions;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Revolutions;
		static const long ID_PANEL31;
		static const long ID_STATICTEXT139;
		static const long ID_SLIDER_Galaxy_Duration;
		static const long ID_VALUECURVE_Galaxy_Duration;
		static const long IDD_TEXTCTRL_Galaxy_Duration;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Duration;
		static const long ID_STATICTEXT140;
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
    void UpdateLinkedSliderFloat2(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
