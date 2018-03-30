#ifndef CANDLEPANEL_H
#define CANDLEPANEL_H

//(*Headers(CandlePanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
//*)

#include "../BulkEditControls.h"

class CandlePanel: public wxPanel
{
    void ValidateWindow();
	public:

		CandlePanel(wxWindow* parent);
		virtual ~CandlePanel();

		//(*Declarations(CandlePanel)
		BulkEditValueCurveButton* BitmapButton_Candle_FlameAgilityVC;
		wxStaticText* StaticText132;
		BulkEditSlider* Slider_Candle_WindVariability;
		BulkEditTextCtrl* TextCtrl_Candle_WindVariability;
		BulkEditTextCtrl* TextCtrl_Candle_FlameAgility;
		BulkEditValueCurveButton* BitmapButton_Candle_WindVariabilityVC;
		wxBitmapButton* BitmapButton_Candle_WindCalmness;
		wxBitmapButton* BitmapButton_Cande_WinfVariability;
		BulkEditTextCtrl* TextCtrl_Candle_WindBaseline;
		BulkEditSlider* Slider_Candle_WindBaseline;
		wxStaticText* StaticText83;
		wxStaticText* StaticText1;
		BulkEditSlider* Slider_Candle_WindCalmness;
		BulkEditValueCurveButton* BitmapButton_Candle_WindBaselineVC;
		BulkEditSlider* Slider_Candle_FlameAgility;
		BulkEditTextCtrl* TextCtrl_Candle_WindCalmness;
		BulkEditCheckBox* CheckBox_PerNode;
		wxStaticText* StaticText30;
		wxBitmapButton* BitmapButton_Candle_FlameAgility;
		BulkEditValueCurveButton* BitmapButton_Candle_WindCalmnessVC;
		wxBitmapButton* BitmapButton_Cande_WindBaseline;
		//*)

	protected:

		//(*Identifiers(CandlePanel)
		static const long ID_STATICTEXT_Candle_FlameAgility;
		static const long IDD_SLIDER_Candle_FlameAgility;
		static const long ID_VALUECURVE_Candle_FlameAgility;
		static const long ID_TEXTCTRL_Candle_FlameAgility;
		static const long ID_BITMAPBUTTON_SLIDER_Candle_FlameAgility;
		static const long ID_STATICTEXT_Candle_WindBaseline;
		static const long IDD_SLIDER_Candle_WindBaseline;
		static const long ID_VALUECURVE_Candle_WindBaseline;
		static const long ID_TEXTCTRL_Candle_WindBaseline;
		static const long ID_BITMAPBUTTON_SLIDER_Candle_WindBaseline;
		static const long ID_STATICTEXT_Candle_WindVariability;
		static const long IDD_SLIDER_Candle_WindVariability;
		static const long ID_VALUECURVE_Candle_WindVariability;
		static const long ID_TEXTCTRL_Candle_WindVariability;
		static const long ID_BITMAPBUTTON_SLIDER_Candle_WindVariability;
		static const long ID_STATICTEXT_Candle_WindCalmness;
		static const long IDD_SLIDER_Candle_WindCalmness;
		static const long ID_VALUECURVE_Candle_WindCalmness;
		static const long ID_TEXTCTRL_Candle_WindCalmness;
		static const long ID_BITMAPBUTTON_SLIDER_Candle_WindCalmness;
		static const long ID_CHECKBOX_PerNode;
		//*)

	public:

		//(*Handlers(CandlePanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnCheckBox_Candle_GrowWithMusicClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
