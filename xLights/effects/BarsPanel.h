#ifndef BARSPANEL_H
#define BARSPANEL_H

//(*Headers(BarsPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
//*)

#include "../ValueCurveButton.h"

class BarsPanel: public wxPanel
{
	public:

		BarsPanel(wxWindow* parent);
		virtual ~BarsPanel();

		//(*Declarations(BarsPanel)
		wxChoice* Choice_Bars_Direction;
		ValueCurveButton* BitmapButton_Bars_Cycles;
		wxSlider* Slider_Bars_BarCount;
		ValueCurveButton* BitmapButton_Bars_BarCount;
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
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
