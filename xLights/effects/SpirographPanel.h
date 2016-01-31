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

class SpirographPanel: public wxPanel
{
	public:

		SpirographPanel(wxWindow* parent);
		virtual ~SpirographPanel();

		//(*Declarations(SpirographPanel)
		wxStaticText* StaticText89;
		wxBitmapButton* BitmapButton_Spirographr;
		wxStaticText* StaticText88;
		wxBitmapButton* BitmapButton15;
		wxStaticText* StaticText190;
		wxBitmapButton* BitmapButton14;
		wxStaticText* StaticText87;
		wxBitmapButton* BitmapButton_SpirographAnimate;
		wxBitmapButton* BitmapButton_SpirographR;
		wxBitmapButton* BitmapButton_Spirographd;
		wxStaticText* StaticText90;
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

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
