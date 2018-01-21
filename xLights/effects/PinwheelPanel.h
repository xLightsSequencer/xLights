#ifndef PINWHEELPANEL_H
#define PINWHEELPANEL_H

//(*Headers(PinwheelPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxBoxSizer;
class wxStaticText;
class wxCheckBox;
//*)

#include "../BulkEditControls.h"

class PinwheelPanel: public wxPanel
{
	public:

		PinwheelPanel(wxWindow* parent);
		virtual ~PinwheelPanel();

		//(*Declarations(PinwheelPanel)
		BulkEditValueCurveButton* BitmapButton_Pinwheel_ArmSizeVC;
		BulkEditSlider* Slider_PinwheelYC;
		BulkEditValueCurveButton* BitmapButton_Pinwheel_SpeedVC;
		wxBitmapButton* BitmapButton_Pinwheel3D;
		BulkEditSlider* Slider_Pinwheel_Speed;
		BulkEditCheckBox* CheckBox_Pinwheel_Rotation;
		BulkEditSlider* Slider_Pinwheel_Twist;
		BulkEditValueCurveButton* BitmapButton_Pinwheel_ThicknessVC;
		wxStaticText* StaticText76;
		BulkEditSlider* Slider_Pinwheel_Thickness;
		wxStaticText* StaticText63;
		BulkEditSlider* Slider_PinwheelXC;
		wxBitmapButton* BitmapButton_PinwheelNumberArms;
		BulkEditChoice* Choice_Pinwheel_3D;
		BulkEditSlider* Slider_Pinwheel_ArmSize;
		wxStaticText* StaticText77;
		ValueCurveButton* BitmapButton_PinwheelYCVC;
		ValueCurveButton* BitmapButton_PinwheelXCVC;
		wxStaticText* StaticText1;
		wxStaticText* StaticText184;
		wxBitmapButton* BitmapButton_PinwheelThickness;
		wxBitmapButton* BitmapButton_PinwheelRotation;
		wxStaticText* StaticText75;
		wxBitmapButton* BitmapButton_PinwheelSpeed;
		BulkEditSlider* Slider_Pinwheel_Arms;
		wxBitmapButton* BitmapButton_PinwheelTwist;
		wxStaticText* StaticText84;
		wxStaticText* StaticText81;
		BulkEditValueCurveButton* BitmapButton_Pinwheel_TwistVC;
		wxBitmapButton* BitmapButton_Pinwheel_ArmSize;
		BulkEditChoice* Choice_Pinwheel_Style;
		wxStaticText* StaticText74;
		wxStaticText* StaticText82;
		//*)

	protected:

		//(*Identifiers(PinwheelPanel)
		static const long ID_STATICTEXT_Pinwheel_Arms;
		static const long ID_SLIDER_Pinwheel_Arms;
		static const long IDD_TEXTCTRL_Pinwheel_Arms;
		static const long ID_BITMAPBUTTON_SLIDER_Pinwheel_Arms;
		static const long ID_STATICTEXT_Pinwheel_ArmSize;
		static const long ID_SLIDER_Pinwheel_ArmSize;
		static const long ID_VALUECURVE_Pinwheel_ArmSize;
		static const long IDD_TEXTCTRL_Pinwheel_ArmSize;
		static const long ID_BITMAPBUTTON_SLIDER_Pinwheel_ArmSize;
		static const long ID_STATICTEXT_Pinwheel_Twist;
		static const long ID_SLIDER_Pinwheel_Twist;
		static const long ID_VALUECURVE_Pinwheel_Twist;
		static const long IDD_TEXTCTRL_Pinwheel_Twist;
		static const long ID_BITMAPBUTTON_SLIDER_Pinwheel_Twist;
		static const long ID_STATICTEXT_Pinwheel_Thickness;
		static const long ID_SLIDER_Pinwheel_Thickness;
		static const long ID_VALUECURVE_Pinwheel_Thickness;
		static const long IDD_TEXTCTRL_Pinwheel_Thickness;
		static const long ID_BITMAPBUTTON_SLIDER_Pinwheel_Thickness;
		static const long ID_STATICTEXT_Pinwheel_Speed;
		static const long ID_SLIDER_Pinwheel_Speed;
		static const long ID_VALUECURVE_Pinwheel_Speed;
		static const long IDD_TEXTCTRL_Pinwheel_Speed;
		static const long ID_BITMAPBUTTON_SLIDER_Pinwheel_Speed;
		static const long ID_STATICTEXT_Pinwheel_Style;
		static const long ID_CHOICE_Pinwheel_Style;
		static const long ID_STATICTEXT78;
		static const long ID_CHECKBOX_Pinwheel_Rotation;
		static const long ID_BITMAPBUTTON_CHECKBOX_Pinwheel_Rotation;
		static const long ID_STATICTEXT_Pinwheel_3D;
		static const long ID_CHOICE_Pinwheel_3D;
		static const long ID_BITMAPBUTTON_CHOICE_Pinwheel_3D;
		static const long ID_STATICTEXT_PinwheelXC;
		static const long ID_SLIDER_PinwheelXC;
		static const long ID_VALUECURVE_PinwheelXC;
		static const long IDD_TEXTCTRL_PinwheelXC;
		static const long ID_STATICTEXT_PinwheelYC;
		static const long ID_VALUECURVE_PinwheelYC;
		static const long IDD_TEXTCTRL_PinwheelYC;
		static const long ID_SLIDER_PinwheelYC;
		//*)

	public:

		//(*Handlers(PinwheelPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
