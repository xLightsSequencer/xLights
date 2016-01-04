#ifndef PINWHEELPANEL_H
#define PINWHEELPANEL_H

//(*Headers(PinwheelPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxBoxSizer;
class wxChoice;
//*)

class PinwheelPanel: public wxPanel
{
	public:

		PinwheelPanel(wxWindow* parent);
		virtual ~PinwheelPanel();

		//(*Declarations(PinwheelPanel)
		wxChoice* Choice_Pinwheel_3D;
		wxBitmapButton* BitmapButton_Pinwheel_ArmSize;
		wxSlider* Slider_PinwheelYC;
		wxStaticText* StaticText74;
		wxStaticText* StaticText75;
		wxStaticText* StaticText76;
		wxStaticText* StaticText82;
		wxSlider* Slider_PinwheelXC;
		wxSlider* Slider_Pinwheel_Twist;
		wxStaticText* StaticText81;
		wxSlider* Slider_Pinwheel_ArmSize;
		wxBitmapButton* BitmapButton_PinwheelTwist;
		wxStaticText* StaticText84;
		wxSlider* Slider_Pinwheel_Arms;
		wxBitmapButton* BitmapButton_Pinwheel3D;
		wxBitmapButton* BitmapButton_PinwheelRotation;
		wxBitmapButton* BitmapButton7;
		wxBitmapButton* BitmapButton_PinwheelThickness;
		wxCheckBox* CheckBox_Pinwheel_Rotation;
		wxSlider* Slider_Pinwheel_Thickness;
		wxBitmapButton* BitmapButton_PinwheelNumberArms;
		wxStaticText* StaticText63;
		wxStaticText* StaticText77;
		//*)

	protected:

		//(*Identifiers(PinwheelPanel)
		static const long ID_STATICTEXT65;
		static const long ID_SLIDER_Pinwheel_Arms;
		static const long IDD_TEXTCTRL_Pinwheel_Arms;
		static const long ID_BITMAPBUTTON11;
		static const long ID_STATICTEXT98;
		static const long ID_SLIDER_Pinwheel_ArmSize;
		static const long IDD_TEXTCTRL_Pinwheel_ArmSize;
		static const long ID_BITMAPBUTTON16;
		static const long ID_STATICTEXT76;
		static const long ID_SLIDER_Pinwheel_Twist;
		static const long IDD_TEXTCTRL_Pinwheel_Twist;
		static const long ID_BITMAPBUTTON12;
		static const long ID_STATICTEXT77;
		static const long ID_SLIDER_Pinwheel_Thickness;
		static const long IDD_TEXTCTRL_Pinwheel_Thickness;
		static const long ID_BITMAPBUTTON18;
		static const long IDD_SLIDER_Pinwheel_Speed;
		static const long ID_TEXTCTRL_Pinwheel_Speed;
		static const long ID_BITMAPBUTTON26;
		static const long ID_STATICTEXT78;
		static const long ID_CHECKBOX_Pinwheel_Rotation;
		static const long ID_BITMAPBUTTON19;
		static const long ID_STATICTEXT82;
		static const long ID_CHOICE_Pinwheel_3D;
		static const long ID_BITMAPBUTTON_Pinwheel3D;
		static const long ID_STATICTEXT85;
		static const long ID_SLIDER_PinwheelXC;
		static const long IDD_TEXTCTRL_PinwheelXC;
		static const long ID_STATICTEXT83;
		static const long IDD_TEXTCTRL_PinwheelYC;
		static const long ID_SLIDER_PinwheelYC;
		//*)

	public:

		//(*Handlers(PinwheelPanel)
    void UpdateLinkedSliderFloat(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    void UpdateLinkedSlider360(wxCommandEvent& event);
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);

    void OnTextCtrl_Pinwheel_ArmsText(wxCommandEvent& event);
    void OnTextCtrl_Pinwheel_TwistText(wxCommandEvent& event);
    void OnTextCtrl_Pinwheel_ThicknessText(wxCommandEvent& event);
    void OnSlider_Pinwheel_ArmsCmdScroll(wxScrollEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
