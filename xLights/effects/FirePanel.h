#ifndef FIREPANEL_H
#define FIREPANEL_H

//(*Headers(FirePanel)
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

class FirePanel: public wxPanel
{
    void ValidateWindow();
	public:

		FirePanel(wxWindow* parent);
		virtual ~FirePanel();

		//(*Declarations(FirePanel)
		wxStaticText* StaticText193;
		wxStaticText* StaticText132;
		wxBitmapButton* BitmapButton_FireGrow;
		BulkEditValueCurveButton* BitmapButton_Fire_GrowthCyclesVC;
		BulkEditSliderF1* Slider_Fire_GrowthCycles;
		BulkEditValueCurveButton* BitmapButton_Fire_HeightVC;
		wxStaticText* StaticText83;
		wxBitmapButton* BitmapButton_Fire_GrowWithMusic;
		BulkEditSlider* Slider_Fire_Height;
		BulkEditValueCurveButton* BitmapButton_Fire_HueShiftVC;
		BulkEditCheckBox* CheckBox_Fire_GrowWithMusic;
		BulkEditTextCtrl* TextCtrl_Fire_Height;
		wxStaticText* StaticText30;
		BulkEditSlider* Slider_Fire_HueShift;
		wxBitmapButton* BitmapButton_FireWidth;
		BulkEditChoice* Choice_Fire_Location;
		wxBitmapButton* BitmapButton_FireHeight;
		BulkEditTextCtrlF1* TextCtrl_Fire_GrowthCycles;
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
		static const long ID_BITMAPBUTTON_SLIDER_Fire_GrowthCycles;
		static const long ID_CHECKBOX_Fire_GrowWithMusic;
		static const long ID_BITMAPBUTTON_CHECKBOX_Fire_GrowWithMusic;
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
