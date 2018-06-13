#ifndef CIRCLESPANEL_H
#define CIRCLESPANEL_H

//(*Headers(CirclesPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class CirclesPanel: public wxPanel
{
	public:

		CirclesPanel(wxWindow* parent);
		virtual ~CirclesPanel();

		//(*Declarations(CirclesPanel)
		BulkEditCheckBox* CheckBox_Circles_Bounce;
		BulkEditCheckBox* CheckBox_Circles_Bubbles;
		BulkEditCheckBox* CheckBox_Circles_Collide;
		BulkEditCheckBox* CheckBox_Circles_Linear_Fade;
		BulkEditCheckBox* CheckBox_Circles_Plasma;
		BulkEditCheckBox* CheckBox_Circles_Radial;
		BulkEditCheckBox* CheckBox_Circles_Radial_3D;
		BulkEditCheckBox* CheckBox_Circles_Random_m;
		BulkEditSlider* Slider_Circles_Count;
		BulkEditSlider* Slider_Circles_Size;
		BulkEditSlider* Slider_Circles_Speed;
		BulkEditValueCurveButton* BitmapButton_Circles_Count;
		BulkEditValueCurveButton* BitmapButton_Circles_Size;
		BulkEditValueCurveButton* BitmapButton_Circles_Speed;
		wxStaticText* StaticText136;
		wxStaticText* StaticText137;
		wxStaticText* StaticText31;
		xlLockButton* BitmapButton2;
		xlLockButton* BitmapButton_CirclesBounce;
		xlLockButton* BitmapButton_CirclesBubbles;
		xlLockButton* BitmapButton_CirclesCollide;
		xlLockButton* BitmapButton_CirclesCount;
		xlLockButton* BitmapButton_CirclesPlasma;
		xlLockButton* BitmapButton_CirclesRadial3D;
		xlLockButton* BitmapButton_CirclesRadial;
		xlLockButton* BitmapButton_CirclesSize;
		xlLockButton* BitmapButton_Circles_Linear_Fade;
		xlLockButton* BitmapButton_RandomMotion;
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
