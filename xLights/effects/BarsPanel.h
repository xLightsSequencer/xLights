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

 //(*Headers(BarsPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class BarsPanel: public wxPanel
{
    void ValidateWindow();

	public:

		BarsPanel(wxWindow* parent);
		virtual ~BarsPanel();

		//(*Declarations(BarsPanel)
		BulkEditCheckBox* CheckBox_Bars_3D;
		BulkEditCheckBox* CheckBox_Bars_Gradient;
		BulkEditCheckBox* CheckBox_Bars_Highlight;
		BulkEditChoice* Choice_Bars_Direction;
		BulkEditSlider* Slider_Bars_BarCount;
		BulkEditSlider* Slider_Bars_Center;
		BulkEditSliderF1* Slider_Bars_Cycles;
		BulkEditTextCtrl* TextCtrl_Bars_Center;
		BulkEditValueCurveButton* BitmapButton_Bars_BarCount;
		BulkEditValueCurveButton* BitmapButton_Bars_Center;
		BulkEditValueCurveButton* BitmapButton_Bars_Cycles;
		wxStaticText* StaticText177;
		wxStaticText* StaticText1;
		wxStaticText* StaticText23;
		wxStaticText* StaticText24;
		//*)

	protected:

		//(*Identifiers(BarsPanel)
		static const long ID_STATICTEXT_Bars_BarCount;
		static const long ID_SLIDER_Bars_BarCount;
		static const long ID_VALUECURVE_Bars_BarCount;
		static const long IDD_TEXTCTRL_Bars_BarCount;
		static const long ID_BITMAPBUTTON_SLIDER_Bars_BarCount;
		static const long ID_STATICTEXT_Bars_Cycles;
		static const long IDD_SLIDER_Bars_Cycles;
		static const long ID_VALUECURVE_Bars_Cycles;
		static const long ID_TEXTCTRL_Bars_Cycles;
		static const long ID_STATICTEXT_Bars_Direction;
		static const long ID_CHOICE_Bars_Direction;
		static const long ID_BITMAPBUTTON_CHOICE_Bars_Direction;
		static const long ID_STATICTEXT_Bars_Center;
		static const long IDD_SLIDER_Bars_Center;
		static const long ID_VALUECURVE_Bars_Center;
		static const long ID_TEXTCTRL_Bars_Center;
		static const long ID_CHECKBOX_Bars_Highlight;
		static const long ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight;
		static const long ID_CHECKBOX_Bars_3D;
		static const long ID_BITMAPBUTTON_CHECKBOX_Bars_3D;
		static const long ID_CHECKBOX_Bars_Gradient;
		static const long ID_BITMAPBUTTON_CHECKBOX_Bars_Gradient;
		//*)

	private:

		//(*Handlers(BarsPanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnChoice_Bars_DirectionSelect(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};
