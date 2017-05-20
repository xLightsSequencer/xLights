#ifndef SPIRALSPANEL_H
#define SPIRALSPANEL_H

//(*Headers(SpiralsPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

#include "../ValueCurveButton.h"

#define SPIRALS_COUNT_MIN 1
#define SPIRALS_COUNT_MAX 5

#define SPIRALS_MOVEMENT_MIN -200
#define SPIRALS_MOVEMENT_MAX 200

#define SPIRALS_ROTATION_MIN -300
#define SPIRALS_ROTATION_MAX 300

#define SPIRALS_THICKNESS_MIN 0
#define SPIRALS_THICKNESS_MAX 100

class SpiralsPanel: public wxPanel
{
	public:

		SpiralsPanel(wxWindow* parent);
		virtual ~SpiralsPanel();

		//(*Declarations(SpiralsPanel)
		wxCheckBox* CheckBox_Spirals_Blend;
		wxCheckBox* CheckBox_Spirals_3D;
		ValueCurveButton* BitmapButton_VCSpirals_Movement;
		wxSlider* Slider_Spirals_Count;
		ValueCurveButton* BitmapButton_VCSpiralsCount;
		wxStaticText* StaticText40;
		wxBitmapButton* BitmapButton_Spirals3D;
		ValueCurveButton* BitmapButton_VCSpirals_Thickness;
		wxStaticText* StaticText38;
		wxBitmapButton* BitmapButton_SpiralsShrink;
		wxSlider* Slider_Spirals_Movement;
		wxBitmapButton* BitmapButton_SpiralsGrow;
		wxStaticText* StaticText34;
		wxBitmapButton* BitmapButton_SpiralsBlend;
		wxBitmapButton* BitmapButton_SpiralsThickness;
		wxSlider* Slider_Spirals_Thickness;
		wxCheckBox* CheckBox_Spirals_Grow;
		ValueCurveButton* BitmapButton_VCSpirals_Rotation;
		wxBitmapButton* BitmapButton_SpiralsDirection;
		wxBitmapButton* BitmapButton_SpiralsCount;
		wxBitmapButton* BitmapButton_SpiralsRotation;
		wxCheckBox* CheckBox_Spirlas_Shrink;
		wxSlider* Slider_Spirals_Rotation;
		//*)

	protected:

		//(*Identifiers(SpiralsPanel)
		static const long ID_STATICTEXT38;
		static const long ID_SLIDER_Spirals_Count;
		static const long ID_VALUECURVE_Spirals_Count;
		static const long IDD_TEXTCTRL_Spirals_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Spirals_Count;
		static const long ID_SLIDER_Spirals_Rotation;
		static const long ID_VALUECURVE_Spirals_Rotation;
		static const long IDD_TEXTCTRL_Spirals_Rotation;
		static const long ID_BITMAPBUTTON_SLIDER_Spirals_Rotation;
		static const long ID_STATICTEXT42;
		static const long ID_SLIDER_Spirals_Thickness;
		static const long ID_VALUECURVE_Spirals_Thickness;
		static const long IDD_TEXTCTRL_Spirals_Thickness;
		static const long ID_BITMAPBUTTON_SLIDER_Spirals_Thickness;
		static const long ID_STATICTEXT44;
		static const long IDD_SLIDER_Spirals_Movement;
		static const long ID_VALUECURVE_Spirals_Movement;
		static const long ID_TEXTCTRL_Spirals_Movement;
		static const long ID_BITMAPBUTTON_SLIDER_Spirals_Direction;
		static const long ID_CHECKBOX_Spirals_Blend;
		static const long ID_BITMAPBUTTON_CHECKBOX_Spirals_Blend;
		static const long ID_CHECKBOX_Spirals_3D;
		static const long ID_BITMAPBUTTON_CHECKBOX_Spirals_3D;
		static const long ID_CHECKBOX_Spirals_Grow;
		static const long ID_BITMAPBUTTON_CHECKBOX_Spirals_Grow;
		static const long ID_CHECKBOX_Spirals_Shrink;
		static const long ID_BITMAPBUTTON_CHECKBOX_Spirals_Shrink;
		//*)

	public:

		//(*Handlers(SpiralsPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
		void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        void UpdateLinkedSliderFloat2(wxCommandEvent& event);
        void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
