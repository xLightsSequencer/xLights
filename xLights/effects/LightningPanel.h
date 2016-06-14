#ifndef LIGHTNINGPANEL_H
#define LIGHTNINGPANEL_H

//(*Headers(LightningPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

#include "../ValueCurveButton.h"

class LightningPanel: public wxPanel
{
	public:

		LightningPanel(wxWindow* parent);
		virtual ~LightningPanel();

		//(*Declarations(LightningPanel)
		wxTextCtrl* TextCtrl88;
		wxStaticText* StaticText203;
		wxCheckBox* ForkedLightning;
		wxBitmapButton* BitmapButton22;
		ValueCurveButton* BitmapButton_Number_BoltsVC;
		wxStaticText* StaticText209;
		wxSlider* Slider_Lightning_TopY;
		wxStaticText* StaticText205;
		wxBitmapButton* BitmapButton26;
		wxBitmapButton* BitmapButton20;
		wxStaticText* StaticText208;
		wxBitmapButton* BitmapButton28;
		ValueCurveButton* BitmapButton_Lightning_TopYVC;
		wxTextCtrl* TextCtrl90;
		wxStaticText* StaticText210;
		wxTextCtrl* TextCtrl86;
		wxSlider* Lightning_BOTX;
		wxStaticText* StaticText206;
		wxSlider* Lightning_TopX;
		wxBitmapButton* BitmapButton23;
		ValueCurveButton* BitmapButton_Lightning_TopXVC;
		wxStaticText* StaticText207;
		ValueCurveButton* BitmapButton_Number_SegmentsVC;
		wxBitmapButton* BitmapButton27;
		wxTextCtrl* TextCtrl89;
		wxBitmapButton* BitmapButton25;
		wxSlider* Slider_Lightning_BOTX;
		//*)

	protected:

		//(*Identifiers(LightningPanel)
		static const long ID_STATICTEXT25;
		static const long ID_SLIDER_Number_Bolts;
		static const long ID_VALUECURVE_Number_Bolts;
		static const long IDD_TEXTCTRL_Number_Bolts;
		static const long ID_BITMAPBUTTON38;
		static const long ID_STATICTEXT26;
		static const long ID_SLIDER_Number_Segments;
		static const long ID_VALUECURVE_Number_Segments;
		static const long IDD_TEXTCTRL_Number_Segments;
		static const long ID_BITMAPBUTTON40;
		static const long ID_STATICTEXT60;
		static const long ID_CHECKBOX_ForkedLightning;
		static const long ID_BITMAPBUTTON41;
		static const long ID_STATICTEXT64;
		static const long ID_SLIDER_Lightning_TopX;
		static const long ID_VALUECURVE_Lightning_TopX;
		static const long IDD_TEXTCTRL_Lightning_TopX;
		static const long ID_BITMAPBUTTON42;
		static const long ID_STATICTEXT67;
		static const long ID_SLIDER_Lightning_TopY;
		static const long ID_VALUECURVE_Lightning_TopY;
		static const long IDD_TEXTCTRL_Lightning_TopY;
		static const long ID_BITMAPBUTTON43;
		static const long ID_STATICTEXT70;
		static const long ID_SLIDER_Lightning_BOTX;
		static const long IDD_TEXTCTRL_Lightning_BOTX;
		static const long ID_BITMAPBUTTON45;
		static const long ID_STATICTEXT73;
		static const long ID_SLIDER_Lightning_BOTY;
		static const long IDD_TEXTCTRL_Lightning_BOTY;
		static const long ID_BITMAPBUTTON52;
		//*)

	public:

		//(*Handlers(LightningPanel)
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

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
