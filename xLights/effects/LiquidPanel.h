#ifndef LiquidPANEL_H
#define LiquidPANEL_H

//(*Headers(LiquidPanel)
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

class LiquidPanel: public wxPanel
{
    void ValidateWindow();
    // these are only used for validation

	public:

		LiquidPanel(wxWindow* parent);
		virtual ~LiquidPanel();

		//(*Declarations(LiquidPanel)
		wxStaticText* StaticText10;
		wxStaticText* StaticText22;
		wxStaticText* StaticText9;
		wxTextCtrl* TextCtrl_X3;
		ValueCurveButton* BitmapButton_Liquid_SourceSize2;
		wxSlider* Slider_LifeTime;
		wxStaticText* StaticText45;
		ValueCurveButton* BitmapButton_Velocity2;
		wxTextCtrl* TextCtrl_Despeckle;
		wxChoice* Choice_ParticleType;
		wxCheckBox* CheckBox_Enabled2;
		wxSlider* Slider_Velocity2;
		ValueCurveButton* BitmapButton_X1;
		wxSlider* Slider_Direction3;
		ValueCurveButton* BitmapButton_Velocity1;
		wxTextCtrl* TextCtrl_X1;
		wxNotebook* Notebook1;
		ValueCurveButton* BitmapButton_Velocity4;
		wxSlider* Slider_Velocity1;
		wxSlider* Slider_X1;
		ValueCurveButton* BitmapButton_LifeTime;
		wxCheckBox* CheckBox_LeftBarrier;
		wxStaticText* StaticText13;
		wxStaticText* StaticText33;
		wxTextCtrl* TextCtrl_Y2;
		wxSlider* Slider_Y1;
		wxStaticText* StaticText2;
		wxPanel* Panel4;
		wxStaticText* StaticText14;
		ValueCurveButton* BitmapButton_Liquid_SourceSize1;
		wxTextCtrl* TextCtrl_Velocity2;
		ValueCurveButton* BitmapButton_Y4;
		wxTextCtrl* TextCtrl_liquid_SourceSize4;
		wxSlider* Slider_Direction2;
		wxStaticText* StaticText26;
		wxStaticText* StaticText6;
		ValueCurveButton* BitmapButton_Direction3;
		ValueCurveButton* BitmapButton_Y2;
		wxTextCtrl* TextCtrl_Y4;
		wxSlider* Slider_X3;
		wxCheckBox* CheckBox_BottomBarrier;
		wxStaticText* StaticText42;
		wxStaticText* StaticText32;
		wxStaticText* StaticText8;
		wxSlider* Slider_Liquid_SourceSize3;
		ValueCurveButton* BitmapButton_Flow2;
		wxSlider* Slider_Flow2;
		wxStaticText* StaticText11;
		wxTextCtrl* TextCtrl_LifeTime;
		wxSlider* Slider_Liquid_SourceSize4;
		wxCheckBox* CheckBox_RightBarrier;
		wxPanel* Panel1;
		wxStaticText* StaticText1;
		wxSlider* Slider_Y2;
		ValueCurveButton* BitmapButton_X2;
		wxTextCtrl* TextCtrl_Liquid_SourceSize1;
		wxStaticText* StaticText3;
		ValueCurveButton* BitmapButton_X4;
		wxSlider* Slider_Y3;
		ValueCurveButton* BitmapButton_Direction4;
		wxTextCtrl* TextCtrl_Flow3;
		wxTextCtrl* TextCtrl_WarmUpFrames;
		wxSlider* Slider_Direction4;
		wxStaticText* StaticText44;
		wxCheckBox* CheckBox_FlowMusic4;
		wxPanel* Panel3;
		ValueCurveButton* BitmapButton_Liquid_SourceSize3;
		ValueCurveButton* BitmapButton_Flow3;
		wxStaticText* StaticText23;
		wxStaticText* StaticText24;
		ValueCurveButton* BitmapButton_Flow4;
		wxSlider* Slider_Liquid_SourceSize1;
		wxTextCtrl* TextCtrl_Y3;
		ValueCurveButton* BitmapButton_Direction1;
		wxTextCtrl* TextCtrl_Flow4;
		wxTextCtrl* TextCtrl_Velocity3;
		wxTextCtrl* TextCtrl_Flow2;
		wxTextCtrl* TextCtrl_Y1;
		wxCheckBox* CheckBox_TopBarrier;
		wxTextCtrl* TextCtrl_Size;
		wxStaticText* StaticText34;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxSlider* Slider_Velocity3;
		wxSlider* Slider_Y4;
		wxTextCtrl* TextCtrl_Flow1;
		wxCheckBox* CheckBox_FlowMusic2;
		wxSlider* Slider_Flow1;
		ValueCurveButton* BitmapButton_Y3;
		wxSlider* Slider_Flow3;
		wxSlider* Slider_X2;
		ValueCurveButton* BitmapButton_Y1;
		wxSlider* Slider_Size;
		wxStaticText* StaticText43;
		ValueCurveButton* BitmapButton_Velocity3;
		wxTextCtrl* TextCtrl_Direction4;
		wxSlider* Slider_Flow4;
		wxTextCtrl* TextCtrl_Direction3;
		ValueCurveButton* BitmapButton_Direction2;
		wxStaticText* StaticText12;
		wxTextCtrl* TextCtrl_X2;
		wxCheckBox* CheckBox_FlowMusic3;
		wxStaticText* StaticText35;
		wxSlider* Slider_Liquid_SourceSize2;
		wxCheckBox* CheckBox_HoldColor;
		wxTextCtrl* TextCtrl_X4;
		wxSlider* Slider_X4;
		wxPanel* Panel2;
		wxSlider* Slider_Velocity4;
		wxCheckBox* CheckBox_MixColors;
		wxTextCtrl* TextCtrl_Velocity1;
		wxStaticText* StaticText25;
		wxTextCtrl* TextCtrl_Liquid_SourceSize2;
		wxCheckBox* CheckBox_Enabled3;
		wxTextCtrl* TextCtrl_Velocity4;
		wxStaticText* StaticText36;
		wxStaticText* StaticText4;
		wxTextCtrl* TextCtrl_Direction1;
		wxSlider* Slider_Despeckle;
		wxSlider* Slider_Direction1;
		wxCheckBox* CheckBox_Enabled4;
		wxTextCtrl* TextCtrl_Liquid_SourceSize3;
		wxTextCtrl* TextCtrl_Direction2;
		ValueCurveButton* BitmapButton_Flow1;
		wxStaticText* StaticText46;
		ValueCurveButton* BitmapButton_X3;
		wxCheckBox* CheckBox_FlowMusic1;
		wxSlider* Slider_WarmUpFrames;
		ValueCurveButton* BitmapButton_Liquid_SourceSize4;
		//*)

