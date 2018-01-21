#ifndef WAVEPANEL_H
#define WAVEPANEL_H

//(*Headers(WavePanel)
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

class WavePanel: public wxPanel
{
    void ValidateWindow();

	public:

		WavePanel(wxWindow* parent);
		virtual ~WavePanel();

		//(*Declarations(WavePanel)
		wxStaticText* StaticText50;
		wxStaticText* StaticText47;
		BulkEditSlider* Slider_Thickness_Percentage;
		wxBitmapButton* BitmapButton_Wave_Direction;
		wxStaticText* StaticText48;
		wxStaticText* StaticText56;
		BulkEditValueCurveButton* BitmapButton_Wave_HeightVC;
		BulkEditCheckBox* CheckBox_Mirror_Wave;
		wxBitmapButton* BitmapButton_Wave_Height;
		BulkEditValueCurveButton* BitmapButton_Thickness_PercentageVC;
		wxStaticText* StaticText55;
		wxBitmapButton* BitmapButton_Wave_Thickness;
		wxBitmapButton* BitmapButton_WaveType;
		wxBitmapButton* BitmapButton_WaveSpeed;
		wxBitmapButton* BitmapButton_NumberWaves;
		BulkEditChoice* Choice_Wave_Direction;
		BulkEditSlider* Slider_Number_Waves;
		BulkEditChoice* Choice_Fill_Colors;
		wxStaticText* StaticText189;
		BulkEditValueCurveButton* BitmapButton_Number_WavesVC;
		BulkEditValueCurveButton* BitmapButton_Wave_SpeedVC;
		BulkEditSlider* Slider_Wave_Speed;
		wxBitmapButton* BitmapButton_Wave_FillColors;
		BulkEditSlider* Slider_Wave_Height;
		wxStaticText* StaticText52;
		wxBitmapButton* BitmapButton_Wave_MirrorWave;
		BulkEditChoice* Choice_Wave_Type;
		//*)

	protected:

		//(*Identifiers(WavePanel)
		static const long ID_STATICTEXT_Wave_Type;
		static const long ID_CHOICE_Wave_Type;
		static const long ID_BITMAPBUTTON_CHOICE_Wave_Type;
		static const long ID_STATICTEXT_Fill_Colors;
		static const long ID_CHOICE_Fill_Colors;
		static const long ID_BITMAPBUTTON_CHOICE_Fill_Colors;
		static const long ID_CHECKBOX_Mirror_Wave;
		static const long ID_BITMAPBUTTON_CHECKBOX_Mirror_Wave;
		static const long ID_STATICTEXT_Number_Waves;
		static const long ID_SLIDER_Number_Waves;
		static const long ID_VALUECURVE_Number_Waves;
		static const long IDD_TEXTCTRL_Number_Waves;
		static const long ID_BITMAPBUTTON_SLIDER_Number_Waves;
		static const long ID_STATICTEXT_Thickness_Percentage;
		static const long ID_SLIDER_Thickness_Percentage;
		static const long ID_VALUECURVE_Thickness_Percentage;
		static const long IDD_TEXTCTRL_Thickness_Percentage;
		static const long ID_BITMAPBUTTON_SLIDER_Thickness_Percentage;
		static const long ID_STATICTEXT_Wave_Height;
		static const long ID_SLIDER_Wave_Height;
		static const long ID_VALUECURVE_Wave_Height;
		static const long IDD_TEXTCTRL_Wave_Height;
		static const long ID_BITMAPBUTTON_SLIDER_Wave_Height;
		static const long ID_STATICTEXT_Wave_Speed;
		static const long ID_SLIDER_Wave_Speed;
		static const long ID_VALUECURVE_Wave_Speed;
		static const long IDD_TEXTCTRL_Wave_Speed;
		static const long ID_BITMAPBUTTON_SLIDER_Wave_Speed;
		static const long ID_STATICTEXT_Wave_Direction;
		static const long ID_CHOICE_Wave_Direction;
		static const long ID_BITMAPBUTTON_CHOICE_Wave_Direction;
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
