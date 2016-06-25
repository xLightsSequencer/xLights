#ifndef DMXPANEL_H
#define DMXPANEL_H

//(*Headers(DMXPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxNotebookEvent;
class wxNotebook;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../ValueCurveButton.h"

class DMXPanel: public wxPanel
{
	public:

		DMXPanel(wxWindow* parent);
		virtual ~DMXPanel();

		//(*Declarations(DMXPanel)
		ValueCurveButton* ValueCurve_DMX16;
		wxNotebook* Notebook7;
		ValueCurveButton* ValueCurve_DMX18;
		ValueCurveButton* ValueCurve_DMX7;
		ValueCurveButton* ValueCurve_DMX17;
		ValueCurveButton* ValueCurve_DMX12;
		ValueCurveButton* ValueCurve_DMX9;
		ValueCurveButton* ValueCurve_DMX6;
		ValueCurveButton* ValueCurve_DMX14;
		wxChoice* Choice_Num_Dmx_Channels;
		ValueCurveButton* ValueCurve_DMX13;
		ValueCurveButton* ValueCurve_DMX4;
		wxPanel* Panel18;
		ValueCurveButton* ValueCurve_DMX2;
		ValueCurveButton* ValueCurve_DMX1;
		ValueCurveButton* ValueCurve_DMX3;
		ValueCurveButton* ValueCurve_DMX15;
		ValueCurveButton* ValueCurve_DMX10;
		ValueCurveButton* ValueCurve_DMX8;
		ValueCurveButton* ValueCurve_DMX11;
		ValueCurveButton* ValueCurve_DMX5;
		wxPanel* Panel19;
		//*)

	protected:

		//(*Identifiers(DMXPanel)
		static const long ID_SLIDER_DMX1;
		static const long ID_VALUECURVE_DMX1;
		static const long IDD_TEXTCTRL_DMX1;
		static const long ID_SLIDER_DMX2;
		static const long ID_VALUECURVE_DMX2;
		static const long IDD_TEXTCTRL_DMX2;
		static const long ID_SLIDER_DMX3;
		static const long ID_VALUECURVE_DMX3;
		static const long IDD_TEXTCTRL_DMX3;
		static const long ID_SLIDER_DMX4;
		static const long ID_VALUECURVE_DMX4;
		static const long IDD_TEXTCTRL_DMX4;
		static const long ID_SLIDER_DMX5;
		static const long ID_VALUECURVE_DMX5;
		static const long IDD_TEXTCTRL_DMX5;
		static const long ID_SLIDER_DMX6;
		static const long ID_VALUECURVE_DMX6;
		static const long IDD_TEXTCTRL_DMX6;
		static const long ID_SLIDER_DMX7;
		static const long ID_VALUECURVE_DMX7;
		static const long IDD_TEXTCTRL_DMX7;
		static const long ID_SLIDER_DMX8;
		static const long ID_VALUECURVE_DMX8;
		static const long IDD_TEXTCTRL_DMX8;
		static const long ID_SLIDER_DMX9;
		static const long ID_VALUECURVE_DMX9;
		static const long IDD_TEXTCTRL_DMX9;
		static const long ID_CHOICE_Num_Dmx_Channels;
		static const long ID_PANEL6;
		static const long ID_SLIDER_DMX10;
		static const long ID_VALUECURVE_DMX10;
		static const long IDD_TEXTCTRL_DMX10;
		static const long ID_SLIDER_DMX11;
		static const long ID_VALUECURVE_DMX11;
		static const long IDD_TEXTCTRL_DMX11;
		static const long ID_SLIDER_DMX12;
		static const long ID_VALUECURVE_DMX12;
		static const long IDD_TEXTCTRL_DMX12;
		static const long ID_SLIDER_DMX13;
		static const long ID_VALUECURVE_DMX13;
		static const long IDD_TEXTCTRL_DMX13;
		static const long ID_SLIDER_DMX14;
		static const long ID_VALUECURVE_DMX14;
		static const long IDD_TEXTCTRL_DMX14;
		static const long ID_SLIDER_DMX15;
		static const long ID_VALUECURVE_DMX15;
		static const long IDD_TEXTCTRL_DMX15;
		static const long ID_SLIDER_DMX16;
		static const long ID_VALUECURVE_DMX16;
		static const long IDD_TEXTCTRL_DMX16;
		static const long ID_SLIDER_DMX17;
		static const long ID_VALUECURVE_DMX17;
		static const long IDD_TEXTCTRL_DMX17;
		static const long ID_SLIDER_DMX18;
		static const long ID_VALUECURVE_DMX18;
		static const long IDD_TEXTCTRL_DMX18;
		static const long ID_PANEL28;
		static const long ID_NOTEBOOK1;
		//*)

	public:

		//(*Handlers(DMXPanel)
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
