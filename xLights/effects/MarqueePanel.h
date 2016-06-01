#ifndef MARQUEEPANEL_H
#define MARQUEEPANEL_H

//(*Headers(MarqueePanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxNotebook;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticLine;
class wxGridBagSizer;
class wxNotebookEvent;
class wxStaticText;
class wxCheckBox;
//*)

class MarqueePanel: public wxPanel
{
	public:

		MarqueePanel(wxWindow* parent);
		virtual ~MarqueePanel();

		//(*Declarations(MarqueePanel)
		wxBitmapButton* BitmapButton_Marquee_ScaleY;
		wxBitmapButton* BitmapButton_Marquee_Thickness;
		wxBitmapButton* BitmapButton_Marquee_Start;
		wxBitmapButton* BitmapButton_Marquee_Skip_Size;
		wxNotebook* Notebook_Marquee;
		wxBitmapButton* BitmapButton_Marquee_Speed;
		wxStaticLine* StaticLine1;
		wxPanel* Panel_Marquee_Settings;
		wxBitmapButton* BitmapButton_Marquee_ScaleX;
		wxBitmapButton* BitmapButton_Marquee_Stagger;
		wxPanel* Panel_Marquee_Position;
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

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
