#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(WavePanel)
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

class WavePanel: public wxPanel
{
    void ValidateWindow();

	public:

		WavePanel(wxWindow* parent);
		virtual ~WavePanel();

		//(*Declarations(WavePanel)
		BulkEditCheckBox* CheckBox_Mirror_Wave;
		BulkEditChoice* Choice_Fill_Colors;
		BulkEditChoice* Choice_Wave_Direction;
		BulkEditChoice* Choice_Wave_Type;
		BulkEditSlider* Slider_Number_Waves;
		BulkEditSlider* Slider_Thickness_Percentage;
		BulkEditSlider* Slider_Wave_Height;
		BulkEditSlider* Slider_Wave_Speed;
		BulkEditSlider* Slider_Y_Offset;
		BulkEditValueCurveButton* BitmapButton_Number_WavesVC;
		BulkEditValueCurveButton* BitmapButton_Thickness_PercentageVC;
		BulkEditValueCurveButton* BitmapButton_Wave_HeightVC;
		BulkEditValueCurveButton* BitmapButton_Wave_SpeedVC;
		BulkEditValueCurveButton* BitmapButton_Wave_YOffsetVC;
		wxStaticText* StaticText189;
		wxStaticText* StaticText1;
		wxStaticText* StaticText47;
		wxStaticText* StaticText48;
		wxStaticText* StaticText50;
		wxStaticText* StaticText52;
		wxStaticText* StaticText55;
		wxStaticText* StaticText56;
		xlLockButton* BitmapButton_NumberWaves;
		xlLockButton* BitmapButton_WaveSpeed;
		xlLockButton* BitmapButton_WaveType;
		xlLockButton* BitmapButton_Wave_Direction;
		xlLockButton* BitmapButton_Wave_FillColors;
		xlLockButton* BitmapButton_Wave_Height;
		xlLockButton* BitmapButton_Wave_MirrorWave;
		xlLockButton* BitmapButton_Wave_Thickness;
		xlLockButton* BitmapButton_YOffset;
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
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_Wave_YOffset;
		static const long ID_VALUECURVE_Wave_YOffset;
		static const long IDD_TEXTCTRL_Wave_YOffset;
		static const long ID_BITMAPBUTTON_Wave_YOffset;
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
