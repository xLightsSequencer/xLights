#ifndef FACESPANEL_H
#define FACESPANEL_H

//(*Headers(FacesPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxRadioButton;
class wxSlider;
class wxStaticBoxSizer;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class FacesPanel: public wxPanel
{
	public:

		FacesPanel(wxWindow* parent);
		virtual ~FacesPanel();

		//(*Declarations(FacesPanel)
		BulkEditCheckBox* CheckBox_Faces_Outline;
		BulkEditCheckBox* CheckBox_SuppressWhenNotSinging;
		BulkEditCheckBox* CheckBox_TransparentBlack;
		BulkEditChoice* Choice_Faces_Eyes;
		BulkEditChoice* Choice_Faces_TimingTrack;
		BulkEditFaceChoice* Face_FaceDefinitonChoice;
		BulkEditSlider* Slider_Faces_TransparentBlack;
		BulkEditTextCtrl* TextCtrl_Faces_TransparentBlack;
		wxChoice* Choice_Faces_Phoneme;
		wxRadioButton* RadioButton1;
		wxRadioButton* RadioButton2;
		wxStaticText* StaticText14;
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
		static const long ID_STATICTEXT_Faces_Eyes;
		static const long ID_CHOICE_Faces_Eyes;
		static const long ID_CHECKBOX_Faces_Outline;
		static const long ID_CHECKBOX_Faces_SuppressWhenNotSinging;
		static const long ID_CHECKBOX_Faces_TransparentBlack;
		static const long IDD_SLIDER_Faces_TransparentBlack;
		static const long ID_TEXTCTRL_Faces_TransparentBlack;
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
