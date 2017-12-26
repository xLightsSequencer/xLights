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

#include "../BulkEditControls.h"

class CirclesPanel: public wxPanel
{
	public:

		CirclesPanel(wxWindow* parent);
		virtual ~CirclesPanel();

		//(*Declarations(CirclesPanel)
		BulkEditValueCurveButton* BitmapButton_Circles_Size;
		BulkEditValueCurveButton* BitmapButton_Circles_Speed;
		BulkEditCheckBox* CheckBox_Circles_Bubbles;
		BulkEditCheckBox* CheckBox_Circles_Radial;
		wxStaticText* StaticText137;
		wxBitmapButton* BitmapButton_CirclesSize;
		BulkEditValueCurveButton* BitmapButton_Circles_Count;
		BulkEditCheckBox* CheckBox_Circles_Bounce;
		wxBitmapButton* BitmapButton_CirclesRadial;
		BulkEditSlider* Slider_Circles_Speed;
		wxBitmapButton* BitmapButton2;
		wxStaticText* StaticText31;
		BulkEditCheckBox* CheckBox_Circles_Radial_3D;
		wxBitmapButton* BitmapButton_CirclesCollide;
		wxBitmapButton* BitmapButton_RandomMotion;
		BulkEditCheckBox* CheckBox_Circles_Random_m;
		BulkEditSlider* Slider_Circles_Size;
		wxStaticText* StaticText136;
		BulkEditCheckBox* CheckBox_Circles_Collide;
		wxBitmapButton* BitmapButton_CirclesBounce;
		wxBitmapButton* BitmapButton_CirclesBubbles;
		wxBitmapButton* BitmapButton_CirclesCount;
		BulkEditSlider* Slider_Circles_Count;
		wxBitmapButton* BitmapButton_Circles_Linear_Fade;
		wxBitmapButton* BitmapButton_CirclesRadial3D;
		BulkEditCheckBox* CheckBox_Circles_Plasma;
		BulkEditCheckBox* CheckBox_Circles_Linear_Fade;
		wxBitmapButton* BitmapButton_CirclesPlasma;
		//*)

	protected:

		//(*Identifiers(CirclesPanel)
		static const long ID_STATICTEXT_Circles_Count;
		static const long ID_SLIDER_Circles_Count;
		static const long ID_VALUECURVE_Circles_Count;
		static const long IDD_TEXTCTRL_Circles_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Circles_Count;
		static const long ID_STATICTEXT_Circles_Size;
		static const long ID_SLIDER_Circles_Size;
		static const long ID_VALUECURVE_Circles_Size;
		static const long IDD_TEXTCTRL_Circles_Size;
		static const long ID_BITMAPBUTTON_SLIDER_Circles_Size;
		static const long ID_STATICTEXT_Circles_Speed;
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
		void OnLockButtonClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnCheckBox_Circles_BubblesClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
