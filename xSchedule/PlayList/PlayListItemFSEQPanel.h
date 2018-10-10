#ifndef PLAYLISTITEMFSEQPANEL_H
#define PLAYLISTITEMFSEQPANEL_H

//(*Headers(PlayListItemFSEQPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/filepicker.h>
#include <wx/spinctrl.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class OutputManager;
class PlayListItemFSEQ;
class FSEQFilePickerCtrl;
class AudioFilePickerCtrl;

class PlayListItemFSEQPanel: public wxPanel
{
    OutputManager* _outputManager;
    PlayListItemFSEQ* _fseq;
    void ValidateWindow();

	public:

		PlayListItemFSEQPanel(wxWindow* parent, OutputManager* outputManager, PlayListItemFSEQ* fseq, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemFSEQPanel();

		//(*Declarations(PlayListItemFSEQPanel)
		wxSlider* Slider1;
		wxStaticText* StaticText_StartChannel;
		wxChoice* Choice_BlendMode;
		wxSpinCtrl* SpinCtrl_Channels;
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		wxCheckBox* CheckBox_FastStartAudio;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxCheckBox* CheckBox_LimitChannels;
		wxTextCtrl* TextCtrl_Delay;
		wxSpinCtrl* SpinCtrl_Priority;
		wxCheckBox* CheckBox_OverrideAudio;
		AudioFilePickerCtrl* FilePickerCtrl_AudioFile;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		FSEQFilePickerCtrl* FilePickerCtrl_FSEQFile;
		wxTextCtrl* TextCtrl_StartChannel;
		wxStaticText* StaticText4;
		wxCheckBox* CheckBox_OverrideVolume;
		//*)

	protected:

		//(*Identifiers(PlayListItemFSEQPanel)
		static const long ID_STATICTEXT1;
		static const long ID_FILEPICKERCTRL1;
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE1;
		static const long ID_CHECKBOX3;
		static const long ID_STATICTEXT6;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT8;
		static const long ID_STATICTEXT7;
		static const long ID_SPINCTRL3;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT2;
		static const long ID_FILEPICKERCTRL2;
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX2;
		static const long ID_SLIDER1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(PlayListItemFSEQPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event);
		void OnFilePickerCtrl2FileChanged(wxFileDirPickerEvent& event);
		void OnCheckBox_OverrideAudioClick(wxCommandEvent& event);
		void OnCheckBox_OverrideVolumeClick(wxCommandEvent& event);
		void OnCheckBox_LimitChannelsClick(wxCommandEvent& event);
		void OnTextCtrl_StartChannelText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
