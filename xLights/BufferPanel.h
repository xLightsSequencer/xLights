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

 //(*Headers(BufferPanel)
 #include <wx/bmpbuttn.h>
 #include <wx/checkbox.h>
 #include <wx/choice.h>
 #include <wx/notebook.h>
 #include <wx/panel.h>
 #include <wx/scrolwin.h>
 #include <wx/sizer.h>
 #include <wx/slider.h>
 #include <wx/spinctrl.h>
 #include <wx/stattext.h>
 #include <wx/textctrl.h>
 //*)

#include <string>

#include "BulkEditControls.h"
#include "effects/EffectPanelUtils.h"

#define BLUR_MIN 1
#define BLUR_MAX 15

#define RZ_ROTATION_MIN 0
#define RZ_ROTATION_MAX 100

#define RZ_ZOOM_MIN 0
#define RZ_ZOOM_MAX 30
#define RZ_ZOOM_DIVISOR 10

#define RZ_ROTATIONS_MIN 0
#define RZ_ROTATIONS_MAX 200
#define RZ_ROTATIONS_DIVISOR 10

#define RZ_PIVOTX_MIN 0
#define RZ_PIVOTX_MAX 100

#define RZ_PIVOTY_MIN 0
#define RZ_PIVOTY_MAX 100

#define RZ_XROTATION_MIN 0
#define RZ_XROTATION_MAX 360

#define RZ_YROTATION_MIN 0
#define RZ_YROTATION_MAX 360

#define RZ_XPIVOT_MIN 0
#define RZ_XPIVOT_MAX 100

#define RZ_YPIVOT_MIN 0
#define RZ_YPIVOT_MAX 100

#define SB_LEFT_BOTTOM_MIN (-100)
#define SB_LEFT_BOTTOM_MAX 99

#define SB_RIGHT_TOP_MIN 1
#define SB_RIGHT_TOP_MAX 200

#define SB_CENTRE_MIN (-100)
#define SB_CENTRE_MAX 100

class Model;
class SubBufferPanel;

class BufferPanel: public xlEffectPanel
{
public:

		BufferPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~BufferPanel();

		static bool CanRenderBufferUseCamera(const std::string& rb);

        wxString GetBufferString();
        void SetDefaultControls(const Model *model, bool optionbased = false);
        void UpdateBufferStyles(const Model* model);
        void UpdateCamera(const Model* model);
        void ValidateWindow();

		static double GetSettingVCMin(const std::string& name)
        {
            if (name == "B_VALUECURVE_Blur")
                return BLUR_MIN;
            if (name == "B_VALUECURVE_Rotation")
                return RZ_ROTATION_MIN;
            if (name == "B_VALUECURVE_Zoom")
                return RZ_ZOOM_MIN;
            if (name == "B_VALUECURVE_Rotations")
                return RZ_ROTATIONS_MIN;
            if (name == "B_VALUECURVE_PivotPointX")
                return RZ_PIVOTX_MIN;
            if (name == "B_VALUECURVE_PivotPointY")
                return RZ_PIVOTY_MIN;
            if (name == "B_VALUECURVE_XRotation")
                return RZ_XROTATION_MIN;
            if (name == "B_VALUECURVE_YRotation")
                return RZ_YROTATION_MIN;
            if (name == "B_VALUECURVE_XPivot")
                return RZ_XPIVOT_MIN;
            if (name == "B_VALUECURVE_YPivot")
                return RZ_YPIVOT_MIN;
            //if (name == "B_VALUECURVE_")
            //    return SB_LEFT_BOTTOM_MIN;
            //if (name == "B_VALUECURVE_")
            //    return SB_RIGHT_TOP_MIN;
            //if (name == "B_VALUECURVE_")
            //    return SB_CENTRE_MIN;
            wxASSERT(false);
            return 0;
        }

