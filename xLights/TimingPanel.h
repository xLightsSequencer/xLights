#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "BulkEditControls.h"
#include "effects/EffectPanelUtils.h"

//(*Headers(TimingPanel)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class Model;

#define IN_TRANSITION_MIN 0
#define IN_TRANSITION_MAX 100

#define OUT_TRANSITION_MIN 0
#define OUT_TRANSITION_MAX 100

class TimingPanel: public xlEffectPanel
{
    std::string _layersSelected;
    int _startLayer;
    int _endLayer;
    bool _modelBlending;
    std::vector<int> _layerWithEffect;

	public:

		TimingPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~TimingPanel();

        void SetLayersBelow(int start, int end, std::vector<int> effects, bool blending) { _startLayer = start; _endLayer = end; _layerWithEffect = effects; _modelBlending = blending;}
        wxString GetTimingString();
        void SetDefaultControls(const Model *model, bool optionbased = false);
        void ValidateWindow();
        void SetLayersSelected(std::string layersSelected) { _layersSelected = layersSelected; }

		static double GetSettingVCMin(const std::string& name)
        {
            if (name == "T_VALUECURVE_In_Transition_Adjust")
                return IN_TRANSITION_MIN;
            if (name == "T_VALUECURVE_Out_Transition_Adjust")
                return OUT_TRANSITION_MIN;
            wxASSERT(false);
            return 0;
        }

        static double GetSettingVCMax(const std::string& name)
        {
            if (name == "T_VALUECURVE_In_Transition_Adjust")
                return IN_TRANSITION_MAX;
            if (name == "T_VALUECURVE_Out_Transition_Adjust")
                return OUT_TRANSITION_MAX;
            wxASSERT(false);
            return 100;
        }

        static int GetSettingVCDivisor(const std::string& name)
        {
            return 1;
        }

		//(*Declarations(TimingPanel)
		BulkEditCheckBox* CheckBox_Canvas;
		BulkEditCheckBox* CheckBox_In_Reverse;
		BulkEditCheckBox* CheckBox_LayerMorph;
		BulkEditCheckBox* CheckBox_Out_Reverse;
		BulkEditChoice* Choice_In_Transition_Type;
		BulkEditChoice* Choice_LayerMethod;
		BulkEditChoice* Choice_Out_Transition_Type;
		BulkEditComboBox* TextCtrl_Fadein;
		BulkEditComboBox* TextCtrl_Fadeout;
		BulkEditSlider* Slider_EffectLayerMix;
		BulkEditSlider* Slider_In_Adjust;
		BulkEditSlider* Slider_Out_Adjust;
		BulkEditSpinCtrl* SpinCtrl_FreezeEffectAtFrame;
		BulkEditSpinCtrl* SpinCtrl_SuppressEffectUntil;
		BulkEditTextCtrl* TextCtrl_EffectLayerMix;
		BulkEditTextCtrl* TextCtrl_In_Adjust;
		BulkEditTextCtrl* TextCtrl_Out_Adjust;
		BulkEditValueCurveButton* BitmapButton_In_Transition_Adjust;
		BulkEditValueCurveButton* BitmapButton_Out_Transition_Adjust;
		wxButton* Button_About_Layers;
		wxButton* Button_Layers;
		wxCheckBox* CheckBox_ResetTimingPanel;
		wxNotebook* Notebook1;
		wxPanel* Panel_Sizer;
		wxScrolledWindow* ScrolledWindowTiming;
		wxStaticText* InAdjustmentText;
		wxStaticText* OutAdjustmentText;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		xlLockButton* BitmapButton_CheckBox_LayerMorph;
		xlLockButton* BitmapButton_EffectLayerMix;
		//*)

	protected:

		//(*Identifiers(TimingPanel)
		static const long ID_CHECKBOX_ResetTimingPanel;
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL_SuppressEffectUntil;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL_FreezeEffectAtFrame;
		static const long ID_CHECKBOX_LayerMorph;
		static const long ID_SLIDER_EffectLayerMix;
		static const long IDD_TEXTCTRL_EffectLayerMix;
		static const long ID_BITMAPBUTTON_CHECKBOX_LayerMorph;
		static const long ID_CHOICE_LayerMethod;
		static const long ID_BUTTON_ABOUT_LAYERS;
		static const long ID_BITMAPBUTTON_SLIDER_EffectLayerMix;
		static const long ID_CHECKBOX_Canvas;
		static const long ID_BUTTON1;
		static const long ID_CHOICE_In_Transition_Type;
		static const long ID_STATICTEXT_Fadein;
		static const long ID_TEXTCTRL_Fadein;
		static const long ID_STATICTEXT_In_Transition_Adjust;
		static const long ID_SLIDER_In_Transition_Adjust;
		static const long ID_VALUECURVE_In_Transition_Adjust;
		static const long IDD_TEXTCTRL_In_Transition_Adjust;
		static const long ID_CHECKBOX_In_Transition_Reverse;
		static const long ID_PANEL2;
		static const long ID_CHOICE_Out_Transition_Type;
		static const long ID_STATICTEXT_Fadeout;
		static const long ID_TEXTCTRL_Fadeout;
		static const long ID_STATICTEXT_Out_Transition_Adjust;
		static const long ID_SLIDER_Out_Transition_Adjust;
		static const long ID_VALUECURVE_Out_Transition_Adjust;
		static const long IDD_TEXTCTRL_Out_Transition_Adjust;
		static const long ID_CHECKBOX_Out_Transition_Reverse;
		static const long ID_PANEL3;
		static const long IDD_NOTEBOOK1;
		static const long ID_SCROLLEDWINDOW1;
		static const long ID_PANEL1;
		//*)

	private:

		//(*Handlers(TimingPanel)
		void OnResize(wxSizeEvent& event);
		void OnTransitionTypeSelect(wxCommandEvent& event);
		void OnCheckBox_ResetTimingPanelClick(wxCommandEvent& event);
		void OnButton_LayersClick(wxCommandEvent& event);
		void OnChoice_LayerMethodSelect(wxCommandEvent& event);
		void OnCheckBox_CanvasClick(wxCommandEvent& event);
		void OnButton_AboutClick(wxCommandEvent& event);
		void OnTextCtrl_FadeinText(wxCommandEvent& event);
		void OnTextCtrl_FadeoutText(wxCommandEvent& event);
		void OnTextCtrl_FadeinDropdown(wxCommandEvent& event);
		void OnTextCtrl_FadeoutDropdown(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
