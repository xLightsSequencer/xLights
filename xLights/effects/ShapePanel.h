#ifndef SHAPEPANEL_H
#define SHAPEPANEL_H

//(*Headers(ShapePanel)
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

class ShapePanel: public wxPanel
{
    void ValidateWindow();

	public:

		ShapePanel(wxWindow* parent);
		virtual ~ShapePanel();
        void SetTimingTracks(wxCommandEvent& event);

		//(*Declarations(ShapePanel)
		BulkEditTextCtrl* TextCtrl_Shape_Points;
		BulkEditSlider* Slider_Shape_Lifetime;
		BulkEditSlider* Slider_Shape_Thickness;
		BulkEditChoice* Choice_Shape_TimingTrack;
		wxStaticText* StaticText2;
		BulkEditChoice* Choice_Shape_ObjectToDraw;
		wxStaticText* StaticText6;
		BulkEditTextCtrl* TextCtrl_Shape_CentreX;
		BulkEditSlider* Slider_Shape_Count;
		BulkEditSlider* Slider_Shape_Points;
		wxStaticText* StaticText8;
		BulkEditTextCtrl* TextCtrl_Shape_CentreY;
		BulkEditSlider* Slider_Shape_CentreX;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		BulkEditValueCurveButton* BitmapButton_Shape_CountVC;
		BulkEditCheckBox* CheckBox_Shape_FireTiming;
		BulkEditSlider* Slider_Shape_StartSize;
		wxStaticText* StaticText72;
		BulkEditCheckBox* CheckBox_Shape_RandomLocation;
		BulkEditCheckBox* CheckBox_Shape_FadeAway;
		BulkEditValueCurveButton* BitmapButton_Shape_GrowthVC;
		wxStaticText* StaticText69;
		BulkEditValueCurveButton* BitmapButton_Shape_CentreXVC;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		BulkEditValueCurveButton* BitmapButton_Shape_ThicknessVC;
		BulkEditValueCurveButton* BitmapButton_Shape_CentreYVC;
		BulkEditCheckBox* CheckBox_Shape_UseMusic;
		BulkEditSlider* Slider_Shape_CentreY;
		BulkEditValueCurveButton* BitmapButton_Shape_StartSizeVC;
		wxStaticText* StaticText4;
		BulkEditSlider* Slider_Shape_Sensitivity;
		wxStaticText* StaticText176;
		BulkEditSlider* Slider_Shape_Growth;
		BulkEditTextCtrl* TextCtrl_Shape_Sensitivity;
		BulkEditValueCurveButton* BitmapButton_Shape_LifetimeVC;
		BulkEditTextCtrl* TextCtrl_Shape_Thickness;
		//*)

	protected:

		//(*Identifiers(ShapePanel)
		static const long ID_STATICTEXT_Shape_ObjectToDraw;
		static const long ID_CHOICE_Shape_ObjectToDraw;
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
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
