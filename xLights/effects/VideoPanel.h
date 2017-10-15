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
        wxSlider* Slider_Video_CropBottom;
        wxChoice* Choice_Video_DurationTreatment;
        wxSlider* Slider_Video_Starttime;
        wxSlider* Slider_Video_CropRight;
        wxStaticText* StaticText2;
        wxCheckBox* CheckBox_Video_AspectRatio;
        wxTextCtrl* TextCtrl_Video_CropRight;
        wxStaticText* StaticText8;
        wxCheckBox* CheckBox_SynchroniseWithAudio;
        wxStaticText* StaticText3;
        xlVideoFilePickerCtrl* FilePicker_Video_Filename;
        wxSlider* Slider_Video_CropLeft;
        wxSlider* Slider_Video_CropTop;
        wxStaticText* StaticText5;
        wxTextCtrl* TextCtrl_Video_CropTop;
        wxTextCtrl* TextCtrl_Video_CropLeft;
        wxTextCtrl* TextCtrl_Video_CropBottom;
        wxTextCtrl* TextCtrl_Video_Starttime;
        wxStaticText* StaticText4;
        //*)

protected:

        std::mutex lock;
        std::map<std::string, unsigned long> videoTimeCache;
    
		//(*Identifiers(VideoPanel)
		static const long ID_FILEPICKERCTRL_Video_Filename;
		static const long ID_STATICTEXT8;
		static const long IDD_SLIDER_Video_Starttime;
		static const long ID_TEXTCTRL_Video_Starttime;
		static const long ID_CHOICE_Video_DurationTreatment;
		static const long ID_CHECKBOX_Video_AspectRatio;
		static const long ID_CHECKBOX_SynchroniseWithAudio;
		static const long ID_STATICTEXT1;
		static const long IDD_SLIDER_Video_CropLeft;
		static const long ID_TEXTCTRL_Video_CropLeft;
		static const long ID_STATICTEXT2;
		static const long IDD_SLIDER_Video_CropRight;
		static const long ID_TEXTCTRL_Video_CropRight;
		static const long ID_STATICTEXT3;
		static const long IDD_SLIDER_Video_CropTop;
		static const long ID_TEXTCTRL_Video_CropTop;
		static const long ID_STATICTEXT4;
		static const long IDD_SLIDER_Video_CropBottom;
		static const long ID_TEXTCTRL_Video_CropBottom;
		//*)

	public:

		//(*Handlers(VideoPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedSliderFloat2(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
		void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnSlider_Video_StarttimeCmdSliderUpdated(wxScrollEvent& event);
		void OnFilePicker_Video_FilenameFileChanged(wxFileDirPickerEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnCheckBox_SynchroniseWithAudioClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
