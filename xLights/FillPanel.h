#ifndef FILLPANEL_H
#define FILLPANEL_H

//(*Headers(FillPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
//*)

#include "../ValueCurveButton.h"

class FillPanel: public wxPanel
{
	public:

		FillPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~FillPanel();

		//(*Declarations(FillPanel)
		wxChoice* Choice_Fill_Direction;
		ValueCurveButton* BitmapButton_Fill_Position;
		wxSlider* Slider_Fill_Position;
		//*)

	protected:

		//(*Identifiers(FillPanel)
		static const long ID_SLIDER_Fill_Position;
		static const long ID_VALUECURVE_Fill_Position;
		static const long IDD_TEXTCTRL_Fill_Position;
		static const long ID_BITMAPBUTTON_SLIDER_Fill_Position;
		static const long ID_CHOICE_Fill_Direction;
		static const long ID_BITMAPBUTTON_CHOICE_Fill_Direction;
		//*)

	private:

		//(*Handlers(FillPanel)
        void UpdateLinkedSliderFloat(wxCommandEvent& event);
        void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
        void UpdateLinkedTextCtrl360(wxScrollEvent& event);
        void UpdateLinkedSlider360(wxCommandEvent& event);
        void UpdateLinkedTextCtrl(wxScrollEvent& event);
        void UpdateLinkedSlider(wxCommandEvent& event);
        void OnLockButtonClick(wxCommandEvent& event);
        void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
        void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
