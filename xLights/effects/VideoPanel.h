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

//(*Headers(VideoPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxButton;
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
#include "EffectPanelUtils.h"

#define VIDEOWILDCARD "Video Files|*.avi;*.mp4;*.mkv;*.mov;*.asf;*.flv;*.mpg;*.mpeg;*.m4v;*.wmv;*.gif"

wxDECLARE_EVENT(EVT_VIDEODETAILS, wxCommandEvent);

class xlVideoFilePickerCtrl : public BulkEditFilePickerCtrl {
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
		: BulkEditFilePickerCtrl(parent, id, path, message, VIDEOWILDCARD, pos, size, style, validator, name) {

	}
	virtual ~xlVideoFilePickerCtrl() {}
};

class VideoPanel: public xlEffectPanel
{
    void AddVideoTime(std::string fn, unsigned long ms);

	public:

		VideoPanel(wxWindow* parent);
		virtual ~VideoPanel();
		virtual void ValidateWindow() override;

        //(*Declarations(VideoPanel)
        BulkEditCheckBox* CheckBox_SynchroniseWithAudio;
        BulkEditCheckBox* CheckBox_TransparentBlack;
        BulkEditCheckBox* CheckBox_Video_AspectRatio;
        BulkEditChoice* Choice_Video_DurationTreatment;
        BulkEditSlider* Slider1;
        BulkEditSlider* Slider_SampleSpacing;
        BulkEditSlider* Slider_Video_CropBottom;
        BulkEditSlider* Slider_Video_CropLeft;
        BulkEditSlider* Slider_Video_CropRight;
        BulkEditSlider* Slider_Video_CropTop;
        BulkEditSliderF2* Slider_Video_Speed;
        BulkEditSliderF2* Slider_Video_Starttime;
        BulkEditTextCtrl* TextCtrl1;
        BulkEditTextCtrl* TextCtrl_SampleSpacing;
        BulkEditTextCtrl* TextCtrl_Video_CropBottom;
        BulkEditTextCtrl* TextCtrl_Video_CropLeft;
        BulkEditTextCtrl* TextCtrl_Video_CropRight;
        BulkEditTextCtrl* TextCtrl_Video_CropTop;
        BulkEditTextCtrlF2* TextCtrl_Video_Speed;
        BulkEditTextCtrlF2* TextCtrl_Video_Starttime;
        BulkEditValueCurveButton* BitmapButton_Video_CropBottomVC;
        BulkEditValueCurveButton* BitmapButton_Video_CropLeftVC;
        BulkEditValueCurveButton* BitmapButton_Video_CropRightVC;
        BulkEditValueCurveButton* BitmapButton_Video_CropTopVC;
        BulkEditValueCurveButton* BitmapButton_Video_Speed;
        wxButton* Button_MatchVideoDuration;
        wxStaticText* StaticText1;
        wxStaticText* StaticText2;
        wxStaticText* StaticText3;
        wxStaticText* StaticText4;
        wxStaticText* StaticText5;
        wxStaticText* StaticText6;
        wxStaticText* StaticText7;
        wxStaticText* StaticText8;
        wxStaticText* StaticText9;
        wxTextCtrl* TextCtrl2;
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
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL_Duration;
		static const long ID_BUTTON1;
		static const long ID_STATICTEXT_Video_DurationTreatment;
		static const long ID_CHOICE_Video_DurationTreatment;
		static const long ID_STATICTEXT2;
		static const long IDD_SLIDER_Video_Speed;
		static const long ID_VALUECURVE_Video_Speed;
		static const long ID_TEXTCTRL_Video_Speed;
		static const long ID_CHECKBOX_Video_AspectRatio;
		static const long ID_CHECKBOX_SynchroniseWithAudio;
		static const long ID_STATICTEXT_Video_CropLeft;
		static const long IDD_SLIDER_Video_CropLeft;
		static const long ID_VALUECURVE_Video_CropLeft;
		static const long ID_TEXTCTRL_Video_CropLeft;
		static const long ID_STATICTEXT_Video_CropRight;
		static const long IDD_SLIDER_Video_CropRight;
		static const long ID_VALUECURVE_Video_CropRight;
		static const long ID_TEXTCTRL_Video_CropRight;
		static const long ID_STATICTEXT_Video_CropTop;
		static const long IDD_SLIDER_Video_CropTop;
		static const long ID_VALUECURVE_Video_CropTop;
		static const long ID_TEXTCTRL_Video_CropTop;
		static const long ID_STATICTEXT_Video_CropBottom;
		static const long IDD_SLIDER_Video_CropBottom;
		static const long ID_VALUECURVE_Video_CropBottom;
		static const long ID_TEXTCTRL_Video_CropBottom;
		static const long ID_CHECKBOX_Video_TransparentBlack;
		static const long IDD_SLIDER_Video_TransparentBlack;
		static const long ID_TEXTCTRL_Video_TransparentBlack;
		static const long ID_STATICTEXT3;
		static const long IDD_SLIDER_SampleSpacing;
		static const long ID_TEXTCTRL_SampleSpacing;
		//*)

	public:

		//(*Handlers(VideoPanel)
		void OnFilePicker_Video_FilenameFileChanged(wxFileDirPickerEvent& event);
		void OnCheckBox_SynchroniseWithAudioClick(wxCommandEvent& event);
		void OnChoice_Video_DurationTreatmentSelect(wxCommandEvent& event);
		void OnButton_MatchVideoDurationClick(wxCommandEvent& event);
		//*)

        void SetVideoDetails(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()
};
