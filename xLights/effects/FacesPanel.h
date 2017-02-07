#ifndef FACESPANEL_H
#define FACESPANEL_H

//(*Headers(FacesPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxRadioButton;
class wxStaticText;
class wxFlexGridSizer;
class wxStaticBoxSizer;
class wxChoice;
//*)

class FacesPanel: public wxPanel
{
	public:

		FacesPanel(wxWindow* parent);
		virtual ~FacesPanel();

		//(*Declarations(FacesPanel)
		wxRadioButton* RadioButton1;
		wxChoice* Choice_Faces_Phoneme;
		wxChoice* Choice_Faces_Eyes;
		wxStaticText* StaticText14;
		wxRadioButton* RadioButton2;
		wxChoice* Choice_Faces_TimingTrack;
		wxStaticText* StaticText71;
		wxCheckBox* CheckBox_Faces_Outline;
		wxChoice* Face_FaceDefinitonChoice;
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
    void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
    void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    void UpdateLinkedSliderFloat2(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
