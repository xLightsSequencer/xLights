#ifndef METEORSPANEL_H
#define METEORSPANEL_H

//(*Headers(MeteorsPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

class MeteorsPanel: public wxPanel
{
	public:

		MeteorsPanel(wxWindow* parent);
		virtual ~MeteorsPanel();

		//(*Declarations(MeteorsPanel)
		wxSlider* Slider_Meteors_Count;
		wxBitmapButton* BitmapButton_MeteorsCount;
		wxSlider* Slider_Meteors_YOffset;
		wxBitmapButton* BitmapButton_MeteorsEffect;
		wxStaticText* StaticText2;
		wxCheckBox* CheckBox_FadeWithDistance;
		wxChoice* Choice_Meteors_Effect;
		wxBitmapButton* BitmapButton_Meteors_YOffset;
		wxSlider* Slider_Meteors_Length;
		wxStaticText* StaticText1;
		wxCheckBox* CheckBox_Meteors_UseMusic;
		wxStaticText* StaticText128;
		wxBitmapButton* BitmapButton_MeteorsType;
		wxStaticText* StaticText39;
		wxStaticText* StaticText130;
		wxBitmapButton* BitmapButton_Meteors_UseMusic;
		wxTextCtrl* TextCtrl_Meteors_Yoffset;
		wxStaticText* StaticText43;
		wxStaticText* StaticText41;
		wxTextCtrl* TextCtrl_Meteors_XOffset;
		wxSlider* Slider_Meteors_XOffset;
		wxSlider* Slider_Meteors_Swirl_Intensity;
		wxBitmapButton* BitmapButton_Meteors_XOffset;
		wxBitmapButton* BitmapButton_MeteorsSwirlIntensity;
		wxBitmapButton* BitmapButton_MeteorsLength;
		wxBitmapButton* BitmapButton_FadeWithDistance;
		//*)

	protected:

		//(*Identifiers(MeteorsPanel)
		static const long ID_STATICTEXT39;
		static const long ID_CHOICE_Meteors_Type;
		static const long ID_BITMAPBUTTON_CHOICE_Meteors_Type;
		static const long ID_STATICTEXT129;
		static const long ID_CHOICE_Meteors_Effect;
		static const long ID_BITMAPBUTTON_CHOICE_Meteors_Effect;
		static const long ID_STATICTEXT41;
		static const long ID_SLIDER_Meteors_Count;
		static const long IDD_TEXTCTRL_Meteors_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_Count;
		static const long ID_STATICTEXT43;
		static const long ID_SLIDER_Meteors_Length;
		static const long IDD_TEXTCTRL_Meteors_Length;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_Length;
		static const long ID_STATICTEXT131;
		static const long ID_SLIDER_Meteors_Swirl_Intensity;
		static const long IDD_TEXTCTRL_Meteors_Swirl_Intensity;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_Swirl_Intensity;
		static const long ID_SLIDER_Meteors_Speed;
		static const long IDD_TEXTCTRL_Meteors_Speed;
		static const long ID_STATICTEXT1;
		static const long IDD_SLIDER_Meteors_XOffset;
		static const long ID_TEXTCTRL_Meteors_XOffset;
		static const long ID_BITMAPBUTTON_Meteors_XOffset;
		static const long ID_STATICTEXT2;
		static const long IDD_SLIDER_Meteors_YOffset;
		static const long ID_TEXTCTRL_Meteors_YOffset;
		static const long ID_BITMAPBUTTON_Meteors_YOffset;
		static const long ID_CHECKBOX_Meteors_UseMusic;
		static const long ID_BITMAPBUTTON_Meteors_UseMusic;
		static const long ID_CHECKBOX_FadeWithDistance;
		static const long ID_BITMAPBUTTON_FadeWithDistance;
		//*)

	public:

		//(*Handlers(MeteorsPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnChoice_Meteors_EffectSelect(wxCommandEvent& event);
        void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
        void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()

        void ValidateWindow();
};

#endif
