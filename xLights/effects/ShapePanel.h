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

//(*Headers(ShapePanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxChoice;
class wxFilePickerCtrl;
class wxFlexGridSizer;
class wxFontPickerCtrl;
class wxSlider;
class wxSpinCtrl;
class wxSpinEvent;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"
#include <wx/fontpicker.h>
#include <list>

class Emoji;
class ClickableStaticText;

class ShapePanel: public xlEffectPanel
{
    std::list<Emoji*> _emojis;

	public:

		ShapePanel(wxWindow* parent);
		virtual ~ShapePanel();
		virtual void ValidateWindow() override;
		void SetTimingTracks(wxCommandEvent& event);
        void EmojiMenu(wxContextMenuEvent& event);
        void OnPopupEmoji(wxCommandEvent& event);
        void OnShowCharMap(wxCommandEvent& event);

		//(*Declarations(ShapePanel)
		BulkEditCheckBox* CheckBox_FilterLabelReg;
		BulkEditCheckBox* CheckBox_Shape_FadeAway;
		BulkEditCheckBox* CheckBox_Shape_FireTiming;
		BulkEditCheckBox* CheckBox_Shape_HoldColour;
		BulkEditCheckBox* CheckBox_Shape_RandomInitial;
		BulkEditCheckBox* CheckBox_Shape_RandomLocation;
		BulkEditCheckBox* CheckBox_Shape_UseMusic;
		BulkEditCheckBox* CheckBox_Shapes_RandomMovement;
		BulkEditChoice* Choice_Shape_ObjectToDraw;
		BulkEditChoice* Choice_Shape_TimingTrack;
		BulkEditFilePickerCtrl* FilePickerCtrl_SVG;
		BulkEditFontPicker* FontPickerCtrl_Font;
		BulkEditSlider* Slider_Shape_CentreX;
		BulkEditSlider* Slider_Shape_CentreY;
		BulkEditSlider* Slider_Shape_Count;
		BulkEditSlider* Slider_Shape_Growth;
		BulkEditSlider* Slider_Shape_Lifetime;
		BulkEditSlider* Slider_Shape_Points;
		BulkEditSlider* Slider_Shape_Rotation;
		BulkEditSlider* Slider_Shape_Sensitivity;
		BulkEditSlider* Slider_Shape_StartSize;
		BulkEditSlider* Slider_Shape_Thickness;
		BulkEditSlider* Slider_Shapes_Direction;
		BulkEditSlider* Slider_Shapes_Velocity;
		BulkEditSpinCtrl* SpinCtrl_CharCode;
		BulkEditTextCtrl* TextCtrl_Shape_CentreX;
		BulkEditTextCtrl* TextCtrl_Shape_CentreY;
		BulkEditTextCtrl* TextCtrl_Shape_FilterLabel;
		BulkEditTextCtrl* TextCtrl_Shape_Points;
		BulkEditTextCtrl* TextCtrl_Shape_Rotation;
		BulkEditTextCtrl* TextCtrl_Shape_Sensitivity;
		BulkEditTextCtrl* TextCtrl_Shape_Thickness;
		BulkEditTextCtrl* TextCtrl_Shapes_Direction;
		BulkEditTextCtrl* TextCtrl_Shapes_Velocity;
		BulkEditValueCurveButton* BitmapButton_Shape_CentreXVC;
		BulkEditValueCurveButton* BitmapButton_Shape_CentreYVC;
		BulkEditValueCurveButton* BitmapButton_Shape_CountVC;
		BulkEditValueCurveButton* BitmapButton_Shape_GrowthVC;
		BulkEditValueCurveButton* BitmapButton_Shape_LifetimeVC;
		BulkEditValueCurveButton* BitmapButton_Shape_RotationVC;
		BulkEditValueCurveButton* BitmapButton_Shape_StartSizeVC;
		BulkEditValueCurveButton* BitmapButton_Shape_ThicknessVC;
		BulkEditValueCurveButton* BitmapButton_Shapes_Direction;
		BulkEditValueCurveButton* BitmapButton_Shapes_Velocity;
		ClickableStaticText* EmojiDisplay;
		wxChoice* SkinToneChoice;
		wxStaticText* StaticText10;
		wxStaticText* StaticText11;
		wxStaticText* StaticText12;
		wxStaticText* StaticText13;
		wxStaticText* StaticText15;
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
		//*)

	protected:

		//(*Identifiers(ShapePanel)
		static const long ID_STATICTEXT_Shape_ObjectToDraw;
		static const long ID_CHOICE_Shape_ObjectToDraw;
		static const long ID_FONTPICKER_Shape_Font;
		static const long ID_SPINCTRL_Shape_Char;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE_Shape_SkinTone;
		static const long ID_STATICTEXT1;
		static const long ID_FILEPICKERCTRL_SVG;
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
		static const long ID_CHECKBOX_Shape_RandomInitial;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER_Shapes_Velocity;
		static const long ID_VALUECURVE_Shapes_Velocity;
		static const long ID_TEXTCTRL_Shapes_Velocity;
		static const long ID_STATICTEXT4;
		static const long ID_SLIDER_Shapes_Direction;
		static const long ID_VALUECURVE_Shapes_Direction;
		static const long ID_TEXTCTRL_Shapes_Direction;
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
		static const long ID_STATICTEXT_Shape_Rotation;
		static const long ID_SLIDER_Shape_Rotation;
		static const long ID_VALUECURVE_Shape_Rotation;
		static const long IDD_TEXTCTRL_Shape_Rotation;
		static const long ID_CHECKBOX_Shape_RandomLocation;
		static const long ID_CHECKBOX_Shapes_RandomMovement;
		static const long ID_CHECKBOX_Shape_FadeAway;
		static const long ID_CHECKBOX_Shape_HoldColour;
		static const long ID_CHECKBOX_Shape_UseMusic;
		static const long ID_STATICTEXT_Shape_Sensitivity;
		static const long ID_SLIDER_Shape_Sensitivity;
		static const long IDD_TEXTCTRL_Shape_Sensitivity;
		static const long ID_CHECKBOX_Shape_FireTiming;
		static const long ID_STATICTEXT_Shape_FireTimingTrack;
		static const long ID_CHOICE_Shape_FireTimingTrack;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL_Shape_FilterLabel;
		static const long ID_CHECKBOX_Shape_FilterReg;
		//*)

	public:

		//(*Handlers(ShapePanel)
		void OnChoice_Shape_ObjectToDrawSelect(wxCommandEvent& event);
		void OnCheckBox_Shape_RandomLocationClick(wxCommandEvent& event);
		void OnCheckBox_Shape_UseMusicClick(wxCommandEvent& event);
		void OnCheckBox_Shape_FireTimingClick(wxCommandEvent& event);
		void OnChoice_Shape_TimingTrackSelect(wxCommandEvent& event);
		void OnFontPickerCtrl_FontFontChanged(wxFontPickerEvent& event);
		void OnSpinCtrl_CharCodeChange(wxSpinEvent& event);
		void OnCheckBox_Shapes_RandomMovementClick(wxCommandEvent& event);
		void OnSkinToneChoiceSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
