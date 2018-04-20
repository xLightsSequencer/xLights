#ifndef TIMINGPANEL_H
#define TIMINGPANEL_H

#include "BulkEditControls.h"

//(*Headers(TimingPanel)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class Model;

class TimingPanel: public wxPanel
{
    std::string _layersSelected;
    int _startLayer;
    int _endLayer;

	public:

		TimingPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~TimingPanel();

        void SetLayersBelow(int start, int end) { _startLayer = start; _endLayer = end; }
        wxString GetTimingString();
        void SetDefaultControls(const Model *model, bool optionbased = false);
        void ValidateWindow();
        void SetLayersSelected(std::string layersSelected) { _layersSelected = layersSelected; }

		//(*Declarations(TimingPanel)
		BulkEditCheckBox* CheckBox_In_Reverse;
		BulkEditCheckBox* CheckBox_Out_Reverse;
		BulkEditChoice* Choice_In_Transition_Type;
		BulkEditChoice* Choice_LayerMethod;
		BulkEditChoice* Choice_Out_Transition_Type;
		BulkEditSlider* Slider_EffectLayerMix;
		BulkEditSlider* Slider_In_Adjust;
		BulkEditSlider* Slider_Out_Adjust;
		BulkEditTextCtrl* TextCtrl_EffectLayerMix;
		BulkEditTextCtrl* TextCtrl_Fadein;
		BulkEditTextCtrl* TextCtrl_Fadeout;
		BulkEditTextCtrl* TextCtrl_In_Adjust;
		BulkEditTextCtrl* TextCtrl_Out_Adjust;
		wxBitmapButton* BitmapButton_CheckBox_LayerMorph;
		wxBitmapButton* BitmapButton_EffectLayerMix;
		wxButton* Button_Layers;
		wxCheckBox* CheckBox_LayerMorph;
		wxCheckBox* CheckBox_ResetTimingPanel;
		wxNotebook* Notebook1;
		wxPanel* Panel_Sizer;
		wxScrolledWindow* ScrolledWindowTiming;
		wxStaticText* InAdjustmentText;
		wxStaticText* OutAdjustmentText;
		wxStaticText* StaticText2;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(TimingPanel)
		static const long ID_CHECKBOX_ResetTimingPanel;
		static const long ID_CHECKBOX_LayerMorph;
		static const long ID_SLIDER_EffectLayerMix;
		static const long IDD_TEXTCTRL_EffectLayerMix;
		static const long ID_BITMAPBUTTON_CHECKBOX_LayerMorph;
		static const long ID_CHOICE_LayerMethod;
		static const long ID_BUTTON1;
		static const long ID_BITMAPBUTTON_SLIDER_EffectLayerMix;
		static const long ID_CHOICE_In_Transition_Type;
		static const long ID_STATICTEXT_Fadein;
		static const long ID_TEXTCTRL_Fadein;
		static const long ID_STATICTEXT_In_Transition_Adjust;
		static const long ID_SLIDER_In_Transition_Adjust;
		static const long IDD_TEXTCTRL_In_Transition_Adjust;
		static const long ID_CHECKBOX_In_Transition_Reverse;
		static const long ID_PANEL2;
		static const long ID_CHOICE_Out_Transition_Type;
		static const long ID_STATICTEXT_Fadeout;
		static const long ID_TEXTCTRL_Fadeout;
		static const long ID_STATICTEXT_Out_Transition_Adjust;
		static const long ID_SLIDER_Out_Transition_Adjust;
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
		void OnLockButtonClick(wxCommandEvent& event);
		void OnTransitionTypeSelect(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnCheckBox_ResetTimingPanelClick(wxCommandEvent& event);
		void OnButton_LayersClick(wxCommandEvent& event);
		void OnChoice_LayerMethodSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
