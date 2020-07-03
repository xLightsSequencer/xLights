#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(MorphPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxNotebook;
class wxNotebookEvent;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

class MorphQuickSet;

#include "../BulkEditControls.h"

class MorphPanel: public wxPanel
{
	public:

		MorphPanel(wxWindow* parent);
		virtual ~MorphPanel();

		//(*Declarations(MorphPanel)
		BulkEditButton* ButtonSwap;
		BulkEditCheckBox* CheckBox_ShowHeadAtStart;
		BulkEditSlider* Slider_MorphAccel;
		BulkEditSlider* Slider_MorphDuration;
		BulkEditSlider* Slider_MorphEndLength;
		BulkEditSlider* Slider_MorphStartLength;
		BulkEditSlider* Slider_Morph_End_X1;
		BulkEditSlider* Slider_Morph_End_X2;
		BulkEditSlider* Slider_Morph_End_Y1;
		BulkEditSlider* Slider_Morph_End_Y2;
		BulkEditSlider* Slider_Morph_Repeat_Count;
		BulkEditSlider* Slider_Morph_Repeat_Skip;
		BulkEditSlider* Slider_Morph_Stagger;
		BulkEditSlider* Slider_Morph_Start_X1;
		BulkEditSlider* Slider_Morph_Start_X2;
		BulkEditSlider* Slider_Morph_Start_Y1;
		BulkEditSlider* Slider_Morph_Start_Y2;
		BulkEditTextCtrl* TextCtrl_Morph_End_X1;
		BulkEditTextCtrl* TextCtrl_Morph_End_X2;
		BulkEditTextCtrl* TextCtrl_Morph_End_Y1;
		BulkEditTextCtrl* TextCtrl_Morph_End_Y2;
		BulkEditTextCtrl* TextCtrl_Morph_Start_X1;
		BulkEditTextCtrl* TextCtrl_Morph_Start_X2;
		BulkEditTextCtrl* TextCtrl_Morph_Start_Y1;
		BulkEditTextCtrl* TextCtrl_Morph_Start_Y2;
		BulkEditValueCurveButton* BitmapButton_MorphAccel;
		BulkEditValueCurveButton* BitmapButton_MorphDuration;
		BulkEditValueCurveButton* BitmapButton_MorphEndLength;
		BulkEditValueCurveButton* BitmapButton_MorphStartLength;
		BulkEditValueCurveButton* BitmapButton_Morph_End_X1;
		BulkEditValueCurveButton* BitmapButton_Morph_End_X2;
		BulkEditValueCurveButton* BitmapButton_Morph_End_Y1;
		BulkEditValueCurveButton* BitmapButton_Morph_End_Y2;
		BulkEditValueCurveButton* BitmapButton_Morph_Repeat_Count;
		BulkEditValueCurveButton* BitmapButton_Morph_Repeat_Skip;
		BulkEditValueCurveButton* BitmapButton_Morph_Stagger;
		BulkEditValueCurveButton* BitmapButton_Morph_Start_X1;
		BulkEditValueCurveButton* BitmapButton_Morph_Start_X2;
		BulkEditValueCurveButton* BitmapButton_Morph_Start_Y1;
		BulkEditValueCurveButton* BitmapButton_Morph_Start_Y2;
		MorphQuickSet* Choice_Morph_QuickSet;
		wxCheckBox* CheckBox_Morph_End_Link;
		wxCheckBox* CheckBox_Morph_Start_Link;
		wxNotebook* Notebook_Morph;
		wxStaticText* StaticText115;
		wxStaticText* StaticText117;
		wxStaticText* StaticText119;
		wxStaticText* StaticText120;
		wxStaticText* StaticText121;
		wxStaticText* StaticText122;
		wxStaticText* StaticText123;
		wxStaticText* StaticText124;
		wxStaticText* StaticText171;
		wxStaticText* StaticText172;
		wxStaticText* StaticText173;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText59;
		wxStaticText* StaticText99;
		xlLockButton* BitmapButton_MorphStagger;
		xlLockButton* BitmapButton_Morph_Accel;
		xlLockButton* BitmapButton_Morph_Duration;
		xlLockButton* BitmapButton_Morph_EndLength;
		xlLockButton* BitmapButton_Morph_EndX1;
		xlLockButton* BitmapButton_Morph_EndX2;
		xlLockButton* BitmapButton_Morph_EndY1;
		xlLockButton* BitmapButton_Morph_EndY2;
		xlLockButton* BitmapButton_Morph_RepeatCount;
		xlLockButton* BitmapButton_Morph_RepeatSkip;
		xlLockButton* BitmapButton_Morph_StartLength;
		xlLockButton* BitmapButton_Morph_StartX1;
		xlLockButton* BitmapButton_Morph_StartX2;
		xlLockButton* BitmapButton_Morph_StartY1;
		xlLockButton* BitmapButton_Morph_StartY2;
		xlLockButton* BitmapButton_ShowHeadAtStart;
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
		static const long ID_MORPH_BUTTON_SWAP;
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
