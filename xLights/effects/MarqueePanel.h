#ifndef MARQUEEPANEL_H
#define MARQUEEPANEL_H

//(*Headers(MarqueePanel)
#include <wx/panel.h>
class wxGridBagSizer;
class wxCheckBox;
class wxTextCtrl;
class wxNotebookEvent;
class wxStaticLine;
class wxNotebook;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

class MarqueePanel: public wxPanel
{
	public:

		MarqueePanel(wxWindow* parent);
		virtual ~MarqueePanel();

		//(*Declarations(MarqueePanel)
		wxBitmapButton* BitmapButton_Marquee_ScaleY;
		wxPanel* Panel_Marquee_Settings;
		wxBitmapButton* BitmapButton_Marquee_Start;
		wxNotebook* Notebook_Marquee;
		wxStaticLine* StaticLine1;
		wxBitmapButton* BitmapButton_Marquee_ScaleX;
		wxBitmapButton* BitmapButton_Marquee_Stagger;
		wxBitmapButton* BitmapButton_Marquee_Speed;
		wxPanel* Panel_Marquee_Position;
		wxBitmapButton* BitmapButton_Marquee_Thickness;
		wxBitmapButton* BitmapButton_Marquee_Skip_Size;
		//*)

	protected:

		//(*Identifiers(MarqueePanel)
		static const long ID_SLIDER_Marquee_Band_Size;
		static const long IDD_TEXTCTRL_Marquee_Band_Size;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_Band_Size;
		static const long ID_SLIDER_Marquee_Skip_Size;
		static const long IDD_TEXTCTRL_Marquee_Skip_Size;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_Skip_Size;
		static const long ID_SLIDER_Marquee_Thickness;
		static const long IDD_TEXTCTRL_Marquee_Thickness;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_Thickness;
		static const long ID_SLIDER_Marquee_Stagger;
		static const long IDD_TEXTCTRL_Marquee_Stagger;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_Stagger;
		static const long ID_SLIDER_Marquee_Speed;
		static const long IDD_TEXTCTRL_Marquee_Speed;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_Speed;
		static const long ID_SLIDER_Marquee_Start;
		static const long IDD_TEXTCTRL_Marquee_Start;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_Start;
		static const long ID_CHECKBOX_Marquee_Reverse;
		static const long ID_PANEL_Marquee_Settings;
		static const long ID_SLIDER_Marquee_ScaleX;
		static const long IDD_TEXTCTRL_Marquee_ScaleX;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_ScaleX;
		static const long ID_SLIDER_Marquee_ScaleY;
		static const long IDD_TEXTCTRL_Marquee_ScaleY;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_ScaleY;
		static const long ID_STATICLINE1;
		static const long ID_CHECKBOX_Marquee_PixelOffsets;
		static const long ID_SLIDER_MarqueeXC;
		static const long ID_CHECKBOX_Marquee_WrapX;
		static const long IDD_TEXTCTRL_MarqueeXC;
		static const long IDD_TEXTCTRL_MarqueeYC;
		static const long ID_SLIDER_MarqueeYC;
		static const long ID_PANEL_Marquee_Position;
		static const long ID_NOTEBOOK_Marquee;
		//*)

	public:

		//(*Handlers(MarqueePanel)
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
