#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*Headers(GarlandsPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class GarlandsPanel: public wxPanel
{
	public:

		GarlandsPanel(wxWindow* parent);
		virtual ~GarlandsPanel();

		//(*Declarations(GarlandsPanel)
		BulkEditChoice* Choice_Garlands_Direction;
		BulkEditSlider* Slider_Garlands_Spacing;
		BulkEditSlider* Slider_Garlands_Type;
		BulkEditSliderF1* Slider_Garlands_Cycles;
		BulkEditValueCurveButton* BitmapButton_Garlands_CyclesVC;
		BulkEditValueCurveButton* BitmapButton_Garlands_SpacingVC;
		wxStaticText* StaticText183;
		wxStaticText* StaticText192;
		wxStaticText* StaticText32;
		wxStaticText* StaticText33;
		xlLockButton* BitmapButton_GarlandsSpacing;
		xlLockButton* BitmapButton_GarlandsType;
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