	protected:

		//(*Identifiers(LiquidPanel)
		static const long ID_CHECKBOX_TopBarrier;
		static const long ID_CHECKBOX_BottomBarrier;
		static const long ID_CHECKBOX_LeftBarrier;
		static const long ID_CHECKBOX_RightBarrier;
		static const long ID_CHECKBOX_HoldColor;
		static const long ID_CHECKBOX_MixColors;
		static const long ID_STATICTEXT7;
		static const long ID_CHOICE_ParticleType;
		static const long ID_STATICTEXT1;
		static const long IDD_SLIDER_LifeTime;
		static const long ID_VALUECURVE_LifeTime;
		static const long ID_TEXTCTRL_LifeTime;
		static const long ID_STATICTEXT8;
		static const long IDD_SLIDER_Size;
		static const long ID_TEXTCTRL_Size;
		static const long ID_STATICTEXT9;
		static const long IDD_SLIDER_WarmUpFrames;
		static const long ID_TEXTCTRL_WarmUpFrames;
		static const long ID_STATICTEXT10;
		static const long IDD_SLIDER_Despeckle;
		static const long ID_TEXTCTRL_Despeckle;
		static const long ID_STATICTEXT2;
		static const long IDD_SLIDER_X1;
		static const long ID_VALUECURVE_X1;
		static const long ID_TEXTCTRL_X1;
		static const long ID_STATICTEXT3;
		static const long IDD_SLIDER_Y1;
		static const long ID_VALUECURVE_Y1;
		static const long ID_TEXTCTRL_Y1;
		static const long ID_STATICTEXT4;
		static const long IDD_SLIDER_Direction1;
		static const long ID_VALUECURVE_Direction1;
		static const long ID_TEXTCTRL_Direction1;
		static const long ID_STATICTEXT5;
		static const long IDD_SLIDER_Velocity1;
		static const long ID_VALUECURVE_Velocity1;
		static const long ID_TEXTCTRL_Velocity1;
		static const long ID_STATICTEXT6;
		static const long IDD_SLIDER_Flow1;
		static const long ID_VALUECURVE_Flow1;
		static const long ID_TEXTCTRL_Flow1;
		static const long ID_STATICTEXT11;
		static const long IDD_SLIDER_Liquid_SourceSize1;
		static const long ID_VALUECURVE_Liquid_SourceSize1;
		static const long ID_TEXTCTRL_Liquid_SourceSize1;
		static const long ID_CHECKBOX_FlowMusic1;
		static const long ID_PANEL1;
		static const long ID_CHECKBOX_Enabled2;
		static const long ID_STATICTEXT22;
		static const long IDD_SLIDER_X2;
		static const long ID_VALUECURVE_X2;
		static const long ID_TEXTCTRL_X2;
		static const long ID_STATICTEXT23;
		static const long IDD_SLIDER_Y2;
		static const long ID_VALUECURVE_Y2;
		static const long ID_TEXTCTRL_Y2;
		static const long ID_STATICTEXT24;
		static const long IDD_SLIDER_Direction2;
		static const long ID_VALUECURVE_Direction2;
		static const long ID_TEXTCTRL_Direction2;
		static const long ID_STATICTEXT25;
		static const long IDD_SLIDER_Velocity2;
		static const long ID_VALUECURVE_Velocity2;
		static const long ID_TEXTCTRL_Velocity2;
		static const long ID_STATICTEXT26;
		static const long IDD_SLIDER_Flow2;
		static const long ID_VALUECURVE_Flow2;
		static const long ID_TEXTCTRL_Flow2;
		static const long ID_STATICTEXT12;
		static const long IDD_SLIDER_Liquid_SourceSize2;
		static const long ID_VALUECURVE_Liquid_SourceSize2;
		static const long ID_TEXTCTRL_Liquid_SourceSize2;
		static const long ID_CHECKBOX_FlowMusic2;
		static const long ID_PANEL2;
		static const long ID_CHECKBOX_Enabled3;
		static const long ID_STATICTEXT32;
		static const long IDD_SLIDER_X3;
		static const long ID_VALUECURVE_X3;
		static const long ID_TEXTCTRL_X3;
		static const long ID_STATICTEXT33;
		static const long IDD_SLIDER_Y3;
		static const long ID_VALUECURVE_Y3;
		static const long ID_TEXTCTRL_Y3;
		static const long ID_STATICTEXT34;
		static const long IDD_SLIDER_Direction3;
		static const long ID_VALUECURVE_Direction3;
		static const long ID_TEXTCTRL_Direction3;
		static const long ID_STATICTEXT35;
		static const long IDD_SLIDER_Velocity3;
		static const long ID_VALUECURVE_Velocity3;
		static const long ID_TEXTCTRL_Velocity3;
		static const long ID_STATICTEXT36;
		static const long IDD_SLIDER_Flow3;
		static const long ID_VALUECURVE_Flow3;
		static const long ID_TEXTCTRL_Flow3;
		static const long ID_STATICTEXT13;
		static const long IDD_SLIDER_Liquid_SourceSize3;
		static const long ID_VALUECURVE_Liquid_SourceSize3;
		static const long ID_TEXTCTRL_Liquid_SourceSize3;
		static const long ID_CHECKBOX_FlowMusic3;
		static const long ID_PANEL3;
		static const long ID_CHECKBOX_Enabled4;
		static const long ID_STATICTEXT42;
		static const long IDD_SLIDER_X4;
		static const long ID_VALUECURVE_X4;
		static const long ID_TEXTCTRL_X4;
		static const long ID_STATICTEXT43;
		static const long IDD_SLIDER_Y4;
		static const long ID_VALUECURVE_Y4;
		static const long ID_TEXTCTRL_Y4;
		static const long ID_STATICTEXT44;
		static const long IDD_SLIDER_Direction4;
		static const long ID_VALUECURVE_Direction4;
		static const long ID_TEXTCTRL_Direction4;
		static const long ID_STATICTEXT45;
		static const long IDD_SLIDER_Velocity4;
		static const long ID_VALUECURVE_Velocity4;
		static const long ID_TEXTCTRL_Velocity4;
		static const long ID_STATICTEXT46;
		static const long IDD_SLIDER_Flow4;
		static const long ID_VALUECURVE_Flow4;
		static const long ID_TEXTCTRL_Flow4;
		static const long ID_STATICTEXT14;
		static const long IDD_SLIDER_Liquid_SourceSize4;
		static const long ID_VALUECURVE_Liquid_SourceSize4;
		static const long ID_TEXTCTRL_Liquid_SourceSize4;
		static const long ID_CHECKBOX_FlowMusic4;
		static const long ID_PANEL4;
		static const long ID_NOTEBOOK1;
		//*)

	public:

		//(*Handlers(LiquidPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
		void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
		void UpdateLinkedSliderFloat2(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnCheckBox_EnabledClick(wxCommandEvent& event);
		void OnSlider_Liquid_SourceSize1CmdScroll(wxScrollEvent& event);
		void OnSlider_Liquid_SourceSize3CmdScroll(wxScrollEvent& event);
		void OnSlider_Liquid_SourceSize1CmdScroll1(wxScrollEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
