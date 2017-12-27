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

#include "../BulkEditControls.h"

class FillPanel: public wxPanel
{
	public:

		FillPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~FillPanel();

		//(*Declarations(FillPanel)
		BulkEditChoice* Choice_Fill_Direction;
		BulkEditSlider* Slider_Fill_Position;
		BulkEditValueCurveButton* BitmapButton_Fill_Band_Size;
		wxStaticText* StaticText2;
		BulkEditSlider* Slider_Fill_Offset;
		BulkEditValueCurveButton* BitmapButton_Fill_Position;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		BulkEditValueCurveButton* BitmapButton_Fill_Skip_Size;
		wxStaticText* StaticText23;
		wxStaticText* StaticText24;
		BulkEditCheckBox* CheckBox_Fill_Offset_In_Pixels;
		BulkEditSlider* Slider_Fill_Band_Size;
		BulkEditValueCurveButton* BitmapButton_Fill_Offset;
		BulkEditCheckBox* CheckBox_Fill_Color_Time;
		BulkEditCheckBox* CheckBox_Fill_Wrap;
		BulkEditSlider* Slider_Fill_Skip_Size;
		//*)

	protected:

		//(*Identifiers(FillPanel)
		static const long ID_STATICTEXT_Fill_Position;
		static const long ID_SLIDER_Fill_Position;
		static const long ID_VALUECURVE_Fill_Position;
		static const long IDD_TEXTCTRL_Fill_Position;
		static const long ID_BITMAPBUTTON_SLIDER_Fill_Position;
		static const long ID_STATICTEXT_Fill_Band_Size;
		static const long ID_SLIDER_Fill_Band_Size;
		static const long ID_VALUECURVE_Fill_Band_Size;
		static const long IDD_TEXTCTRL_Fill_Band_Size;
		static const long ID_BITMAPBUTTON_SLIDER_Fill_Band_Size;
		static const long ID_STATICTEXT_Fill_Skip_Size;
		static const long ID_SLIDER_Fill_Skip_Size;
		static const long ID_VALUECURVE_Fill_Skip_Size;
		static const long IDD_TEXTCTRL_Fill_Skip_Size;
		static const long ID_BITMAPBUTTON_SLIDER_Fill_Skip_Size;
		static const long ID_STATICTEXT_Fill_Offset;
		static const long ID_SLIDER_Fill_Offset;
		static const long ID_VALUECURVE_Fill_Offset;
		static const long IDD_TEXTCTRL_Fill_Offset;
		static const long ID_BITMAPBUTTON_SLIDER_Fill_Offset;
		static const long ID_CHECKBOX_Fill_Offset_In_Pixels;
		static const long ID_CHECKBOX_Fill_Color_Time;
		static const long ID_CHECKBOX_Fill_Wrap;
		static const long ID_STATICTEXT_Fill_Direction;
		static const long ID_CHOICE_Fill_Direction;
		static const long ID_BITMAPBUTTON_CHOICE_Fill_Direction;
		//*)

	private:

		//(*Handlers(FillPanel)
        void OnLockButtonClick(wxCommandEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

    DECLARE_EVENT_TABLE()
};

#endif