        static double GetSettingVCMax(const std::string& name)
        {
            if (name == "B_VALUECURVE_Blur")
                return BLUR_MAX;
            if (name == "B_VALUECURVE_Rotation")
                return RZ_ROTATION_MAX;
            if (name == "B_VALUECURVE_Zoom")
                return RZ_ZOOM_MAX;
            if (name == "B_VALUECURVE_Rotations")
                return RZ_ROTATIONS_MAX;
            if (name == "B_VALUECURVE_PivotPointX")
                return RZ_PIVOTX_MAX;
            if (name == "B_VALUECURVE_PivotPointY")
                return RZ_PIVOTY_MAX;
            if (name == "B_VALUECURVE_XRotation")
                return RZ_XROTATION_MAX;
            if (name == "B_VALUECURVE_YRotation")
                return RZ_YROTATION_MAX;
            if (name == "B_VALUECURVE_XPivot")
                return RZ_XPIVOT_MAX;
            if (name == "B_VALUECURVE_YPivot")
                return RZ_YPIVOT_MAX;
            //if (name == "B_VALUECURVE_")
            //    return SB_LEFT_BOTTOM_MAX;
            //if (name == "B_VALUECURVE_")
            //    return SB_RIGHT_TOP_MAX;
            //if (name == "B_VALUECURVE_")
            //    return SB_CENTRE_MAX;
            wxASSERT(false);
            return 100;
        }

		static int GetSettingVCDivisor(const std::string& name)
        {
            if (name == "B_VALUECURVE_")
                return RZ_ZOOM_DIVISOR;
            if (name == "B_VALUECURVE_")
                return RZ_ROTATIONS_DIVISOR;
            return 1;
        }

		//(*Declarations(BufferPanel)
		BulkEditCheckBox* CheckBox_OverlayBkg;
		BulkEditChoice* BufferStyleChoice;
		BulkEditChoice* BufferTransform;
		BulkEditChoice* ChoiceRotateOrder;
		BulkEditChoice* Choice_PerPreviewCamera;
		BulkEditSlider* Slider_EffectBlur;
		BulkEditSlider* Slider_PivotPointX;
		BulkEditSlider* Slider_PivotPointY;
		BulkEditSlider* Slider_Rotation;
		BulkEditSlider* Slider_XPivot;
		BulkEditSlider* Slider_XRotation;
		BulkEditSlider* Slider_YPivot;
		BulkEditSlider* Slider_YRotation;
		BulkEditSlider* Slider_ZoomQuality;
		BulkEditSliderF1* Slider_Rotations;
		BulkEditSliderF1* Slider_Zoom;
		BulkEditTextCtrl* TextCtrl_EffectBlur;
		BulkEditTextCtrl* TextCtrl_PivotPointX;
		BulkEditTextCtrl* TextCtrl_PivotPointY;
		BulkEditTextCtrl* TextCtrl_Rotation;
		BulkEditTextCtrl* TextCtrl_XPivot;
		BulkEditTextCtrl* TextCtrl_XRotation;
		BulkEditTextCtrl* TextCtrl_YPivot;
		BulkEditTextCtrl* TextCtrl_YRotation;
		BulkEditTextCtrl* TextCtrl_ZoomQuality;
		BulkEditTextCtrlF1* TextCtrl_Rotations;
		BulkEditTextCtrlF1* TextCtrl_Zoom;
		BulkEditValueCurveButton* BitmapButton_Blur;
		BulkEditValueCurveButton* BitmapButton_VCPivotPointX;
		BulkEditValueCurveButton* BitmapButton_VCPivotPointY;
		BulkEditValueCurveButton* BitmapButton_VCRotation;
		BulkEditValueCurveButton* BitmapButton_VCRotations;
		BulkEditValueCurveButton* BitmapButton_VCZoom;
		BulkEditValueCurveButton* BitmapButton_XPivot;
		BulkEditValueCurveButton* BitmapButton_XRotation;
		BulkEditValueCurveButton* BitmapButton_YPivot;
		BulkEditValueCurveButton* BitmapButton_YRotation;
		wxCheckBox* CheckBox_ResetBufferPanel;
		wxChoice* Choice_Preset;
		wxFlexGridSizer* BufferSizer;
		wxFlexGridSizer* FullBufferSizer;
		wxFlexGridSizer* RotoZoomSizer;
		wxFlexGridSizer* SubBufferPanelSizer;
		wxNotebook* Notebook1;
		wxScrolledWindow* ScrolledWindow1;
		wxScrolledWindow* ScrolledWindow2;
		wxSpinCtrl* SpinCtrl_BufferStagger;
		wxStaticText* StaticText10;
		wxStaticText* StaticText12;
		wxStaticText* StaticText13;
		wxStaticText* StaticText14;
		wxStaticText* StaticText15;
		wxStaticText* StaticText16;
		wxStaticText* StaticText17;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		xlLockButton* BitmapButton_EffectBlur;
		xlLockButton* BitmapButton_OverlayBkg;
		xlLockButton* BitmapButton_PivotPointX;
		xlLockButton* BitmapButton_PivotPointY;
		xlLockButton* BitmapButton_Rotation;
		xlLockButton* BitmapButton_Rotations;
		xlLockButton* BitmapButton_Zoom;
		xlLockButton* BitmapButton_ZoomQuality;
		//*)

