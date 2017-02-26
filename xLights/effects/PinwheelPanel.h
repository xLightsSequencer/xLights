#ifndef PINWHEELPANEL_H
#define PINWHEELPANEL_H

//(*Headers(PinwheelPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxBoxSizer;
class wxChoice;
//*)

#include "../ValueCurveButton.h"

class PinwheelPanel: public wxPanel
{
	public:

		PinwheelPanel(wxWindow* parent);
		virtual ~PinwheelPanel();

		//(*Declarations(PinwheelPanel)
		wxChoice* Choice_Pinwheel_3D;
		ValueCurveButton* BitmapButton_PinwheelYCVC;
		wxBitmapButton* BitmapButton_Pinwheel_ArmSize;
		wxSlider* Slider_PinwheelYC;
		wxStaticText* StaticText74;
		wxStaticText* StaticText75;
		wxStaticText* StaticText76;
		wxStaticText* StaticText82;
		wxSlider* Slider_PinwheelXC;
		wxSlider* Slider_Pinwheel_Twist;
		ValueCurveButton* BitmapButton_Pinwheel_ArmSizeVC;
		wxStaticText* StaticText81;
		wxSlider* Slider_Pinwheel_ArmSize;
		wxBitmapButton* BitmapButton_PinwheelTwist;
		wxStaticText* StaticText84;
		wxSlider* Slider_Pinwheel_Arms;
		wxStaticText* StaticText1;
		ValueCurveButton* BitmapButton_Pinwheel_SpeedVC;
		wxBitmapButton* BitmapButton_Pinwheel3D;
		ValueCurveButton* BitmapButton_Pinwheel_ThicknessVC;
		wxBitmapButton* BitmapButton_PinwheelRotation;
		ValueCurveButton* BitmapButton_Pinwheel_TwistVC;
		wxChoice* Choice_Pinwheel_Style;
		wxSlider* Slider_Pinwheel_Speed;
		wxBitmapButton* BitmapButton7;
		wxBitmapButton* BitmapButton_PinwheelThickness;
		wxCheckBox* CheckBox_Pinwheel_Rotation;
		wxSlider* Slider_Pinwheel_Thickness;
		ValueCurveButton* BitmapButton_PinwheelXCVC;
		wxBitmapButton* BitmapButton_PinwheelNumberArms;
		wxStaticText* StaticText63;
		wxStaticText* StaticText77;
		//*)

	protected:

		//(*Identifiers(PinwheelPanel)
		static const long ID_STATICTEXT65;
		static const long ID_SLIDER_Pinwheel_Arms;
		static const long IDD_TEXTCTRL_Pinwheel_Arms;
		static const long ID_BITMAPBUTTON11;
		static const long ID_STATICTEXT98;
		static const long ID_SLIDER_Pinwheel_ArmSize;
		static const long ID_VALUECURVE_Pinwheel_ArmSize;
		static const long IDD_TEXTCTRL_Pinwheel_ArmSize;
		static const long ID_BITMAPBUTTON16;
		static const long ID_STATICTEXT76;
		static const long ID_SLIDER_Pinwheel_Twist;
		static const long ID_VALUECURVE_Pinwheel_Twist;
		static const long IDD_TEXTCTRL_Pinwheel_Twist;
		static const long ID_BITMAPBUTTON12;
		static const long ID_STATICTEXT77;
		static const long ID_SLIDER_Pinwheel_Thickness;
		static const long ID_VALUECURVE_Pinwheel_Thickness;
		static const long IDD_TEXTCTRL_Pinwheel_Thickness;
		static const long ID_BITMAPBUTTON18;
		static const long ID_SLIDER_Pinwheel_Speed;
		static const long ID_VALUECURVE_Pinwheel_Speed;
		static const long IDD_TEXTCTRL_Pinwheel_Speed;
		static const long ID_BITMAPBUTTON26;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_Pinwheel_Style;
		static const long ID_STATICTEXT78;
		static const long ID_CHECKBOX_Pinwheel_Rotation;
		static const long ID_BITMAPBUTTON19;
		static const long ID_STATICTEXT82;
		static const long ID_CHOICE_Pinwheel_3D;
		static const long ID_BITMAPBUTTON_Pinwheel3D;
		static const long ID_STATICTEXT85;
		static const long ID_SLIDER_PinwheelXC;
		static const long ID_VALUECURVE_PinwheelXC;
		static const long IDD_TEXTCTRL_PinwheelXC;
		static const long ID_STATICTEXT83;
		static const long ID_VALUECURVE_PinwheelYC;
		static const long IDD_TEXTCTRL_PinwheelYC;
		static const long ID_SLIDER_PinwheelYC;
		//*)

	public:

		//(*Handlers(PinwheelPanel)
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
    void UpdateLinkedSliderFloat2(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
