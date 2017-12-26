#ifndef SPIROGRAPHPANEL_H
#define SPIROGRAPHPANEL_H

//(*Headers(SpirographPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

#include "../BulkEditControls.h"

class SpirographPanel: public wxPanel
{
	public:

		SpirographPanel(wxWindow* parent);
		virtual ~SpirographPanel();

		//(*Declarations(SpirographPanel)
		BulkEditValueCurveButton* BitmapButton_Spirograph_LengthVC;
		wxBitmapButton* BitmapButton15;
		wxBitmapButton* BitmapButton_Spirographd;
		BulkEditSlider* Slider_Spirograph_r;
		BulkEditSlider* Slider_Spirograph_d;
		wxStaticText* StaticText87;
		wxStaticText* StaticText90;
		wxBitmapButton* BitmapButton14;
		BulkEditSlider* Slider_Spirograph_Length;
		wxBitmapButton* BitmapButton_SpirographAnimate;
		BulkEditValueCurveButton* BitmapButton_Spirograph_rVC;
		BulkEditValueCurveButton* BitmapButton_Spirograph_RVC;
		BulkEditValueCurveButton* BitmapButton_Spirograph_AnimateVC;
		BulkEditValueCurveButton* BitmapButton_Spirograph_SpeedrVC;
		wxStaticText* StaticText191;
		wxBitmapButton* BitmapButton_Spirographr;
		wxStaticText* StaticText190;
		BulkEditSlider* Slider_Spirograph_Animate;
		wxStaticText* StaticText92;
		wxStaticText* StaticText88;
		BulkEditSlider* Slider_Spirograph_Speed;
		wxStaticText* StaticText89;
		wxBitmapButton* BitmapButton_SpirographR;
		BulkEditValueCurveButton* BitmapButton_Spirograph_dVC;
		BulkEditSlider* Slider_Spirograph_R;
		//*)

	protected:

		//(*Identifiers(SpirographPanel)
		static const long ID_STATICTEXT_Spirograph_Speed;
		static const long IDD_SLIDER_Spirograph_Speed;
		static const long ID_VALUECURVE_Spirograph_Speed;
		static const long ID_TEXTCTRL_Spirograph_Speed;
		static const long ID_BITMAPBUTTON32;
		static const long ID_STATICTEXT_Spirograph_R;
		static const long ID_SLIDER_Spirograph_R;
		static const long ID_VALUECURVE_Spirograph_R;
		static const long IDD_TEXTCTRL_Spirograph_R;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_R;
		static const long ID_STATICTEXT_Spirograph_r;
		static const long ID_SLIDER_Spirograph_r;
		static const long ID_VALUECURVE_Spirograph_r;
		static const long IDD_TEXTCTRL_Spirograph_r;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_r;
		static const long ID_STATICTEXT_Spirograph_d;
		static const long ID_SLIDER_Spirograph_d;
		static const long ID_VALUECURVE_Spirograph_d;
		static const long IDD_TEXTCTRL_Spirograph_d;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_d;
		static const long ID_STATICTEXT_Spirograph_Animate;
		static const long IDD_SLIDER_Spirograph_Animate;
		static const long ID_VALUECURVE_Spirograph_Animate;
		static const long ID_TEXTCTRL_Spirograph_Animate;
		static const long ID_BITMAPBUTTON_CHECKBOX_Spirograph_Animate;
		static const long ID_STATICTEXT_Spirograph_Length;
		static const long IDD_SLIDER_Spirograph_Length;
		static const long ID_VALUECURVE_Spirograph_Length;
		static const long ID_TEXTCTRL_Spirograph_Length;
		static const long ID_BITMAPBUTTON33;
		static const long ID_STATICTEXT91;
		//*)

	public:

		//(*Handlers(SpirographPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
