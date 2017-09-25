#ifndef BUFFERPANEL_H
#define BUFFERPANEL_H

//(*Headers(BufferPanel)
#include <wx/scrolwin.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
//*)

#include "ValueCurve.h"
#include "ValueCurveButton.h"
#include <string>

class Model;
class SubBufferPanel;

class BufferPanel: public wxPanel
{
    void ValidateWindow();
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

    
		//(*Declarations(BufferPanel)
		wxStaticText* StaticText10;
		wxBitmapButton* BitmapButton_EffectBlur;
		wxBitmapButton* BitmapButton_Zoom;
		wxSlider* Slider_YRotation;
		wxNotebook* Notebook1;
		wxSlider* Slider_Zoom;
		wxBitmapButton* BitmapButton_Rotation;
		wxTextCtrl* TextCtrl_XPivot;
		wxStaticText* StaticText13;
		wxTextCtrl* TextCtrl_Rotation;
		wxStaticText* StaticText14;
		wxScrolledWindow* ScrolledWindow1;
		wxSlider* Slider_PivotPointY;
		wxChoice* BufferStyleChoice;
		wxTextCtrl* TextCtrl_Rotations;
		ValueCurveButton* BitmapButton_XPivot;
		wxFlexGridSizer* SubBufferPanelSizer;
		wxFlexGridSizer* BufferSizer;
		ValueCurveButton* BitmapButton_YPivot;
		wxSlider* Slider_XPivot;
		wxBitmapButton* BitmapButton_OverlayBkg;
		wxSlider* Slider_XRotation;
		wxSlider* Slider_PivotPointX;
		wxTextCtrl* TextCtrl_PivotPointX;
		wxChoice* BufferTransform;
		wxTextCtrl* TextCtrl_XRotation;
		wxBitmapButton* BitmapButton_Rotations;
		wxTextCtrl* TextCtrl_ZoomQuality;
		wxSlider* Slider_ZoomQuality;
		wxStaticText* StaticText5;
		ValueCurveButton* BitmapButton_VCPivotPointY;
		wxTextCtrl* TextCtrl_Zoom;
		wxTextCtrl* TextCtrl_EffectBlur;
		ValueCurveButton* BitmapButton_YRotation;
		ValueCurveButton* BitmapButton_VCRotation;
		wxScrolledWindow* ScrolledWindow2;
		wxChoice* Choice_Preset;
		wxSlider* Slider_YPivot;
		wxSlider* Slider_Rotation;
		ValueCurveButton* BitmapButton_VCZoom;
		ValueCurveButton* BitmapButton_Blur;
		wxTextCtrl* TextCtrl_YPivot;
		wxTextCtrl* TextCtrl_PivotPointY;
		wxCheckBox* CheckBox_ResetBufferPanel;
		wxStaticText* StaticText12;
		wxSlider* Slider_Rotations;
		wxTextCtrl* TextCtrl_YRotation;
		wxCheckBox* CheckBox_OverlayBkg;
		wxFlexGridSizer* RotoZoomSizer;
		ValueCurveButton* BitmapButton_VCPivotPointX;
		wxBitmapButton* BitmapButton_PivotPointX;
		ValueCurveButton* BitmapButton_XRotation;
		wxBitmapButton* BitmapButton_ZoomQuality;
		ValueCurveButton* BitmapButton_VCRotations;
		wxFlexGridSizer* FullBufferSizer;
		wxBitmapButton* BitmapButton_PivotPointY;
		wxSlider* Slider_EffectBlur;
		//*)

        SubBufferPanel *subBufferPanel;
	protected:

