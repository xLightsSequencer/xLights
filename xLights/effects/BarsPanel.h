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

class BarsPanel: public wxPanel
{
	public:

		BarsPanel(wxWindow* parent);
		virtual ~BarsPanel();

		//(*Declarations(BarsPanel)
		wxChoice* Choice_Bars_Direction;
		wxSlider* Slider_Bars_BarCount;
		//*)

	protected:

		//(*Identifiers(BarsPanel)
		static const long ID_SLIDER_Bars_BarCount;
		static const long IDD_TEXTCTRL_Bars_BarCount;
		static const long ID_BITMAPBUTTON_SLIDER_Bars_BarCount;
		static const long IDD_SLIDER_Bars_Cycles;
		static const long ID_TEXTCTRL_Bars_Cycles;
		static const long ID_CHOICE_Bars_Direction;
		static const long ID_BITMAPBUTTON_CHOICE_Bars_Direction;
		static const long ID_CHECKBOX_Bars_Highlight;
		static const long ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight;
		static const long ID_CHECKBOX_Bars_3D;
		static const long ID_BITMAPBUTTON_CHECKBOX_Bars_3D;
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
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
