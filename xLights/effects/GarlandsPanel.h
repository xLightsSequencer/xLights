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

#include "../BulkEditControls.h"

class GarlandsPanel: public wxPanel
{
	public:

		GarlandsPanel(wxWindow* parent);
		virtual ~GarlandsPanel();

		//(*Declarations(GarlandsPanel)
		BulkEditValueCurveButton* BitmapButton_Garlands_CyclesVC;
		BulkEditSliderF1* Slider_Garlands_Cycles;
		wxBitmapButton* BitmapButton_GarlandsType;
		wxStaticText* StaticText33;
		BulkEditValueCurveButton* BitmapButton_Garlands_SpacingVC;
		wxBitmapButton* BitmapButton_GarlandsSpacing;
		wxStaticText* StaticText192;
		wxStaticText* StaticText32;
		wxStaticText* StaticText183;
		BulkEditChoice* Choice_Garlands_Direction;
		BulkEditSlider* Slider_Garlands_Type;
		BulkEditSlider* Slider_Garlands_Spacing;
		//*)

	protected:

		//(*Identifiers(GarlandsPanel)
		static const long ID_STATICTEXT_Garlands_Type;
		static const long ID_SLIDER_Garlands_Type;
		static const long IDD_TEXTCTRL_Garlands_Type;
		static const long ID_BITMAPBUTTON_SLIDER_Garlands_Type;
		static const long ID_STATICTEXT_Garlands_Spacing;
		static const long ID_SLIDER_Garlands_Spacing;
		static const long ID_VALUECURVE_Garlands_Spacing;
		static const long IDD_TEXTCTRL_Garlands_Spacing;
		static const long ID_BITMAPBUTTON_SLIDER_Garlands_Spacing;
		static const long ID_STATICTEXT_Garlands_Cycles;
		static const long IDD_SLIDER_Garlands_Cycles;
		static const long ID_VALUECURVE_Garlands_Cycles;
		static const long ID_TEXTCTRL_Garlands_Cycles;
		static const long ID_STATICTEXT40;
		static const long ID_CHOICE_Garlands_Direction;
		//*)

	public:

		//(*Handlers(GarlandsPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
