#ifndef SINGLESTRANDPANEL_H
#define SINGLESTRANDPANEL_H

//(*Headers(SingleStrandPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxNotebook;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxNotebookEvent;
class wxStaticText;
class wxCheckBox;
//*)

#include "../BulkEditControls.h"

class SingleStrandPanel: public wxPanel
{
	public:

		SingleStrandPanel(wxWindow* parent);
		virtual ~SingleStrandPanel();

		//(*Declarations(SingleStrandPanel)
		wxBitmapButton* BitmapButton_Chase_3dFade1;
		wxBitmapButton* BitmapButton_ChaseRotations;
		wxBitmapButton* BitmapButton_Skips_Direction;
		wxStaticText* StaticText60;
		wxPanel* Panel1;
		BulkEditCheckBox* CheckBox_Chase_Group_All;
		BulkEditSlider* Slider_Skips_SkipSize;
		wxBitmapButton* BitmapButton_Color_Mix1;
		wxStaticText* StaticText101;
		BulkEditValueCurveButton* BitmapButton_Chase_Rotations;
		wxBitmapButton* BitmapButton_SkipsAdvance;
		wxStaticText* StaticText179;
		BulkEditValueCurveButton* BitmapButton_Number_ChasesVC;
		wxStaticText* StaticText109;
		wxStaticText* StaticText10;
		wxPanel* Panel2;
		wxBitmapButton* BitmapButton_Chase_Type1;
		wxStaticText* StaticText105;
		BulkEditChoice* Choice_Skips_Direction;
		wxNotebook* SingleStrandEffectType;
		BulkEditSlider* Slider_Skips_BandSize;
		wxBitmapButton* BitmapButton_Chase_Group_All;
		BulkEditSliderF1* Slider_Chase_Rotations;
		wxBitmapButton* BitmapButton_SingleStrand_Colors;
		wxStaticText* StaticText106;
		wxStaticText* StaticText61;
		BulkEditCheckBox* CheckBox_Chase_3dFade1;
		BulkEditSlider* Slider_Number_Chases;
		wxBitmapButton* BitmapButton_Skips_BandSize;
		BulkEditSlider* Slider_Color_Mix1;
		BulkEditChoice* Choice_SingleStrand_Colors;
		BulkEditSlider* Slider_Skips_Advance;
		BulkEditValueCurveButton* BitmapButton_Color_Mix1VC;
		BulkEditSlider* Slider_Skips_StartPos;
		wxStaticText* StaticText9;
		wxStaticText* StaticText11;
		wxBitmapButton* BitmapButton_Skips_StartingPosition;
		wxBitmapButton* BitmapButton_Skips_SkipSize;
		wxBitmapButton* BitmapButton_Number_Chases;
		BulkEditChoice* Choice_Chase_Type1;
		//*)

	protected:

		//(*Identifiers(SingleStrandPanel)
		static const long ID_STATICTEXT_SingleStrand_Colors;
		static const long ID_CHOICE_SingleStrand_Colors;
		static const long ID_BITMAPBUTTON_CHOICE_SingleStrand_Colors;
		static const long ID_STATICTEXT_Number_Chases;
		static const long ID_SLIDER_Number_Chases;
		static const long ID_VALUECURVE_Number_Chases;
		static const long IDD_TEXTCTRL_Number_Chases;
		static const long ID_BITMAPBUTTON_SLIDER_Number_Chases;
		static const long ID_STATICTEXT_Color_Mix1;
		static const long ID_SLIDER_Color_Mix1;
		static const long ID_VALUECURVE_Color_Mix1;
		static const long IDD_TEXTCTRL_Color_Mix1;
		static const long ID_BITMAPBUTTON_SLIDER_Color_Mix1;
		static const long ID_STATICTEXT_Chase_Rotations;
		static const long IDD_SLIDER_Chase_Rotations;
		static const long ID_VALUECURVE_Chase_Rotations;
		static const long ID_TEXTCTRL_Chase_Rotations;
		static const long ID_BITMAPBUTTON_SLIDER_Chase_Rotations;
		static const long ID_STATICTEXT_Chase_Type1;
		static const long ID_CHOICE_Chase_Type1;
		static const long ID_BITMAPBUTTON_CHOICE_Chase_Type1;
		static const long ID_CHECKBOX_Chase_3dFade1;
		static const long ID_BITMAPBUTTON_CHECKBOX_Chase_3dFade1;
		static const long ID_CHECKBOX_Chase_Group_All;
		static const long ID_BITMAPBUTTON_CHECKBOX_Chase_Group_All;
		static const long ID_PANEL3;
		static const long ID_STATICTEXT_Skips_BandSize;
		static const long ID_SLIDER_Skips_BandSize;
		static const long IDD_TEXTCTRL_Skips_BandSize;
		static const long ID_BITMAPBUTTON_SLIDER_Skips_BandSize;
		static const long ID_STATICTEXT_Skips_SkipSize;
		static const long ID_SLIDER_Skips_SkipSize;
		static const long IDD_TEXTCTRL_Skips_SkipSize;
		static const long ID_BITMAPBUTTON_SLIDER_Skips_SkipSize;
		static const long ID_STATICTEXT_Skips_StartPos;
		static const long ID_SLIDER_Skips_StartPos;
		static const long IDD_TEXTCTRL_Skips_StartPos;
		static const long ID_BITMAPBUTTON_SLIDER_Skips_StartPos;
		static const long IS_STATICTEXT_Skips_Advance;
		static const long ID_SLIDER_Skips_Advance;
		static const long IDD_TEXTCTRL_Skips_Advance;
		static const long ID_BITMAPBUTTON_SLIDER_Skips_Advance;
		static const long ID_STATICTEXT_Skips_Direction;
		static const long ID_CHOICE_Skips_Direction;
		static const long ID_BITMAPBUTTON_CHOICE_Skips_Direction;
		static const long ID_PANEL21;
		static const long ID_NOTEBOOK_SSEFFECT_TYPE;
		//*)

	public:

		//(*Handlers(SingleStrandPanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
