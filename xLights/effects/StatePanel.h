#ifndef STATEPANEL_H
#define STATEPANEL_H

//(*Headers(StatePanel)
#include <wx/panel.h>
class wxRadioButton;
class wxStaticText;
class wxFlexGridSizer;
class wxStaticBoxSizer;
class wxChoice;
//*)

class StateEffect;
class Model;

class StatePanel: public wxPanel
{
    StateEffect* _effect;
    Model* _model;
    void UpdateStateList();

	public:
        void SetEffect(StateEffect* effect, Model* model);

		StatePanel(wxWindow* parent);
		virtual ~StatePanel();

		//(*Declarations(StatePanel)
		wxRadioButton* RadioButton1;
		wxChoice* Choice_State_Mode;
		wxChoice* Choice_State_State;
		wxStaticText* StaticText2;
		wxChoice* Choice_State_TimingTrack;
		wxStaticText* StaticText14;
		wxRadioButton* RadioButton2;
		wxChoice* Choice_StateDefinitonChoice;
		wxStaticText* StaticText1;
		wxChoice* Choice_State_Color;
		//*)

	protected:

		//(*Identifiers(StatePanel)
		static const long ID_STATICTEXT15;
		static const long ID_CHOICE_State_StateDefinition;
		static const long IDD_RADIOBUTTON_State_State;
		static const long ID_CHOICE_State_State;
		static const long IDD_RADIOBUTTON_State_TimingTrack;
		static const long ID_CHOICE_State_TimingTrack;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_State_Mode;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE_State_Color;
		//*)

	public:

		//(*Handlers(StatePanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnMouthMovementTypeSelected(wxCommandEvent& event);
		void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
		void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnState_StateDefinitonChoiceSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
