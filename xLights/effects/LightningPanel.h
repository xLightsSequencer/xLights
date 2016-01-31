#ifndef LIGHTNINGPANEL_H
#define LIGHTNINGPANEL_H

//(*Headers(LightningPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
//*)

class LightningPanel: public wxPanel
{
	public:

		LightningPanel(wxWindow* parent);
		virtual ~LightningPanel();

		//(*Declarations(LightningPanel)
		wxBitmapButton* BitmapButton20;
		wxTextCtrl* TextCtrl86;
		wxTextCtrl* TextCtrl88;
		wxTextCtrl* TextCtrl89;
		wxStaticText* StaticText205;
		wxStaticText* StaticText210;
		wxSlider* Lightning_BOTX;
		wxSlider* Slider_Lightning_TopY;
		wxStaticText* StaticText208;
		wxBitmapButton* BitmapButton26;
		wxBitmapButton* BitmapButton27;
		wxBitmapButton* BitmapButton25;
		wxBitmapButton* BitmapButton28;
		wxBitmapButton* BitmapButton22;
		wxStaticText* StaticText207;
		wxBitmapButton* BitmapButton23;
		wxTextCtrl* TextCtrl90;
		wxSlider* Slider_Lightning_BOTX;
		wxSlider* Lightning_TopX;
		wxStaticText* StaticText203;
		wxStaticText* StaticText209;
		wxCheckBox* ForkedLightning;
		wxStaticText* StaticText206;
		//*)

	protected:

		//(*Identifiers(LightningPanel)
		static const long ID_STATICTEXT25;
		static const long ID_SLIDER_Number_Bolts;
		static const long IDD_TEXTCTRL_Number_Bolts;
		static const long ID_BITMAPBUTTON38;
		static const long ID_STATICTEXT26;
		static const long ID_SLIDER_Number_Segments;
		static const long IDD_TEXTCTRL_Number_Segments;
		static const long ID_BITMAPBUTTON40;
		static const long ID_STATICTEXT60;
		static const long ID_CHECKBOX_ForkedLightning;
		static const long ID_BITMAPBUTTON41;
		static const long ID_STATICTEXT64;
		static const long ID_SLIDER_Lightning_TopX;
		static const long IDD_TEXTCTRL_Lightning_TopX;
		static const long ID_BITMAPBUTTON42;
		static const long ID_STATICTEXT67;
		static const long ID_SLIDER_Lightning_TopY;
		static const long IDD_TEXTCTRL_Lightning_TopY;
		static const long ID_BITMAPBUTTON43;
		static const long ID_STATICTEXT70;
		static const long ID_SLIDER_Lightning_BOTX;
		static const long IDD_TEXTCTRL_Lightning_BOTX;
		static const long ID_BITMAPBUTTON45;
		static const long ID_STATICTEXT73;
		static const long ID_SLIDER_Lightning_BOTY;
		static const long IDD_TEXTCTRL_Lightning_BOTY;
		static const long ID_BITMAPBUTTON52;
		//*)

	public:

		//(*Handlers(LightningPanel)
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
