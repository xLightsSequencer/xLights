#ifndef BARSPANEL_H
#define BARSPANEL_H

//(*Headers(BarsPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../BulkEditControls.h"

class BarsPanel: public wxPanel
{
    void ValidateWindow();

	public:

		BarsPanel(wxWindow* parent);
		virtual ~BarsPanel();

		//(*Declarations(BarsPanel)
		BulkEditValueCurveButton* BitmapButton_Bars_Center;
		wxStaticText* StaticText177;
		BulkEditValueCurveButton* BitmapButton_Bars_BarCount;
		BulkEditSlider* Slider_Bars_Center;
		BulkEditTextCtrl* TextCtrl_Bars_Center;
		BulkEditCheckBox* CheckBox_Bars_Highlight;
		wxStaticText* StaticText1;
		BulkEditChoice* Choice_Bars_Direction;
		wxStaticText* StaticText23;
		wxStaticText* StaticText24;
		BulkEditSlider* Slider_Bars_BarCount;
		BulkEditValueCurveButton* BitmapButton_Bars_Cycles;
		BulkEditCheckBox* CheckBox_Bars_3D;
		BulkEditSliderF1* Slider_Bars_Cycles;
		BulkEditCheckBox* CheckBox_Bars_Gradient;
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

#endif
