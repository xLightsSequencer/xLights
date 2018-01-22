#ifndef SPIRALSPANEL_H
#define SPIRALSPANEL_H

//(*Headers(SpiralsPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
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
		BulkEditSlider* Slider_Spirals_Thickness;
		BulkEditSliderF1* Slider_Spirals_Rotation;
		BulkEditValueCurveButton* BitmapButton_VCSpirals_Rotation;
		wxBitmapButton* BitmapButton_SpiralsRotation;
		BulkEditCheckBox* CheckBox_Spirals_Grow;
		BulkEditValueCurveButton* BitmapButton_VCSpiralsCount;
		wxBitmapButton* BitmapButton_SpiralsDirection;
		BulkEditValueCurveButton* BitmapButton_VCSpirals_Movement;
		wxBitmapButton* BitmapButton_SpiralsGrow;
		wxBitmapButton* BitmapButton_SpiralsShrink;
		BulkEditSlider* Slider_Spirals_Count;
		wxBitmapButton* BitmapButton_Spirals3D;
		BulkEditCheckBox* CheckBox_Spirals_3D;
		wxBitmapButton* BitmapButton_SpiralsCount;
		wxStaticText* StaticText34;
		wxStaticText* StaticText38;
		BulkEditSliderF1* Slider_Spirals_Movement;
		wxStaticText* StaticText40;
		BulkEditCheckBox* CheckBox_Spirlas_Shrink;
		BulkEditValueCurveButton* BitmapButton_VCSpirals_Thickness;
		wxStaticText* StaticText36;
		wxBitmapButton* BitmapButton_SpiralsThickness;
		BulkEditCheckBox* CheckBox_Spirals_Blend;
		wxBitmapButton* BitmapButton_SpiralsBlend;
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
