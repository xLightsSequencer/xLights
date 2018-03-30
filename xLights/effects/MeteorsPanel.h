#ifndef METEORSPANEL_H
#define METEORSPANEL_H

//(*Headers(MeteorsPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
//*)

#include "../BulkEditControls.h"

class MeteorsPanel: public wxPanel
{
	public:

		MeteorsPanel(wxWindow* parent);
		virtual ~MeteorsPanel();

		//(*Declarations(MeteorsPanel)
		BulkEditSlider* Slider_Meteors_Speed;
		BulkEditSlider* Slider_Meteors_Swirl_Intensity;
		wxStaticText* StaticText43;
		BulkEditSlider* Slider_Meteors_Length;
		wxBitmapButton* BitmapButton_MeteorsEffect;
		wxStaticText* StaticText130;
		wxBitmapButton* BitmapButton_MeteorsType;
		BulkEditTextCtrl* TextCtrl_Meteors_Yoffset;
		wxStaticText* StaticText41;
		wxBitmapButton* BitmapButton_MeteorsSwirlIntensity;
		wxStaticText* StaticText1;
		BulkEditCheckBox* CheckBox_Meteors_UseMusic;
		wxStaticText* StaticText128;
		BulkEditSlider* Slider_Meteors_XOffset;
		BulkEditSlider* Slider_Meteors_Count;
		BulkEditSlider* Slider_Meteors_YOffset;
		wxBitmapButton* BitmapButton_FadeWithDistance;
		wxBitmapButton* BitmapButton_MeteorsLength;
		wxStaticText* StaticText4;
		wxStaticText* StaticText2;
		wxStaticText* StaticText39;
		BulkEditChoice* Choice_Meteors_Effect;
		BulkEditTextCtrl* TextCtrl_Meteors_XOffset;
		BulkEditChoice* Choice_Meteors_Type;
		BulkEditCheckBox* CheckBox_FadeWithDistance;
		wxBitmapButton* BitmapButton_MeteorsCount;
		wxBitmapButton* BitmapButton_Meteors_YOffset;
		wxBitmapButton* BitmapButton_Meteors_XOffset;
		wxBitmapButton* BitmapButton_Meteors_UseMusic;
		//*)

	protected:

		//(*Identifiers(MeteorsPanel)
		static const long ID_STATICTEXT_Meteors_Type;
		static const long ID_CHOICE_Meteors_Type;
		static const long ID_BITMAPBUTTON_CHOICE_Meteors_Type;
		static const long ID_STATICTEXT_Meteors_Effect;
		static const long ID_CHOICE_Meteors_Effect;
		static const long ID_BITMAPBUTTON_CHOICE_Meteors_Effect;
		static const long ID_STATICTEXT_Meteors_Count;
		static const long ID_SLIDER_Meteors_Count;
		static const long IDD_TEXTCTRL_Meteors_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_Count;
		static const long ID_STATICTEXT_Meteors_Length;
		static const long ID_SLIDER_Meteors_Length;
		static const long IDD_TEXTCTRL_Meteors_Length;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_Length;
		static const long ID_STATICTEXT_Meteors_Swirl_Intensity;
		static const long ID_SLIDER_Meteors_Swirl_Intensity;
		static const long IDD_TEXTCTRL_Meteors_Swirl_Intensity;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_Swirl_Intensity;
		static const long ID_STATICTEXT_Meteors_Speed;
		static const long ID_SLIDER_Meteors_Speed;
		static const long IDD_TEXTCTRL_Meteors_Speed;
		static const long ID_STATICTEXT_Meteors_XOffset;
		static const long IDD_SLIDER_Meteors_XOffset;
		static const long ID_TEXTCTRL_Meteors_XOffset;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_XOffset;
		static const long ID_STATICTEXT_Meteors_YOffset;
		static const long IDD_SLIDER_Meteors_YOffset;
		static const long ID_TEXTCTRL_Meteors_YOffset;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_YOffset;
		static const long ID_CHECKBOX_Meteors_UseMusic;
		static const long ID_BITMAPBUTTON_CHECKBOX_Meteors_UseMusic;
		static const long ID_CHECKBOX_FadeWithDistance;
		static const long ID_BITMAPBUTTON_CHECKBOX_FadeWithDistance;
		//*)

	public:

		//(*Handlers(MeteorsPanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnChoice_Meteors_EffectSelect(wxCommandEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()

        void ValidateWindow();
};

#endif
