#ifndef WAVEPANEL_H
#define WAVEPANEL_H

//(*Headers(WavePanel)
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

class WavePanel: public wxPanel
{
    void ValidateWindow();

	public:

		WavePanel(wxWindow* parent);
		virtual ~WavePanel();

		//(*Declarations(WavePanel)
		wxBitmapButton* BitmapButton_Wave_Direction;
		ValueCurveButton* BitmapButton_Thickness_PercentageVC;
		wxBitmapButton* BitmapButton_Wave_Height;
		ValueCurveButton* BitmapButton_Wave_HeightVC;
		wxStaticText* StaticText50;
		wxBitmapButton* BitmapButton13;
		wxBitmapButton* BitmapButton_Wave_MirrorWave;
		wxStaticText* StaticText56;
		wxCheckBox* CheckBox_Mirror_Wave;
		wxStaticText* StaticText55;
		wxStaticText* StaticText49;
		wxSlider* Slider_Thickness_Percentage;
		wxStaticText* StaticText189;
		wxChoice* Choice_Wave_Direction;
		wxSlider* Slider_Wave_Speed;
		wxStaticText* StaticText47;
		wxStaticText* StaticText52;
		ValueCurveButton* BitmapButton_Number_WavesVC;
		wxBitmapButton* BitmapButton_Wave_Thickness;
		wxBitmapButton* BitmapButton_NumberWaves;
		wxBitmapButton* BitmapButton_Wave_FillColors;
		wxChoice* Choice_Fill_Colors;
		wxSlider* Slider_Number_Waves;
		wxChoice* Choice_Wave_Type;
		wxBitmapButton* BitmapButton_WaveType;
		wxStaticText* StaticText48;
		ValueCurveButton* BitmapButton_Wave_SpeedVC;
    	wxSlider* Slider_Wave_Height;
    	wxStaticText* StaticText57;
    	wxSlider* Slider_Wave_Y_Position;
    	wxBitmapButton* BitmapButton_Wave_Y_Position;
    	ValueCurveButton* BitmapButton_Wave_Y_PositionVC;
    	wxTextCtrl* TextCtrl_Wave_Y_Position;
		//*)

	protected:

		//(*Identifiers(WavePanel)
		static const long ID_STATICTEXT49;
		static const long ID_CHOICE_Wave_Type;
		static const long ID_BITMAPBUTTON2;
		static const long ID_STATICTEXT50;
		static const long ID_CHOICE_Fill_Colors;
		static const long ID_BITMAPBUTTON3;
		static const long ID_STATICTEXT52;
		static const long ID_CHECKBOX_Mirror_Wave;
		static const long ID_BITMAPBUTTON4;
		static const long ID_STATICTEXT55;
		static const long ID_SLIDER_Number_Waves;
		static const long ID_VALUECURVE_Number_Waves;
		static const long IDD_TEXTCTRL_Number_Waves;
		static const long ID_BITMAPBUTTON5;
		static const long ID_STATICTEXT56;
		static const long ID_SLIDER_Thickness_Percentage;
		static const long ID_VALUECURVE_Thickness_Percentage;
		static const long IDD_TEXTCTRL_Thickness_Percentage;
		static const long ID_BITMAPBUTTON6;
		static const long ID_STATICTEXT58;
		static const long ID_SLIDER_Wave_Height;
		static const long ID_VALUECURVE_Wave_Height;
		static const long IDD_TEXTCTRL_Wave_Height;
		static const long ID_BITMAPBUTTON7;
		static const long ID_STATICTEXT32;
		static const long ID_SLIDER_Wave_Speed;
		static const long ID_VALUECURVE_Wave_Speed;
		static const long IDD_TEXTCTRL_Wave_Speed;
		static const long ID_BITMAPBUTTON31;
		static const long ID_STATICTEXT59;
		static const long ID_CHOICE_Wave_Direction;
    	static const long ID_BITMAPBUTTON8;
    	static const long ID_STATICTEXT57;
    	static const long ID_SLIDER_Wave_Y_Position;
    	static const long ID_VALUECURVE_Wave_Y_Position;
    	static const long IDD_TEXTCTRL_Wave_Y_Position;
    	static const long ID_BITMAPBUTTON57;
		//*)

	public:

		//(*Handlers(WavePanel)
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
		void OnChoice_Wave_TypeSelect(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        void UpdateLinkedSliderFloat2(wxCommandEvent& event);
        void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
