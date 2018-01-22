#ifndef LIGHTNINGPANEL_H
#define LIGHTNINGPANEL_H

//(*Headers(LightningPanel)
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

class LightningPanel: public wxPanel
{
	public:

		LightningPanel(wxWindow* parent);
		virtual ~LightningPanel();

		//(*Declarations(LightningPanel)
		BulkEditSlider* Slider_Number_Bolts;
		BulkEditSlider* Slider_Lightning_BOTY;
		BulkEditValueCurveButton* BitmapButton_Lightning_TopXVC;
		wxStaticText* StaticText205;
		BulkEditCheckBox* CheckBox_ForkedLightning;
		wxStaticText* StaticText210;
		wxBitmapButton* BitmapButton_LightningBOTX;
		BulkEditSlider* Slider_Lightning_TopX;
		wxStaticText* StaticText208;
		wxStaticText* StaticText1;
		wxBitmapButton* BitmapButton_NumberBolts;
		BulkEditValueCurveButton* BitmapButton_Lightning_TopYVC;
		BulkEditTextCtrl* TextCtrl89;
		BulkEditSlider* Slider_Lightning_BOTX;
		BulkEditTextCtrl* TextCtrl90;
		wxBitmapButton* BitmapButton_NumberSegments;
		BulkEditChoice* CHOICE_Lightning_Direction;
		wxBitmapButton* BitmapButton_ForkedLightning;
		wxBitmapButton* BitmapButton_LightningTopY;
		wxBitmapButton* BitmapButton_LightningBOTY;
		wxStaticText* StaticText207;
		BulkEditValueCurveButton* BitmapButton_Number_BoltsVC;
		BulkEditValueCurveButton* BitmapButton_Number_SegmentsVC;
		BulkEditTextCtrl* TextCtrl86;
		wxBitmapButton* BitmapButton_LightningTopX;
		BulkEditTextCtrl* TextCtrl88;
		wxStaticText* StaticText203;
		wxStaticText* StaticText209;
		wxStaticText* StaticText206;
		BulkEditSlider* Slider_Lightning_TopY;
		BulkEditSlider* Slider_Number_Segments;
		//*)

	protected:

		//(*Identifiers(LightningPanel)
		static const long ID_STATICTEXT_Number_Bolts;
		static const long ID_SLIDER_Number_Bolts;
		static const long ID_VALUECURVE_Number_Bolts;
		static const long IDD_TEXTCTRL_Number_Bolts;
		static const long ID_BITMAPBUTTON_SLIDER_Number_Bolts;
		static const long ID_STATICTEXT_Number_Segments;
		static const long ID_SLIDER_Number_Segments;
		static const long ID_VALUECURVE_Number_Segments;
		static const long IDD_TEXTCTRL_Number_Segments;
		static const long ID_BITMAPBUTTON_SLIDER_Number_Segments;
		static const long ID_STATICTEXT_ForkedLightning;
		static const long ID_CHECKBOX_ForkedLightning;
		static const long ID_BITMAPBUTTON_CHECKBOX_ForkedLightning;
		static const long ID_STATICTEXT_Lightning_TopX;
		static const long ID_SLIDER_Lightning_TopX;
		static const long ID_VALUECURVE_Lightning_TopX;
		static const long IDD_TEXTCTRL_Lightning_TopX;
		static const long ID_BITMAPBUTTON_SLIDER_Lightning_TopX;
		static const long ID_STATICTEXT_Lightning_TopY;
		static const long ID_SLIDER_Lightning_TopY;
		static const long ID_VALUECURVE_Lightning_TopY;
		static const long IDD_TEXTCTRL_Lightning_TopY;
		static const long ID_BITMAPBUTTON_SLIDER_Lightning_TopY;
		static const long ID_STATICTEXT_Lightning_BOTX;
		static const long ID_SLIDER_Lightning_BOTX;
		static const long IDD_TEXTCTRL_Lightning_BOTX;
		static const long ID_BITMAPBUTTON_SLIDER_Lightning_BOTX;
		static const long ID_STATICTEXT_Lightning_BOTY;
		static const long ID_SLIDER_Lightning_BOTY;
		static const long IDD_TEXTCTRL_Lightning_BOTY;
		static const long ID_BITMAPBUTTON_SLIDER_Lightning_BOTY;
		static const long ID_STATICTEXT_Lightning_Direction;
		static const long ID_CHOICE_Lightning_Direction;
		//*)

	public:

		//(*Handlers(LightningPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};
#endif
