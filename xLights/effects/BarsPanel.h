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

#include "../ValueCurveButton.h"

class BarsPanel: public wxPanel
{
    void ValidateWindow();

	public:

		BarsPanel(wxWindow* parent);
		virtual ~BarsPanel();

		//(*Declarations(BarsPanel)
		ValueCurveButton* BitmapButton_Bars_BarCount;
		wxSlider* Slider_Bars_Center;
		wxStaticText* StaticText1;
		wxSlider* Slider_Bars_BarCount;
		wxChoice* Choice_Bars_Direction;
		wxTextCtrl* TextCtrl_Bars_Center;
		ValueCurveButton* BitmapButton_Bars_Cycles;
		//*)

	protected:

		//(*Identifiers(BarsPanel)
		static const long ID_SLIDER_Bars_BarCount;
		static const long ID_VALUECURVE_Bars_BarCount;
		static const long IDD_TEXTCTRL_Bars_BarCount;
		static const long ID_BITMAPBUTTON_SLIDER_Bars_BarCount;
		static const long IDD_SLIDER_Bars_Cycles;
		static const long ID_VALUECURVE_Bars_Cycles;
		static const long ID_TEXTCTRL_Bars_Cycles;
		static const long ID_CHOICE_Bars_Direction;
		static const long ID_BITMAPBUTTON_CHOICE_Bars_Direction;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_Bars_Center;
		static const long IDD_TEXTCTRL_Bars_Center;
		static const long ID_CHECKBOX_Bars_Highlight;
		static const long ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight;
		static const long ID_CHECKBOX_Bars_3D;
		static const long ID_BITMAPBUTTON_CHECKBOX_Bars_3D;
		static const long ID_CHECKBOX_Bars_Gradient;
		static const long ID_BITMAPBUTTON_CHECKBOX_Bars_Gradient;
		//*)

	private:

		//(*Handlers(BarsPanel)
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
		void OnChoice_Bars_DirectionSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
