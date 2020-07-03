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

 //(*Headers(PlayListItemAudioPanel)
#include <wx/checkbox.h>
#include <wx/filepicker.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemAudio;
class AudioFilePickerCtrl;

class PlayListItemAudioPanel: public wxPanel
{
    PlayListItemAudio* _audio;
    void ValidateWindow();

	public:

		PlayListItemAudioPanel(wxWindow* parent, PlayListItemAudio* audio, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemAudioPanel();

		//(*Declarations(PlayListItemAudioPanel)
		AudioFilePickerCtrl* FilePickerCtrl_AudioFile;
		wxCheckBox* CheckBox_FastStartAudio;
		wxCheckBox* CheckBox_OverrideVolume;
		wxSlider* Slider1;
		wxSpinCtrl* SpinCtrl_Priority;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxTextCtrl* TextCtrl_Delay;
		//*)

	protected:

		//(*Identifiers(PlayListItemAudioPanel)
		static const long ID_STATICTEXT2;
		static const long ID_FILEPICKERCTRL2;
		static const long ID_CHECKBOX2;
		static const long ID_SLIDER1;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(PlayListItemAudioPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnFilePickerCtrl2FileChanged(wxFileDirPickerEvent& event);
		void OnCheckBox_OverrideVolumeClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

