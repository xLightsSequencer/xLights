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

#include "../BulkEditControls.h"

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
		BulkEditValueCurveButton* BitmapButton_VCSpirals_Movement;
		wxStaticText* StaticText40;
		BulkEditCheckBox* CheckBox_Spirals_3D;
		wxBitmapButton* BitmapButton_Spirals3D;
		wxStaticText* StaticText38;
		wxBitmapButton* BitmapButton_SpiralsShrink;
		BulkEditValueCurveButton* BitmapButton_VCSpirals_Rotation;
		BulkEditSlider* Slider_Spirals_Count;
		BulkEditCheckBox* CheckBox_Spirals_Blend;
		wxBitmapButton* BitmapButton_SpiralsGrow;
		BulkEditSliderF1* Slider_Spirals_Rotation;
		wxStaticText* StaticText34;
		wxBitmapButton* BitmapButton_SpiralsBlend;
		wxBitmapButton* BitmapButton_SpiralsThickness;
		BulkEditValueCurveButton* BitmapButton_VCSpiralsCount;
		BulkEditCheckBox* CheckBox_Spirals_Grow;
		BulkEditSlider* Slider_Spirals_Thickness;
		BulkEditCheckBox* CheckBox_Spirlas_Shrink;
		BulkEditValueCurveButton* BitmapButton_VCSpirals_Thickness;
		wxBitmapButton* BitmapButton_SpiralsDirection;
		wxBitmapButton* BitmapButton_SpiralsCount;
		BulkEditSliderF1* Slider_Spirals_Movement;
		wxBitmapButton* BitmapButton_SpiralsRotation;
		wxStaticText* StaticText36;
		//*)

	protected:

		//(*Identifiers(SpiralsPanel)
		static const long ID_STATICTEXT_Spirals_Count;
		static const long ID_SLIDER_Spirals_Count;
		static const long ID_VALUECURVE_Spirals_Count;
		static const long IDD_TEXTCTRL_Spirals_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Spirals_Count;
		static const long ID_STATICTEXT_Spirals_Rotation;
		static const long ID_SLIDER_Spirals_Rotation;
		static const long ID_VALUECURVE_Spirals_Rotation;
		static const long IDD_TEXTCTRL_Spirals_Rotation;
		static const long ID_BITMAPBUTTON_SLIDER_Spirals_Rotation;
		static const long ID_STATICTEXT_Spirals_Thickness;
		static const long ID_SLIDER_Spirals_Thickness;
		static const long ID_VALUECURVE_Spirals_Thickness;
		static const long IDD_TEXTCTRL_Spirals_Thickness;
		static const long ID_BITMAPBUTTON_SLIDER_Spirals_Thickness;
		static const long ID_STATICTEXT_Spirals_Movement;
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
		void OnLockButtonClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
