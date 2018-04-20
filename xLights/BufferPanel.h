#ifndef BUFFERPANEL_H
#define BUFFERPANEL_H

//(*Headers(BufferPanel)
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include "BulkEditControls.h"
#include <string>

class Model;
class SubBufferPanel;

class BufferPanel: public wxPanel
{
    void OnVCChanged(wxCommandEvent& event);

    int __blur;
    int __rotation;
    float __rotations;
    float __zoom;
    int __zoomquality;
    int __pivotpointx;
    int __pivotpointy;

public:

		BufferPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~BufferPanel();

        wxString GetBufferString();
        void SetDefaultControls(const Model *model, bool optionbased = false);
        void ValidateWindow();
    
		//(*Declarations(BufferPanel)
		BulkEditCheckBox* CheckBox_OverlayBkg;
		BulkEditChoice* BufferStyleChoice;
		BulkEditChoice* BufferTransform;
		BulkEditChoice* ChoiceRotateOrder;
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
		wxBitmapButton* BitmapButton_EffectBlur;
		wxBitmapButton* BitmapButton_OverlayBkg;
		wxBitmapButton* BitmapButton_PivotPointX;
		wxBitmapButton* BitmapButton_PivotPointY;
		wxBitmapButton* BitmapButton_Rotation;
		wxBitmapButton* BitmapButton_Rotations;
		wxBitmapButton* BitmapButton_Zoom;
		wxBitmapButton* BitmapButton_ZoomQuality;
		wxCheckBox* CheckBox_ResetBufferPanel;
		wxChoice* Choice_Preset;
		wxFlexGridSizer* BufferSizer;
		wxFlexGridSizer* FullBufferSizer;
		wxFlexGridSizer* RotoZoomSizer;
		wxFlexGridSizer* SubBufferPanelSizer;
		wxNotebook* Notebook1;
		wxScrolledWindow* ScrolledWindow1;
		wxScrolledWindow* ScrolledWindow2;
		wxStaticText* StaticText10;
		wxStaticText* StaticText12;
		wxStaticText* StaticText13;
		wxStaticText* StaticText14;
		wxStaticText* StaticText15;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		//*)

        SubBufferPanel *subBufferPanel;
	protected:

		//(*Identifiers(BufferPanel)
		static const long ID_CHECKBOX_ResetBufferPanel;
		static const long ID_STATICTEXT_BufferStyle;
		static const long ID_CHOICE_BufferStyle;
		static const long ID_BITMAPBUTTON_CHOICE_BufferStyle;
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
		void OnLockButtonClick(wxCommandEvent& event);
		void OnBufferTransformSelect(wxCommandEvent& event);
		void OnChoice_PresetSelect(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnButton_ResetClick(wxCommandEvent& event);
		void OnCheckBox_ResetBufferPanelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
