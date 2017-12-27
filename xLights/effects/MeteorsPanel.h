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

#include "../BulkEditControls.h"

class MeteorsPanel: public wxPanel
{
	public:

		MeteorsPanel(wxWindow* parent);
		virtual ~MeteorsPanel();

		//(*Declarations(MeteorsPanel)
		BulkEditTextCtrl* TextCtrl_Meteors_Yoffset;
		wxBitmapButton* BitmapButton_MeteorsCount;
		BulkEditChoice* Choice_Meteors_Effect;
		BulkEditSlider* Slider_Meteors_YOffset;
		wxBitmapButton* BitmapButton_MeteorsEffect;
		wxStaticText* StaticText2;
		wxBitmapButton* BitmapButton_Meteors_YOffset;
		BulkEditSlider* Slider_Meteors_Count;
		wxStaticText* StaticText1;
		BulkEditCheckBox* CheckBox_Meteors_UseMusic;
		wxStaticText* StaticText128;
		wxBitmapButton* BitmapButton_MeteorsType;
		BulkEditTextCtrl* TextCtrl_Meteors_XOffset;
		wxStaticText* StaticText39;
		BulkEditCheckBox* CheckBox_FadeWithDistance;
		wxStaticText* StaticText130;
		wxBitmapButton* BitmapButton_Meteors_UseMusic;
		BulkEditSlider* Slider_Meteors_XOffset;
		BulkEditSlider* Slider_Meteors_Length;
		BulkEditSlider* Slider_Meteors_Swirl_Intensity;
		wxStaticText* StaticText43;
		wxStaticText* StaticText41;
		BulkEditChoice* Choice_Meteors_Type;
		wxBitmapButton* BitmapButton_Meteors_XOffset;
		wxBitmapButton* BitmapButton_MeteorsSwirlIntensity;
		wxStaticText* StaticText4;
		wxBitmapButton* BitmapButton_MeteorsLength;
		wxBitmapButton* BitmapButton_FadeWithDistance;
		BulkEditSlider* Slider_Meteors_Speed;
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
		static const long ID_BITMAPBUTTON_Meteors_XOffset;
		static const long ID_STATICTEXT_Meteors_YOffset;
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
		void OnLockButtonClick(wxCommandEvent& event);
		void OnChoice_Meteors_EffectSelect(wxCommandEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()

        void ValidateWindow();
};

#endif
