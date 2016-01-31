#ifndef FACESPANEL_H
#define FACESPANEL_H

//(*Headers(FacesPanel)
#include <wx/panel.h>
class wxStaticBoxSizer;
class wxChoice;
class wxFlexGridSizer;
class wxStaticText;
class wxRadioButton;
class wxCheckBox;
//*)

class FacesPanel: public wxPanel
{
	public:

		FacesPanel(wxWindow* parent);
		virtual ~FacesPanel();

		//(*Declarations(FacesPanel)
		wxChoice* Choice_Faces_Phoneme;
		wxRadioButton* RadioButton1;
		wxStaticText* StaticText14;
		wxChoice* Choice_Faces_Eyes;
		wxRadioButton* RadioButton2;
		wxChoice* Face_FaceDefinitonChoice;
		wxChoice* Choice_Faces_TimingTrack;
		wxCheckBox* CheckBox_Faces_Outline;
		wxStaticText* StaticText71;
		//*)

	protected:

		//(*Identifiers(FacesPanel)
		static const long IDD_RADIOBUTTON_Faces_Phoneme;
		static const long ID_CHOICE_Faces_Phoneme;
		static const long IDD_RADIOBUTTON_Faces_TimingTrack;
		static const long ID_CHOICE_Faces_TimingTrack;
		static const long ID_STATICTEXT15;
		static const long ID_CHOICE_Faces_FaceDefinition;
		static const long ID_STATICTEXT66;
		static const long ID_CHOICE_Faces_Eyes;
		static const long ID_CHECKBOX_Faces_Outline;
		//*)

	public:

		//(*Handlers(FacesPanel)
    void UpdateLinkedSliderFloat(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    void UpdateLinkedSlider360(wxCommandEvent& event);
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);
    void OnMouthMovementTypeSelected(wxCommandEvent& event);

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
