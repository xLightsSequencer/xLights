#ifndef FILLPANEL_H
#define FILLPANEL_H

//(*Headers(FillPanel)
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

class FillPanel: public wxPanel
{
	public:

		FillPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~FillPanel();

		//(*Declarations(FillPanel)
		wxCheckBox* CheckBox_Fill_Color_Time;
		wxCheckBox* CheckBox_Fill_Wrap;
		ValueCurveButton* BitmapButton_Fill_Band_Size;
		wxSlider* Slider_Fill_Offset;
		wxCheckBox* CheckBox_Fill_Offset_In_Pixels;
		ValueCurveButton* BitmapButton_Fill_Offset;
		wxChoice* Choice_Fill_Direction;
		wxSlider* Slider_Fill_Skip_Size;
		ValueCurveButton* BitmapButton_Fill_Skip_Size;
		ValueCurveButton* BitmapButton_Fill_Position;
		wxSlider* Slider_Fill_Band_Size;
		wxSlider* Slider_Fill_Position;
		//*)

	protected:

		//(*Identifiers(FillPanel)
		static const long ID_SLIDER_Fill_Position;
		static const long ID_VALUECURVE_Fill_Position;
		static const long IDD_TEXTCTRL_Fill_Position;
		static const long ID_BITMAPBUTTON_SLIDER_Fill_Position;
		static const long ID_SLIDER_Fill_Band_Size;
		static const long ID_VALUECURVE_Fill_Band_Size;
		static const long IDD_TEXTCTRL_Fill_Band_Size;
		static const long ID_BITMAPBUTTON_SLIDER_Fill_Band_Size;
		static const long ID_SLIDER_Fill_Skip_Size;
		static const long ID_VALUECURVE_Fill_Skip_Size;
		static const long IDD_TEXTCTRL_Fill_Skip_Size;
		static const long ID_BITMAPBUTTON_SLIDER_Fill_Skip_Size;
		static const long ID_SLIDER_Fill_Offset;
		static const long ID_VALUECURVE_Fill_Offset;
		static const long IDD_TEXTCTRL_Fill_Offset;
		static const long ID_BITMAPBUTTON_SLIDER_Fill_Offset;
		static const long ID_CHECKBOX_Fill_Offset_In_Pixels;
		static const long ID_CHECKBOX_Fill_Color_Time;
		static const long ID_CHECKBOX_Fill_Wrap;
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
        void OnVCChanged(wxCommandEvent& event);
        void UpdateLinkedSliderFloat2(wxCommandEvent& event);
        void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
