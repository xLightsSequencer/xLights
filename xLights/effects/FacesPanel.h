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

#include "../BulkEditControls.h"

class FacesPanel: public wxPanel
{
	public:

		FacesPanel(wxWindow* parent);
		virtual ~FacesPanel();

		//(*Declarations(FacesPanel)
		BulkEditCheckBox* CheckBox_Faces_Outline;
		wxRadioButton* RadioButton1;
		wxChoice* Choice_Faces_Phoneme;
		wxStaticText* StaticText14;
		wxRadioButton* RadioButton2;
		wxChoice* Choice_Faces_TimingTrack;
		wxStaticText* StaticText71;
		wxChoice* Face_FaceDefinitonChoice;
		BulkEditChoice* Choice_Faces_Eyes;
		//*)

	protected:

		//(*Identifiers(FacesPanel)
		static const long IDD_RADIOBUTTON_Faces_Phoneme;
		static const long ID_CHOICE_Faces_Phoneme;
		static const long IDD_RADIOBUTTON_Faces_TimingTrack;
		static const long ID_CHOICE_Faces_TimingTrack;
		static const long ID_STATICTEXT15;
		static const long ID_CHOICE_Faces_FaceDefinition;
		static const long ID_STATICTEXT_Faces_Eyes;
		static const long ID_CHOICE_Faces_Eyes;
		static const long ID_CHECKBOX_Faces_Outline;
		//*)

	public:

		//(*Handlers(FacesPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnMouthMovementTypeSelected(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
