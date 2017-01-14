#ifndef PLAYLISTITEMAUDIOPANEL_H
#define PLAYLISTITEMAUDOPANEL_H

//(*Headers(PlayListItemAudioPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/filepicker.h>
#include <wx/spinctrl.h>
#include <wx/slider.h>
#include <wx/panel.h>
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
		wxSlider* Slider1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Delay;
		wxSpinCtrl* SpinCtrl_Priority;
		AudioFilePickerCtrl* FilePickerCtrl_AudioFile;
		wxStaticText* StaticText4;
		wxCheckBox* CheckBox_OverrideVolume;
		//*)

	protected:

		//(*Identifiers(PlayListItemAudioPanel)
		static const long ID_STATICTEXT2;
		static const long ID_FILEPICKERCTRL2;
		static const long ID_CHECKBOX2;
		static const long ID_SLIDER1;
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

#endif
