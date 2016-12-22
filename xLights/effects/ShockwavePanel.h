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

#include "../ValueCurveButton.h"

class ShockwavePanel: public wxPanel
{
	public:

		ShockwavePanel(wxWindow* parent);
		virtual ~ShockwavePanel();

		//(*Declarations(ShockwavePanel)
		ValueCurveButton* BitmapButton_Shockwave_Start_Width;
		wxStaticText* StaticText156;
		ValueCurveButton* BitmapButton_Shockwave_End_Width;
		wxSlider* Slider_Shockwave_End_Width;
		wxCheckBox* CheckBox_Shockwave_Blend_Edges;
		ValueCurveButton* BitmapButton_Shockwave_CenterX;
		ValueCurveButton* BitmapButton_Shockwave_CenterY;
		wxSlider* Slider_Shockwave_End_Radius;
		wxSlider* Slider_Shockwave_Start_Width;
		wxBitmapButton* BitmapButton_Shockwave_EndWidth;
		wxBitmapButton* BitmapButton_Shockwave_StartRadius;
		wxBitmapButton* BitmapButton_Shockwave_Center_X;
		wxBitmapButton* BitmapButton_Shockwave_Accel;
		wxBitmapButton* BitmapButton_Shockwave_Center_Y;
		wxSlider* Slider_Shockwave_CenterY;
		wxBitmapButton* BitmapButton_Shockwave_StartWidth;
		wxSlider* Slider_Shockwave_Start_Radius;
		wxStaticText* StaticText158;
		ValueCurveButton* BitmapButton_Shockwave_Start_Radius;
		wxSlider* Slider_Shockwave_Accel;
		wxStaticText* StaticText159;
		wxStaticText* StaticText157;
		wxSlider* Slider_Shockwave_CenterX;
		wxNotebook* Notebook3;
		ValueCurveButton* BitmapButton_Shockwave_End_Radius;
		wxBitmapButton* BitmapButton_Shockwave_EndRadius;
		wxStaticText* StaticText155;
		wxStaticText* StaticText166;
		//*)

	protected:

		//(*Identifiers(ShockwavePanel)
		static const long ID_SLIDER_Shockwave_CenterX;
		static const long ID_VALUECURVE_Shockwave_CenterX;
		static const long IDD_TEXTCTRL_Shockwave_CenterX;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_CenterX;
		static const long ID_STATICTEXT151;
		static const long ID_SLIDER_Shockwave_CenterY;
		static const long ID_VALUECURVE_Shockwave_CenterY;
		static const long IDD_TEXTCTRL_Shockwave_CenterY;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_CenterY;
		static const long ID_STATICTEXT152;
		static const long ID_SLIDER_Shockwave_Start_Radius;
		static const long ID_VALUECURVE_Shockwave_Start_Radius;
		static const long IDD_TEXTCTRL_Shockwave_Start_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_Start_Radius;
		static const long ID_STATICTEXT154;
		static const long ID_SLIDER_Shockwave_End_Radius;
		static const long ID_VALUECURVE_Shockwave_End_Radius;
		static const long IDD_TEXTCTRL_Shockwave_End_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_End_Radius;
		static const long ID_STATICTEXT153;
		static const long ID_SLIDER_Shockwave_Start_Width;
		static const long ID_VALUECURVE_Shockwave_Start_Width;
		static const long IDD_TEXTCTRL_Shockwave_Start_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_Start_Width;
		static const long ID_STATICTEXT155;
		static const long ID_SLIDER_Shockwave_End_Width;
		static const long ID_VALUECURVE_Shockwave_End_Width;
		static const long IDD_TEXTCTRL_Shockwave_End_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_End_Width;
		static const long ID_PANEL40;
		static const long ID_STATICTEXT162;
		static const long ID_SLIDER_Shockwave_Accel;
		static const long IDD_TEXTCTRL_Shockwave_Accel;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_Accel;
		static const long ID_CHECKBOX_Shockwave_Blend_Edges;
		static const long ID_PANEL44;
		static const long ID_NOTEBOOK_Shockwave;
		//*)

	public:

		//(*Handlers(ShockwavePanel)
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
