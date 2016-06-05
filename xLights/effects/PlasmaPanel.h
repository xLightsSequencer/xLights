#ifndef PLASMAPANEL_H
#define PLASMAPANEL_H

//(*Headers(PlasmaPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../ValueCurveButton.h"

class PlasmaPanel: public wxPanel
{
	public:

		PlasmaPanel(wxWindow* parent);
		virtual ~PlasmaPanel();

		//(*Declarations(PlasmaPanel)
		ValueCurveButton* BitmapButton_Plasma_SpeedVC;
		wxChoice* Choice_Plasma_Color;
		//*)

	protected:

		//(*Identifiers(PlasmaPanel)
		static const long ID_CHOICE_Plasma_Color;
		static const long ID_BITMAPBUTTON35;
		static const long ID_SLIDER_Plasma_Style;
		static const long IDD_TEXTCTRL_Plasma_Style;
		static const long ID_BITMAPBUTTON36;
		static const long ID_SLIDER_Plasma_Line_Density;
		static const long IDD_TEXTCTRL_Plasma_Line_Density;
		static const long ID_BITMAPBUTTON37;
		static const long ID_SLIDER_Plasma_Speed;
		static const long ID_VALUECURVE_Plasma_Speed;
		static const long IDD_TEXTCTRL_Plasma_Speed;
		static const long ID_BITMAPBUTTON39;
		//*)

	public:

		//(*Handlers(PlasmaPanel)
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
