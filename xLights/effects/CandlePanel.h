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

#include "../ValueCurveButton.h"

class CandlePanel: public wxPanel
{
    void ValidateWindow();
	public:

		CandlePanel(wxWindow* parent);
		virtual ~CandlePanel();

		//(*Declarations(CandlePanel)
		wxStaticText* StaticText83;
		ValueCurveButton* BitmapButton_Candle_WindBaselineVC;
		wxSlider* Slider_Candle_WindCalmness;
		wxBitmapButton* BitmapButton_Cande_WinfVariability;
		wxTextCtrl* TextCtrl_Candle_WindVariability;
		wxTextCtrl* TextCtrl_Candle_FlameAgility;
		wxTextCtrl* TextCtrl_Candle_WindCalmness;
		wxSlider* Slider_Candle_FlameAgility;
		wxBitmapButton* BitmapButton_Candle_FlameAgility;
		wxTextCtrl* TextCtrl_Candle_WindBaseline;
		wxBitmapButton* BitmapButton_Candle_WindCalmness;
		wxBitmapButton* BitmapButton_Cande_WindBaseline;
		wxCheckBox* CheckBox_PerNode;
		wxSlider* Slider_Candle_WindBaseline;
		wxStaticText* StaticText132;
		wxSlider* Slider_Candle_WindVariability;
		ValueCurveButton* BitmapButton_Candle_WindCalmnessVC;
		ValueCurveButton* BitmapButton_Candle_WindVariabilityVC;
		ValueCurveButton* BitmapButton_Candle_FlameAgilityVC;
		//*)

	protected:

		//(*Identifiers(CandlePanel)
		static const long ID_STATICTEXT84;
		static const long IDD_SLIDER_Candle_FlameAgility;
		static const long ID_VALUECURVE_Candle_FlameAgility;
		static const long ID_TEXTCTRL_Candle_FlameAgility;
		static const long ID_BITMAPBUTTON_SLIDER_Candle_FlameAgility;
		static const long ID_STATICTEXT133;
		static const long IDD_SLIDER_Candle_WindBaseline;
		static const long ID_VALUECURVE_Candle_WindBaseline;
		static const long ID_TEXTCTRL_Candle_WindBaseline;
		static const long ID_BITMAPBUTTON_SLIDER_Cande_WindBaseline;
		static const long IDD_SLIDER_Candle_WindVariability;
		static const long ID_VALUECURVE_Candle_WindVariability;
		static const long ID_TEXTCTRL_Candle_WindVariability;
		static const long ID_BITMAPBUTTON_CHECKBOX_Cande_WindVariability;
		static const long IDD_SLIDER_Candle_WindCalmness;
		static const long ID_VALUECURVE_Candle_WindCalmness;
		static const long ID_TEXTCTRL_Candle_WindCalmness;
		static const long ID_BITMAPBUTTON_Candle_WindCalmness;
		static const long ID_CHECKBOX_PerNode;
		//*)

	public:

		//(*Handlers(CandlePanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnCheckBox_Candle_GrowWithMusicClick(wxCommandEvent& event);
		void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
		void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void UpdateLinkedSliderFloat2(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
		void OnCheckBox1Click(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
