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

#include "../BulkEditControls.h"

class PinwheelPanel: public wxPanel
{
	public:

		PinwheelPanel(wxWindow* parent);
		virtual ~PinwheelPanel();

		//(*Declarations(PinwheelPanel)
		BulkEditValueCurveButton* BitmapButton_Pinwheel_ArmSizeVC;
		ValueCurveButton* BitmapButton_PinwheelYCVC;
		wxBitmapButton* BitmapButton_Pinwheel_ArmSize;
		wxStaticText* StaticText74;
		wxStaticText* StaticText75;
		wxStaticText* StaticText76;
		wxStaticText* StaticText82;
		BulkEditChoice* Choice_Pinwheel_Style;
		wxStaticText* StaticText81;
		BulkEditValueCurveButton* BitmapButton_Pinwheel_ThicknessVC;
		wxBitmapButton* BitmapButton_PinwheelTwist;
		wxStaticText* StaticText84;
		wxStaticText* StaticText1;
		BulkEditSlider* Slider_Pinwheel_ArmSize;
		BulkEditSlider* Slider_Pinwheel_Arms;
		BulkEditCheckBox* CheckBox_Pinwheel_Rotation;
		BulkEditSlider* Slider_Pinwheel_Twist;
		wxBitmapButton* BitmapButton_Pinwheel3D;
		wxBitmapButton* BitmapButton_PinwheelRotation;
		BulkEditSlider* Slider_PinwheelYC;
		BulkEditSlider* Slider_PinwheelXC;
		wxBitmapButton* BitmapButton7;
		BulkEditValueCurveButton* BitmapButton_Pinwheel_SpeedVC;
		wxBitmapButton* BitmapButton_PinwheelThickness;
		BulkEditValueCurveButton* BitmapButton_Pinwheel_TwistVC;
		BulkEditSlider* Slider_Pinwheel_Thickness;
		BulkEditChoice* Choice_Pinwheel_3D;
		ValueCurveButton* BitmapButton_PinwheelXCVC;
		wxBitmapButton* BitmapButton_PinwheelNumberArms;
		BulkEditSlider* Slider_Pinwheel_Speed;
		wxStaticText* StaticText63;
		wxStaticText* StaticText184;
		wxStaticText* StaticText77;
		//*)

	protected:

		//(*Identifiers(PinwheelPanel)
		static const long ID_STATICTEXT_Pinwheel_Arms;
		static const long ID_SLIDER_Pinwheel_Arms;
		static const long IDD_TEXTCTRL_Pinwheel_Arms;
		static const long ID_BITMAPBUTTON11;
		static const long ID_STATICTEXT_Pinwheel_ArmSize;
		static const long ID_SLIDER_Pinwheel_ArmSize;
		static const long ID_VALUECURVE_Pinwheel_ArmSize;
		static const long IDD_TEXTCTRL_Pinwheel_ArmSize;
		static const long ID_BITMAPBUTTON16;
		static const long ID_STATICTEXT_Pinwheel_Twist;
		static const long ID_SLIDER_Pinwheel_Twist;
		static const long ID_VALUECURVE_Pinwheel_Twist;
		static const long IDD_TEXTCTRL_Pinwheel_Twist;
		static const long ID_BITMAPBUTTON12;
		static const long ID_STATICTEXT_Pinwheel_Thickness;
		static const long ID_SLIDER_Pinwheel_Thickness;
		static const long ID_VALUECURVE_Pinwheel_Thickness;
		static const long IDD_TEXTCTRL_Pinwheel_Thickness;
		static const long ID_BITMAPBUTTON18;
		static const long ID_STATICTEXT_Pinwheel_Speed;
		static const long ID_SLIDER_Pinwheel_Speed;
		static const long ID_VALUECURVE_Pinwheel_Speed;
		static const long IDD_TEXTCTRL_Pinwheel_Speed;
		static const long ID_BITMAPBUTTON26;
		static const long ID_STATICTEXT_Pinwheel_Style;
		static const long ID_CHOICE_Pinwheel_Style;
		static const long ID_STATICTEXT78;
		static const long ID_CHECKBOX_Pinwheel_Rotation;
		static const long ID_BITMAPBUTTON19;
		static const long ID_STATICTEXT_Pinwheel_3D;
		static const long ID_CHOICE_Pinwheel_3D;
		static const long ID_BITMAPBUTTON_Pinwheel3D;
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
