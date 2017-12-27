#ifndef SHOCKWAVEPANEL_H
#define SHOCKWAVEPANEL_H

//(*Headers(ShockwavePanel)
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

class ShockwavePanel: public wxPanel
{
	public:

		ShockwavePanel(wxWindow* parent);
		virtual ~ShockwavePanel();

		//(*Declarations(ShockwavePanel)
		wxStaticText* StaticText156;
		BulkEditValueCurveButton* BitmapButton_Shockwave_Start_Width;
		BulkEditSlider* Slider_Shockwave_CenterY;
		BulkEditCheckBox* CheckBox_Shockwave_Blend_Edges;
		BulkEditSlider* Slider_Shockwave_End_Width;
		BulkEditValueCurveButton* BitmapButton_Shockwave_CenterX;
		wxStaticText* StaticText154;
		BulkEditSlider* Slider_Shockwave_Start_Width;
		BulkEditValueCurveButton* BitmapButton_Shockwave_CenterY;
		BulkEditSlider* Slider_Shockwave_Start_Radius;
		wxBitmapButton* BitmapButton_Shockwave_EndWidth;
		wxBitmapButton* BitmapButton_Shockwave_StartRadius;
		wxBitmapButton* BitmapButton_Shockwave_Center_X;
		wxBitmapButton* BitmapButton_Shockwave_Accel;
		BulkEditSlider* Slider_Shockwave_End_Radius;
		wxBitmapButton* BitmapButton_Shockwave_Center_Y;
		wxBitmapButton* BitmapButton_Shockwave_StartWidth;
		wxStaticText* StaticText158;
		BulkEditSlider* Slider_Shockwave_CenterX;
		BulkEditValueCurveButton* BitmapButton_Shockwave_Start_Radius;
		wxStaticText* StaticText159;
		wxStaticText* StaticText157;
		BulkEditValueCurveButton* BitmapButton_Shockwave_End_Width;
		wxNotebook* Notebook3;
		wxBitmapButton* BitmapButton_Shockwave_EndRadius;
		BulkEditSlider* Slider_Shockwave_Accel;
		wxStaticText* StaticText155;
		wxStaticText* StaticText166;
		BulkEditValueCurveButton* BitmapButton_Shockwave_End_Radius;
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
		static const long ID_CHECKBOX_Shockwave_Blend_Edges;
		static const long ID_PANEL44;
		static const long ID_NOTEBOOK_Shockwave;
		//*)

	public:

		//(*Handlers(ShockwavePanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
