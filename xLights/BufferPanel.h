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

#include "BulkEditControls.h"
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
		BulkEditTextCtrl* TextCtrl_XRotation;
		wxStaticText* StaticText9;
		BulkEditTextCtrl* TextCtrl_PivotPointY;
		BulkEditSlider* Slider_PivotPointX;
		wxBitmapButton* BitmapButton_EffectBlur;
		wxBitmapButton* BitmapButton_Zoom;
		wxNotebook* Notebook1;
		wxBitmapButton* BitmapButton_Rotation;
		BulkEditValueCurveButton* BitmapButton_XPivot;
		BulkEditSlider* Slider_EffectBlur;
		BulkEditTextCtrlF1* TextCtrl_Zoom;
		BulkEditSlider* Slider_XRotation;
		BulkEditSlider* Slider_YRotation;
		wxStaticText* StaticText13;
		wxStaticText* StaticText2;
		wxStaticText* StaticText14;
		wxScrolledWindow* ScrolledWindow1;
		wxStaticText* StaticText6;
		wxFlexGridSizer* SubBufferPanelSizer;
		wxFlexGridSizer* BufferSizer;
		BulkEditTextCtrl* TextCtrl_YPivot;
		wxStaticText* StaticText8;
		BulkEditValueCurveButton* BitmapButton_VCZoom;
		BulkEditSlider* Slider_PivotPointY;
		wxBitmapButton* BitmapButton_OverlayBkg;
		BulkEditTextCtrlF1* TextCtrl_Rotations;
		BulkEditCheckBox* CheckBox_OverlayBkg;
		wxStaticText* StaticText1;
		BulkEditTextCtrl* TextCtrl_YRotation;
		BulkEditTextCtrl* TextCtrl_ZoomQuality;
		wxStaticText* StaticText3;
		BulkEditTextCtrl* TextCtrl_PivotPointX;
		BulkEditValueCurveButton* BitmapButton_XRotation;
		BulkEditSlider* Slider_Rotation;
		wxBitmapButton* BitmapButton_Rotations;
		BulkEditChoice* BufferTransform;
		BulkEditTextCtrl* TextCtrl_XPivot;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		BulkEditValueCurveButton* BitmapButton_VCPivotPointY;
		wxScrolledWindow* ScrolledWindow2;
		BulkEditValueCurveButton* BitmapButton_YRotation;
		wxChoice* Choice_Preset;
		BulkEditValueCurveButton* BitmapButton_VCRotations;
		BulkEditSlider* Slider_YPivot;
		BulkEditSlider* Slider_XPivot;
		wxCheckBox* CheckBox_ResetBufferPanel;
		wxStaticText* StaticText12;
		BulkEditSliderF1* Slider_Zoom;
		BulkEditValueCurveButton* BitmapButton_VCPivotPointX;
		wxFlexGridSizer* RotoZoomSizer;
		wxBitmapButton* BitmapButton_PivotPointX;
		BulkEditTextCtrl* TextCtrl_Rotation;
		wxStaticText* StaticText4;
		wxBitmapButton* BitmapButton_ZoomQuality;
		wxFlexGridSizer* FullBufferSizer;
		BulkEditSlider* Slider_ZoomQuality;
		BulkEditValueCurveButton* BitmapButton_Blur;
		BulkEditTextCtrl* TextCtrl_EffectBlur;
		BulkEditChoice* BufferStyleChoice;
		wxBitmapButton* BitmapButton_PivotPointY;
		BulkEditValueCurveButton* BitmapButton_YPivot;
		BulkEditSliderF1* Slider_Rotations;
		BulkEditValueCurveButton* BitmapButton_VCRotation;
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
		static const long ID_SCROLLEDWINDOW2;
		static const long ID_PANEL4;
		static const long ID_NOTEBOOK1;
		//*)

	private:

		//(*Handlers(BufferPanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnBufferTransformSelect(wxCommandEvent& event);
		void OnChoice_PresetSelect(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnButton_ResetClick(wxCommandEvent& event);
		void OnCheckBox_ResetBufferPanelClick(wxCommandEvent& event);
		//*)

        void UpdateLinkedSliderFloat(wxCommandEvent& event);
        void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
        void UpdateLinkedTextCtrl360(wxScrollEvent& event);
        void UpdateLinkedSlider360(wxCommandEvent& event);
        void UpdateLinkedTextCtrl(wxScrollEvent& event);
        void UpdateLinkedSlider(wxCommandEvent& event);
        void UpdateLinkedSliderFloat2(wxCommandEvent& event);
        void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
        void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
        void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);

		DECLARE_EVENT_TABLE()
};

#endif
