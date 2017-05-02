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

#include "../ValueCurveButton.h"

class SingleStrandPanel: public wxPanel
{
	public:

		SingleStrandPanel(wxWindow* parent);
		virtual ~SingleStrandPanel();

		//(*Declarations(SingleStrandPanel)
		wxStaticText* StaticText9;
		wxBitmapButton* BitmapButton_Chase_3dFade1;
		wxSlider* Slider_Skips_BandSize;
		wxBitmapButton* BitmapButton_Chase_Group_All;
		wxSlider* Slider_Chase_Rotations;
		wxSlider* Slider_Skips_Advance;
		wxBitmapButton* BitmapButton_SingleStrand_Colors;
		wxNotebook* SingleStrandEffectType;
		wxSlider* Slider_Number_Chases;
		wxBitmapButton* BitmapButton_Color_Mix1;
		wxBitmapButton* BitmapButton_Skips_StartingPosition;
		wxStaticText* StaticText11;
		wxCheckBox* CheckBox_Chase_3dFade1;
		wxPanel* Panel1;
		wxCheckBox* CheckBox_Chase_Group_All;
		wxStaticText* StaticText109;
		wxBitmapButton* BitmapButton_Skips_Direction;
		wxChoice* Choice_Chase_Type1;
		ValueCurveButton* BitmapButton_Chase_Rotations;
		wxChoice* Choice_SingleStrand_Colors;
		wxSlider* Slider_Color_Mix1;
		wxSlider* Slider_Skips_StartPos;
		wxStaticText* StaticText61;
		wxBitmapButton* BitmapButton_Skips_SkipSize;
		wxBitmapButton* BitmapButton4;
		wxSlider* Slider_Skips_SkipSize;
		wxBitmapButton* BitmapButton3;
		wxStaticText* StaticText60;
		wxBitmapButton* BitmapButton_Number_Chases;
		wxPanel* Panel2;
		wxBitmapButton* BitmapButton_Skips_BandSize;
		wxBitmapButton* BitmapButton_Chase_Type1;
		ValueCurveButton* BitmapButton_Color_Mix1VC;
		ValueCurveButton* BitmapButton_Number_ChasesVC;
		wxChoice* Choice_Skips_Direction;
		//*)

	protected:

		//(*Identifiers(SingleStrandPanel)
		static const long ID_STATICTEXT63;
		static const long ID_CHOICE_SingleStrand_Colors;
		static const long ID_BITMAPBUTTON_SingleStrand_Colors;
		static const long ID_STATICTEXT62;
		static const long ID_SLIDER_Number_Chases;
		static const long ID_VALUECURVE_Number_Chases;
		static const long IDD_TEXTCTRL_Number_Chases;
		static const long ID_BITMAPBUTTON_Number_Chases;
		static const long ID_STATICTEXT8;
		static const long ID_SLIDER_Color_Mix1;
		static const long ID_VALUECURVE_Color_Mix1;
		static const long IDD_TEXTCTRL_Color_Mix1;
		static const long ID_BITMAPBUTTON_SLIDER_Color_Mix1;
		static const long IDD_SLIDER_Chase_Rotations;
		static const long ID_VALUECURVE_Chase_Rotations;
		static const long ID_TEXTCTRL_Chase_Rotations;
		static const long ID_BITMAPBUTTON23;
		static const long ID_STATICTEXT10;
		static const long ID_CHOICE_Chase_Type1;
		static const long ID_BITMAPBUTTON_CHOICE_Chase_Type1;
		static const long ID_CHECKBOX_Chase_3dFade1;
		static const long ID_BITMAPBUTTON_CHECKBOX_Chase_3dFade1;
		static const long ID_CHECKBOX_Chase_Group_All;
		static const long ID_BITMAPBUTTON_Chase_Group_All;
		static const long ID_PANEL3;
		static const long ID_SLIDER_Skips_BandSize;
		static const long IDD_TEXTCTRL_Skips_BandSize;
		static const long ID_BITMAPBUTTON44;
		static const long ID_SLIDER_Skips_SkipSize;
		static const long IDD_TEXTCTRL_Skips_SkipSize;
		static const long ID_BITMAPBUTTON46;
		static const long ID_SLIDER_Skips_StartPos;
		static const long IDD_TEXTCTRL_Skips_StartPos;
		static const long ID_BITMAPBUTTON47;
		static const long ID_SLIDER_Skips_Advance;
		static const long IDD_TEXTCTRL_Skips_Advance;
		static const long ID_BITMAPBUTTON22;
		static const long ID_STATICTEXT111;
		static const long ID_CHOICE_Skips_Direction;
		static const long ID_BITMAPBUTTON48;
		static const long ID_PANEL21;
		static const long ID_NOTEBOOK_SSEFFECT_TYPE;
		//*)

	public:

		//(*Handlers(SingleStrandPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
		void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void UpdateLinkedSliderFloat2(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
		void OnCheckBox_Chase_3dFade1Click(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
