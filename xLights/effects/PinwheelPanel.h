#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(PinwheelPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxBoxSizer;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class PinwheelPanel: public wxPanel
{
	public:

		PinwheelPanel(wxWindow* parent);
		virtual ~PinwheelPanel();

		//(*Declarations(PinwheelPanel)
		BulkEditCheckBox* CheckBox_Pinwheel_Rotation;
		BulkEditChoice* Choice_Pinwheel_3D;
		BulkEditChoice* Choice_Pinwheel_Style;
		BulkEditSlider* Slider_PinwheelXC;
		BulkEditSlider* Slider_PinwheelYC;
		BulkEditSlider* Slider_Pinwheel_ArmSize;
		BulkEditSlider* Slider_Pinwheel_Arms;
		BulkEditSlider* Slider_Pinwheel_Speed;
		BulkEditSlider* Slider_Pinwheel_Thickness;
		BulkEditSlider* Slider_Pinwheel_Twist;
		BulkEditValueCurveButton* BitmapButton_PinwheelXCVC;
		BulkEditValueCurveButton* BitmapButton_PinwheelYCVC;
		BulkEditValueCurveButton* BitmapButton_Pinwheel_ArmSizeVC;
		BulkEditValueCurveButton* BitmapButton_Pinwheel_SpeedVC;
		BulkEditValueCurveButton* BitmapButton_Pinwheel_ThicknessVC;
		BulkEditValueCurveButton* BitmapButton_Pinwheel_TwistVC;
		wxStaticText* StaticText184;
		wxStaticText* StaticText1;
		wxStaticText* StaticText63;
		wxStaticText* StaticText74;
		wxStaticText* StaticText75;
		wxStaticText* StaticText76;
		wxStaticText* StaticText77;
		wxStaticText* StaticText81;
		wxStaticText* StaticText82;
		wxStaticText* StaticText84;
		xlLockButton* BitmapButton_Pinwheel3D;
		xlLockButton* BitmapButton_PinwheelNumberArms;
		xlLockButton* BitmapButton_PinwheelRotation;
		xlLockButton* BitmapButton_PinwheelSpeed;
		xlLockButton* BitmapButton_PinwheelThickness;
		xlLockButton* BitmapButton_PinwheelTwist;
		xlLockButton* BitmapButton_Pinwheel_ArmSize;
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
