#ifndef TENDRILPANEL_H
#define TENDRILPANEL_H

//(*Headers(TendrilPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
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
		wxStaticText* StaticText10;
		BulkEditTextCtrl* TextCtrl_Tendril_YOffset;
		wxStaticText* StaticText9;
		BulkEditSlider* Slider_Tendril_ManualX;
		BulkEditTextCtrl* TextCtrl_Tendril_TuneMovement;
		BulkEditTextCtrl* TextCtrl_Tendril_Length;
		BulkEditSlider* Slider_Tendril_Dampening;
		BulkEditTextCtrl* TextCtrl_Tendril_Friction;
		BulkEditSlider* Slider_Tendril_Friction;
		wxBitmapButton* BitmapButton_Tendril_XOffset;
		BulkEditTextCtrl* TextCtrl_Tendril_Tension;
		wxStaticText* StaticText13;
		wxStaticText* StaticText2;
		wxBitmapButton* BitmapButton_TendrilSpeed;
		BulkEditValueCurveButton* BitmapButton_Tendril_ManualXVC;
		BulkEditValueCurveButton* BitmapButton_Tendril_TuneMovementVC;
		BulkEditValueCurveButton* BitmapButton_Tendril_ManualYVC;
		BulkEditTextCtrl* TextCtrl_Tendril_Dampening;
		BulkEditSlider* Slider_Tendril_Speed;
		wxStaticText* StaticText6;
		BulkEditSlider* Slider_Tendril_XOffset;
		BulkEditValueCurveButton* BitmapButton_Tendril_XOffsetVC;
		BulkEditTextCtrl* TextCtrl_Tendril_ManualX;
		wxStaticText* StaticText8;
		wxStaticText* StaticText11;
		wxBitmapButton* BitmapButton_TendrilDampening;
		BulkEditTextCtrl* TextCtrl_Tendril_Speed;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		BulkEditSlider* Slider_Tendril_TuneMovement;
		BulkEditSlider* Slider_Tendril_ManualY;
		BulkEditTextCtrl* TextCtrl_Tendril_XOffset;
		wxBitmapButton* BitmapButton_TendrilTension;
		wxBitmapButton* BitmapButton_TendrilFriction;
		BulkEditTextCtrl* TextCtrl_Tendril_Thickness;
		wxStaticText* StaticText5;
		wxBitmapButton* BitmapButton_TendrilLength;
		wxStaticText* StaticText7;
		wxBitmapButton* BitmapButton_Tendril_Movement;
		BulkEditTextCtrl* TextCtrl_Tendril_Trails;
		wxBitmapButton* BitmapButton_Tendril_Thickness;
		BulkEditTextCtrl* TextCtrl_Tendril_ManualY;
		BulkEditSlider* Slider_Tendril_Length;
		BulkEditChoice* Choice_Tendril_Movement;
		wxStaticText* StaticText12;
		wxStaticText* StaticText68;
		BulkEditSlider* Slider_Tendril_YOffset;
		wxBitmapButton* BitmapButton_Tendril_YOffset;
		wxBitmapButton* BitmapButton_TendrilTrails;
		BulkEditValueCurveButton* BitmapButton_Tendril_YOffsetVC;
		wxStaticText* StaticText4;
		wxBitmapButton* BitmapButton_Tendril_TuneMovement;
		BulkEditSlider* Slider_Tendril_Tension;
		BulkEditSlider* Slider_Tendril_Trails;
		BulkEditSlider* Slider_Tendril_Thickness;
		BulkEditValueCurveButton* BitmapButton_Tendril_ThicknessVC;
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
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_TuneMovement;
		static const long ID_STATICTEXT_Tendril_Thickness;
		static const long IDD_SLIDER_Tendril_Thickness;
		static const long ID_VALUECURVE_Tendril_Thickness;
		static const long ID_TEXTCTRL_Tendril_Thickness;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Thickness;
		static const long ID_STATICTEXT_Tendril_Friction;
		static const long IDD_SLIDER_Tendril_Friction;
		static const long ID_TEXTCTRL_Tendril_Friction;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Friction;
		static const long ID_STATICTEXT_Tendril_Dampening;
		static const long IDD_SLIDER_Tendril_Dampening;
		static const long ID_TEXTCTRL_Tendril_Dampening;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Dampening;
		static const long ID_STATICTEXT_Tendril_Tension;
		static const long IDD_SLIDER_Tendril_Tension;
		static const long ID_TEXTCTRL_Tendril_Tension;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Tension;
		static const long ID_STATICTEXT_Tendril_Trails;
		static const long IDD_SLIDER_Tendril_Trails;
		static const long ID_TEXTCTRL_Tendril_Trails;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Trails;
		static const long ID_STATICTEXT_Tendril_Length;
		static const long IDD_SLIDER_Tendril_Length;
		static const long ID_TEXTCTRL_Tendril_Length;
		static const long ID_BITMAPBUTTON_CHOICE_Tendril_Length;
		static const long ID_STATICTEXT_Tendril_Speed;
		static const long IDD_SLIDER_Tendril_Speed;
		static const long ID_TEXTCTRL_Tendril_Speed;
		static const long IID_BITMAPBUTTON_CHOICE_Tendril_Speed;
		static const long ID_STATICTEXT_Tendril_XOffset;
		static const long IDD_SLIDER_Tendril_XOffset;
		static const long ID_VALUECURVE_Tendril_XOffset;
		static const long ID_TEXTCTRL_Tendril_XOffset;
		static const long ID_BITMAPBUTTON_Tendril_XOffset;
		static const long ID_STATICTEXT_Tendril_YOffset;
		static const long IDD_SLIDER_Tendril_YOffset;
		static const long ID_VALUECURVE_Tendril_YOffset;
		static const long ID_TEXTCTRL_Tendril_YOffset;
		static const long ID_BITMAPBUTTON_Tendril_YOffset;
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

#endif
