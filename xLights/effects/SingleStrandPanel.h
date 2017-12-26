#ifndef SINGLESTRANDPANEL_H
#define SINGLESTRANDPANEL_H

//(*Headers(SingleStrandPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxNotebookEvent;
class wxNotebook;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../BulkEditControls.h"

class SingleStrandPanel: public wxPanel
{
	public:

		SingleStrandPanel(wxWindow* parent);
		virtual ~SingleStrandPanel();

		//(*Declarations(SingleStrandPanel)
		wxStaticText* StaticText10;
		wxStaticText* StaticText9;
		wxBitmapButton* BitmapButton_Chase_3dFade1;
		BulkEditSlider* Slider_Skips_SkipSize;
		wxBitmapButton* BitmapButton_Chase_Group_All;
		BulkEditChoice* Choice_Chase_Type1;
		wxStaticText* StaticText106;
		BulkEditSlider* Slider_Skips_StartPos;
		wxBitmapButton* BitmapButton_SingleStrand_Colors;
		wxNotebook* SingleStrandEffectType;
		wxStaticText* StaticText101;
		wxBitmapButton* BitmapButton_Color_Mix1;
		BulkEditValueCurveButton* BitmapButton_Chase_Rotations;
		BulkEditCheckBox* CheckBox_Chase_3dFade1;
		wxBitmapButton* BitmapButton_Skips_StartingPosition;
		wxStaticText* StaticText11;
		BulkEditSlider* Slider_Skips_Advance;
		wxStaticText* StaticText179;
		wxPanel* Panel1;
		BulkEditCheckBox* CheckBox_Chase_Group_All;
		wxStaticText* StaticText105;
		BulkEditChoice* Choice_SingleStrand_Colors;
		wxStaticText* StaticText109;
		wxBitmapButton* BitmapButton_Skips_Direction;
		BulkEditSlider* Slider_Skips_BandSize;
		wxStaticText* StaticText61;
		BulkEditChoice* Choice_Skips_Direction;
		wxBitmapButton* BitmapButton_Skips_SkipSize;
		BulkEditSlider* Slider_Color_Mix1;
		wxBitmapButton* BitmapButton4;
		BulkEditValueCurveButton* BitmapButton_Color_Mix1VC;
		wxBitmapButton* BitmapButton3;
		wxStaticText* StaticText60;
		wxBitmapButton* BitmapButton_Number_Chases;
		wxPanel* Panel2;
		BulkEditValueCurveButton* BitmapButton_Number_ChasesVC;
		wxBitmapButton* BitmapButton_Skips_BandSize;
		wxBitmapButton* BitmapButton_Chase_Type1;
		BulkEditSliderF1* Slider_Chase_Rotations;
		BulkEditSlider* Slider_Number_Chases;
		//*)

	protected:

		//(*Identifiers(SingleStrandPanel)
		static const long ID_STATICTEXT_SingleStrand_Colors;
		static const long ID_CHOICE_SingleStrand_Colors;
		static const long ID_BITMAPBUTTON_SingleStrand_Colors;
		static const long ID_STATICTEXT_Number_Chases;
		static const long ID_SLIDER_Number_Chases;
		static const long ID_VALUECURVE_Number_Chases;
		static const long IDD_TEXTCTRL_Number_Chases;
		static const long ID_BITMAPBUTTON_Number_Chases;
		static const long ID_STATICTEXT_Color_Mix1;
		static const long ID_SLIDER_Color_Mix1;
		static const long ID_VALUECURVE_Color_Mix1;
		static const long IDD_TEXTCTRL_Color_Mix1;
		static const long ID_BITMAPBUTTON_SLIDER_Color_Mix1;
		static const long ID_STATICTEXT_Chase_Rotations;
		static const long IDD_SLIDER_Chase_Rotations;
		static const long ID_VALUECURVE_Chase_Rotations;
		static const long ID_TEXTCTRL_Chase_Rotations;
		static const long ID_BITMAPBUTTON23;
		static const long ID_STATICTEXT_Chase_Type1;
		static const long ID_CHOICE_Chase_Type1;
		static const long ID_BITMAPBUTTON_CHOICE_Chase_Type1;
		static const long ID_CHECKBOX_Chase_3dFade1;
		static const long ID_BITMAPBUTTON_CHECKBOX_Chase_3dFade1;
		static const long ID_CHECKBOX_Chase_Group_All;
		static const long ID_BITMAPBUTTON_Chase_Group_All;
		static const long ID_PANEL3;
		static const long ID_STATICTEXT_Skips_BandSize;
		static const long ID_SLIDER_Skips_BandSize;
		static const long IDD_TEXTCTRL_Skips_BandSize;
		static const long ID_BITMAPBUTTON44;
		static const long ID_STATICTEXT_Skips_SkipSize;
		static const long ID_SLIDER_Skips_SkipSize;
		static const long IDD_TEXTCTRL_Skips_SkipSize;
		static const long ID_BITMAPBUTTON46;
		static const long ID_STATICTEXT_Skips_StartPos;
		static const long ID_SLIDER_Skips_StartPos;
		static const long IDD_TEXTCTRL_Skips_StartPos;
		static const long ID_BITMAPBUTTON47;
		static const long IS_STATICTEXT_Skips_Advance;
		static const long ID_SLIDER_Skips_Advance;
		static const long IDD_TEXTCTRL_Skips_Advance;
		static const long ID_BITMAPBUTTON22;
		static const long ID_STATICTEXT_Skips_Direction;
		static const long ID_CHOICE_Skips_Direction;
		static const long ID_BITMAPBUTTON48;
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
