#ifndef RIPPLEPANEL_H
#define RIPPLEPANEL_H

//(*Headers(RipplePanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
//*)

class RipplePanel: public wxPanel
{
	public:

		RipplePanel(wxWindow* parent);
		virtual ~RipplePanel();

		//(*Declarations(RipplePanel)
		wxSlider* Slider_Ripple_Thickness;
		wxStaticText* StaticText70;
		wxBitmapButton* BitmapButton_Ripple_Movement;
		wxStaticText* StaticText69;
		wxBitmapButton* BitmapButton_Ripple_Object_To_Draw;
		wxBitmapButton* BitmapButton_RippleThickness;
		wxStaticText* StaticText72;
		wxChoice* Choice_Ripple_Object_To_Draw;
		wxCheckBox* CheckBox_Ripple3D;
		wxBitmapButton* BitmapButton_Ripple3D;
		wxChoice* Choice_Ripple_Movement;
		//*)

	protected:

		//(*Identifiers(RipplePanel)
		static const long ID_STATICTEXT71;
		static const long ID_CHOICE_Ripple_Object_To_Draw;
		static const long ID_BITMAPBUTTON20;
		static const long ID_STATICTEXT72;
		static const long ID_CHOICE_Ripple_Movement;
		static const long ID_BITMAPBUTTON13;
		static const long ID_STATICTEXT74;
		static const long ID_SLIDER_Ripple_Thickness;
		static const long IDD_TEXTCTRL_Ripple_Thickness;
		static const long ID_BITMAPBUTTON14;
		static const long IDD_SLIDER_Ripple_Cycles;
		static const long ID_TEXTCTRL_Ripple_Cycles;
		static const long ID_CHECKBOX_Ripple3D;
		static const long ID_BITMAPBUTTON17;
		//*)

	public:

		//(*Handlers(RipplePanel)
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
