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

#include "../BulkEditControls.h"

class MarqueePanel: public wxPanel
{
	public:

		MarqueePanel(wxWindow* parent);
		virtual ~MarqueePanel();

		//(*Declarations(MarqueePanel)
		BulkEditSlider* Slider_Marquee_Start;
		BulkEditCheckBox* CheckBox_Marquee_WrapX;
		BulkEditCheckBox* CheckBox_Marquee_PixelOffsets;
		wxStaticText* StaticText200;
		BulkEditSlider* Slider_MarqueeYC;
		wxBitmapButton* BitmapButton_Marquee_ScaleY;
		BulkEditSlider* Slider_Marquee_ScaleY;
		wxStaticText* StaticText195;
		BulkEditSlider* Slider_Marquee_Stagger;
		BulkEditSlider* Slider_Marquee_Thickness;
		BulkEditSlider* Slider_Marquee_Speed;
		wxPanel* Panel_Marquee_Settings;
		wxStaticText* StaticText62;
		BulkEditSlider* Slider_Marquee_ScaleX;
		wxBitmapButton* BitmapButton_Marquee_Start;
		wxStaticText* StaticText244;
		wxStaticText* StaticText194;
		BulkEditCheckBox* CheckBox_Marquee_Reverse;
		wxNotebook* Notebook_Marquee;
		BulkEditSlider* Slider_MarqueeXC;
		BulkEditSlider* Slider_Marquee_Band_Size;
		wxStaticLine* StaticLine1;
		wxStaticText* StaticText116;
		wxStaticText* StaticText196;
		wxStaticText* StaticText201;
		wxBitmapButton* BitmapButton_Marquee_ScaleX;
		wxBitmapButton* BitmapButton_Marquee_Stagger;
		wxStaticText* StaticText198;
		wxBitmapButton* BitmapButton_Marquee_Speed;
		wxPanel* Panel_Marquee_Position;
		wxStaticText* StaticText199;
		BulkEditSlider* Slider_Marquee_Skip_Size;
		wxBitmapButton* BitmapButton_Marquee_Thickness;
		wxBitmapButton* BitmapButton_Marquee_Skip_Size;
		//*)

	protected:

		//(*Identifiers(MarqueePanel)
		static const long ID_STATICTEXT_Marquee_Band_Size;
		static const long ID_SLIDER_Marquee_Band_Size;
		static const long IDD_TEXTCTRL_Marquee_Band_Size;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_Band_Size;
		static const long ID_STATICTEXT_Marquee_Skip_Size;
		static const long ID_SLIDER_Marquee_Skip_Size;
		static const long IDD_TEXTCTRL_Marquee_Skip_Size;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_Skip_Size;
		static const long ID_STATICTEXT_Marquee_Thickness;
		static const long ID_SLIDER_Marquee_Thickness;
		static const long IDD_TEXTCTRL_Marquee_Thickness;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_Thickness;
		static const long ID_STATICTEXT_Marquee_Stagger;
		static const long ID_SLIDER_Marquee_Stagger;
		static const long IDD_TEXTCTRL_Marquee_Stagger;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_Stagger;
		static const long ID_STATICTEXT_Marquee_Speed;
		static const long ID_SLIDER_Marquee_Speed;
		static const long IDD_TEXTCTRL_Marquee_Speed;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_Speed;
		static const long ID_STATICTEXT_Marquee_Start;
		static const long ID_SLIDER_Marquee_Start;
		static const long IDD_TEXTCTRL_Marquee_Start;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_Start;
		static const long ID_CHECKBOX_Marquee_Reverse;
		static const long ID_PANEL_Marquee_Settings;
		static const long ID_STATICTEXT_Marquee_ScaleX;
		static const long ID_SLIDER_Marquee_ScaleX;
		static const long IDD_TEXTCTRL_Marquee_ScaleX;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_ScaleX;
		static const long ID_STATICTEXT_Marquee_ScaleY;
		static const long ID_SLIDER_Marquee_ScaleY;
		static const long IDD_TEXTCTRL_Marquee_ScaleY;
		static const long ID_BITMAPBUTTON_SLIDER_Marquee_ScaleY;
		static const long ID_STATICLINE1;
		static const long ID_CHECKBOX_Marquee_PixelOffsets;
		static const long ID_STATICTEXT_MarqueeXC;
		static const long ID_SLIDER_MarqueeXC;
		static const long ID_CHECKBOX_Marquee_WrapX;
		static const long IDD_TEXTCTRL_MarqueeXC;
		static const long ID_STATICTEXT_MarqueeYC;
		static const long IDD_TEXTCTRL_MarqueeYC;
		static const long ID_SLIDER_MarqueeYC;
		static const long ID_PANEL_Marquee_Position;
		static const long ID_NOTEBOOK_Marquee;
		//*)

	public:

		//(*Handlers(MarqueePanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
