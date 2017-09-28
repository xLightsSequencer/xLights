#ifndef MORPHPANEL_H
#define MORPHPANEL_H

//(*Headers(MorphPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxNotebookEvent;
class wxNotebook;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxButton;
class wxChoice;
//*)

#include "../ValueCurveButton.h"

class MorphPanel: public wxPanel
{
	public:

		MorphPanel(wxWindow* parent);
		virtual ~MorphPanel();

		//(*Declarations(MorphPanel)
		wxBitmapButton* BitmapButton_Morph_RepeatSkip;
		wxSlider* Slider_Morph_Start_X1;
		ValueCurveButton* BitmapButton_Morph_End_X2;
		wxTextCtrl* TextCtrl_Morph_End_X2;
		wxBitmapButton* BitmapButton_MorphStagger;
		wxBitmapButton* BitmapButton_Morph_StartX2;
		wxBitmapButton* BitmapButton_Morph_EndY1;
		wxSlider* Slider_MorphAccel;
		wxSlider* Slider_Morph_End_X1;
		ValueCurveButton* BitmapButton_Morph_End_X1;
		wxTextCtrl* TextCtrl_Morph_Start_X2;
		wxStaticText* StaticText117;
		wxTextCtrl* TextCtrl_Morph_End_Y1;
		wxStaticText* StaticText121;
		ValueCurveButton* BitmapButton_MorphEndLength;
		wxStaticText* StaticText120;
		wxCheckBox* CheckBox_Morph_Start_Link;
		wxBitmapButton* BitmapButton_Morph_EndX2;
		wxSlider* Slider_Morph_Start_X2;
		wxBitmapButton* BitmapButton_Morph_StartX1;
		wxStaticText* StaticText124;
		wxTextCtrl* TextCtrl_Morph_Start_Y1;
		wxSlider* Slider_Morph_Start_Y1;
		ValueCurveButton* BitmapButton_Morph_Start_X1;
		wxBitmapButton* BitmapButton_Morph_EndX1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Morph_Start_Y2;
		wxSlider* Slider_Morph_End_Y1;
		ValueCurveButton* BitmapButton_Morph_End_Y2;
		wxBitmapButton* BitmapButton_Morph_Accel;
		wxStaticText* StaticText123;
		ValueCurveButton* BitmapButton_MorphDuration;
		wxSlider* Slider_Morph_End_X2;
		wxBitmapButton* BitmapButton_Morph_StartLength;
		wxSlider* Slider_MorphDuration;
		wxSlider* Slider_Morph_Repeat_Skip;
		wxBitmapButton* BitmapButton_Morph_StartY1;
		wxBitmapButton* BitmapButton_Morph_RepeatCount;
		wxTextCtrl* TextCtrl_Morph_End_X1;
		wxStaticText* StaticText115;
		ValueCurveButton* BitmapButton_Morph_Repeat_Count;
		wxStaticText* StaticText119;
		wxSlider* Slider_Morph_Start_Y2;
		wxStaticText* StaticText122;
		wxSlider* Slider_Morph_End_Y2;
		wxStaticText* StaticText171;
		wxBitmapButton* BitmapButton_Morph_EndY2;
		wxNotebook* Notebook_Morph;
		ValueCurveButton* BitmapButton_Morph_Start_Y2;
		wxSlider* Slider_Morph_Stagger;
		wxStaticText* StaticText172;
		wxCheckBox* CheckBox_Morph_End_Link;
		wxSlider* Slider_MorphEndLength;
		wxTextCtrl* TextCtrl_Morph_End_Y2;
		wxCheckBox* CheckBox_ShowHeadAtStart;
		wxChoice* Choice_Morph_QuickSet;
		wxTextCtrl* TextCtrl_Morph_Start_X1;
		wxButton* ButtonSwap;
		ValueCurveButton* BitmapButton_Morph_End_Y1;
		ValueCurveButton* BitmapButton_Morph_Start_X2;
		wxBitmapButton* BitmapButton_ShowHeadAtStart;
		wxStaticText* StaticText59;
		ValueCurveButton* BitmapButton_Morph_Start_Y1;
		wxBitmapButton* BitmapButton_Morph_EndLength;
		ValueCurveButton* BitmapButton_MorphStartLength;
		ValueCurveButton* BitmapButton_MorphAccel;
		wxStaticText* StaticText173;
		ValueCurveButton* BitmapButton_Morph_Stagger;
		wxBitmapButton* BitmapButton_Morph_StartY2;
		wxStaticText* StaticText99;
		wxSlider* Slider_MorphStartLength;
		wxBitmapButton* BitmapButton_Morph_Duration;
		ValueCurveButton* BitmapButton_Morph_Repeat_Skip;
		wxSlider* Slider_Morph_Repeat_Count;
		//*)

	protected:

