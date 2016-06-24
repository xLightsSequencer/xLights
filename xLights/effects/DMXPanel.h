#ifndef DMXPANEL_H
#define DMXPANEL_H

//(*Headers(DMXPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxNotebook;
class wxFlexGridSizer;
class wxSlider;
class wxNotebookEvent;
class wxStaticText;
class wxCheckBox;
//*)

class DMXPanel: public wxPanel
{
	public:

		DMXPanel(wxWindow* parent);
		virtual ~DMXPanel();

		//(*Declarations(DMXPanel)
		wxPanel* Panel21;
		wxCheckBox* CheckBox_Use_Dmx_Ramps;
		wxPanel* Panel20;
		wxPanel* Panel18;
		wxChoice* Choice_Num_Dmx_Channels;
		wxPanel* Panel19;
		wxNotebook* Notebook7;
		//*)

	protected:

		//(*Identifiers(DMXPanel)
		static const long ID_SLIDER_DMX1;
		static const long IDD_TEXTCTRL_DMX1;
		static const long ID_SLIDER_DMX2;
		static const long IDD_TEXTCTRL_DMX2;
		static const long ID_SLIDER_DMX3;
		static const long IDD_TEXTCTRL_DMX3;
		static const long ID_SLIDER_DMX4;
		static const long IDD_TEXTCTRL_DMX4;
		static const long ID_SLIDER_DMX5;
		static const long IDD_TEXTCTRL_DMX5;
		static const long ID_SLIDER_DMX6;
		static const long IDD_TEXTCTRL_DMX6;
		static const long ID_SLIDER_DMX7;
		static const long IDD_TEXTCTRL_DMX7;
		static const long ID_SLIDER_DMX8;
		static const long IDD_TEXTCTRL_DMX8;
		static const long ID_SLIDER_DMX9;
		static const long IDD_TEXTCTRL_DMX9;
		static const long ID_CHOICE_Num_Dmx_Channels;
		static const long ID_CHECKBOX_Use_Dmx_Ramps;
		static const long ID_PANEL6;
		static const long ID_SLIDER_DMX10;
		static const long IDD_TEXTCTRL_DMX10;
		static const long ID_SLIDER_DMX11;
		static const long IDD_TEXTCTRL_DMX11;
		static const long ID_SLIDER_DMX12;
		static const long IDD_TEXTCTRL_DMX12;
		static const long ID_SLIDER_DMX13;
		static const long IDD_TEXTCTRL_DMX13;
		static const long ID_SLIDER_DMX14;
		static const long IDD_TEXTCTRL_DMX14;
		static const long ID_SLIDER_DMX15;
		static const long IDD_TEXTCTRL_DMX15;
		static const long ID_PANEL28;
		static const long ID_SLIDER_DMX1_Ramp;
		static const long IDD_TEXTCTRL_DMX1_Ramp;
		static const long ID_SLIDER_DMX2_Ramp;
		static const long IDD_TEXTCTRL_DMX2_Ramp;
		static const long ID_SLIDER_DMX3_Ramp;
		static const long IDD_TEXTCTRL_DMX3_Ramp;
		static const long ID_SLIDER_DMX4_Ramp;
		static const long IDD_TEXTCTRL_DMX4_Ramp;
		static const long ID_SLIDER_DMX5_Ramp;
		static const long IDD_TEXTCTRL_DMX5_Ramp;
		static const long ID_SLIDER_DMX6_Ramp;
		static const long IDD_TEXTCTRL_DMX6_Ramp;
		static const long ID_SLIDER_DMX7_Ramp;
		static const long IDD_TEXTCTRL_DMX7_Ramp;
		static const long ID_SLIDER_DMX8_Ramp;
		static const long IDD_TEXTCTRL_DMX8_Ramp;
		static const long ID_SLIDER_DMX9_Ramp;
		static const long IDD_TEXTCTRL_DMX9_Ramp;
		static const long ID_PANEL48;
		static const long ID_SLIDER_DMX10_Ramp;
		static const long IDD_TEXTCTRL_DMX10_Ramp;
		static const long ID_SLIDER_DMX11_Ramp;
		static const long IDD_TEXTCTRL_DMX11_Ramp;
		static const long ID_SLIDER_DMX12_Ramp;
		static const long IDD_TEXTCTRL_DMX12_Ramp;
		static const long ID_SLIDER_DMX13_Ramp;
		static const long IDD_TEXTCTRL_DMX13_Ramp;
		static const long ID_SLIDER_DMX14_Ramp;
		static const long IDD_TEXTCTRL_DMX14_Ramp;
		static const long ID_SLIDER_DMX15_Ramp;
		static const long IDD_TEXTCTRL_DMX15_Ramp;
		static const long ID_PANEL49;
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
