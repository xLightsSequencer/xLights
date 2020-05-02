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

//(*Headers(TendrilPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class TendrilPanel: public wxPanel
{
    void ValidateWindow();
    // these are only used for validation
    int __tuneMovement;
    int __thickness;
    int __friction;
    int __dampening;
    int __tension;
    int __trails;
    int __length;
    int __speed;
    int __horizontalOffset;
    int __verticalOffset;

	public:

		TendrilPanel(wxWindow* parent);
		virtual ~TendrilPanel();

		//(*Declarations(TendrilPanel)
		BulkEditChoice* Choice_Tendril_Movement;
		BulkEditSlider* Slider_Tendril_Dampening;
		BulkEditSlider* Slider_Tendril_Friction;
		BulkEditSlider* Slider_Tendril_Length;
		BulkEditSlider* Slider_Tendril_ManualX;
		BulkEditSlider* Slider_Tendril_ManualY;
		BulkEditSlider* Slider_Tendril_Speed;
		BulkEditSlider* Slider_Tendril_Tension;
		BulkEditSlider* Slider_Tendril_Thickness;
		BulkEditSlider* Slider_Tendril_Trails;
		BulkEditSlider* Slider_Tendril_TuneMovement;
		BulkEditSlider* Slider_Tendril_XOffset;
		BulkEditSlider* Slider_Tendril_YOffset;
		BulkEditTextCtrl* TextCtrl_Tendril_Dampening;
		BulkEditTextCtrl* TextCtrl_Tendril_Friction;
		BulkEditTextCtrl* TextCtrl_Tendril_Length;
		BulkEditTextCtrl* TextCtrl_Tendril_ManualX;
		BulkEditTextCtrl* TextCtrl_Tendril_ManualY;
		BulkEditTextCtrl* TextCtrl_Tendril_Speed;
		BulkEditTextCtrl* TextCtrl_Tendril_Tension;
		BulkEditTextCtrl* TextCtrl_Tendril_Thickness;
		BulkEditTextCtrl* TextCtrl_Tendril_Trails;
		BulkEditTextCtrl* TextCtrl_Tendril_TuneMovement;
		BulkEditTextCtrl* TextCtrl_Tendril_XOffset;
		BulkEditTextCtrl* TextCtrl_Tendril_YOffset;
		BulkEditValueCurveButton* BitmapButton_Tendril_ManualXVC;
		BulkEditValueCurveButton* BitmapButton_Tendril_ManualYVC;
		BulkEditValueCurveButton* BitmapButton_Tendril_ThicknessVC;
		BulkEditValueCurveButton* BitmapButton_Tendril_TuneMovementVC;
		BulkEditValueCurveButton* BitmapButton_Tendril_XOffsetVC;
		BulkEditValueCurveButton* BitmapButton_Tendril_YOffsetVC;
		wxStaticText* StaticText10;
		wxStaticText* StaticText11;
		wxStaticText* StaticText12;
		wxStaticText* StaticText13;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText68;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		xlLockButton* BitmapButton_TendrilDampening;
		xlLockButton* BitmapButton_TendrilFriction;
		xlLockButton* BitmapButton_TendrilLength;
		xlLockButton* BitmapButton_TendrilSpeed;
		xlLockButton* BitmapButton_TendrilTension;
		xlLockButton* BitmapButton_TendrilTrails;
		xlLockButton* BitmapButton_Tendril_Movement;
		xlLockButton* BitmapButton_Tendril_Thickness;
		xlLockButton* BitmapButton_Tendril_TuneMovement;
		xlLockButton* BitmapButton_Tendril_XOffset;
		xlLockButton* BitmapButton_Tendril_YOffset;
		//*)

	protected:

		//(*Identifiers(TendrilPanel)
		static const long ID_STATICTEXT_Tendril_Movement;
		static const long ID_CHOICE_Tendril_Movement;
		static const long ID_STATICTEXT9;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Movement;
		static const long ID_STATICTEXT_Tendril_TuneMovement;
		static const long IDD_SLIDER_Tendril_TuneMovement;
		static const long ID_VALUECURVE_Tendril_TuneMovement;
		static const long ID_TEXTCTRL_Tendril_TuneMovement;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_TuneMovement;
		static const long ID_STATICTEXT_Tendril_Thickness;
		static const long IDD_SLIDER_Tendril_Thickness;
		static const long ID_VALUECURVE_Tendril_Thickness;
		static const long ID_TEXTCTRL_Tendril_Thickness;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_Thickness;
		static const long ID_STATICTEXT_Tendril_Friction;
		static const long IDD_SLIDER_Tendril_Friction;
		static const long ID_TEXTCTRL_Tendril_Friction;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_Friction;
		static const long ID_STATICTEXT_Tendril_Dampening;
		static const long IDD_SLIDER_Tendril_Dampening;
		static const long ID_TEXTCTRL_Tendril_Dampening;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_Dampening;
		static const long ID_STATICTEXT_Tendril_Tension;
		static const long IDD_SLIDER_Tendril_Tension;
		static const long ID_TEXTCTRL_Tendril_Tension;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_Tension;
		static const long ID_STATICTEXT_Tendril_Trails;
		static const long IDD_SLIDER_Tendril_Trails;
		static const long ID_TEXTCTRL_Tendril_Trails;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_Trails;
		static const long ID_STATICTEXT_Tendril_Length;
		static const long IDD_SLIDER_Tendril_Length;
		static const long ID_TEXTCTRL_Tendril_Length;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_Length;
		static const long ID_STATICTEXT_Tendril_Speed;
		static const long IDD_SLIDER_Tendril_Speed;
		static const long ID_TEXTCTRL_Tendril_Speed;
		static const long IID_BITMAPBUTTON_SLIDER_Tendril_Speed;
		static const long ID_STATICTEXT_Tendril_XOffset;
		static const long IDD_SLIDER_Tendril_XOffset;
		static const long ID_VALUECURVE_Tendril_XOffset;
		static const long ID_TEXTCTRL_Tendril_XOffset;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_XOffset;
		static const long ID_STATICTEXT_Tendril_YOffset;
		static const long IDD_SLIDER_Tendril_YOffset;
		static const long ID_VALUECURVE_Tendril_YOffset;
		static const long ID_TEXTCTRL_Tendril_YOffset;
		static const long ID_BITMAPBUTTON_SLIDER_Tendril_YOffset;
		static const long ID_STATICTEXT_Tendril_ManualX;
		static const long IDD_SLIDER_Tendril_ManualX;
		static const long ID_VALUECURVE_Tendril_ManualX;
		static const long ID_TEXTCTRL_Tendril_ManualX;
		static const long ID_STATICTEXT_Tendril_ManualY;
		static const long IDD_SLIDER_Tendril_ManualY;
		static const long ID_VALUECURVE_Tendril_ManualY;
		static const long ID_TEXTCTRL_Tendril_ManualY;
		//*)

	public:

		//(*Handlers(TendrilPanel)
		void OnLockButtonClick(wxCommandEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
		void OnChoice_Tendril_MovementSelect(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

    DECLARE_EVENT_TABLE()
};
