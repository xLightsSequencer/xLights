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

//(*Headers(PlayListItemFSEQVideoPanel)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class OutputManager;
class PlayListItemFSEQVideo;
class FSEQFilePickerCtrl;
class AudioFilePickerCtrl;
class VideoFilePickerCtrl;

class PlayListItemFSEQVideoPanel: public wxPanel
{
    OutputManager* _outputManager;
    PlayListItemFSEQVideo* _fseq;
    void ValidateWindow();
    void SetWindowPositionText();

	public:

		PlayListItemFSEQVideoPanel(wxWindow* parent, OutputManager* outputManager, PlayListItemFSEQVideo* fseq, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemFSEQVideoPanel();

		//(*Declarations(PlayListItemFSEQVideoPanel)
		AudioFilePickerCtrl* FilePickerCtrl_AudioFile;
		FSEQFilePickerCtrl* FilePickerCtrl_FSEQFile;
		VideoFilePickerCtrl* FilePickerCtrl_VideoFile;
		wxButton* Button_PositionWindow;
		wxCheckBox* CheckBox_CacheVideo;
		wxCheckBox* CheckBox_FastStartAudio;
		wxCheckBox* CheckBox_LimitChannels;
		wxCheckBox* CheckBox_LoopVideo;
		wxCheckBox* CheckBox_OverrideAudio;
		wxCheckBox* CheckBox_OverrideVolume;
		wxCheckBox* CheckBox_SuppressVirtualMatrix;
		wxCheckBox* CheckBox_Topmost;
		wxCheckBox* CheckBox_UseMediaPlayer;
		wxChoice* Choice_BlendMode;
		wxSlider* Slider1;
		wxSpinCtrl* SpinCtrl_Channels;
		wxSpinCtrl* SpinCtrl_FadeIn;
		wxSpinCtrl* SpinCtrl_FadeOut;
		wxSpinCtrl* SpinCtrl_Priority;
		wxStaticText* StaticText10;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		wxStaticText* StaticText_Position;
		wxStaticText* StaticText_StartChannel;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_StartChannel;
		//*)

	protected:

		//(*Identifiers(PlayListItemFSEQVideoPanel)
		static const long ID_STATICTEXT1;
		static const long ID_FILEPICKERCTRL1;
		static const long ID_STATICTEXT8;
		static const long ID_FILEPICKERCTRL3;
		static const long ID_BUTTON1;
		static const long ID_STATICTEXT11;
		static const long ID_CHECKBOX5;
		static const long ID_CHECKBOX6;
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE1;
		static const long ID_CHECKBOX3;
		static const long ID_STATICTEXT6;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT12;
		static const long ID_STATICTEXT7;
		static const long ID_SPINCTRL3;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT2;
		static const long ID_FILEPICKERCTRL2;
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX7;
		static const long ID_CHECKBOX8;
		static const long ID_CHECKBOX9;
		static const long ID_CHECKBOX2;
		static const long ID_SLIDER1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT10;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT9;
		static const long ID_SPINCTRL5;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(PlayListItemFSEQVideoPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event);
		void OnFilePickerCtrl2FileChanged(wxFileDirPickerEvent& event);
		void OnCheckBox_OverrideAudioClick(wxCommandEvent& event);
		void OnCheckBox_OverrideVolumeClick(wxCommandEvent& event);
		void OnCheckBox_LimitChannelsClick(wxCommandEvent& event);
		void OnFilePickerCtrl_VideoFileFileChanged(wxFileDirPickerEvent& event);
		void OnButton_PositionWindowClick(wxCommandEvent& event);
		void OnTextCtrl_StartChannelText(wxCommandEvent& event);
		void OnCheckBox_UseMediaPlayerClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