        SubBufferPanel *subBufferPanel = nullptr;
        std::string _defaultCamera = "2D";
        bool _mg = false;

	protected:

		//(*Identifiers(BufferPanel)
		static const long ID_CHECKBOX_ResetBufferPanel;
		static const long ID_STATICTEXT_BufferStyle;
		static const long ID_CHOICE_BufferStyle;
		static const long ID_BITMAPBUTTON_CHOICE_BufferStyle;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL_BufferStagger;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE_PerPreviewCamera;
		static const long ID_STATICTEXT_BufferTransform;
		static const long ID_CHOICE_BufferTransform;
		static const long ID_BITMAPBUTTON_CHOICE_BufferTransform;
		static const long ID_STATICTEXT_Blur;
		static const long ID_SLIDER_Blur;
		static const long ID_VALUECURVE_Blur;
		static const long IDD_TEXTCTRL_Blur;
		static const long ID_BITMAPBUTTON_SLIDER_EffectBlur;
		static const long ID_CHECKBOX_OverlayBkg;
		static const long ID_BITMAPBUTTON_OverlayBkg;
		static const long ID_SCROLLEDWINDOW1;
		static const long ID_PANEL1;
		static const long ID_CHOICE_Preset;
		static const long ID_STATICTEXT_Rotation;
		static const long ID_SLIDER_Rotation;
		static const long ID_VALUECURVE_Rotation;
		static const long IDD_TEXTCTRL_Rotation;
		static const long ID_BITMAPBUTTON_Rotation;
		static const long ID_STATICTEXT_Rotations;
		static const long ID_SLIDER_Rotations;
		static const long ID_VALUECURVE_Rotations;
		static const long IDD_TEXTCTRL_Rotations;
		static const long ID_BITMAPBUTTON_Rotations;
		static const long ID_STATICTEXT_PivotPointX;
		static const long ID_SLIDER_PivotPointX;
		static const long ID_VALUECURVE_PivotPointX;
		static const long IDD_TEXTCTRL_PivotPointX;
		static const long ID_BITMAPBUTTON_PivotPointX;
		static const long ID_STATICTEXT_PivotPointY;
		static const long ID_SLIDER_PivotPointY;
		static const long ID_VALUECURVE_PivotPointY;
		static const long IDD_TEXTCTRL_PivotPointY;
		static const long ID_BITMAPBUTTON_PivotPointY;
		static const long ID_STATICTEXT_Zoom;
		static const long ID_SLIDER_Zoom;
		static const long ID_VALUECURVE_Zoom;
		static const long IDD_TEXTCTRL_Zoom;
		static const long ID_BITMAPBUTTON_Zoom;
		static const long ID_STATICTEXT_ZoomQuality;
		static const long ID_SLIDER_ZoomQuality;
		static const long IDD_TEXTCTRL_ZoomQuality;
		static const long ID_BITMAPBUTTON_ZoomQuality;
		static const long ID_STATICTEXT_XRotation;
		static const long ID_SLIDER_XRotation;
		static const long ID_VALUECURVE_XRotation;
		static const long IDD_TEXTCTRL_XRotation;
		static const long ID_STATICTEXT_XPivot;
		static const long ID_SLIDER_XPivot;
		static const long ID_VALUECURVE_XPivot;
		static const long IDD_TEXTCTRL_XPivot;
		static const long ID_STATICTEXT_YRotation;
		static const long ID_SLIDER_YRotation;
		static const long ID_VALUECURVE_YRotation;
		static const long IDD_TEXTCTRL_YRotation;
		static const long ID_STATICTEXT_YPivot;
		static const long ID_SLIDER_YPivot;
		static const long ID_VALUECURVE_YPivot;
		static const long IDD_TEXTCTRL_YPivot;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_RZ_RotationOrder;
		static const long ID_SCROLLEDWINDOW2;
		static const long ID_PANEL4;
		static const long ID_NOTEBOOK1;
		//*)

	private:

		//(*Handlers(BufferPanel)
		void OnBufferTransformSelect(wxCommandEvent& event);
		void OnChoice_PresetSelect(wxCommandEvent& event);
		void OnButton_ResetClick(wxCommandEvent& event);
		void OnCheckBox_ResetBufferPanelClick(wxCommandEvent& event);
		void OnBufferStyleChoiceSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
