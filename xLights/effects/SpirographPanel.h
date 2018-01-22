#ifndef SPIROGRAPHPANEL_H
#define SPIROGRAPHPANEL_H

//(*Headers(SpirographPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
//*)

#include "../BulkEditControls.h"

class SpirographPanel: public wxPanel
{
	public:

		SpirographPanel(wxWindow* parent);
		virtual ~SpirographPanel();

		//(*Declarations(SpirographPanel)
		wxStaticText* StaticText89;
		BulkEditSlider* Slider_Spirograph_Length;
		BulkEditSlider* Slider_Spirograph_Animate;
		wxBitmapButton* BitmapButton_Spirographr;
		wxStaticText* StaticText88;
		BulkEditValueCurveButton* BitmapButton_Spirograph_SpeedrVC;
		wxBitmapButton* BitmapButton_SpirographLength;
		wxStaticText* StaticText92;
		BulkEditValueCurveButton* BitmapButton_Spirograph_AnimateVC;
		BulkEditValueCurveButton* BitmapButton_Spirograph_rVC;
		BulkEditValueCurveButton* BitmapButton_Spirograph_RVC;
		wxStaticText* StaticText190;
		wxStaticText* StaticText87;
		wxBitmapButton* BitmapButton_SpirographAnimate;
		wxBitmapButton* BitmapButton_SpirographSpeed;
		wxStaticText* StaticText191;
		BulkEditSlider* Slider_Spirograph_d;
		wxBitmapButton* BitmapButton_SpirographR;
		BulkEditSlider* Slider_Spirograph_Speed;
		wxBitmapButton* BitmapButton_Spirographd;
		BulkEditSlider* Slider_Spirograph_r;
		BulkEditValueCurveButton* BitmapButton_Spirograph_dVC;
		wxStaticText* StaticText90;
		BulkEditValueCurveButton* BitmapButton_Spirograph_LengthVC;
		BulkEditSlider* Slider_Spirograph_R;
		//*)

	protected:

		//(*Identifiers(SpirographPanel)
		static const long ID_STATICTEXT_Spirograph_Speed;
		static const long IDD_SLIDER_Spirograph_Speed;
		static const long ID_VALUECURVE_Spirograph_Speed;
		static const long ID_TEXTCTRL_Spirograph_Speed;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_Speed;
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
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_Animate;
		static const long ID_STATICTEXT_Spirograph_Length;
		static const long IDD_SLIDER_Spirograph_Length;
		static const long ID_VALUECURVE_Spirograph_Length;
		static const long ID_TEXTCTRL_Spirograph_Length;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_Length;
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
