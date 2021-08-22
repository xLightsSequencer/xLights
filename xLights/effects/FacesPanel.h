#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*Headers(FacesPanel)
 #include <wx/panel.h>
 class wxCheckBox;
 class wxChoice;
 class wxFlexGridSizer;
 class wxRadioButton;
 class wxSlider;
 class wxSpinCtrl;
 class wxSpinEvent;
 class wxStaticBoxSizer;
 class wxStaticText;
 class wxTextCtrl;
 //*)

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class FacesPanel: public xlEffectPanel
{
	public:

		FacesPanel(wxWindow* parent);
		virtual ~FacesPanel();
		void ValidateWindow();

		//(*Declarations(FacesPanel)
		BulkEditCheckBox* CheckBox_Faces_Outline;
		BulkEditCheckBox* CheckBox_SuppressWhenNotSinging;
		BulkEditCheckBox* CheckBox_TransparentBlack;
		BulkEditChoice* Choice_Faces_Eyes;
		BulkEditChoice* Choice_Faces_TimingTrack;
		BulkEditFaceChoice* Face_FaceDefinitonChoice;
		BulkEditSlider* Slider_Faces_TransparentBlack;
		BulkEditTextCtrl* TextCtrl_Faces_TransparentBlack;
		wxCheckBox* CheckBox_Fade;
		wxChoice* Choice_Faces_Phoneme;
		wxRadioButton* RadioButton1;
		wxRadioButton* RadioButton2;
		wxSpinCtrl* SpinCtrl_LeadFrames;
		wxStaticText* StaticText14;
		wxStaticText* StaticText1;
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
		static const long ID_STATICTEXT_Faces_Lead_Frames;
		static const long ID_SPINCTRL_Faces_LeadFrames;
		static const long ID_CHECKBOX_Faces_Fade;
		static const long ID_CHECKBOX_Faces_TransparentBlack;
		static const long IDD_SLIDER_Faces_TransparentBlack;
		static const long ID_TEXTCTRL_Faces_TransparentBlack;
		//*)

	public:

		//(*Handlers(FacesPanel)
		void OnMouthMovementTypeSelected(wxCommandEvent& event);
		void OnCheckBox_SuppressWhenNotSingingClick(wxCommandEvent& event);
		void OnCheckBox_FadeClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
