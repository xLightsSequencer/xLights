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

#include "../ValueCurveButton.h"

class VUMeterPanel: public wxPanel
{
	public:

		VUMeterPanel(wxWindow* parent);
		virtual ~VUMeterPanel();

		//(*Declarations(VUMeterPanel)
		wxStaticText* StaticText10;
		wxStaticText* StaticText9;
		wxTextCtrl* TextCtrl_VUMeter_StartNote;
		wxBitmapButton* BitmapButton_VUMeter_EndNote;
		wxSlider* Slider_VUMeter_Bars;
		wxStaticText* StaticText13;
		wxStaticText* StaticText2;
		wxStaticText* StaticText14;
		wxStaticText* StaticText6;
		wxBitmapButton* BitmapButton_VUMeter_SlowDownFalls;
		wxStaticText* StaticText8;
		wxStaticText* StaticText11;
		ValueCurveButton* BitmapButton_VUMeter_YOffsetVC;
		wxBitmapButton* BitmapButton_VUMeter_Sensitivity;
		wxStaticText* StaticText1;
		wxSlider* Slider_VUMeter_YOffset;
		wxStaticText* StaticText3;
		wxBitmapButton* BitmapButton_VUMeter_Bars;
		wxBitmapButton* BitmapButton_VUMeter_Type;
		wxSlider* Slider_VUMeter_EndNote;
		wxBitmapButton* BitmapButton_VUMeter_XOffset;
		wxSlider* Slider_VUMeter_Sensitivity;
		wxBitmapButton* BitmapButton_VUMeter_Shape;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxBitmapButton* BitmapButton_VUMeter_YOffset;
		wxTextCtrl* TextCtrl_VUMeter_XOffset;
		wxTextCtrl* TextCtrl_VUMeter_YOffset;
		wxCheckBox* CheckBox_VUMeter_SlowDownFalls;
		wxBitmapButton* BitmapButton_VUMeter_TimingTrack;
		wxStaticText* StaticText12;
		wxTextCtrl* TextCtrl_VUMeter_Bars;
		wxChoice* Choice_VUMeter_Type;
		wxChoice* Choice_VUMeter_Shape;
		wxSlider* Slider_VUMeter_StartNote;
		wxChoice* Choice_VUMeter_TimingTrack;
		wxSlider* Slider_VUMeter_XOffset;
		wxBitmapButton* BitmapButton_VUMeter_StartNote;
		wxStaticText* StaticText4;
		wxTextCtrl* TextCtrl_VUMeter_Sensitivity;
		wxTextCtrl* TextCtrl_VUMeter_EndNote;
		//*)

	protected:

		//(*Identifiers(VUMeterPanel)
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_VUMeter_Bars;
		static const long IDD_TEXTCTRL_VUMeter_Bars;
		static const long ID_BITMAPBUTTON_CHOICE_VUMeter_Bars;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE_VUMeter_Type;
		static const long ID_STATICTEXT5;
		static const long ID_BITMAPBUTTON_CHOICE_VUMeter_Type;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE_VUMeter_TimingTrack;
		static const long ID_STATICTEXT4;
		static const long ID_BITMAPBUTTON_CHOICE_VUMeter_TimingTrack;
		static const long ID_STATICTEXT6;
		static const long ID_SLIDER_VUMeter_Sensitivity;
		static const long IDD_TEXTCTRL_VUMeter_Sensitivity;
		static const long ID_BITMAPBUTTON_SLIDER_VUMeter_Sensitivity;
		static const long ID_STATICTEXT7;
		static const long ID_CHOICE_VUMeter_Shape;
		static const long ID_STATICTEXT8;
		static const long ID_BITMAPBUTTON_CHOICE_VUMeter_Shape;
		static const long ID_STATICTEXT9;
		static const long ID_CHECKBOX_VUMeter_SlowDownFalls;
		static const long ID_STATICTEXT10;
		static const long ID_BITMAPBUTTON_CHECKBOX_VUMeter_SlowDownFalls;
		static const long ID_STATICTEXT12;
		static const long ID_SLIDER_VUMeter_StartNote;
		static const long IDD_TEXTCTRL_VUMeter_StartNote;
		static const long ID_BITMAPBUTTON_VUMeter_StartNote;
		static const long ID_STATICTEXT11;
		static const long ID_SLIDER_VUMeter_EndNote;
		static const long IDD_TEXTCTRL_VUMeter_EndNote;
		static const long ID_BITMAPBUTTON_VUMeter_EndNote;
		static const long ID_STATICTEXT13;
		static const long ID_SLIDER_VUMeter_XOffset;
		static const long IDD_TEXTCTRL_VUMeter_XOffset;
		static const long ID_BITMAPBUTTON_VUMeter_XOffset;
		static const long ID_STATICTEXT14;
		static const long ID_SLIDER_VUMeter_YOffset;
		static const long ID_VALUECURVE_VUMeter_YOffset;
		static const long IDD_TEXTCTRL_VUMeter_YOffset;
		static const long ID_BITMAPBUTTON_VUMeter_YOffset;
		//*)

	public:

		//(*Handlers(VUMeterPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
        void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
        void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
        void OnChoiceVUMeterDirectionSelect(wxCommandEvent& event);
		void OnTextCtrl1Text(wxCommandEvent& event);
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
