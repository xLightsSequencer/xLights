#ifndef LIGHTNINGPANEL_H
#define LIGHTNINGPANEL_H

//(*Headers(LightningPanel)
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

class LightningPanel: public wxPanel
{
	public:

		LightningPanel(wxWindow* parent);
		virtual ~LightningPanel();

		//(*Declarations(LightningPanel)
		BulkEditSlider* Slider_Lightning_TopX;
		BulkEditValueCurveButton* BitmapButton_Lightning_TopXVC;
		wxStaticText* StaticText203;
		BulkEditTextCtrl* TextCtrl88;
		BulkEditSlider* Slider_Lightning_TopY;
		wxBitmapButton* BitmapButton22;
		BulkEditTextCtrl* TextCtrl90;
		wxStaticText* StaticText209;
		BulkEditTextCtrl* TextCtrl86;
		wxStaticText* StaticText205;
		wxBitmapButton* BitmapButton26;
		wxBitmapButton* BitmapButton20;
		wxStaticText* StaticText208;
		BulkEditSlider* Slider_Number_Bolts;
		wxStaticText* StaticText1;
		BulkEditSlider* Slider_Lightning_BOTX;
		wxBitmapButton* BitmapButton28;
		BulkEditChoice* CHOICE_Lightning_Direction;
		BulkEditValueCurveButton* BitmapButton_Number_SegmentsVC;
		wxStaticText* StaticText210;
		BulkEditSlider* Slider_Lightning_BOTY;
		BulkEditValueCurveButton* BitmapButton_Number_BoltsVC;
		wxStaticText* StaticText206;
		wxBitmapButton* BitmapButton23;
		BulkEditValueCurveButton* BitmapButton_Lightning_TopYVC;
		BulkEditTextCtrl* TextCtrl89;
		wxStaticText* StaticText207;
		BulkEditCheckBox* CheckBox_ForkedLightning;
		wxBitmapButton* BitmapButton27;
		wxBitmapButton* BitmapButton25;
		BulkEditSlider* Slider_Number_Segments;
		//*)

	protected:

		//(*Identifiers(LightningPanel)
		static const long ID_STATICTEXT_Number_Bolts;
		static const long ID_SLIDER_Number_Bolts;
		static const long ID_VALUECURVE_Number_Bolts;
		static const long IDD_TEXTCTRL_Number_Bolts;
		static const long ID_BITMAPBUTTON38;
		static const long ID_STATICTEXT_Number_Segments;
		static const long ID_SLIDER_Number_Segments;
		static const long ID_VALUECURVE_Number_Segments;
		static const long IDD_TEXTCTRL_Number_Segments;
		static const long ID_BITMAPBUTTON40;
		static const long ID_STATICTEXT_ForkedLightning;
		static const long ID_CHECKBOX_ForkedLightning;
		static const long ID_BITMAPBUTTON41;
		static const long ID_STATICTEXT_Lightning_TopX;
		static const long ID_SLIDER_Lightning_TopX;
		static const long ID_VALUECURVE_Lightning_TopX;
		static const long IDD_TEXTCTRL_Lightning_TopX;
		static const long ID_BITMAPBUTTON42;
		static const long ID_STATICTEXT_Lightning_TopY;
		static const long ID_SLIDER_Lightning_TopY;
		static const long ID_VALUECURVE_Lightning_TopY;
		static const long IDD_TEXTCTRL_Lightning_TopY;
		static const long ID_BITMAPBUTTON43;
		static const long ID_STATICTEXT_Lightning_BOTX;
		static const long ID_SLIDER_Lightning_BOTX;
		static const long IDD_TEXTCTRL_Lightning_BOTX;
		static const long ID_BITMAPBUTTON45;
		static const long ID_STATICTEXT_Lightning_BOTY;
		static const long ID_SLIDER_Lightning_BOTY;
		static const long IDD_TEXTCTRL_Lightning_BOTY;
		static const long ID_BITMAPBUTTON52;
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
