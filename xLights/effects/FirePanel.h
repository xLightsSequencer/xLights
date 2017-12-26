#ifndef FIREPANEL_H
#define FIREPANEL_H

//(*Headers(FirePanel)
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

class FirePanel: public wxPanel
{
    void ValidateWindow();
	public:

		FirePanel(wxWindow* parent);
		virtual ~FirePanel();

		//(*Declarations(FirePanel)
		wxStaticText* StaticText193;
		wxBitmapButton* BitmapButton_FireWidth;
		wxStaticText* StaticText83;
		wxStaticText* StaticText30;
		BulkEditValueCurveButton* BitmapButton_Fire_HueShiftVC;
		BulkEditTextCtrlF1* TextCtrl_Fire_GrowthCycles;
		BulkEditSlider* Slider_Fire_Height;
		BulkEditSliderF1* Slider_Fire_GrowthCycles;
		wxBitmapButton* BitmapButton_FireGrow;
		BulkEditValueCurveButton* BitmapButton_Fire_GrowthCyclesVC;
		BulkEditSlider* Slider_Fire_HueShift;
		BulkEditCheckBox* CheckBox_Fire_GrowWithMusic;
		BulkEditTextCtrl* TextCtrl_Fire_Height;
		wxStaticText* StaticText132;
		BulkEditChoice* Choice_Fire_Location;
		wxBitmapButton* BitmapButton_Fire_GrowWithMusic;
		wxBitmapButton* BitmapButton_FireHeight;
		BulkEditValueCurveButton* BitmapButton_Fire_HeightVC;
		//*)

	protected:

		//(*Identifiers(FirePanel)
		static const long ID_STATICTEXT_Fire_Height;
		static const long ID_SLIDER_Fire_Height;
		static const long ID_VALUECURVE_Fire_Height;
		static const long IDD_TEXTCTRL_Fire_Height;
		static const long ID_BITMAPBUTTON_SLIDER_Fire_Height;
		static const long ID_STATICTEXT_Fire_HueShift;
		static const long ID_SLIDER_Fire_HueShift;
		static const long ID_VALUECURVE_Fire_HueShift;
		static const long IDD_TEXTCTRL_Fire_HueShift;
		static const long ID_BITMAPBUTTON_SLIDER_Fire_HueShift;
		static const long ID_STATICTEXT_Fire_GrowthCycles;
		static const long IDD_SLIDER_Fire_GrowthCycles;
		static const long ID_VALUECURVE_Fire_GrowthCycles;
		static const long ID_TEXTCTRL_Fire_GrowthCycles;
		static const long ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire;
		static const long ID_CHECKBOX_Fire_GrowWithMusic;
		static const long ID_BITMAPBUTTON_Fire_GrowWithMusic;
		static const long ID_STATICTEXT_Fire_Location;
		static const long ID_CHOICE_Fire_Location;
		//*)

	public:

		//(*Handlers(FirePanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnCheckBox_Fire_GrowWithMusicClick(wxCommandEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
