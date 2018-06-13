#ifndef CANDLEPANEL_H
#define CANDLEPANEL_H

//(*Headers(CandlePanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class CandlePanel: public wxPanel
{
    void ValidateWindow();
	public:

		CandlePanel(wxWindow* parent);
		virtual ~CandlePanel();

		//(*Declarations(CandlePanel)
		BulkEditCheckBox* CheckBox_PerNode;
		BulkEditSlider* Slider_Candle_FlameAgility;
		BulkEditSlider* Slider_Candle_WindBaseline;
		BulkEditSlider* Slider_Candle_WindCalmness;
		BulkEditSlider* Slider_Candle_WindVariability;
		BulkEditTextCtrl* TextCtrl_Candle_FlameAgility;
		BulkEditTextCtrl* TextCtrl_Candle_WindBaseline;
		BulkEditTextCtrl* TextCtrl_Candle_WindCalmness;
		BulkEditTextCtrl* TextCtrl_Candle_WindVariability;
		BulkEditValueCurveButton* BitmapButton_Candle_FlameAgilityVC;
		BulkEditValueCurveButton* BitmapButton_Candle_WindBaselineVC;
		BulkEditValueCurveButton* BitmapButton_Candle_WindCalmnessVC;
		BulkEditValueCurveButton* BitmapButton_Candle_WindVariabilityVC;
		wxStaticText* StaticText132;
		wxStaticText* StaticText1;
		wxStaticText* StaticText30;
		wxStaticText* StaticText83;
		xlLockButton* BitmapButton_Cande_WindBaseline;
		xlLockButton* BitmapButton_Cande_WinfVariability;
		xlLockButton* BitmapButton_Candle_FlameAgility;
		xlLockButton* BitmapButton_Candle_WindCalmness;
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
