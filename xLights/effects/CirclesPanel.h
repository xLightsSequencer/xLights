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

#include "../BulkEditControls.h"

class CirclesPanel: public wxPanel
{
	public:

		CirclesPanel(wxWindow* parent);
		virtual ~CirclesPanel();

		//(*Declarations(CirclesPanel)
		wxBitmapButton* BitmapButton2;
		BulkEditCheckBox* CheckBox_Circles_Bounce;
		wxBitmapButton* BitmapButton_CirclesBounce;
		wxStaticText* StaticText137;
		wxBitmapButton* BitmapButton_CirclesRadial3D;
		wxBitmapButton* BitmapButton_Circles_Linear_Fade;
		BulkEditCheckBox* CheckBox_Circles_Bubbles;
		BulkEditSlider* Slider_Circles_Count;
		wxBitmapButton* BitmapButton_CirclesSize;
		wxBitmapButton* BitmapButton_CirclesRadial;
		wxStaticText* StaticText136;
		BulkEditSlider* Slider_Circles_Speed;
		BulkEditCheckBox* CheckBox_Circles_Plasma;
		BulkEditSlider* Slider_Circles_Size;
		wxBitmapButton* BitmapButton_CirclesCount;
		BulkEditCheckBox* CheckBox_Circles_Radial_3D;
		BulkEditCheckBox* CheckBox_Circles_Radial;
		BulkEditValueCurveButton* BitmapButton_Circles_Speed;
		BulkEditValueCurveButton* BitmapButton_Circles_Count;
		wxBitmapButton* BitmapButton_CirclesPlasma;
		BulkEditCheckBox* CheckBox_Circles_Random_m;
		wxBitmapButton* BitmapButton_CirclesCollide;
		wxBitmapButton* BitmapButton_CirclesBubbles;
		BulkEditCheckBox* CheckBox_Circles_Collide;
		BulkEditCheckBox* CheckBox_Circles_Linear_Fade;
		wxBitmapButton* BitmapButton_RandomMotion;
		wxStaticText* StaticText31;
		BulkEditValueCurveButton* BitmapButton_Circles_Size;
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
		static const long ID_BITMAPBUTTON_SLIDER_Circles_Speed;
		static const long ID_CHECKBOX_Circles_Bounce;
		static const long ID_BITMAPBUTTON_CHECKBOX_Circles_Bounce;
		static const long ID_CHECKBOX_Circles_Radial;
		static const long ID_BITMAPBUTTON_CHECKBOX_Circles_Radial;
		static const long ID_CHECKBOX_Circles_Plasma;
		static const long ID_BITMAPBUTTON_CHECKBOX_Circles_Plasma;
		static const long ID_CHECKBOX_Circles_Radial_3D;
		static const long ID_BITMAPBUTTON_CHECKBOX_Circles_Radial_3D;
		static const long ID_CHECKBOX_Circles_Bubbles;
		static const long ID_BITMAPBUTTON_CHECKBOX_Circles_Bubbles;
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
