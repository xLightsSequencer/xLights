#ifndef GALAXYPANEL_H
#define GALAXYPANEL_H

//(*Headers(GalaxyPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxNotebook;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxNotebookEvent;
class wxStaticText;
class wxCheckBox;
//*)

#include "../BulkEditControls.h"

class GalaxyPanel: public wxPanel
{
	public:

		GalaxyPanel(wxWindow* parent);
		virtual ~GalaxyPanel();

		//(*Declarations(GalaxyPanel)
		wxStaticText* StaticText139;
		BulkEditSlider* Slider_Galaxy_Accel;
		BulkEditValueCurveButton* ValueCurve_Galaxy_End_Width;
		wxBitmapButton* BitmapButton_Galaxy_CenterX;
		wxBitmapButton* BitmapButton_Galaxy_End_Radius;
		BulkEditValueCurveButton* ValueCurve_Galaxy_Start_Radius;
		BulkEditSlider* Slider_Galaxy_Duration;
		wxStaticText* StaticText126;
		BulkEditCheckBox* CheckBox_Galaxy_Blend_Edges;
		BulkEditSliderF360* Slider_Galaxy_Revolutions;
		BulkEditSlider* Slider_Galaxy_End_Radius;
		BulkEditValueCurveButton* ValueCurve_Galaxy_Revolutions;
		wxBitmapButton* BitmapButton_Galaxy_Revolutions;
		BulkEditSlider* Slider_Galaxy_Start_Radius;
		BulkEditValueCurveButton* ValueCurve_Galaxy_CenterY;
		BulkEditSlider* Slider_Galaxy_Start_Width;
		BulkEditValueCurveButton* ValueCurve_Galaxy_Start_Angle;
		wxStaticText* StaticText131;
		wxStaticText* StaticText141;
		wxBitmapButton* BitmapButton_Galaxy_Start_Width;
		wxNotebook* Notebook_Galaxy;
		BulkEditCheckBox* CheckBox_Galaxy_Reverse;
		BulkEditSlider* Slider_Galaxy_End_Width;
		wxStaticText* StaticText127;
		BulkEditSlider* Slider_Galaxy_CenterX;
		BulkEditValueCurveButton* ValueCurve_Galaxy_Duration;
		wxStaticText* StaticText138;
		wxBitmapButton* BitmapButton_Galaxy_End_Width;
		BulkEditValueCurveButton* ValueCurve_Galaxy_CenterX;
		BulkEditValueCurveButton* ValueCurve_Galaxy_Accel;
		BulkEditSlider* Slider_Galaxy_CenterY;
		wxBitmapButton* BitmapButton_Galaxy_Start_Angle;
		wxBitmapButton* BitmapButton_Galaxy_Start_Radius;
		wxStaticText* StaticText142;
		wxStaticText* StaticText140;
		wxBitmapButton* BitmapButton_Galaxy_Accel;
		wxStaticText* StaticText125;
		wxStaticText* StaticText129;
		BulkEditCheckBox* CheckBox_Galaxy_Inward;
		BulkEditSlider* Slider_Galaxy_Start_Angle;
		BulkEditValueCurveButton* ValueCurve_Galaxy_Start_Width;
		wxBitmapButton* BitmapButton_Galaxy_Duration;
		wxBitmapButton* BitmapButton_Galaxy_CenterY;
		BulkEditValueCurveButton* ValueCurve_Galaxy_End_Radius;
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

#endif
