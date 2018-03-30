#ifndef RIPPLEPANEL_H
#define RIPPLEPANEL_H

//(*Headers(RipplePanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
//*)

#include "../BulkEditControls.h"

class RipplePanel: public wxPanel
{
    void ValidateWindow();

	public:

		RipplePanel(wxWindow* parent);
		virtual ~RipplePanel();

		//(*Declarations(RipplePanel)
		wxBitmapButton* BitmapButton_Ripple_Rotation;
		BulkEditChoice* Choice_Ripple_Object_To_Draw;
		wxStaticText* StaticText70;
		BulkEditChoice* Choice_Ripple_Movement;
		BulkEditTextCtrl* TextCtrl_Ripple_Points;
		wxStaticText* StaticText_Ripple_Rotation;
		BulkEditSlider* Slider_Ripple_Rotation;
		wxStaticText* StaticText1;
		wxBitmapButton* BitmapButton_Ripple_Movement;
		wxStaticText* StaticText176;
		wxStaticText* StaticText69;
		wxBitmapButton* BitmapButton_Ripple_Points;
		BulkEditSlider* Slider_Ripple_Thickness;
		wxBitmapButton* BitmapButton_Ripple_Object_To_Draw;
		BulkEditValueCurveButton* BitmapButton_Ripple_ThicknessVC;
		BulkEditSlider* Slider_Ripple_Points;
		wxBitmapButton* BitmapButton_RippleThickness;
		wxStaticText* StaticText72;
		BulkEditSliderF1* Slider_Ripple_Cycles;
		BulkEditCheckBox* CheckBox_Ripple3D;
		BulkEditTextCtrl* TextCtrl_Ripple_Rotation;
		wxBitmapButton* BitmapButton_Ripple3D;
		BulkEditValueCurveButton* BitmapButton_Ripple_CyclesVC;
		//*)

	protected:

		//(*Identifiers(RipplePanel)
		static const long ID_STATICTEXT_Ripple_Object_To_Draw;
		static const long ID_CHOICE_Ripple_Object_To_Draw;
		static const long ID_BITMAPBUTTON_CHOICE_Ripple_Object_To_Draw;
		static const long ID_STATICTEXT_Ripple_Movement;
		static const long ID_CHOICE_Ripple_Movement;
		static const long ID_BITMAPBUTTON_CHOICE_Ripple_Movement;
		static const long ID_STATICTEXT_Ripple_Thickness;
		static const long ID_SLIDER_Ripple_Thickness;
		static const long ID_VALUECURVE_Ripple_Thickness;
		static const long IDD_TEXTCTRL_Ripple_Thickness;
		static const long ID_BITMAPBUTTON_SLIDER_Ripple_Thickness;
		static const long ID_STATICTEXT_Ripple_Cycles;
		static const long IDD_SLIDER_Ripple_Cycles;
		static const long ID_VALUECURVE_Ripple_Cycles;
		static const long ID_TEXTCTRL_Ripple_Cycles;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_RIPPLE_POINTS;
		static const long IDD_TEXTCTRL_RIPPLE_POINTS;
		static const long ID_BITMAPBUTTON_SLIDER_RIPPLE_POINTS;
		static const long ID_STATICTEXT_Ripple_Rotation;
		static const long ID_SLIDER_Ripple_Rotation;
		static const long IDD_TEXTCTRL_Ripple_Rotation;
		static const long ID_BITMAPBUTTON_SLIDER_Ripple_Rotation;
		static const long ID_CHECKBOX_Ripple3D;
		static const long ID_BITMAPBUTTON_CHECKBOX_Ripple3D;
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
