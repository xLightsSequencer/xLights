#ifndef VIDEOPANEL_H
#define VIDEOPANEL_H

//(*Headers(VideoPanel)
#include <wx/panel.h>
class wxFilePickerCtrl;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

#include <wx/filepicker.h>

#define VIDEOWILDCARD "Video Files|*.avi"

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

		//(*Declarations(VideoPanel)
		wxSlider* Slider_Video_Starttime;
		wxBitmapButton* BitmapButton_Video_Starttime;
		wxStaticText* StaticText8;
		xlVideoFilePickerCtrl* FilePicker_Video_Filename;
		wxBitmapButton* BitmapButton_Filename;
		wxTextCtrl* TextCtrl_Video_Starttime;
		//*)

	protected:

		//(*Identifiers(VideoPanel)
		static const long ID_FILEPICKERCTRL_Video_Filename;
		static const long ID_BITMAPBUTTON_Video_Filename;
		static const long ID_STATICTEXT8;
		static const long ID_SLIDER_Video_Starttime;
		static const long ID_TEXTCTRL_Video_Starttime;
		static const long ID_BITMAPBUTTON_Video_Starttime;
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
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
