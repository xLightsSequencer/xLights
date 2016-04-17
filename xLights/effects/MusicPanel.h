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
		wxTextCtrl* TextCtrl_Music_StartNote;
		wxStaticText* StaticText11;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxBitmapButton* BitmapButton_Music_Bars;
		wxSlider* Slider_Music_Bars;
		wxStaticText* StaticText5;
		wxTextCtrl* TextCtrl_Music_Offset;
		wxChoice* Choice_Music_Type;
		wxTextCtrl* TextCtrl_Music_Bars;
		wxStaticText* StaticText12;
		wxBitmapButton* BitmapButton_Music_Offset;
		wxBitmapButton* BitmapButton_Music_Type;
		wxSlider* Slider_Music_EndNote;
		wxBitmapButton* BitmapButton_Music_StartNote;
		wxBitmapButton* BitmapButton_Music_Scale;
		wxSlider* Slider_Music_Offset;
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
		static const long ID_STATICTEXT5;
		static const long ID_BITMAPBUTTON_CHOICE_Music_Type;
		static const long ID_STATICTEXT12;
		static const long IDD_SLIDER_Music_StartNote;
		static const long ID_TEXTCTRL_Music_StartNote;
		static const long ID_BITMAPBUTTON_Music_StartNote;
		static const long ID_STATICTEXT11;
		static const long IDD_SLIDER_Music_EndNote;
		static const long ID_TEXTCTRL_Music_EndNote;
		static const long ID_BITMAPBUTTON_Music_EndNote;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER_Music_Offset;
		static const long ID_TEXTCTRL_Music_Offset;
		static const long ID_BITMAPBUTTON_Music_Offset;
		static const long ID_CHECKBOX_Music_Scale;
		static const long ID_BITMAPBUTTON_Music_Scale;
		static const long ID_CHECKBOX_Music_ScaleNotes;
		static const long ID_BITMAPBUTTON_Music_ScaleNotes;
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
