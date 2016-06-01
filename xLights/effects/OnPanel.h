#ifndef ONPANEL_H
#define ONPANEL_H


//(*Headers(OnPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxCheckBox;
//*)


class OnPanel: public wxPanel
{
	public:

		OnPanel(wxWindow* parent);
		virtual ~OnPanel();

		//(*Declarations(OnPanel)
		wxSlider* SliderCycles;
		wxCheckBox* CheckBoxShimmer;
		wxTextCtrl* TextCtrlEnd;
		wxTextCtrl* TextCtrlCycles;
		wxTextCtrl* TextCtrlStart;
		wxSlider* SliderStart;
		wxSlider* SliderEnd;
		//*)

	protected:

		//(*Identifiers(OnPanel)
		static const long IDD_SLIDER_Eff_On_Start;
		static const long ID_TEXTCTRL_Eff_On_Start;
		static const long IDD_SLIDER_Eff_On_End;
		static const long ID_TEXTCTRL_Eff_On_End;
		static const long IDD_SLIDER_On_Cycles;
		static const long ID_TEXTCTRL_On_Cycles;
		static const long ID_CHECKBOX_On_Shimmer;
		//*)

	private:

		//(*Handlers(OnPanel)
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
