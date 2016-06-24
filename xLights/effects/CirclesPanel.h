#ifndef CIRCLESPANEL_H
#define CIRCLESPANEL_H

//(*Headers(CirclesPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

#include "../ValueCurveButton.h"

class CirclesPanel: public wxPanel
{
	public:

		CirclesPanel(wxWindow* parent);
		virtual ~CirclesPanel();

		//(*Declarations(CirclesPanel)
		wxCheckBox* CheckBox_Circles_Radial_3D;
		ValueCurveButton* BitmapButton_Circles_Size;
		wxCheckBox* CheckBox_Circles_Linear_Fade;
		wxStaticText* StaticText137;
		wxBitmapButton* BitmapButton_CirclesSize;
		wxBitmapButton* BitmapButton_CirclesRadial;
		ValueCurveButton* BitmapButton_Circles_Speed;
		wxBitmapButton* BitmapButton2;
		wxBitmapButton* BitmapButton_CirclesCollide;
		wxBitmapButton* BitmapButton_RandomMotion;
		wxStaticText* StaticText136;
		wxCheckBox* CheckBox_Circles_Collide;
		wxCheckBox* CheckBox_Circles_Plasma;
		wxCheckBox* CheckBox_Circles_Bounce;
		wxBitmapButton* BitmapButton_CirclesBounce;
		wxBitmapButton* BitmapButton_CirclesBubbles;
		wxCheckBox* CheckBox_Circles_Radial;
		wxBitmapButton* BitmapButton_CirclesCount;
		wxBitmapButton* BitmapButton_Circles_Linear_Fade;
		wxBitmapButton* BitmapButton_CirclesRadial3D;
		ValueCurveButton* BitmapButton_Circles_Count;
		wxCheckBox* CheckBox_Circles_Random_m;
		wxBitmapButton* BitmapButton_CirclesPlasma;
		wxCheckBox* CheckBox_Circles_Bubbles;
		//*)

	protected:

		//(*Identifiers(CirclesPanel)
		static const long ID_STATICTEXT137;
		static const long ID_SLIDER_Circles_Count;
		static const long ID_VALUECURVE_Circles_Count;
		static const long IDD_TEXTCTRL_Circles_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Circles_Count;
		static const long ID_STATICTEXT138;
		static const long ID_SLIDER_Circles_Size;
		static const long ID_VALUECURVE_Circles_Size;
		static const long IDD_TEXTCTRL_Circles_Size;
		static const long ID_BITMAPBUTTON_SLIDER_Circles_Size;
		static const long ID_SLIDER_Circles_Speed;
		static const long ID_VALUECURVE_Circles_Speed;
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
		void OnCheckBox_Circles_BubblesClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
