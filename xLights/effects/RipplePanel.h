#ifndef RIPPLEPANEL_H
#define RIPPLEPANEL_H

//(*Headers(RipplePanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../ValueCurveButton.h"

class RipplePanel: public wxPanel
{
	public:

		RipplePanel(wxWindow* parent);
		virtual ~RipplePanel();

		//(*Declarations(RipplePanel)
		wxBitmapButton* BitmapButton_Ripple3D;
		ValueCurveButton* BitmapButton_Ripple_ThicknessVC;
		wxBitmapButton* BitmapButton_Ripple_Movement;
		wxChoice* Choice_Ripple_Movement;
		wxBitmapButton* BitmapButton_RippleThickness;
		wxBitmapButton* BitmapButton_Ripple_Object_To_Draw;
		wxSlider* Slider_Ripple_Cycles;
		wxStaticText* StaticText72;
		wxStaticText* StaticText69;
		ValueCurveButton* BitmapButton_Ripple_CyclesVC;
		wxCheckBox* CheckBox_Ripple3D;
		wxStaticText* StaticText70;
		wxChoice* Choice_Ripple_Object_To_Draw;
		wxSlider* Slider_Ripple_Thickness;
		//*)

	protected:

		//(*Identifiers(RipplePanel)
		static const long ID_STATICTEXT71;
		static const long ID_CHOICE_Ripple_Object_To_Draw;
		static const long ID_BITMAPBUTTON20;
		static const long ID_STATICTEXT72;
		static const long ID_CHOICE_Ripple_Movement;
		static const long ID_BITMAPBUTTON13;
		static const long ID_STATICTEXT74;
		static const long ID_SLIDER_Ripple_Thickness;
		static const long ID_VALUECURVE_Ripple_Thickness;
		static const long IDD_TEXTCTRL_Ripple_Thickness;
		static const long ID_BITMAPBUTTON14;
		static const long IDD_SLIDER_Ripple_Cycles;
		static const long ID_VALUECURVE_Ripple_Cycles;
		static const long ID_TEXTCTRL_Ripple_Cycles;
		static const long ID_CHECKBOX_Ripple3D;
		static const long ID_BITMAPBUTTON17;
		//*)

	public:

		//(*Handlers(RipplePanel)
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
