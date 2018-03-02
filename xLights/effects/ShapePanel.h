#ifndef SHAPEPANEL_H
#define SHAPEPANEL_H

//(*Headers(ShapePanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxFontPickerCtrl;
class wxSlider;
class wxSpinCtrl;
class wxSpinEvent;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"
#include <wx/fontpicker.h>
#include <list>

class Emoji;

class ShapePanel: public wxPanel
{
    std::list<Emoji*> _emojis;
    void ValidateWindow();

	public:

		ShapePanel(wxWindow* parent);
		virtual ~ShapePanel();
        void SetTimingTracks(wxCommandEvent& event);
        void EmojiMenu(wxContextMenuEvent& event);
        void OnPopupEmoji(wxCommandEvent& event);

		//(*Declarations(ShapePanel)
		BulkEditCheckBox* CheckBox_Shape_FadeAway;
		BulkEditCheckBox* CheckBox_Shape_FireTiming;
		BulkEditCheckBox* CheckBox_Shape_RandomLocation;
		BulkEditCheckBox* CheckBox_Shape_UseMusic;
		BulkEditChoice* Choice_Shape_ObjectToDraw;
		BulkEditChoice* Choice_Shape_TimingTrack;
		BulkEditSlider* Slider_Shape_CentreX;
		BulkEditSlider* Slider_Shape_CentreY;
		BulkEditSlider* Slider_Shape_Count;
		BulkEditSlider* Slider_Shape_Growth;
		BulkEditSlider* Slider_Shape_Lifetime;
		BulkEditSlider* Slider_Shape_Points;
		BulkEditSlider* Slider_Shape_Sensitivity;
		BulkEditSlider* Slider_Shape_StartSize;
		BulkEditSlider* Slider_Shape_Thickness;
		BulkEditSpinCtrl* SpinCtrl_CharCode;
		BulkEditTextCtrl* TextCtrl_Shape_CentreX;
		BulkEditTextCtrl* TextCtrl_Shape_CentreY;
		BulkEditTextCtrl* TextCtrl_Shape_Points;
		BulkEditTextCtrl* TextCtrl_Shape_Sensitivity;
		BulkEditTextCtrl* TextCtrl_Shape_Thickness;
		BulkEditValueCurveButton* BitmapButton_Shape_CentreXVC;
		BulkEditValueCurveButton* BitmapButton_Shape_CentreYVC;
		BulkEditValueCurveButton* BitmapButton_Shape_CountVC;
		BulkEditValueCurveButton* BitmapButton_Shape_GrowthVC;
		BulkEditValueCurveButton* BitmapButton_Shape_LifetimeVC;
		BulkEditValueCurveButton* BitmapButton_Shape_StartSizeVC;
		BulkEditValueCurveButton* BitmapButton_Shape_ThicknessVC;
		wxFontPickerCtrl* FontPickerCtrl_Font;
		wxStaticText* StaticText10;
		wxStaticText* StaticText176;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText69;
		wxStaticText* StaticText6;
		wxStaticText* StaticText72;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		//*)

	protected:

		//(*Identifiers(ShapePanel)
		static const long ID_STATICTEXT_Shape_ObjectToDraw;
		static const long ID_CHOICE_Shape_ObjectToDraw;
		static const long ID_STATICTEXT1;
		static const long ID_FONTPICKER_Shape_Font;
		static const long ID_SPINCTRL_Shape_Char;
		static const long ID_STATICTEXT2;
		static const long ID_STATICTEXT_Shape_Thickness;
		static const long ID_SLIDER_Shape_Thickness;
		static const long ID_VALUECURVE_Shape_Thickness;
		static const long IDD_TEXTCTRL_Shape_Thickness;
		static const long ID_STATICTEXT_Shape_Count;
		static const long IDD_SLIDER_Shape_Count;
		static const long ID_VALUECURVE_Shape_Count;
		static const long ID_TEXTCTRL_Shape_Count;
		static const long ID_STATICTEXT_Shape_StartSize;
		static const long ID_SLIDER_Shape_StartSize;
		static const long ID_VALUECURVE_Shape_StartSize;
		static const long IDD_TEXTCTRL_Shape_StartSize;
		static const long ID_STATICTEXT_Shape_Lifetime;
		static const long ID_SLIDER_Shape_Lifetime;
		static const long ID_VALUECURVE_Shape_Lifetime;
		static const long IDD_TEXTCTRL_Shape_Lifetime;
		static const long ID_STATICTEXT_Shape_Growth;
		static const long ID_SLIDER_Shape_Growth;
		static const long ID_VALUECURVE_Shape_Growth;
		static const long IDD_TEXTCTRL_Shape_Growth;
		static const long ID_STATICTEXT_Shape_CentreX;
		static const long ID_SLIDER_Shape_CentreX;
		static const long ID_VALUECURVE_Shape_CentreX;
		static const long IDD_TEXTCTRL_Shape_CentreX;
		static const long ID_STATICTEXT_Shape_CentreY;
		static const long ID_SLIDER_Shape_CentreY;
		static const long ID_VALUECURVE_Shape_CentreY;
		static const long IDD_TEXTCTRL_Shape_CentreY;
		static const long ID_STATICTEXT_Shape_Points;
		static const long ID_SLIDER_Shape_Points;
		static const long IDD_TEXTCTRL_Shape_Points;
		static const long ID_CHECKBOX_Shape_RandomLocation;
		static const long ID_CHECKBOX_Shape_FadeAway;
		static const long ID_CHECKBOX_Shape_UseMusic;
		static const long ID_STATICTEXT_Shape_Sensitivity;
		static const long ID_SLIDER_Shape_Sensitivity;
		static const long IDD_TEXTCTRL_Shape_Sensitivity;
		static const long ID_CHECKBOX_Shape_FireTiming;
		static const long ID_STATICTEXT_Shape_FireTimingTrack;
		static const long ID_CHOICE_Shape_FireTimingTrack;
		//*)

	public:

		//(*Handlers(ShapePanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnChoice_Shape_ObjectToDrawSelect(wxCommandEvent& event);
		void OnCheckBox_Shape_RandomLocationClick(wxCommandEvent& event);
		void OnCheckBox_Shape_UseMusicClick(wxCommandEvent& event);
		void OnCheckBox_Shape_FireTimingClick(wxCommandEvent& event);
		void OnChoice_Shape_TimingTrackSelect(wxCommandEvent& event);
		void OnFontPickerCtrl_FontFontChanged(wxFontPickerEvent& event);
		void OnSpinCtrl_CharCodeChange(wxSpinEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
