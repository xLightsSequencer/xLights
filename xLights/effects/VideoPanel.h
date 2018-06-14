#ifndef VIDEOPANEL_H
#define VIDEOPANEL_H

//(*Headers(VideoPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxChoice;
class wxFilePickerCtrl;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include <wx/filepicker.h>
#include <mutex>
#include <map>

#include "../BulkEditControls.h"

#define VIDEOWILDCARD "Video Files|*.avi;*.mp4;*.mkv;*.mov;*.asf;*.flv;*.mpg;*.mpeg;*.m4v"

wxDECLARE_EVENT(EVT_VIDEODETAILS, wxCommandEvent);

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
    void AddVideoTime(std::string fn, unsigned long ms);

	public:

		VideoPanel(wxWindow* parent);
		virtual ~VideoPanel();

        //(*Declarations(VideoPanel)
        BulkEditCheckBox* CheckBox_SynchroniseWithAudio;
        BulkEditCheckBox* CheckBox_Video_AspectRatio;
        BulkEditChoice* Choice_Video_DurationTreatment;
        BulkEditSlider* Slider_Video_CropBottom;
        BulkEditSlider* Slider_Video_CropLeft;
        BulkEditSlider* Slider_Video_CropRight;
        BulkEditSlider* Slider_Video_CropTop;
        BulkEditSliderF2* Slider_Video_Starttime;
        BulkEditTextCtrl* TextCtrl_Video_CropBottom;
        BulkEditTextCtrl* TextCtrl_Video_CropLeft;
        BulkEditTextCtrl* TextCtrl_Video_CropRight;
        BulkEditTextCtrl* TextCtrl_Video_CropTop;
        BulkEditTextCtrlF2* TextCtrl_Video_Starttime;
        wxStaticText* StaticText1;
        wxStaticText* StaticText2;
        wxStaticText* StaticText3;
        wxStaticText* StaticText4;
        wxStaticText* StaticText5;
        wxStaticText* StaticText8;
        xlVideoFilePickerCtrl* FilePicker_Video_Filename;
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
		void OnFilePicker_Video_FilenameFileChanged(wxFileDirPickerEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnCheckBox_SynchroniseWithAudioClick(wxCommandEvent& event);
		//*)

        void SetVideoDetails(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()
};

#endif
