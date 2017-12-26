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

#include "../BulkEditControls.h"

class WavePanel: public wxPanel
{
    void ValidateWindow();

	public:

		WavePanel(wxWindow* parent);
		virtual ~WavePanel();

		//(*Declarations(WavePanel)
		BulkEditSlider* Slider_Thickness_Percentage;
		BulkEditChoice* Choice_Wave_Type;
		wxBitmapButton* BitmapButton_Wave_Direction;
		BulkEditSlider* Slider_Number_Waves;
		BulkEditValueCurveButton* BitmapButton_Wave_HeightVC;
		BulkEditValueCurveButton* BitmapButton_Number_WavesVC;
		BulkEditValueCurveButton* BitmapButton_Thickness_PercentageVC;
		wxBitmapButton* BitmapButton_Wave_Height;
		BulkEditValueCurveButton* BitmapButton_Wave_SpeedVC;
		wxStaticText* StaticText50;
		wxBitmapButton* BitmapButton13;
		wxBitmapButton* BitmapButton_Wave_MirrorWave;
		BulkEditChoice* Choice_Wave_Direction;
		wxStaticText* StaticText56;
		wxStaticText* StaticText55;
		BulkEditChoice* Choice_Fill_Colors;
		wxStaticText* StaticText189;
		wxStaticText* StaticText47;
		BulkEditSlider* Slider_Wave_Speed;
		wxStaticText* StaticText52;
		wxBitmapButton* BitmapButton_Wave_Thickness;
		wxBitmapButton* BitmapButton_NumberWaves;
		wxBitmapButton* BitmapButton_Wave_FillColors;
		wxBitmapButton* BitmapButton_WaveType;
		wxStaticText* StaticText48;
		BulkEditCheckBox* CheckBox_Mirror_Wave;
		BulkEditSlider* Slider_Wave_Height;
		//*)

	protected:

		//(*Identifiers(WavePanel)
		static const long ID_STATICTEXT_Wave_Type;
		static const long ID_CHOICE_Wave_Type;
		static const long ID_BITMAPBUTTON2;
		static const long ID_STATICTEXT_Fill_Colors;
		static const long ID_CHOICE_Fill_Colors;
		static const long ID_BITMAPBUTTON3;
		static const long ID_CHECKBOX_Mirror_Wave;
		static const long ID_BITMAPBUTTON4;
		static const long ID_STATICTEXT_Number_Waves;
		static const long ID_SLIDER_Number_Waves;
		static const long ID_VALUECURVE_Number_Waves;
		static const long IDD_TEXTCTRL_Number_Waves;
		static const long ID_BITMAPBUTTON5;
		static const long ID_STATICTEXT_Thickness_Percentage;
		static const long ID_SLIDER_Thickness_Percentage;
		static const long ID_VALUECURVE_Thickness_Percentage;
		static const long IDD_TEXTCTRL_Thickness_Percentage;
		static const long ID_BITMAPBUTTON6;
		static const long ID_STATICTEXT_Wave_Height;
		static const long ID_SLIDER_Wave_Height;
		static const long ID_VALUECURVE_Wave_Height;
		static const long IDD_TEXTCTRL_Wave_Height;
		static const long ID_BITMAPBUTTON7;
		static const long ID_STATICTEXT_Wave_Speed;
		static const long ID_SLIDER_Wave_Speed;
		static const long ID_VALUECURVE_Wave_Speed;
		static const long IDD_TEXTCTRL_Wave_Speed;
		static const long ID_BITMAPBUTTON31;
		static const long ID_STATICTEXT_Wave_Direction;
		static const long ID_CHOICE_Wave_Direction;
		static const long ID_BITMAPBUTTON8;
		//*)

	public:

		//(*Handlers(WavePanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnChoice_Wave_TypeSelect(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
