#ifndef SPIRALSPANEL_H
#define SPIRALSPANEL_H

//(*Headers(SpiralsPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
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
		BulkEditCheckBox* CheckBox_Spirals_3D;
		BulkEditCheckBox* CheckBox_Spirals_Blend;
		BulkEditCheckBox* CheckBox_Spirals_Grow;
		BulkEditCheckBox* CheckBox_Spirlas_Shrink;
		BulkEditSlider* Slider_Spirals_Count;
		BulkEditSlider* Slider_Spirals_Thickness;
		BulkEditSliderF1* Slider_Spirals_Movement;
		BulkEditSliderF1* Slider_Spirals_Rotation;
		BulkEditValueCurveButton* BitmapButton_VCSpiralsCount;
		BulkEditValueCurveButton* BitmapButton_VCSpirals_Movement;
		BulkEditValueCurveButton* BitmapButton_VCSpirals_Rotation;
		BulkEditValueCurveButton* BitmapButton_VCSpirals_Thickness;
		wxStaticText* StaticText34;
		wxStaticText* StaticText36;
		wxStaticText* StaticText38;
		wxStaticText* StaticText40;
		xlLockButton* BitmapButton_Spirals3D;
		xlLockButton* BitmapButton_SpiralsBlend;
		xlLockButton* BitmapButton_SpiralsCount;
		xlLockButton* BitmapButton_SpiralsDirection;
		xlLockButton* BitmapButton_SpiralsGrow;
		xlLockButton* BitmapButton_SpiralsRotation;
		xlLockButton* BitmapButton_SpiralsShrink;
		xlLockButton* BitmapButton_SpiralsThickness;
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
		static const long ID_BITMAPBUTTON_SLIDER_Spirals_Movement;
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
