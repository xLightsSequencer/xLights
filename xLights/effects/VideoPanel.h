#ifndef VIDEOPANEL_H
#define VIDEOPANEL_H

//(*Headers(VideoPanel)
#include <wx/panel.h>
class wxFilePickerCtrl;
class wxTextCtrl;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
//*)

#include <wx/filepicker.h>
#include <mutex>
#include <map>

#define VIDEOWILDCARD "Video Files|*.avi;*.mp4;*.mkv;*.mov;*.asf;*.flv;*.mpg;*.mpeg"

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
	public:

		VideoPanel(wxWindow* parent);
		virtual ~VideoPanel();

    
        void addVideoTime(std::string fn, unsigned long ms);
	protected:
        //(*Declarations(VideoPanel)
        xlVideoFilePickerCtrl* FilePicker_Video_Filename;
        wxSlider* Slider_Video_Starttime;
        wxCheckBox* CheckBox_Video_AspectRatio;
        wxBitmapButton* BitmapButton_Filename;
        wxStaticText* StaticText8;
        wxBitmapButton* BitmapButton_Video_Starttime;
        wxTextCtrl* TextCtrl_Video_Starttime;
        //*)

    
        std::mutex lock;
        std::map<std::string, unsigned long> videoTimeCache;
    


		//(*Identifiers(VideoPanel)
		static const long ID_FILEPICKERCTRL_Video_Filename;
		static const long ID_BITMAPBUTTON_Video_Filename;
		static const long ID_STATICTEXT8;
		static const long ID_SLIDER_Video_Starttime;
		static const long ID_TEXTCTRL_Video_Starttime;
		static const long ID_BITMAPBUTTON_Video_Starttime;
		static const long ID_CHECKBOX_Video_AspectRatio;
		//*)

	public:

		//(*Handlers(VideoPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnChoiceVideoDirectionSelect(wxCommandEvent& event);
		void OnTextCtrl1Text(wxCommandEvent& event);
		void OnSlider_Video_StarttimeCmdSliderUpdated(wxScrollEvent& event);
		void OnFilePicker_Video_FilenameFileChanged(wxFileDirPickerEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
