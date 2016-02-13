#ifndef VUMETERPANEL_H
#define VUMETERPANEL_H

//(*Headers(VUMeterPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

class VUMeterPanel: public wxPanel
{
	public:

		VUMeterPanel(wxWindow* parent);
		virtual ~VUMeterPanel();

		//(*Declarations(VUMeterPanel)
		wxSlider* Slider_VUMeter_Bars;
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxBitmapButton* BitmapButton_VUMeter_Bars;
		wxBitmapButton* BitmapButton_VUMeter_Type;
		wxStaticText* StaticText5;
		wxBitmapButton* BitmapButton_VUMeter_TimingTrack;
		wxTextCtrl* TextCtrl_VUMeter_Bars;
		wxChoice* Choice_VUMeter_Type;
		wxChoice* Choice_VUMeter_TimingTrack;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(VUMeterPanel)
		static const long ID_STATICTEXT1;
		static const long IDD_SLIDER_VUMeter_Bars;
		static const long ID_TEXTCTRL_VUMeter_Bars;
		static const long ID_BITMAPBUTTON_CHOICE_VUMeter_Bars;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE_VUMeter_Type;
		static const long ID_STATICTEXT5;
		static const long ID_BITMAPBUTTON_CHOICE_VUMeter_Type;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE_VUMeter_TimingTrack;
		static const long ID_STATICTEXT4;
		static const long ID_BITMAPBUTTON_CHOICE_VUMeter_TimingTrack;
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
		void OnChoiceVUMeterDirectionSelect(wxCommandEvent& event);
		void OnTextCtrl1Text(wxCommandEvent& event);
		void OnChoice_VUMeter_TypeSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

	public:

		void ValidateWindow();
};

#endif
