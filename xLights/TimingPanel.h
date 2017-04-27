#ifndef TIMINGPANEL_H
#define TIMINGPANEL_H

//(*Headers(TimingPanel)
#include <wx/scrolwin.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
//*)

class Model;

class TimingPanel: public wxPanel
{
	public:

		TimingPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~TimingPanel();

        wxString GetTimingString();

		//(*Declarations(TimingPanel)
		wxChoice* Choice_In_Transition_Type;
		wxStaticText* InAdjustmentText;
		wxTextCtrl* TextCtrl_In_Adjust;
		wxNotebook* Notebook1;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_Out_Adjust;
		wxPanel* Panel_Sizer;
		wxTextCtrl* TextCtrl_Fadein;
		wxSlider* Slider_EffectLayerMix;
		wxCheckBox* CheckBox_Out_Reverse;
		wxBitmapButton* BitmapButton_CheckBox_LayerMorph;
		wxCheckBox* CheckBox_LayerMorph;
		wxCheckBox* CheckBox_ResetTimingPanel;
		wxStaticText* OutAdjustmentText;
		wxChoice* Choice_Out_Transition_Type;
		wxChoice* Choice_LayerMethod;
		wxBitmapButton* BitmapButton_EffectLayerMix;
		wxCheckBox* CheckBox_In_Reverse;
		wxTextCtrl* TextCtrl_Fadeout;
		wxScrolledWindow* ScrolledWindowTiming;
		wxSlider* Slider_Out_Adjust;
		wxStaticText* StaticText4;
		wxTextCtrl* txtCtlEffectMix;
		wxSlider* Slider_In_Adjust;
		//*)
    
    
        void SetDefaultControls(const Model *model, bool optionbased = false);

	protected:

		//(*Identifiers(TimingPanel)
		static const long ID_CHECKBOX_ResetTimingPanel;
		static const long ID_CHECKBOX_LayerMorph;
		static const long ID_BITMAPBUTTON_CHECKBOX_LayerMorph;
		static const long ID_CHOICE_LayerMethod;
		static const long ID_SLIDER_EffectLayerMix;
		static const long IDD_TEXTCTRL_EffectLayerMix;
		static const long ID_BITMAPBUTTON_SLIDER_EffectLayerMix;
		static const long ID_CHOICE_In_Transition_Type;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL_Fadein;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_In_Transition_Adjust;
		static const long IDD_TEXTCTRL_In_Transition_Adjust;
		static const long ID_CHECKBOX_In_Transition_Reverse;
		static const long ID_PANEL2;
		static const long ID_CHOICE_Out_Transition_Type;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL_Fadeout;
		static const long ID_STATICTEXT5;
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
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
		void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
		void OnTransitionTypeSelect(wxCommandEvent& event);
		void OnEffectTimeChange(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void UpdateLinkedSliderFloat2(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
		void OnCheckBox_ResetTimingPanelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
