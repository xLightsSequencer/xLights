#ifndef LiquidPANEL_H
#define LiquidPANEL_H

//(*Headers(LiquidPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxNotebook;
class wxNotebookEvent;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class LiquidPanel: public wxPanel
{
    void ValidateWindow();
    // these are only used for validation

	public:

		LiquidPanel(wxWindow* parent);
		virtual ~LiquidPanel();

		//(*Declarations(LiquidPanel)
		BulkEditCheckBox* CheckBox_BottomBarrier;
		BulkEditCheckBox* CheckBox_FlowMusic1;
		BulkEditCheckBox* CheckBox_FlowMusic2;
		BulkEditCheckBox* CheckBox_FlowMusic3;
		BulkEditCheckBox* CheckBox_FlowMusic4;
		BulkEditCheckBox* CheckBox_HoldColor;
		BulkEditCheckBox* CheckBox_LeftBarrier;
		BulkEditCheckBox* CheckBox_MixColors;
		BulkEditCheckBox* CheckBox_RightBarrier;
		BulkEditCheckBox* CheckBox_TopBarrier;
		BulkEditChoice* Choice_ParticleType;
		BulkEditSlider* Slider_Despeckle;
		BulkEditSlider* Slider_Direction1;
		BulkEditSlider* Slider_Direction2;
		BulkEditSlider* Slider_Direction3;
		BulkEditSlider* Slider_Direction4;
		BulkEditSlider* Slider_Flow1;
		BulkEditSlider* Slider_Flow2;
		BulkEditSlider* Slider_Flow3;
		BulkEditSlider* Slider_Flow4;
		BulkEditSlider* Slider_LifeTime;
		BulkEditSlider* Slider_Liquid_SourceSize1;
		BulkEditSlider* Slider_Liquid_SourceSize2;
		BulkEditSlider* Slider_Liquid_SourceSize3;
		BulkEditSlider* Slider_Liquid_SourceSize4;
		BulkEditSlider* Slider_Size;
		BulkEditSlider* Slider_Velocity1;
		BulkEditSlider* Slider_Velocity2;
		BulkEditSlider* Slider_Velocity3;
		BulkEditSlider* Slider_Velocity4;
		BulkEditSlider* Slider_WarmUpFrames;
		BulkEditSlider* Slider_X1;
		BulkEditSlider* Slider_X2;
		BulkEditSlider* Slider_X3;
		BulkEditSlider* Slider_X4;
		BulkEditSlider* Slider_Y1;
		BulkEditSlider* Slider_Y2;
		BulkEditSlider* Slider_Y3;
		BulkEditSlider* Slider_Y4;
		BulkEditSliderF1* Slider_Liquid_Gravity;
		BulkEditTextCtrl* TextCtrl_Despeckle;
		BulkEditTextCtrl* TextCtrl_Direction1;
		BulkEditTextCtrl* TextCtrl_Direction2;
		BulkEditTextCtrl* TextCtrl_Direction3;
		BulkEditTextCtrl* TextCtrl_Direction4;
		BulkEditTextCtrl* TextCtrl_Flow1;
		BulkEditTextCtrl* TextCtrl_Flow2;
		BulkEditTextCtrl* TextCtrl_Flow3;
		BulkEditTextCtrl* TextCtrl_Flow4;
		BulkEditTextCtrl* TextCtrl_LifeTime;
		BulkEditTextCtrl* TextCtrl_Liquid_SourceSize1;
		BulkEditTextCtrl* TextCtrl_Liquid_SourceSize2;
		BulkEditTextCtrl* TextCtrl_Liquid_SourceSize3;
		BulkEditTextCtrl* TextCtrl_Size;
		BulkEditTextCtrl* TextCtrl_Velocity1;
		BulkEditTextCtrl* TextCtrl_Velocity2;
		BulkEditTextCtrl* TextCtrl_Velocity3;
		BulkEditTextCtrl* TextCtrl_Velocity4;
		BulkEditTextCtrl* TextCtrl_WarmUpFrames;
		BulkEditTextCtrl* TextCtrl_X1;
		BulkEditTextCtrl* TextCtrl_X2;
		BulkEditTextCtrl* TextCtrl_X3;
		BulkEditTextCtrl* TextCtrl_X4;
		BulkEditTextCtrl* TextCtrl_Y1;
		BulkEditTextCtrl* TextCtrl_Y2;
		BulkEditTextCtrl* TextCtrl_Y3;
		BulkEditTextCtrl* TextCtrl_Y4;
		BulkEditTextCtrl* TextCtrl_liquid_SourceSize4;
		BulkEditValueCurveButton* BitmapButton_Direction1;
		BulkEditValueCurveButton* BitmapButton_Direction2;
		BulkEditValueCurveButton* BitmapButton_Direction3;
		BulkEditValueCurveButton* BitmapButton_Direction4;
		BulkEditValueCurveButton* BitmapButton_Flow1;
		BulkEditValueCurveButton* BitmapButton_Flow2;
		BulkEditValueCurveButton* BitmapButton_Flow3;
		BulkEditValueCurveButton* BitmapButton_Flow4;
		BulkEditValueCurveButton* BitmapButton_LifeTime;
		BulkEditValueCurveButton* BitmapButton_Liquid_SourceSize1;
		BulkEditValueCurveButton* BitmapButton_Liquid_SourceSize2;
		BulkEditValueCurveButton* BitmapButton_Liquid_SourceSize3;
		BulkEditValueCurveButton* BitmapButton_Liquid_SourceSize4;
		BulkEditValueCurveButton* BitmapButton_Velocity1;
		BulkEditValueCurveButton* BitmapButton_Velocity2;
		BulkEditValueCurveButton* BitmapButton_Velocity3;
		BulkEditValueCurveButton* BitmapButton_Velocity4;
		BulkEditValueCurveButton* BitmapButton_X1;
		BulkEditValueCurveButton* BitmapButton_X2;
		BulkEditValueCurveButton* BitmapButton_X3;
		BulkEditValueCurveButton* BitmapButton_X4;
		BulkEditValueCurveButton* BitmapButton_Y1;
		BulkEditValueCurveButton* BitmapButton_Y2;
		BulkEditValueCurveButton* BitmapButton_Y3;
		BulkEditValueCurveButton* BitmapButton_Y4;
		wxCheckBox* CheckBox_Enabled2;
		wxCheckBox* CheckBox_Enabled3;
		wxCheckBox* CheckBox_Enabled4;
		wxNotebook* Notebook1;
		wxPanel* Panel1;
		wxPanel* Panel2;
		wxPanel* Panel3;
		wxPanel* Panel4;
		wxStaticText* StaticText10;
		wxStaticText* StaticText11;
		wxStaticText* StaticText12;
		wxStaticText* StaticText13;
		wxStaticText* StaticText14;
		wxStaticText* StaticText15;
		wxStaticText* StaticText1;
		wxStaticText* StaticText22;
		wxStaticText* StaticText23;
		wxStaticText* StaticText24;
		wxStaticText* StaticText25;
		wxStaticText* StaticText26;
		wxStaticText* StaticText2;
		wxStaticText* StaticText32;
		wxStaticText* StaticText33;
		wxStaticText* StaticText34;
		wxStaticText* StaticText35;
		wxStaticText* StaticText36;
		wxStaticText* StaticText3;
		wxStaticText* StaticText42;
		wxStaticText* StaticText43;
		wxStaticText* StaticText44;
		wxStaticText* StaticText45;
		wxStaticText* StaticText46;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		//*)

