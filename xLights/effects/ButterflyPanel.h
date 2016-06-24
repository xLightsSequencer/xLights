#ifndef ButterfLYPANEL_H
#define ButterfLYPANEL_H

//(*Headers(ButterflyPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../ValueCurveButton.h"

class ButterflyPanel: public wxPanel
{
	public:

		ButterflyPanel(wxWindow* parent);
		virtual ~ButterflyPanel();

		//(*Declarations(ButterflyPanel)
		ValueCurveButton* BitmapButton_Butterfly_Skip;
		ValueCurveButton* BitmapButton_Butterfly_Speed;
		ValueCurveButton* BitmapButton_Butterfly_Chunks;
		//*)

	protected:

		//(*Identifiers(ButterflyPanel)
		static const long ID_CHOICE_Butterfly_Colors;
		static const long ID_BITMAPBUTTON_CHOICE_Butterfly_Color;
		static const long ID_SLIDER_Butterfly_Style;
		static const long IDD_TEXTCTRL_Butterfly_Style;
		static const long ID_BITMAPBUTTON_SLIDER_Butterfly_Style;
		static const long ID_SLIDER_Butterfly_Chunks;
		static const long ID_VALUECURVE_Butterfly_Chunks;
		static const long IDD_TEXTCTRL_Butterfly_Chunks;
		static const long ID_BITMAPBUTTON_SLIDER_Butterfly_Chunks;
		static const long ID_SLIDER_Butterfly_Skip;
		static const long ID_VALUECURVE_Butterfly_Skip;
		static const long IDD_TEXTCTRL_Butterfly_Skip;
		static const long ID_BITMAPBUTTON15;
		static const long ID_SLIDER_Butterfly_Speed;
		static const long ID_VALUECURVE_Butterfly_Speed;
		static const long IDD_TEXTCTRL_Butterfly_Speed;
		static const long ID_BITMAPBUTTON_SLIDER_Butterfly_Skip;
		static const long ID_CHOICE_Butterfly_Direction;
		static const long ID_BITMAPBUTTON9;
		//*)

	public:

		//(*Handlers(ButterflyPanel)
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
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
