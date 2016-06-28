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

class SpirographPanel: public wxPanel
{
	public:

		SpirographPanel(wxWindow* parent);
		virtual ~SpirographPanel();

		//(*Declarations(SpirographPanel)
		wxSlider* Slider_Spirograph_Length;
		wxBitmapButton* BitmapButton15;
		wxBitmapButton* BitmapButton_Spirographd;
		wxSlider* Slider_Spirograph_Animate;
		wxSlider* Slider_Spirograph_Speed;
		wxStaticText* StaticText87;
		wxStaticText* StaticText90;
		wxBitmapButton* BitmapButton14;
		wxSlider* Slider_Spirograph_d;
		wxSlider* Slider_Spirograph_R;
		wxBitmapButton* BitmapButton_SpirographAnimate;
		wxBitmapButton* BitmapButton_Spirographr;
		wxStaticText* StaticText190;
		wxSlider* Slider_Spirograph_r;
		wxStaticText* StaticText88;
		wxStaticText* StaticText89;
		wxBitmapButton* BitmapButton_SpirographR;
		//*)

	protected:

		//(*Identifiers(SpirographPanel)
		static const long ID_STATICTEXT33;
		static const long IDD_SLIDER_Spirograph_Speed;
		static const long ID_TEXTCTRL_Spirograph_Speed;
		static const long ID_BITMAPBUTTON32;
		static const long ID_STATICTEXT88;
		static const long ID_SLIDER_Spirograph_R;
		static const long IDD_TEXTCTRL_Spirograph_R;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_R;
		static const long ID_STATICTEXT89;
		static const long ID_SLIDER_Spirograph_r;
		static const long IDD_TEXTCTRL_Spirograph_r;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_r;
		static const long ID_STATICTEXT90;
		static const long ID_SLIDER_Spirograph_d;
		static const long IDD_TEXTCTRL_Spirograph_d;
		static const long ID_BITMAPBUTTON_SLIDER_Spirograph_d;
		static const long IDD_SLIDER_Spirograph_Animate;
		static const long ID_TEXTCTRL_Spirograph_Animate;
		static const long ID_BITMAPBUTTON_CHECKBOX_Spirograph_Animate;
		static const long IDD_SLIDER_Spirograph_Length;
		static const long ID_TEXTCTRL_Spirograph_Length;
		static const long ID_BITMAPBUTTON33;
		static const long ID_STATICTEXT91;
		//*)

	public:

		//(*Handlers(SpirographPanel)
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
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
