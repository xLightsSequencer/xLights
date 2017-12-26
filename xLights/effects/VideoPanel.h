#ifndef VIDEOPANEL_H
#define VIDEOPANEL_H

//(*Headers(VideoPanel)
#include <wx/panel.h>
class wxFilePickerCtrl;
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxFlexGridSizer;
class wxChoice;
//*)

#include <wx/filepicker.h>
#include <mutex>
#include <map>

#include "../BulkEditControls.h"

#define VIDEOWILDCARD "Video Files|*.avi;*.mp4;*.mkv;*.mov;*.asf;*.flv;*.mpg;*.mpeg;*.m4v"

class xlVideoFilePickerCtrl : public wxFilePickerCtrl {
public:
	xlVideoFilePickerCtrl(wxWindow *parent,
		wxWindowID id,
		const wxString& path = wxEmptyString,
		const wxString& message = wxFileSelectorPromptStr,
		const wxString& wildcard = wxFileSelectorDefaultWildcardStr,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxFLP_DEFAULT_STYLE,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = wxFilePickerCtrlNameStr)
		: wxFilePickerCtrl(parent, id, path, message, VIDEOWILDCARD, pos, size, style, validator, name) {

	}
	virtual ~xlVideoFilePickerCtrl() {}
};

class VideoPanel: public wxPanel
{
    void ValidateWindow();

	public:

		VideoPanel(wxWindow* parent);
		virtual ~VideoPanel();
        void addVideoTime(std::string fn, unsigned long ms);

        //(*Declarations(VideoPanel)
        BulkEditTextCtrl* TextCtrl_Video_CropBottom;
        wxStaticText* StaticText2;
        BulkEditSlider* Slider_Video_CropTop;
        BulkEditChoice* Choice_Video_DurationTreatment;
        BulkEditCheckBox* CheckBox_Video_AspectRatio;
        wxStaticText* StaticText8;
        BulkEditSlider* Slider_Video_CropRight;
        wxStaticText* StaticText1;
        wxStaticText* StaticText3;
        BulkEditTextCtrlF2* TextCtrl_Video_Starttime;
        BulkEditSlider* Slider_Video_CropBottom;
        xlVideoFilePickerCtrl* FilePicker_Video_Filename;
        wxStaticText* StaticText5;
        BulkEditSliderF2* Slider_Video_Starttime;
        BulkEditCheckBox* CheckBox_SynchroniseWithAudio;
        BulkEditTextCtrl* TextCtrl_Video_CropTop;
        wxStaticText* StaticText4;
        BulkEditSlider* Slider_Video_CropLeft;
        BulkEditTextCtrl* TextCtrl_Video_CropLeft;
        BulkEditTextCtrl* TextCtrl_Video_CropRight;
        //*)

protected:

        std::mutex lock;
        std::map<std::string, unsigned long> videoTimeCache;
    
		//(*Identifiers(VideoPanel)
		static const long ID_FILEPICKERCTRL_Video_Filename;
		static const long ID_STATICTEXT_Video_Starttime;
		static const long IDD_SLIDER_Video_Starttime;
		static const long ID_TEXTCTRL_Video_Starttime;
		static const long ID_STATICTEXT_Video_DurationTreatment;
		static const long ID_CHOICE_Video_DurationTreatment;
		static const long ID_CHECKBOX_Video_AspectRatio;
		static const long ID_CHECKBOX_SynchroniseWithAudio;
		static const long ID_STATICTEXT_Video_CropLeft;
		static const long IDD_SLIDER_Video_CropLeft;
		static const long ID_TEXTCTRL_Video_CropLeft;
		static const long ID_STATICTEXT_Video_CropRight;
		static const long IDD_SLIDER_Video_CropRight;
		static const long ID_TEXTCTRL_Video_CropRight;
		static const long ID_STATICTEXT_Video_CropTop;
		static const long IDD_SLIDER_Video_CropTop;
		static const long ID_TEXTCTRL_Video_CropTop;
		static const long ID_STATICTEXT_Video_CropBottom;
		static const long IDD_SLIDER_Video_CropBottom;
		static const long ID_TEXTCTRL_Video_CropBottom;
		//*)

	public:

		//(*Handlers(VideoPanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnSlider_Video_StarttimeCmdSliderUpdated(wxScrollEvent& event);
		void OnFilePicker_Video_FilenameFileChanged(wxFileDirPickerEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnCheckBox_SynchroniseWithAudioClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
