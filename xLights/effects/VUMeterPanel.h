#ifndef VUMETERPANEL_H
#define VUMETERPANEL_H

//(*Headers(VUMeterPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

class VUMeterPanel: public wxPanel
{
	public:

		VUMeterPanel(wxWindow* parent);
		virtual ~VUMeterPanel();

		//(*Declarations(VUMeterPanel)
		wxSlider* Slider_VUMeter_Bars;
		wxStaticText* StaticText1;
		wxBitmapButton* BitmapButton_VUMeter_Bars;
		wxTextCtrl* TextCtrl_VUMeter_Bars;
		//*)

	protected:

		//(*Identifiers(VUMeterPanel)
		static const long ID_STATICTEXT1;
		static const long IDD_SLIDER_VUMeter_Bars;
		static const long ID_TEXTCTRL_VUMeter_Bars;
		static const long ID_BITMAPBUTTON_CHOICE_VUMeter_Bars;
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
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