		//(*Identifiers(MorphPanel)
		static const long ID_SLIDER_Morph_Start_X1;
		static const long ID_VALUECURVE_Morph_Start_X1;
		static const long IDD_TEXTCTRL_Morph_Start_X1;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Start_X1;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER_Morph_Start_Y1;
		static const long ID_VALUECURVE_Morph_Start_Y1;
		static const long IDD_TEXTCTRL_Morph_Start_Y1;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Start_Y1;
		static const long ID_STATICTEXT61;
		static const long ID_SLIDER_Morph_Start_X2;
		static const long ID_VALUECURVE_Morph_Start_X2;
		static const long IDD_TEXTCTRL_Morph_Start_X2;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Start_X2;
		static const long ID_STATICTEXT102;
		static const long ID_SLIDER_Morph_Start_Y2;
		static const long ID_VALUECURVE_Morph_Start_Y2;
		static const long IDD_TEXTCTRL_Morph_Start_Y2;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Start_Y2;
		static const long ID_STATICTEXT115;
		static const long ID_SLIDER_MorphStartLength;
		static const long ID_VALUECURVE_MorphStartLength;
		static const long IDD_TEXTCTRL_MorphStartLength;
		static const long ID_BITMAPBUTTON_SLIDER_MorphStartLength;
		static const long ID_CHECKBOX_Morph_Start_Link;
		static const long IDD_CHOICE_Morph_QuickSet;
		static const long ID_PANEL4;
		static const long ID_STATICTEXT116;
		static const long ID_SLIDER_Morph_End_X1;
		static const long ID_VALUECURVE_Morph_End_X1;
		static const long IDD_TEXTCTRL_Morph_End_X1;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_End_X1;
		static const long ID_STATICTEXT117;
		static const long ID_SLIDER_Morph_End_Y1;
		static const long ID_VALUECURVE_Morph_End_Y1;
		static const long IDD_TEXTCTRL_Morph_End_Y1;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_End_Y1;
		static const long ID_STATICTEXT119;
		static const long ID_SLIDER_Morph_End_X2;
		static const long ID_VALUECURVE_Morph_End_X2;
		static const long IDD_TEXTCTRL_Morph_End_X2;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_End_X2;
		static const long ID_STATICTEXT121;
		static const long ID_SLIDER_Morph_End_Y2;
		static const long ID_VALUECURVE_Morph_End_Y2;
		static const long IDD_TEXTCTRL_Morph_End_Y2;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_End_Y2;
		static const long ID_STATICTEXT122;
		static const long ID_SLIDER_MorphEndLength;
		static const long ID_VALUECURVE_MorphEndLength;
		static const long IDD_TEXTCTRL_MorphEndLength;
		static const long ID_BITMAPBUTTON_SLIDER_MorphEndLength;
		static const long ID_CHECKBOX_Morph_End_Link;
		static const long ID_PANEL27;
		static const long ID_STATICTEXT123;
		static const long ID_SLIDER_MorphDuration;
		static const long ID_VALUECURVE_MorphDuration;
		static const long IDD_TEXTCTRL_MorphDuration;
		static const long ID_BITMAPBUTTON_SLIDER_MorphDuration;
		static const long ID_STATICTEXT124;
		static const long ID_SLIDER_MorphAccel;
		static const long ID_VALUECURVE_MorphAccel;
		static const long IDD_TEXTCTRL_MorphAccel;
		static const long ID_BITMAPBUTTON_SLIDER_MorphAccel;
		static const long ID_STATICTEXT97;
		static const long ID_SLIDER_Morph_Repeat_Count;
		static const long ID_VALUECURVE_Morph_Repeat_Count;
		static const long IDD_TEXTCTRL_Morph_Repeat_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Repeat_Count;
		static const long ID_STATICTEXT156;
		static const long ID_SLIDER_Morph_Repeat_Skip;
		static const long ID_VALUECURVE_Morph_Repeat_Skip;
		static const long IDD_TEXTCTRL_Morph_Repeat_Skip;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Repeat_Skip;
		static const long ID_STATICTEXT157;
		static const long ID_SLIDER_Morph_Stagger;
		static const long ID_VALUECURVE_Morph_Stagger;
		static const long IDD_TEXTCTRL_Morph_Stagger;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Stagger;
		static const long ID_CHECKBOX_ShowHeadAtStart;
		static const long ID_BITMAPBUTTON_CHECKBOX_ShowHeadAtStart;
		static const long ID_BUTTONSWAP;
		static const long ID_PANEL30;
		static const long ID_NOTEBOOK_Morph;
		//*)

	public:

		//(*Handlers(MorphPanel)
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
		void UpdateLinkedSliderFloat2(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
		void OnCheckBox_Morph_Start_LinkClick(wxCommandEvent& event);
		void OnCheckBox_Morph_End_LinkClick(wxCommandEvent& event);
		void OnChoice_Morph_QuickSetSelect(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnButtonSwapClick(wxCommandEvent& event);
		void OnButtonSwapClick1(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
