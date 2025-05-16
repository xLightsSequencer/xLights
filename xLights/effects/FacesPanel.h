#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
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
		virtual void ValidateWindow() override;

		//(*Declarations(FacesPanel)
		BulkEditCheckBox* CheckBox_Faces_Outline;
		BulkEditCheckBox* CheckBox_Fade;
		BulkEditCheckBox* CheckBox_SuppressShimmer;
		BulkEditCheckBox* CheckBox_SuppressWhenNotSinging;
		BulkEditCheckBox* CheckBox_TransparentBlack;
		BulkEditChoice* Choice_Faces_EyeBlinkDuration;
		BulkEditChoice* Choice_Faces_EyeBlinkFrequency;
		BulkEditChoice* Choice_Faces_Eyes;
		BulkEditChoice* Choice_Faces_Phoneme;
		BulkEditChoice* Choice_Faces_TimingTrack;
		BulkEditFaceChoice* Face_FaceDefinitonChoice;
		BulkEditSlider* Slider_Faces_TransparentBlack;
		BulkEditSpinCtrl* SpinCtrl_LeadFrames;
		BulkEditStateChoice* Choice1;
		BulkEditTextCtrl* TextCtrl_Faces_TransparentBlack;
		wxRadioButton* RadioButton1;
		wxRadioButton* RadioButton2;
		wxStaticText* StaticText14;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText71;
		wxStaticText* StaticText_Faces_EyeBlinkFrequency;
		//*)

	protected:

		//(*Identifiers(FacesPanel)
		static const wxWindowID IDD_RADIOBUTTON_Faces_Phoneme;
		static const wxWindowID ID_CHOICE_Faces_Phoneme;
		static const wxWindowID IDD_RADIOBUTTON_Faces_TimingTrack;
		static const wxWindowID ID_CHOICE_Faces_TimingTrack;
		static const wxWindowID ID_STATICTEXT15;
		static const wxWindowID ID_CHOICE_Faces_FaceDefinition;
		static const wxWindowID ID_STATICTEXT_Faces_Eyes;
		static const wxWindowID ID_CHOICE_Faces_Eyes;
		static const wxWindowID ID_STATICTEXT_EYEBLINKFREQUENCY;
		static const wxWindowID ID_STATICTEXT3;
		static const wxWindowID ID_CHOICE_Faces_EyeBlinkFrequency;
		static const wxWindowID ID_STATICTEXT2;
		static const wxWindowID ID_CHOICE_Faces_EyeBlinkDuration;
		static const wxWindowID ID_CHECKBOX_Faces_Outline;
		static const wxWindowID ID_CHECKBOX_Faces_SuppressShimmer;
		static const wxWindowID ID_STATICTEXT1;
		static const wxWindowID ID_CHOICE_Faces_UseState;
		static const wxWindowID ID_CHECKBOX_Faces_SuppressWhenNotSinging;
		static const wxWindowID ID_STATICTEXT_Faces_Lead_Frames;
		static const wxWindowID ID_SPINCTRL_Faces_LeadFrames;
		static const wxWindowID ID_CHECKBOX_Faces_Fade;
		static const wxWindowID ID_CHECKBOX_Faces_TransparentBlack;
		static const wxWindowID IDD_SLIDER_Faces_TransparentBlack;
		static const wxWindowID ID_TEXTCTRL_Faces_TransparentBlack;
		//*)

	public:

		//(*Handlers(FacesPanel)
		void OnMouthMovementTypeSelected(wxCommandEvent& event);
		void OnCheckBox_SuppressWhenNotSingingClick(wxCommandEvent& event);
		void OnCheckBox_FadeClick(wxCommandEvent& event);
		void OnCheckBox_Faces_OutlineClick(wxCommandEvent& event);
		void OnChoice_Faces_EyeBlinkFrequencySelect(wxCommandEvent& event);
		void OnChoice_Faces_EyesSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
