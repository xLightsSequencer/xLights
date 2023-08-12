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
#include "EffectPanelUtils.h"

class MeteorsPanel : public xlEffectPanel
{
public:

	MeteorsPanel(wxWindow* parent);
	virtual ~MeteorsPanel();
	virtual void ValidateWindow() override;

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
	BulkEditSlider* Slider_WarmupFrames;
	BulkEditTextCtrl* TextCtrl_Meteors_XOffset;
	BulkEditTextCtrl* TextCtrl_Meteors_YOffset;
	BulkEditValueCurveButton* BitmapButton_Meteors_Count;
	BulkEditValueCurveButton* BitmapButton_Meteors_Length;
	BulkEditValueCurveButton* BitmapButton_Meteors_Speed;
	BulkEditValueCurveButton* BitmapButton_Meteors_Swirl_Intensity;
	BulkEditValueCurveButton* BitmapButton_Meteors_XOffsetVC;
	BulkEditValueCurveButton* BitmapButton_Meteors_YOffsetVC;
	wxStaticText* StaticText128;
	wxStaticText* StaticText130;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;
	wxStaticText* StaticText39;
	wxStaticText* StaticText3;
	wxStaticText* StaticText41;
	wxStaticText* StaticText43;
	wxStaticText* StaticText4;
	xlLockButton* BitmapButton_FadeWithDistance;
	xlLockButton* BitmapButton_MeteorsCount;
	xlLockButton* BitmapButton_MeteorsEffect;
	xlLockButton* BitmapButton_MeteorsLength;
	xlLockButton* BitmapButton_MeteorsSwirlIntensity;
	xlLockButton* BitmapButton_MeteorsType;
	xlLockButton* BitmapButton_Meteors_UseMusic;
	xlLockButton* BitmapButton_Meteors_XOffset;
	xlLockButton* BitmapButton_Meteors_YOffset;
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
	static const long ID_STATICTEXT1;
	static const long ID_SLIDER_Meteors_WamupFrames;
	static const long IDD_TEXTCTRL_Meteors_WamupFrames;
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
	void OnChoice_Meteors_EffectSelect(wxCommandEvent& event);
	//*)

	DECLARE_EVENT_TABLE()
};