		//(*Identifiers(BufferPanel)
		static const long ID_CHECKBOX_ResetBufferPanel;
		static const long ID_CHOICE_BufferStyle;
		static const long ID_BITMAPBUTTON_CHOICE_BufferStyle;
		static const long ID_CHOICE_BufferTransform;
		static const long ID_BITMAPBUTTON_CHOICE_BufferTransform;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_Blur;
		static const long ID_VALUECURVE_Blur;
		static const long IDD_TEXTCTRL_Blur;
		static const long ID_BITMAPBUTTON_SLIDER_EffectBlur;
		static const long ID_CHECKBOX_OverlayBkg;
		static const long ID_BITMAPBUTTON_OverlayBkg;
		static const long ID_SCROLLEDWINDOW1;
		static const long ID_PANEL1;
		static const long ID_CHOICE_Preset;
		static const long ID_SLIDER_Rotation;
		static const long ID_VALUECURVE_Rotation;
		static const long IDD_TEXTCTRL_Rotation;
		static const long ID_BITMAPBUTTON_Rotation;
		static const long ID_SLIDER_Rotations;
		static const long ID_VALUECURVE_Rotations;
		static const long IDD_TEXTCTRL_Rotations;
		static const long ID_BITMAPBUTTON_Rotations;
		static const long ID_SLIDER_PivotPointX;
		static const long ID_VALUECURVE_PivotPointX;
		static const long IDD_TEXTCTRL_PivotPointX;
		static const long ID_BITMAPBUTTON_PivotPointX;
		static const long ID_SLIDER_PivotPointY;
		static const long ID_VALUECURVE_PivotPointY;
		static const long IDD_TEXTCTRL_PivotPointY;
		static const long ID_BITMAPBUTTON_PivotPointY;
		static const long ID_SLIDER_Zoom;
		static const long ID_VALUECURVE_Zoom;
		static const long IDD_TEXTCTRL_Zoom;
		static const long ID_BITMAPBUTTON_Zoom;
		static const long ID_SLIDER_ZoomQuality;
		static const long IDD_TEXTCTRL_ZoomQuality;
		static const long ID_BITMAPBUTTON_ZoomQuality;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_XRotation;
		static const long ID_VALUECURVE_XRotation;
		static const long IDD_TEXTCTRL_XRotation;
		static const long ID_STATICTEXT4;
		static const long ID_SLIDER_XPivot;
		static const long ID_VALUECURVE_XPivot;
		static const long IDD_TEXTCTRL_XPivot;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER_YRotation;
		static const long ID_VALUECURVE_YRotation;
		static const long IDD_TEXTCTRL_YRotation;
		static const long ID_STATICTEXT5;
		static const long ID_SLIDER_YPivot;
		static const long ID_VALUECURVE_YPivot;
		static const long IDD_TEXTCTRL_YPivot;
		static const long ID_SCROLLEDWINDOW2;
		static const long ID_PANEL4;
		static const long ID_NOTEBOOK1;
		//*)

	private:

		//(*Handlers(BufferPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnBitmapButton_BlurClick(wxCommandEvent& event);
		void OnBitmapButton_RotationClick(wxCommandEvent& event);
		void OnBitmapButton_ZoomClick(wxCommandEvent& event);
		void OnSlider_EffectBlurCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_ZoomCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_RotationCmdSliderUpdated(wxScrollEvent& event);
		void OnBufferTransformSelect(wxCommandEvent& event);
		void OnSlider_RotationsCmdSliderUpdated(wxScrollEvent& event);
		void OnTextCtrl_RotationsText(wxCommandEvent& event);
		void OnTextCtrl_ZoomText(wxCommandEvent& event);
		void OnSlider_PivotPointXCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_PivotPointYCmdSliderUpdated(wxScrollEvent& event);
		void OnBitmapButton_VCRotationsClick(wxCommandEvent& event);
		void OnBitmapButton_VCPivotPointXClick(wxCommandEvent& event);
		void OnBitmapButton_VCPivotPointYClick(wxCommandEvent& event);
		void OnChoice_PresetSelect(wxCommandEvent& event);
		void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
		void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnButton_ResetClick(wxCommandEvent& event);
		void UpdateLinkedSliderFloat2(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
		void OnCheckBox_ResetBufferPanelClick(wxCommandEvent& event);
		void OnBitmapButton_YRotationClick(wxCommandEvent& event);
		void OnBitmapButton_XRotationClick(wxCommandEvent& event);
		void OnSlider_XRotationCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_YRotationCmdSliderUpdated(wxScrollEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
