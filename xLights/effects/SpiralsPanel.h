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

class SpiralsPanel: public wxPanel
{
	public:

		SpiralsPanel(wxWindow* parent);
		virtual ~SpiralsPanel();

		//(*Declarations(SpiralsPanel)
		wxCheckBox* CheckBox_Spirals_Blend;
		wxBitmapButton* BitmapButton_SpiralsRotation;
		wxBitmapButton* BitmapButton_SpiralsDirection;
		wxBitmapButton* BitmapButton_SpiralsGrow;
		wxBitmapButton* BitmapButton_SpiralsShrink;
		wxBitmapButton* BitmapButton_Spirals3D;
		wxSlider* Slider_Spirals_Thickness;
		wxBitmapButton* BitmapButton_SpiralsCount;
		wxStaticText* StaticText34;
		wxStaticText* StaticText38;
		wxCheckBox* CheckBox_Spirlas_Shrink;
		wxStaticText* StaticText40;
		wxCheckBox* CheckBox_Spirals_Grow;
		wxCheckBox* CheckBox_Spirals_3D;
		wxBitmapButton* BitmapButton_SpiralsThickness;
		wxSlider* Slider_Spirals_Count;
		wxBitmapButton* BitmapButton_SpiralsBlend;
		//*)

	protected:

		//(*Identifiers(SpiralsPanel)
		static const long ID_STATICTEXT38;
		static const long ID_SLIDER_Spirals_Count;
		static const long IDD_TEXTCTRL_Spirals_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Spirals_Count;
		static const long ID_SLIDER_Spirals_Rotation;
		static const long IDD_TEXTCTRL_Spirals_Rotation;
		static const long ID_BITMAPBUTTON_SLIDER_Spirals_Rotation;
		static const long ID_STATICTEXT42;
		static const long ID_SLIDER_Spirals_Thickness;
		static const long IDD_TEXTCTRL_Spirals_Thickness;
		static const long ID_BITMAPBUTTON_SLIDER_Spirals_Thickness;
		static const long ID_STATICTEXT44;
		static const long IDD_SLIDER_Spirals_Movement;
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

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
