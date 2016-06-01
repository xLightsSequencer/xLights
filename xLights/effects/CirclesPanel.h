#ifndef CIRCLESPANEL_H
#define CIRCLESPANEL_H

//(*Headers(CirclesPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
//*)

class CirclesPanel: public wxPanel
{
	public:

		CirclesPanel(wxWindow* parent);
		virtual ~CirclesPanel();

		//(*Declarations(CirclesPanel)
		wxBitmapButton* BitmapButton2;
		wxCheckBox* CheckBox_Circles_Radial;
		wxBitmapButton* BitmapButton_CirclesBounce;
		wxStaticText* StaticText137;
		wxBitmapButton* BitmapButton_CirclesRadial3D;
		wxBitmapButton* BitmapButton_Circles_Linear_Fade;
		wxBitmapButton* BitmapButton_CirclesSize;
		wxBitmapButton* BitmapButton_CirclesRadial;
		wxStaticText* StaticText136;
		wxCheckBox* CheckBox_Circles_Bubbles;
		wxCheckBox* CheckBox_Circles_Random_m;
		wxBitmapButton* BitmapButton_CirclesCount;
		wxCheckBox* CheckBox_Circles_Bounce;
		wxCheckBox* CheckBox_Circles_Linear_Fade;
		wxBitmapButton* BitmapButton_CirclesPlasma;
		wxBitmapButton* BitmapButton_CirclesCollide;
		wxBitmapButton* BitmapButton_CirclesBubbles;
		wxCheckBox* CheckBox_Circles_Radial_3D;
		wxCheckBox* CheckBox_Circles_Collide;
		wxBitmapButton* BitmapButton_RandomMotion;
		wxCheckBox* CheckBox_Circles_Plasma;
		//*)

	protected:

		//(*Identifiers(CirclesPanel)
		static const long ID_STATICTEXT137;
		static const long ID_SLIDER_Circles_Count;
		static const long IDD_TEXTCTRL_Circles_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Circles_Count;
		static const long ID_STATICTEXT138;
		static const long ID_SLIDER_Circles_Size;
		static const long IDD_TEXTCTRL_Circles_Size;
		static const long ID_BITMAPBUTTON_SLIDER_Circles_Size;
		static const long ID_SLIDER_Circles_Speed;
		static const long IDD_TEXTCTRL_Circles_Speed;
		static const long ID_BITMAPBUTTON21;
		static const long ID_CHECKBOX_Circles_Bounce;
		static const long ID_BITMAPBUTTON_CHECKBOX_Circles_Bounce;
		static const long ID_CHECKBOX_Circles_Radial;
		static const long ID_BITMAPBUTTON_CHECKBOX_Circles_Radial;
		static const long ID_CHECKBOX_Circles_Plasma;
		static const long ID_BITMAPBUTTON_CHECKBOX_Circles_Plasma;
		static const long ID_CHECKBOX_Circles_Radial_3D;
		static const long ID_BITMAPBUTTON_CHECKBOX_Circles_Radial_3D;
		static const long ID_CHECKBOX_Circles_Bubbles;
		static const long ID_BITMAPBUTTON_CirclesBubbles;
		static const long ID_CHECKBOX_Circles_Collide;
		static const long ID_BITMAPBUTTON_CHECKBOX_Circles_Collide;
		static const long ID_CHECKBOX_Circles_Random_m;
		static const long ID_BITMAPBUTTON_CHECKBOX_Circles_Random_m;
		static const long ID_CHECKBOX_Circles_Linear_Fade;
		static const long ID_BITMAPBUTTON_CHECKBOX_Circles_Linear_Fade;
		//*)

	public:

		//(*Handlers(CirclesPanel)
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
