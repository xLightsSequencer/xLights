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

#include "../BulkEditControls.h"

class MorphPanel: public wxPanel
{
	public:

		MorphPanel(wxWindow* parent);
		virtual ~MorphPanel();

		//(*Declarations(MorphPanel)
		wxBitmapButton* BitmapButton_Morph_RepeatSkip;
		BulkEditSlider* Slider_Morph_Start_Y1;
		BulkEditSlider* Slider_Morph_Start_Y2;
		wxBitmapButton* BitmapButton_MorphStagger;
		BulkEditSlider* Slider_MorphEndLength;
		wxBitmapButton* BitmapButton_Morph_StartX2;
		wxBitmapButton* BitmapButton_Morph_EndY1;
		BulkEditValueCurveButton* BitmapButton_Morph_Repeat_Count;
		BulkEditSlider* Slider_Morph_End_X1;
		BulkEditValueCurveButton* BitmapButton_Morph_End_X2;
		BulkEditTextCtrl* TextCtrl_Morph_End_X1;
		wxStaticText* StaticText117;
		BulkEditSlider* Slider_Morph_End_Y2;
		wxStaticText* StaticText121;
		BulkEditSlider* Slider_Morph_Stagger;
		BulkEditSlider* Slider_MorphAccel;
		wxStaticText* StaticText2;
		BulkEditValueCurveButton* BitmapButton_Morph_End_X1;
		BulkEditSlider* Slider_Morph_Start_X1;
		BulkEditSlider* Slider_MorphDuration;
		BulkEditTextCtrl* TextCtrl_Morph_Start_X2;
		wxStaticText* StaticText120;
		wxCheckBox* CheckBox_Morph_Start_Link;
		wxBitmapButton* BitmapButton_Morph_EndX2;
		BulkEditValueCurveButton* BitmapButton_Morph_Repeat_Skip;
		wxBitmapButton* BitmapButton_Morph_StartX1;
		wxStaticText* StaticText124;
		wxBitmapButton* BitmapButton_Morph_EndX1;
		BulkEditValueCurveButton* BitmapButton_Morph_Start_Y2;
		wxStaticText* StaticText3;
		BulkEditCheckBox* CheckBox_ShowHeadAtStart;
		wxBitmapButton* BitmapButton_Morph_Accel;
		wxStaticText* StaticText123;
		BulkEditValueCurveButton* BitmapButton_MorphStartLength;
		BulkEditValueCurveButton* BitmapButton_MorphDuration;
		BulkEditTextCtrl* TextCtrl_Morph_Start_Y1;
		wxBitmapButton* BitmapButton_Morph_StartLength;
		BulkEditTextCtrl* TextCtrl_Morph_End_Y2;
		BulkEditValueCurveButton* BitmapButton_Morph_Start_X2;
		wxBitmapButton* BitmapButton_Morph_StartY1;
		BulkEditValueCurveButton* BitmapButton_Morph_End_Y2;
		wxBitmapButton* BitmapButton_Morph_RepeatCount;
		wxStaticText* StaticText115;
		wxStaticText* StaticText119;
		wxStaticText* StaticText122;
		wxStaticText* StaticText171;
		BulkEditValueCurveButton* BitmapButton_Morph_Start_X1;
		BulkEditSlider* Slider_Morph_Repeat_Skip;
		wxBitmapButton* BitmapButton_Morph_EndY2;
		BulkEditSlider* Slider_Morph_End_Y1;
		wxNotebook* Notebook_Morph;
		wxStaticText* StaticText172;
		wxCheckBox* CheckBox_Morph_End_Link;
		wxChoice* Choice_Morph_QuickSet;
		wxButton* ButtonSwap;
		BulkEditTextCtrl* TextCtrl_Morph_Start_X1;
		BulkEditValueCurveButton* BitmapButton_Morph_Start_Y1;
		wxBitmapButton* BitmapButton_ShowHeadAtStart;
		BulkEditTextCtrl* TextCtrl_Morph_Start_Y2;
		wxStaticText* StaticText59;
		wxBitmapButton* BitmapButton_Morph_EndLength;
		BulkEditSlider* Slider_MorphStartLength;
		wxStaticText* StaticText173;
		wxBitmapButton* BitmapButton_Morph_StartY2;
		BulkEditTextCtrl* TextCtrl_Morph_End_Y1;
		BulkEditValueCurveButton* BitmapButton_MorphAccel;
		BulkEditValueCurveButton* BitmapButton_MorphEndLength;
		BulkEditSlider* Slider_Morph_Repeat_Count;
		BulkEditValueCurveButton* BitmapButton_Morph_End_Y1;
		BulkEditSlider* Slider_Morph_Start_X2;
		wxStaticText* StaticText99;
		BulkEditSlider* Slider_Morph_End_X2;
		BulkEditValueCurveButton* BitmapButton_Morph_Stagger;
		BulkEditTextCtrl* TextCtrl_Morph_End_X2;
		wxBitmapButton* BitmapButton_Morph_Duration;
		//*)

