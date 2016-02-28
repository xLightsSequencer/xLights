#ifndef TIMINGPANEL_H
#define TIMINGPANEL_H

//(*Headers(TimingPanel)
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
//*)

class Model;

class TimingPanel: public wxPanel
{
	public:

		TimingPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~TimingPanel();

        wxString GetTimingString();

		//(*Declarations(TimingPanel)
		wxTextCtrl* TextCtrl_Out_Adjust;
		wxSlider* Slider_Out_Adjust;
		wxChoice* Choice_In_Transition_Type;
		wxSlider* Slider_EffectLayerMix;
		wxTextCtrl* TextCtrl_In_Adjust;
		wxTextCtrl* TextCtrl_Fadeout;
		wxStaticText* InAdjustmentText;
		wxPanel* Panel_Sizer;
		wxStaticText* OutAdjustmentText;
		wxSlider* Slider_In_Adjust;
		wxChoice* Choice_Out_Transition_Type;
		wxCheckBox* CheckBox_Out_Reverse;
		wxTextCtrl* TextCtrl_Fadein;
		wxTextCtrl* txtCtlEffectMix;
		wxStaticText* StaticText4;
		wxCheckBox* CheckBox_LayerMorph;
		wxStaticText* StaticText2;
		wxNotebook* Notebook1;
		wxBitmapButton* BitmapButton_EffectLayerMix;
		wxChoice* Choice_LayerMethod;
		wxCheckBox* CheckBox_In_Reverse;
		wxScrolledWindow* ScrolledWindowTiming;
		wxBitmapButton* BitmapButton_CheckBox_LayerMorph;
		//*)
    
    
        void SetDefaultControls(const Model *model);

	protected:

		//(*Identifiers(TimingPanel)
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
		void OnTransitionTypeSelect(wxCommandEvent& event);
		void OnEffectTimeChange(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
