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
//*)

#include "../ValueCurveButton.h"

class DMXPanel: public wxPanel
{
	public:

		DMXPanel(wxWindow* parent);
		virtual ~DMXPanel();

		//(*Declarations(DMXPanel)
		wxSlider* Slider_DMX16;
		ValueCurveButton* ValueCurve_DMX16;
		wxSlider* Slider_DMX3;
		wxNotebook* Notebook7;
		wxFlexGridSizer* FlexGridSizer_Main;
		wxSlider* Slider_DMX6;
		ValueCurveButton* ValueCurve_DMX18;
		wxFlexGridSizer* FlexGridSizer_Panel1;
		wxSlider* Slider_DMX14;
		wxSlider* Slider_DMX2;
		wxSlider* Slider_DMX4;
		ValueCurveButton* ValueCurve_DMX7;
		wxSlider* Slider_DMX15;
		wxSlider* Slider_DMX18;
		ValueCurveButton* ValueCurve_DMX17;
		ValueCurveButton* ValueCurve_DMX12;
		wxSlider* Slider_DMX1;
		wxPanel* ChannelPanel1;
		ValueCurveButton* ValueCurve_DMX9;
		ValueCurveButton* ValueCurve_DMX6;
		ValueCurveButton* ValueCurve_DMX14;
		ValueCurveButton* ValueCurve_DMX13;
		wxPanel* ChannelPanel2;
		wxSlider* Slider_DMX11;
		wxSlider* Slider_DMX7;
		ValueCurveButton* ValueCurve_DMX4;
		ValueCurveButton* ValueCurve_DMX2;
		wxFlexGridSizer* FlexGridSizer_Panel2;
		wxSlider* Slider_DMX13;
		wxSlider* Slider_DMX9;
		wxSlider* Slider_DMX17;
		ValueCurveButton* ValueCurve_DMX1;
		ValueCurveButton* ValueCurve_DMX3;
		wxSlider* Slider_DMX8;
		ValueCurveButton* ValueCurve_DMX15;
		ValueCurveButton* ValueCurve_DMX10;
		ValueCurveButton* ValueCurve_DMX8;
		ValueCurveButton* ValueCurve_DMX11;
		ValueCurveButton* ValueCurve_DMX5;
		wxSlider* Slider_DMX10;
		wxSlider* Slider_DMX5;
		wxSlider* Slider_DMX12;
		//*)

	protected:

		//(*Identifiers(DMXPanel)
		static const long ID_LABEL_DMX1;
		static const long ID_SLIDER_DMX1;
		static const long ID_VALUECURVE_DMX1;
		static const long IDD_TEXTCTRL_DMX1;
		static const long ID_LABEL_DMX2;
		static const long ID_SLIDER_DMX2;
		static const long ID_VALUECURVE_DMX2;
		static const long IDD_TEXTCTRL_DMX2;
		static const long ID_LABEL_DMX3;
		static const long ID_SLIDER_DMX3;
		static const long ID_VALUECURVE_DMX3;
		static const long IDD_TEXTCTRL_DMX3;
		static const long ID_LABEL_DMX4;
		static const long ID_SLIDER_DMX4;
		static const long ID_VALUECURVE_DMX4;
		static const long IDD_TEXTCTRL_DMX4;
		static const long ID_LABEL_DMX5;
		static const long ID_SLIDER_DMX5;
		static const long ID_VALUECURVE_DMX5;
		static const long IDD_TEXTCTRL_DMX5;
		static const long ID_LABEL_DMX6;
		static const long ID_SLIDER_DMX6;
		static const long ID_VALUECURVE_DMX6;
		static const long IDD_TEXTCTRL_DMX6;
		static const long ID_LABEL_DMX7;
		static const long ID_SLIDER_DMX7;
		static const long ID_VALUECURVE_DMX7;
		static const long IDD_TEXTCTRL_DMX7;
		static const long ID_LABEL_DMX8;
		static const long ID_SLIDER_DMX8;
		static const long ID_VALUECURVE_DMX8;
		static const long IDD_TEXTCTRL_DMX8;
		static const long ID_LABEL_DMX9;
		static const long ID_SLIDER_DMX9;
		static const long ID_VALUECURVE_DMX9;
		static const long IDD_TEXTCTRL_DMX9;
		static const long ID_PANEL6;
		static const long ID_LABEL_DMX10;
		static const long ID_SLIDER_DMX10;
		static const long ID_VALUECURVE_DMX10;
		static const long IDD_TEXTCTRL_DMX10;
		static const long ID_LABEL_DMX11;
		static const long ID_SLIDER_DMX11;
		static const long ID_VALUECURVE_DMX11;
		static const long IDD_TEXTCTRL_DMX11;
		static const long ID_LABEL_DMX12;
		static const long ID_SLIDER_DMX12;
		static const long ID_VALUECURVE_DMX12;
		static const long IDD_TEXTCTRL_DMX12;
		static const long ID_LABEL_DMX13;
		static const long ID_SLIDER_DMX13;
		static const long ID_VALUECURVE_DMX13;
		static const long IDD_TEXTCTRL_DMX13;
		static const long ID_LABEL_DMX14;
		static const long ID_SLIDER_DMX14;
		static const long ID_VALUECURVE_DMX14;
		static const long IDD_TEXTCTRL_DMX14;
		static const long ID_LABEL_DMX15;
		static const long ID_SLIDER_DMX15;
		static const long ID_VALUECURVE_DMX15;
		static const long IDD_TEXTCTRL_DMX15;
		static const long ID_LABEL_DMX16;
		static const long ID_SLIDER_DMX16;
		static const long ID_VALUECURVE_DMX16;
		static const long IDD_TEXTCTRL_DMX16;
		static const long ID_LABEL_DMX17;
		static const long ID_SLIDER_DMX17;
		static const long ID_VALUECURVE_DMX17;
		static const long IDD_TEXTCTRL_DMX17;
		static const long ID_LABEL_DMX18;
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