	protected:

		//(*Identifiers(LiquidPanel)
		static const long ID_CHECKBOX_TopBarrier;
		static const long ID_CHECKBOX_BottomBarrier;
		static const long ID_CHECKBOX_LeftBarrier;
		static const long ID_CHECKBOX_RightBarrier;
		static const long ID_CHECKBOX_HoldColor;
		static const long ID_CHECKBOX_MixColors;
		static const long ID_STATICTEXT_ParticleType;
		static const long ID_CHOICE_ParticleType;
		static const long ID_STATICTEXT_LifeTime;
		static const long IDD_SLIDER_LifeTime;
		static const long ID_VALUECURVE_LifeTime;
		static const long ID_TEXTCTRL_LifeTime;
		static const long ID_STATICTEXT_Size;
		static const long IDD_SLIDER_Size;
		static const long ID_TEXTCTRL_Size;
		static const long ID_STATICTEXT_WarmUpFrames;
		static const long IDD_SLIDER_WarmUpFrames;
		static const long ID_TEXTCTRL_WarmUpFrames;
		static const long ID_STATICTEXT_Despeckle;
		static const long IDD_SLIDER_Despeckle;
		static const long ID_TEXTCTRL_Despeckle;
		static const long ID_STATICTEXT1;
		static const long IDD_SLIDER_Liquid_Gravity;
		static const long ID_TEXTCTRL_Liquid_Gravity;
		static const long ID_STATICTEXT_X1;
		static const long IDD_SLIDER_X1;
		static const long ID_VALUECURVE_X1;
		static const long ID_TEXTCTRL_X1;
		static const long ID_STATICTEXT_Y1;
		static const long IDD_SLIDER_Y1;
		static const long ID_VALUECURVE_Y1;
		static const long ID_TEXTCTRL_Y1;
		static const long ID_STATICTEXT_Direction1;
		static const long IDD_SLIDER_Direction1;
		static const long ID_VALUECURVE_Direction1;
		static const long ID_TEXTCTRL_Direction1;
		static const long ID_STATICTEXT_Velocity1;
		static const long IDD_SLIDER_Velocity1;
		static const long ID_VALUECURVE_Velocity1;
		static const long ID_TEXTCTRL_Velocity1;
		static const long ID_STATICTEXT_Flow1;
		static const long IDD_SLIDER_Flow1;
		static const long ID_VALUECURVE_Flow1;
		static const long ID_TEXTCTRL_Flow1;
		static const long ID_STATICTEXT_Liquid_SourceSize1;
		static const long IDD_SLIDER_Liquid_SourceSize1;
		static const long ID_VALUECURVE_Liquid_SourceSize1;
		static const long ID_TEXTCTRL_Liquid_SourceSize1;
		static const long ID_CHECKBOX_FlowMusic1;
		static const long ID_PANEL1;
		static const long ID_CHECKBOX_Enabled2;
		static const long ID_STATICTEXT_X2;
		static const long IDD_SLIDER_X2;
		static const long ID_VALUECURVE_X2;
		static const long ID_TEXTCTRL_X2;
		static const long ID_STATICTEXT_Y2;
		static const long IDD_SLIDER_Y2;
		static const long ID_VALUECURVE_Y2;
		static const long ID_TEXTCTRL_Y2;
		static const long ID_STATICTEXT_Direction2;
		static const long IDD_SLIDER_Direction2;
		static const long ID_VALUECURVE_Direction2;
		static const long ID_TEXTCTRL_Direction2;
		static const long ID_STATICTEXT_Velocity2;
		static const long IDD_SLIDER_Velocity2;
		static const long ID_VALUECURVE_Velocity2;
		static const long ID_TEXTCTRL_Velocity2;
		static const long ID_STATICTEXT_Flow2;
		static const long IDD_SLIDER_Flow2;
		static const long ID_VALUECURVE_Flow2;
		static const long ID_TEXTCTRL_Flow2;
		static const long ID_STATICTEXT_SourceSize2;
		static const long IDD_SLIDER_Liquid_SourceSize2;
		static const long ID_VALUECURVE_Liquid_SourceSize2;
		static const long ID_TEXTCTRL_Liquid_SourceSize2;
		static const long ID_CHECKBOX_FlowMusic2;
		static const long ID_PANEL2;
		static const long ID_CHECKBOX_Enabled3;
		static const long ID_STATICTEXT_X3;
		static const long IDD_SLIDER_X3;
		static const long ID_VALUECURVE_X3;
		static const long ID_TEXTCTRL_X3;
		static const long ID_STATICTEXT_Y3;
		static const long IDD_SLIDER_Y3;
		static const long ID_VALUECURVE_Y3;
		static const long ID_TEXTCTRL_Y3;
		static const long ID_STATICTEXT_Direction3;
		static const long IDD_SLIDER_Direction3;
		static const long ID_VALUECURVE_Direction3;
		static const long ID_TEXTCTRL_Direction3;
		static const long ID_STATICTEXT_Velocity3;
		static const long IDD_SLIDER_Velocity3;
		static const long ID_VALUECURVE_Velocity3;
		static const long ID_TEXTCTRL_Velocity3;
		static const long ID_STATICTEXT_Flow3;
		static const long IDD_SLIDER_Flow3;
		static const long ID_VALUECURVE_Flow3;
		static const long ID_TEXTCTRL_Flow3;
		static const long ID_STATICTEXT_SourceSize3;
		static const long IDD_SLIDER_Liquid_SourceSize3;
		static const long ID_VALUECURVE_Liquid_SourceSize3;
		static const long ID_TEXTCTRL_Liquid_SourceSize3;
		static const long ID_CHECKBOX_FlowMusic3;
		static const long ID_PANEL3;
		static const long ID_CHECKBOX_Enabled4;
		static const long ID_STATICTEXT_X4;
		static const long IDD_SLIDER_X4;
		static const long ID_VALUECURVE_X4;
		static const long ID_TEXTCTRL_X4;
		static const long ID_STATICTEXT_Y4;
		static const long IDD_SLIDER_Y4;
		static const long ID_VALUECURVE_Y4;
		static const long ID_TEXTCTRL_Y4;
		static const long ID_STATICTEXT_Direction4;
		static const long IDD_SLIDER_Direction4;
		static const long ID_VALUECURVE_Direction4;
		static const long ID_TEXTCTRL_Direction4;
		static const long ID_STATICTEXT_Velocity4;
		static const long IDD_SLIDER_Velocity4;
		static const long ID_VALUECURVE_Velocity4;
		static const long ID_TEXTCTRL_Velocity4;
		static const long ID_STATICTEXT_Flow4;
		static const long IDD_SLIDER_Flow4;
		static const long ID_VALUECURVE_Flow4;
		static const long ID_TEXTCTRL_Flow4;
		static const long ID_STATICTEXT_Liquid_SourceSize4;
		static const long IDD_SLIDER_Liquid_SourceSize4;
		static const long ID_VALUECURVE_Liquid_SourceSize4;
		static const long ID_TEXTCTRL_Liquid_SourceSize4;
		static const long ID_CHECKBOX_FlowMusic4;
		static const long ID_PANEL4;
		static const long ID_NOTEBOOK1;
		//*)

	public:

		//(*Handlers(LiquidPanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnCheckBox_EnabledClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
