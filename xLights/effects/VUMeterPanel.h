#ifndef VUMETERPANEL_H
#define VUMETERPANEL_H

//(*Headers(VUMeterPanel)
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

class VUMeterPanel: public wxPanel
{
	public:

		VUMeterPanel(wxWindow* parent);
		virtual ~VUMeterPanel();

		//(*Declarations(VUMeterPanel)
		wxStaticText* StaticText10;
		wxStaticText* StaticText9;
		BulkEditSlider* Slider_VUMeter_Sensitivity;
		BulkEditChoice* Choice_VUMeter_Shape;
		BulkEditChoice* Choice_VUMeter_TimingTrack;
		wxBitmapButton* BitmapButton_VUMeter_EndNote;
		wxStaticText* StaticText13;
		wxStaticText* StaticText2;
		BulkEditSlider* Slider_VUMeter_StartNote;
		wxStaticText* StaticText14;
		BulkEditTextCtrl* TextCtrl_VUMeter_Bars;
		wxStaticText* StaticText6;
		wxBitmapButton* BitmapButton_VUMeter_SlowDownFalls;
		BulkEditSlider* Slider_VUMeter_EndNote;
		wxStaticText* StaticText8;
		wxStaticText* StaticText11;
		wxBitmapButton* BitmapButton_VUMeter_Sensitivity;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxBitmapButton* BitmapButton_VUMeter_Bars;
		wxBitmapButton* BitmapButton_VUMeter_Type;
		BulkEditSlider* Slider_VUMeter_Bars;
		wxBitmapButton* BitmapButton_VUMeter_XOffset;
		BulkEditTextCtrl* TextCtrl_VUMeter_StartNote;
		BulkEditCheckBox* CheckBox_VUMeter_SlowDownFalls;
		BulkEditSlider* Slider_VUMeter_YOffset;
		BulkEditTextCtrl* TextCtrl_VUMeter_XOffset;
		wxBitmapButton* BitmapButton_VUMeter_Shape;
		BulkEditTextCtrl* TextCtrl_VUMeter_YOffset;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxBitmapButton* BitmapButton_VUMeter_YOffset;
		BulkEditValueCurveButton* BitmapButton_VUMeter_YOffsetVC;
		BulkEditChoice* Choice_VUMeter_Type;
		BulkEditTextCtrl* TextCtrl_VUMeter_EndNote;
		wxBitmapButton* BitmapButton_VUMeter_TimingTrack;
		wxStaticText* StaticText12;
		BulkEditSlider* Slider_VUMeter_XOffset;
		wxBitmapButton* BitmapButton_VUMeter_StartNote;
		wxStaticText* StaticText4;
		BulkEditTextCtrl* TextCtrl_VUMeter_Sensitivity;
		//*)

	protected:

		//(*Identifiers(VUMeterPanel)
		static const long ID_STATICTEXT_VUMeter_Bars;
		static const long ID_SLIDER_VUMeter_Bars;
		static const long IDD_TEXTCTRL_VUMeter_Bars;
		static const long ID_BITMAPBUTTON_CHOICE_VUMeter_Bars;
		static const long ID_STATICTEXT_VUMeter_Type;
		static const long ID_CHOICE_VUMeter_Type;
		static const long ID_STATICTEXT5;
		static const long ID_BITMAPBUTTON_CHOICE_VUMeter_Type;
		static const long ID_STATICTEXT_VUMeter_TimingTrack;
		static const long ID_CHOICE_VUMeter_TimingTrack;
		static const long ID_STATICTEXT4;
		static const long ID_BITMAPBUTTON_CHOICE_VUMeter_TimingTrack;
		static const long ID_STATICTEXT_VUMeter_Sensitivity;
		static const long ID_SLIDER_VUMeter_Sensitivity;
		static const long IDD_TEXTCTRL_VUMeter_Sensitivity;
		static const long ID_BITMAPBUTTON_SLIDER_VUMeter_Sensitivity;
		static const long ID_STATICTEXT_VUMeter_Shape;
		static const long ID_CHOICE_VUMeter_Shape;
		static const long ID_STATICTEXT8;
		static const long ID_BITMAPBUTTON_CHOICE_VUMeter_Shape;
		static const long ID_STATICTEXT9;
		static const long ID_CHECKBOX_VUMeter_SlowDownFalls;
		static const long ID_STATICTEXT10;
		static const long ID_BITMAPBUTTON_CHECKBOX_VUMeter_SlowDownFalls;
		static const long ID_STATICTEXT_VUMeter_StartNote;
		static const long ID_SLIDER_VUMeter_StartNote;
		static const long IDD_TEXTCTRL_VUMeter_StartNote;
		static const long ID_BITMAPBUTTON_VUMeter_StartNote;
		static const long ID_STATICTEXT_VUMeter_EndNote;
		static const long ID_SLIDER_VUMeter_EndNote;
		static const long IDD_TEXTCTRL_VUMeter_EndNote;
		static const long ID_BITMAPBUTTON_VUMeter_EndNote;
		static const long ID_STATICTEXT_VUMeter_XOffset;
		static const long ID_SLIDER_VUMeter_XOffset;
		static const long IDD_TEXTCTRL_VUMeter_XOffset;
		static const long ID_BITMAPBUTTON_VUMeter_XOffset;
		static const long ID_STATICTEXT_VUMeter_YOffset;
		static const long ID_SLIDER_VUMeter_YOffset;
		static const long ID_VALUECURVE_VUMeter_YOffset;
		static const long IDD_TEXTCTRL_VUMeter_YOffset;
		static const long ID_BITMAPBUTTON_VUMeter_YOffset;
		//*)

	public:

		//(*Handlers(VUMeterPanel)
		void OnLockButtonClick(wxCommandEvent& event);
        void OnChoice_VUMeter_TypeSelect(wxCommandEvent& event);
		void OnSlider_VUMeter_StartNoteCmdSliderUpdated(wxScrollEvent& event);
		void OnTextCtrl_VUMeter_StartNoteText(wxCommandEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()

	public:

		void ValidateWindow();
};

#endif
