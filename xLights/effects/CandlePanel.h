#ifndef CANDLEPANEL_H
#define CANDLEPANEL_H

//(*Headers(CandlePanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

#include "../BulkEditControls.h"

class CandlePanel: public wxPanel
{
    void ValidateWindow();
	public:

		CandlePanel(wxWindow* parent);
		virtual ~CandlePanel();

		//(*Declarations(CandlePanel)
		wxStaticText* StaticText83;
		BulkEditSlider* Slider_Candle_FlameAgility;
		BulkEditValueCurveButton* BitmapButton_Candle_WindVariabilityVC;
		wxBitmapButton* BitmapButton_Cande_WinfVariability;
		BulkEditSlider* Slider_Candle_WindVariability;
		BulkEditValueCurveButton* BitmapButton_Candle_WindCalmnessVC;
		wxStaticText* StaticText30;
		BulkEditValueCurveButton* BitmapButton_Candle_FlameAgilityVC;
		wxStaticText* StaticText1;
		BulkEditSlider* Slider_Candle_WindCalmness;
		wxBitmapButton* BitmapButton_Candle_FlameAgility;
		BulkEditCheckBox* CheckBox_PerNode;
		BulkEditTextCtrl* TextCtrl_Candle_WindBaseline;
		BulkEditValueCurveButton* BitmapButton_Candle_WindBaselineVC;
		wxBitmapButton* BitmapButton_Candle_WindCalmness;
		wxBitmapButton* BitmapButton_Cande_WindBaseline;
		BulkEditTextCtrl* TextCtrl_Candle_FlameAgility;
		wxStaticText* StaticText132;
		BulkEditSlider* Slider_Candle_WindBaseline;
		BulkEditTextCtrl* TextCtrl_Candle_WindVariability;
		BulkEditTextCtrl* TextCtrl_Candle_WindCalmness;
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
		static const long ID_BITMAPBUTTON_SLIDER_Cande_WindBaseline;
		static const long ID_STATICTEXT_Candle_WindVariability;
		static const long IDD_SLIDER_Candle_WindVariability;
		static const long ID_VALUECURVE_Candle_WindVariability;
		static const long ID_TEXTCTRL_Candle_WindVariability;
		static const long ID_BITMAPBUTTON_CHECKBOX_Cande_WindVariability;
		static const long ID_STATICTEXT_Candle_WindCalmness;
		static const long IDD_SLIDER_Candle_WindCalmness;
		static const long ID_VALUECURVE_Candle_WindCalmness;
		static const long ID_TEXTCTRL_Candle_WindCalmness;
		static const long ID_BITMAPBUTTON_Candle_WindCalmness;
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
