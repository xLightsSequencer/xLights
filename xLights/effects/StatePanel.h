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

class StatePanel: public wxPanel
{
	public:

		StatePanel(wxWindow* parent);
		virtual ~StatePanel();

		//(*Declarations(StatePanel)
		wxRadioButton* RadioButton1;
		wxChoice* Choice_State_State;
		wxChoice* Choice_State_TimingTrack;
		wxStaticText* StaticText14;
		wxRadioButton* RadioButton2;
		wxChoice* State_StateDefinitonChoice;
		//*)

	protected:

		//(*Identifiers(StatePanel)
		static const long ID_STATICTEXT15;
		static const long ID_CHOICE_State_StateDefinition;
		static const long IDD_RADIOBUTTON_State_State;
		static const long ID_CHOICE_State_State;
		static const long IDD_RADIOBUTTON_State_TimingTrack;
		static const long ID_CHOICE_State_TimingTrack;
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

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
