#ifndef METEORSPANEL_H
#define METEORSPANEL_H

//(*Headers(MeteorsPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class MeteorsPanel: public wxPanel
{
	public:

		MeteorsPanel(wxWindow* parent);
		virtual ~MeteorsPanel();

		//(*Declarations(MeteorsPanel)
		BulkEditCheckBox* CheckBox_FadeWithDistance;
		BulkEditCheckBox* CheckBox_Meteors_UseMusic;
		BulkEditChoice* Choice_Meteors_Effect;
		BulkEditChoice* Choice_Meteors_Type;
		BulkEditSlider* Slider_Meteors_Count;
		BulkEditSlider* Slider_Meteors_Length;
		BulkEditSlider* Slider_Meteors_Speed;
		BulkEditSlider* Slider_Meteors_Swirl_Intensity;
		BulkEditSlider* Slider_Meteors_XOffset;
		BulkEditSlider* Slider_Meteors_YOffset;
		BulkEditTextCtrl* TextCtrl_Meteors_XOffset;
		BulkEditTextCtrl* TextCtrl_Meteors_YOffset;
		BulkEditValueCurveButton* BitmapButton_Meteors_Count;
		BulkEditValueCurveButton* BitmapButton_Meteors_Length;
		BulkEditValueCurveButton* BitmapButton_Meteors_Speed;
		BulkEditValueCurveButton* BitmapButton_Meteors_Swirl_Intensity;
		BulkEditValueCurveButton* BitmapButton_Meteors_XOffsetVC;
		BulkEditValueCurveButton* BitmapButton_Meteors_YOffsetVC;
		wxBitmapButton* BitmapButton_FadeWithDistance;
		wxBitmapButton* BitmapButton_MeteorsCount;
		wxBitmapButton* BitmapButton_MeteorsEffect;
		wxBitmapButton* BitmapButton_MeteorsLength;
		wxBitmapButton* BitmapButton_MeteorsSwirlIntensity;
		wxBitmapButton* BitmapButton_MeteorsType;
		wxBitmapButton* BitmapButton_Meteors_UseMusic;
		wxBitmapButton* BitmapButton_Meteors_XOffset;
		wxBitmapButton* BitmapButton_Meteors_YOffset;
		wxStaticText* StaticText128;
		wxStaticText* StaticText130;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText39;
		wxStaticText* StaticText41;
		wxStaticText* StaticText43;
		wxStaticText* StaticText4;
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
		static const long ID_VALUECURVE_Meteors_Count;
		static const long IDD_TEXTCTRL_Meteors_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_Count;
		static const long ID_STATICTEXT_Meteors_Length;
		static const long ID_SLIDER_Meteors_Length;
		static const long ID_VALUECURVE_Meteors_Length;
		static const long IDD_TEXTCTRL_Meteors_Length;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_Length;
		static const long ID_STATICTEXT_Meteors_Swirl_Intensity;
		static const long ID_SLIDER_Meteors_Swirl_Intensity;
		static const long ID_VALUECURVE_Meteors_Swirl_Intensity;
		static const long IDD_TEXTCTRL_Meteors_Swirl_Intensity;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_Swirl_Intensity;
		static const long ID_STATICTEXT_Meteors_Speed;
		static const long ID_SLIDER_Meteors_Speed;
		static const long ID_VALUECURVE_Meteors_Speed;
		static const long IDD_TEXTCTRL_Meteors_Speed;
		static const long ID_STATICTEXT_Meteors_XOffset;
		static const long IDD_SLIDER_Meteors_XOffset;
		static const long ID_VALUECURVE_Meteors_XOffset;
		static const long ID_TEXTCTRL_Meteors_XOffset;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_XOffset;
		static const long ID_STATICTEXT_Meteors_YOffset;
		static const long IDD_SLIDER_Meteors_YOffset;
		static const long ID_VALUECURVE_Meteors_YOffset;
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
