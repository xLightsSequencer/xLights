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

#include "../BulkEditControls.h"

class RipplePanel: public wxPanel
{
    void ValidateWindow();

	public:

		RipplePanel(wxWindow* parent);
		virtual ~RipplePanel();

		//(*Declarations(RipplePanel)
		BulkEditChoice* Choice_Ripple_Movement;
		wxBitmapButton* BitmapButton_Ripple3D;
		BulkEditSlider* Slider_Ripple_Thickness;
		BulkEditSliderF1* Slider_Ripple_Cycles;
		wxBitmapButton* BitmapButton_Ripple_Movement;
		wxBitmapButton* BitmapButton_RippleThickness;
		wxBitmapButton* BitmapButton_Ripple_Points;
		BulkEditSlider* Slider_Ripple_Points;
		BulkEditCheckBox* CheckBox_Ripple3D;
		BulkEditValueCurveButton* BitmapButton_Ripple_CyclesVC;
		wxStaticText* StaticText1;
		wxBitmapButton* BitmapButton_Ripple_Object_To_Draw;
		wxStaticText* StaticText72;
		wxStaticText* StaticText_Ripple_Rotation;
		wxStaticText* StaticText69;
		BulkEditValueCurveButton* BitmapButton_Ripple_ThicknessVC;
		wxStaticText* StaticText70;
		BulkEditSlider* Slider_Ripple_Rotation;
		BulkEditChoice* Choice_Ripple_Object_To_Draw;
		BulkEditTextCtrl* TextCtrl_Ripple_Rotation;
		wxBitmapButton* BitmapButton_Ripple_Rotation;
		BulkEditTextCtrl* TextCtrl_Ripple_Points;
		wxStaticText* StaticText176;
		//*)

	protected:

		//(*Identifiers(RipplePanel)
		static const long ID_STATICTEXT_Ripple_Object_To_Draw;
		static const long ID_CHOICE_Ripple_Object_To_Draw;
		static const long ID_BITMAPBUTTON20;
		static const long ID_STATICTEXT_Ripple_Movement;
		static const long ID_CHOICE_Ripple_Movement;
		static const long ID_BITMAPBUTTON13;
		static const long ID_STATICTEXT_Ripple_Thickness;
		static const long ID_SLIDER_Ripple_Thickness;
		static const long ID_VALUECURVE_Ripple_Thickness;
		static const long IDD_TEXTCTRL_Ripple_Thickness;
		static const long ID_BITMAPBUTTON14;
		static const long ID_STATICTEXT_Ripple_Cycles;
		static const long IDD_SLIDER_Ripple_Cycles;
		static const long ID_VALUECURVE_Ripple_Cycles;
		static const long ID_TEXTCTRL_Ripple_Cycles;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_RIPPLE_POINTS;
		static const long IDD_TEXTCTRL_RIPPLE_POINTS;
		static const long ID_BITMAPBUTTON_RIPPLE_POINTS;
		static const long ID_STATICTEXT_Ripple_Rotation;
		static const long ID_SLIDER_Ripple_Rotation;
		static const long IDD_TEXTCTRL_Ripple_Rotation;
		static const long ID_BITMAPBUTTON_Ripple_Rotation;
		static const long ID_CHECKBOX_Ripple3D;
		static const long ID_BITMAPBUTTON17;
		//*)

	public:

		//(*Handlers(RipplePanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
        void OnChoice_Ripple_Object_To_DrawSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
