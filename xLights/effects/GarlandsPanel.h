#ifndef GARLANDSPANEL_H
#define GARLANDSPANEL_H

//(*Headers(GarlandsPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../ValueCurveButton.h"

class GarlandsPanel: public wxPanel
{
	public:

		GarlandsPanel(wxWindow* parent);
		virtual ~GarlandsPanel();

		//(*Declarations(GarlandsPanel)
		wxBitmapButton* BitmapButton_GarlandsType;
		ValueCurveButton* BitmapButton_Garlands_SpacingVC;
		wxStaticText* StaticText33;
		wxBitmapButton* BitmapButton_GarlandsSpacing;
		wxStaticText* StaticText192;
		wxStaticText* StaticText32;
		wxSlider* Slider_Garlands_Spacing;
		ValueCurveButton* BitmapButton_Garlands_CyclesVC;
		wxSlider* Slider_Garlands_Type;
		//*)

	protected:

		//(*Identifiers(GarlandsPanel)
		static const long ID_STATICTEXT34;
		static const long ID_SLIDER_Garlands_Type;
		static const long IDD_TEXTCTRL_Garlands_Type;
		static const long ID_BITMAPBUTTON_SLIDER_Garlands_Type;
		static const long ID_STATICTEXT35;
		static const long ID_SLIDER_Garlands_Spacing;
		static const long ID_VALUECURVE_Garlands_Spacing;
		static const long IDD_TEXTCTRL_Garlands_Spacing;
		static const long ID_BITMAPBUTTON_SLIDER_Garlands_Spacing;
		static const long IDD_SLIDER_Garlands_Cycles;
		static const long ID_VALUECURVE_Garlands_Cycles;
		static const long ID_TEXTCTRL_Garlands_Cycles;
		static const long ID_STATICTEXT40;
		static const long ID_CHOICE_Garlands_Direction;
		//*)

	public:

		//(*Handlers(GarlandsPanel)
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
