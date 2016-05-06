#ifndef MUSICPANEL_H
#define MUSICPANEL_H

//(*Headers(MusicPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

class MusicPanel: public wxPanel
{

    // these are only used for validation
    int __bars;
    int __startNote;
    int __endNote;
    int __sensitivity;
    int __offset;

	public:

		MusicPanel(wxWindow* parent);
		virtual ~MusicPanel();

		//(*Declarations(MusicPanel)
		wxCheckBox* CheckBox_Music_ScaleNotes;
		wxSlider* Slider_Music_StartNote;
		wxBitmapButton* BitmapButton_Music_ScaleNotes;
		wxTextCtrl* TextCtrl_Music_EndNote;
		wxBitmapButton* BitmapButton_Music_EndNote;
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		wxTextCtrl* TextCtrl_Music_StartNote;
		wxSlider* Slider_Music_Sensitivity;
		wxStaticText* StaticText11;
		wxBitmapButton* BitmapButton_Music_Sensitivity;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxBitmapButton* BitmapButton_Music_Bars;
		wxSlider* Slider_Music_Bars;
		wxTextCtrl* TextCtrl_Music_Offset;
		wxBitmapButton* BitmapButton_Music_Colour;
		wxChoice* Choice_Music_Type;
		wxChoice* Choice_Music_Colour;
		wxTextCtrl* TextCtrl_Music_Bars;
		wxStaticText* StaticText12;
		wxBitmapButton* BitmapButton_Music_Offset;
		wxBitmapButton* BitmapButton_Music_Type;
		wxSlider* Slider_Music_EndNote;
		wxBitmapButton* BitmapButton_Music_Fade;
		wxBitmapButton* BitmapButton_Music_StartNote;
		wxBitmapButton* BitmapButton_Music_Scale;
		wxStaticText* StaticText4;
		wxSlider* Slider_Music_Offset;
		wxCheckBox* CheckBox_Music_Fade;
		wxTextCtrl* TextCtrl_Music_Sensitivity;
		wxCheckBox* CheckBox_Music_Scale;
		//*)

	protected:

		//(*Identifiers(MusicPanel)
		static const long ID_STATICTEXT1;
		static const long IDD_SLIDER_Music_Bars;
		static const long ID_TEXTCTRL_Music_Bars;
		static const long ID_BITMAPBUTTON_CHOICE_Music_Bars;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE_Music_Type;
		static const long ID_BITMAPBUTTON_CHOICE_Music_Type;
		static const long ID_STATICTEXT12;
		static const long IDD_SLIDER_Music_StartNote;
		static const long ID_TEXTCTRL_Music_StartNote;
		static const long ID_BITMAPBUTTON_Music_StartNote;
		static const long ID_STATICTEXT11;
		static const long IDD_SLIDER_Music_EndNote;
		static const long ID_TEXTCTRL_Music_EndNote;
		static const long ID_BITMAPBUTTON_Music_EndNote;
		static const long ID_STATICTEXT4;
		static const long IDD_SLIDER_Music_Sensitivity;
		static const long ID_TEXTCTRL_Music_Sensitivity;
		static const long ID_BITMAPBUTTON_Music_Sensitivty;
		static const long ID_STATICTEXT3;
		static const long IDD_SLIDER_Music_Offset;
		static const long ID_TEXTCTRL_Music_Offset;
		static const long ID_BITMAPBUTTON_Music_Offset;
		static const long ID_CHECKBOX_Music_Scale;
		static const long ID_BITMAPBUTTON_Music_Scale;
		static const long ID_CHECKBOX_Music_ScaleNotes;
		static const long ID_BITMAPBUTTON_Music_ScaleNotes;
		static const long ID_STATICTEXT6;
		static const long ID_CHOICE_Music_Colour;
		static const long ID_BITMAPBUTTON_Music_Colour;
		static const long ID_CHECKBOX_Music_Fade;
		static const long ID_BITMAPBUTTON_Music_Fade;
		//*)

	public:

		//(*Handlers(MusicPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnChoiceMusicDirectionSelect(wxCommandEvent& event);
		void OnTextCtrl1Text(wxCommandEvent& event);
		void OnChoice_Music_TypeSelect(wxCommandEvent& event);
		void OnSlider_Music_StartNoteCmdSliderUpdated(wxScrollEvent& event);
		void OnTextCtrl_Music_StartNoteText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

	public:

		void ValidateWindow();
};

#endif