	protected:

		//(*Identifiers(MorphPanel)
		static const long ID_STATICTEXT_Morph_Start_X1;
		static const long ID_SLIDER_Morph_Start_X1;
		static const long ID_VALUECURVE_Morph_Start_X1;
		static const long IDD_TEXTCTRL_Morph_Start_X1;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Start_X1;
		static const long ID_STATICTEXT_Morph_Start_Y1;
		static const long ID_SLIDER_Morph_Start_Y1;
		static const long ID_VALUECURVE_Morph_Start_Y1;
		static const long IDD_TEXTCTRL_Morph_Start_Y1;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Start_Y1;
		static const long ID_STATICTEXT_Morph_Start_X2;
		static const long ID_SLIDER_Morph_Start_X2;
		static const long ID_VALUECURVE_Morph_Start_X2;
		static const long IDD_TEXTCTRL_Morph_Start_X2;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Start_X2;
		static const long ID_STATICTEXT_Morph_Start_Y2;
		static const long ID_SLIDER_Morph_Start_Y2;
		static const long ID_VALUECURVE_Morph_Start_Y2;
		static const long IDD_TEXTCTRL_Morph_Start_Y2;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Start_Y2;
		static const long ID_STATICTEXT_MorphStartLength;
		static const long ID_SLIDER_MorphStartLength;
		static const long ID_VALUECURVE_MorphStartLength;
		static const long IDD_TEXTCTRL_MorphStartLength;
		static const long ID_BITMAPBUTTON_SLIDER_MorphStartLength;
		static const long ID_CHECKBOX_Morph_Start_Link;
		static const long IDD_CHOICE_Morph_QuickSet;
		static const long ID_PANEL4;
		static const long ID_STATICTEXT_Morph_End_X1;
		static const long ID_SLIDER_Morph_End_X1;
		static const long ID_VALUECURVE_Morph_End_X1;
		static const long IDD_TEXTCTRL_Morph_End_X1;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_End_X1;
		static const long ID_STATICTEXT_Morph_End_Y1;
		static const long ID_SLIDER_Morph_End_Y1;
		static const long ID_VALUECURVE_Morph_End_Y1;
		static const long IDD_TEXTCTRL_Morph_End_Y1;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_End_Y1;
		static const long ID_STATICTEXT_Morph_End_X2;
		static const long ID_SLIDER_Morph_End_X2;
		static const long ID_VALUECURVE_Morph_End_X2;
		static const long IDD_TEXTCTRL_Morph_End_X2;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_End_X2;
		static const long ID_STATICTEXT_Morph_End_Y2;
		static const long ID_SLIDER_Morph_End_Y2;
		static const long ID_VALUECURVE_Morph_End_Y2;
		static const long IDD_TEXTCTRL_Morph_End_Y2;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_End_Y2;
		static const long ID_STATICTEXT_MorphEndLength;
		static const long ID_SLIDER_MorphEndLength;
		static const long ID_VALUECURVE_MorphEndLength;
		static const long IDD_TEXTCTRL_MorphEndLength;
		static const long ID_BITMAPBUTTON_SLIDER_MorphEndLength;
		static const long ID_CHECKBOX_Morph_End_Link;
		static const long ID_PANEL27;
		static const long ID_STATICTEXT_MorphDuration;
		static const long ID_SLIDER_MorphDuration;
		static const long ID_VALUECURVE_MorphDuration;
		static const long IDD_TEXTCTRL_MorphDuration;
		static const long ID_BITMAPBUTTON_SLIDER_MorphDuration;
		static const long ID_STATICTEXT_MorphAccel;
		static const long ID_SLIDER_MorphAccel;
		static const long ID_VALUECURVE_MorphAccel;
		static const long IDD_TEXTCTRL_MorphAccel;
		static const long ID_BITMAPBUTTON_SLIDER_MorphAccel;
		static const long ID_STATICTEXT_Morph_Repeat_Count;
		static const long ID_SLIDER_Morph_Repeat_Count;
		static const long ID_VALUECURVE_Morph_Repeat_Count;
		static const long IDD_TEXTCTRL_Morph_Repeat_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Repeat_Count;
		static const long ID_STATICTEXT_Morph_Repeat_Skip;
		static const long ID_SLIDER_Morph_Repeat_Skip;
		static const long ID_VALUECURVE_Morph_Repeat_Skip;
		static const long IDD_TEXTCTRL_Morph_Repeat_Skip;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Repeat_Skip;
		static const long ID_STATICTEXT_Morph_Stagger;
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
		void OnLockButtonClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnCheckBox_Morph_Start_LinkClick(wxCommandEvent& event);
		void OnCheckBox_Morph_End_LinkClick(wxCommandEvent& event);
		void OnChoice_Morph_QuickSetSelect(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnButtonSwapClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
