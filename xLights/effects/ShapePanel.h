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

#include "../ValueCurveButton.h"

class ShapePanel: public wxPanel
{
    void ValidateWindow();

	public:

		ShapePanel(wxWindow* parent);
		virtual ~ShapePanel();
        void SetTimingTracks(wxCommandEvent& event);

		//(*Declarations(ShapePanel)
		ValueCurveButton* BitmapButton_Shape_StartSizeVC;
		wxTextCtrl* TextCtrl_Shape_Points;
		wxChoice* Choice_Shape_ObjectToDraw;
		wxSlider* Slider_Shape_Count;
		ValueCurveButton* BitmapButton_Shape_GrowthVC;
		wxSlider* Slider_Shape_CentreX;
		wxSlider* Slider_Shape_Growth;
		wxStaticText* StaticText2;
		ValueCurveButton* BitmapButton_Shape_ThicknessVC;
		wxCheckBox* CheckBox_Shape_RandomLocation;
		wxSlider* Slider_Shape_Lifetime;
		wxStaticText* StaticText6;
		wxChoice* Choice_Shape_TimingTrack;
		wxCheckBox* CheckBox_Shape_UseMusic;
		wxTextCtrl* TextCtrl_Shape_Sensitivity;
		wxStaticText* StaticText8;
		wxSlider* Slider_Shape_Thickness;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Shape_CentreX;
		wxCheckBox* CheckBox_Shape_FireTiming;
		wxStaticText* StaticText72;
		ValueCurveButton* BitmapButton_Shape_CentreXVC;
		wxTextCtrl* TextCtrl_Shape_CentreY;
		wxStaticText* StaticText69;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxSlider* Slider_Shape_CentreY;
		ValueCurveButton* BitmapButton_Shape_CentreYVC;
		wxCheckBox* CheckBox_Shape_FadeAway;
		ValueCurveButton* BitmapButton_Shape_LifetimeVC;
		ValueCurveButton* BitmapButton_Shape_CountVC;
		wxSlider* Slider_Shape_Points;
		wxSlider* Slider_Shape_StartSize;
		wxStaticText* StaticText4;
		wxSlider* Slider_Shape_Sensitivity;
		//*)

	protected:

		//(*Identifiers(ShapePanel)
		static const long ID_STATICTEXT71;
		static const long ID_CHOICE_Shape_ObjectToDraw;
		static const long ID_STATICTEXT74;
		static const long ID_SLIDER_Shape_Thickness;
		static const long ID_VALUECURVE_Shape_Thickness;
		static const long IDD_TEXTCTRL_Shape_Thickness;
		static const long IDD_SLIDER_Shape_Count;
		static const long ID_VALUECURVE_Shape_Count;
		static const long ID_TEXTCTRL_Shape_Count;
		static const long ID_STATICTEXT6;
		static const long ID_SLIDER_Shape_StartSize;
		static const long ID_VALUECURVE_Shape_StartSize;
		static const long IDD_TEXTCTRL_Shape_StartSize;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_Shape_Lifetime;
		static const long ID_VALUECURVE_Shape_Lifetime;
		static const long IDD_TEXTCTRL_Shape_Lifetime;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER_Shape_Growth;
		static const long ID_VALUECURVE_Shape_Growth;
		static const long IDD_TEXTCTRL_Shape_Growth;
		static const long ID_STATICTEXT4;
		static const long ID_SLIDER_Shape_CentreX;
		static const long ID_VALUECURVE_Shape_CentreX;
		static const long IDD_TEXTCTRL_Shape_CentreX;
		static const long ID_STATICTEXT5;
		static const long ID_SLIDER_Shape_CentreY;
		static const long ID_VALUECURVE_Shape_CentreY;
		static const long IDD_TEXTCTRL_Shape_CentreY;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_Shape_Points;
		static const long IDD_TEXTCTRL_Shape_Points;
		static const long ID_CHECKBOX_Shape_RandomLocation;
		static const long ID_CHECKBOX_Shape_FadeAway;
		static const long ID_CHECKBOX_Shape_UseMusic;
		static const long ID_STATICTEXT7;
		static const long ID_SLIDER_Shape_Sensitivity;
		static const long IDD_TEXTCTRL_Shape_Sensitivity;
		static const long ID_CHECKBOX_Shape_FireTiming;
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE_Shape_FireTimingTrack;
		//*)

	public:

		//(*Handlers(ShapePanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
		void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void UpdateLinkedSliderFloat2(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
		void OnChoice_Shape_ObjectToDrawSelect(wxCommandEvent& event);
		void OnCheckBox_Shape_RandomLocationClick(wxCommandEvent& event);
		void OnCheckBox_Shape_UseMusicClick(wxCommandEvent& event);
		void OnCheckBox_Shape_FireTimingClick(wxCommandEvent& event);
		void OnChoice_Shape_TimingTrackSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
