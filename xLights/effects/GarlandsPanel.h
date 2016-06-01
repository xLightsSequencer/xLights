#ifndef GARLANDSPANEL_H
#define GARLANDSPANEL_H

//(*Headers(GarlandsPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
//*)

class GarlandsPanel: public wxPanel
{
	public:

		GarlandsPanel(wxWindow* parent);
		virtual ~GarlandsPanel();

		//(*Declarations(GarlandsPanel)
		wxStaticText* StaticText32;
		wxBitmapButton* BitmapButton_GarlandsType;
		wxStaticText* StaticText192;
		wxSlider* Slider_Garlands_Type;
		wxSlider* Slider_Garlands_Spacing;
		wxBitmapButton* BitmapButton_GarlandsSpacing;
		wxStaticText* StaticText33;
		//*)

	protected:

		//(*Identifiers(GarlandsPanel)
		static const long ID_STATICTEXT34;
		static const long ID_SLIDER_Garlands_Type;
		static const long IDD_TEXTCTRL_Garlands_Type;
		static const long ID_BITMAPBUTTON_SLIDER_Garlands_Type;
		static const long ID_STATICTEXT35;
		static const long ID_SLIDER_Garlands_Spacing;
		static const long IDD_TEXTCTRL_Garlands_Spacing;
		static const long ID_BITMAPBUTTON_SLIDER_Garlands_Spacing;
		static const long IDD_SLIDER_Garlands_Cycles;
		static const long ID_TEXTCTRL_Garlands_Cycles;
		static const long ID_STATICTEXT40;
		static const long ID_CHOICE_Garlands_Direction;
		//*)

	public:

		//(*Handlers(GarlandsPanel)
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
