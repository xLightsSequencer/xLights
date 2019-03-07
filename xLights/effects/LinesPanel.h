#ifndef LINESPANEL_H
#define LINESPANEL_H

//(*Headers(LinesPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class LinesPanel: public wxPanel
{
	public:

		LinesPanel(wxWindow* parent);
		virtual ~LinesPanel();

		//(*Declarations(LinesPanel)
		BulkEditCheckBox* CheckBox_FadeTrails;
		BulkEditSlider* Slider_Lines_Objects;
		BulkEditSlider* Slider_Lines_Segments;
		BulkEditSlider* Slider_Lines_Speed;
		BulkEditSlider* Slider_Lines_Thickness;
		BulkEditSlider* Slider_Lines_Trails;
		BulkEditTextCtrl* TextCtrl_Lines_Objects;
		BulkEditTextCtrl* TextCtrl_Lines_Segments;
		BulkEditTextCtrl* TextCtrl_Lines_Speed;
		BulkEditTextCtrl* TextCtrl_Lines_Thickness;
		BulkEditTextCtrl* TextCtrl_Lines_Trails;
		BulkEditValueCurveButton* BitmapButton_Lines_Speed;
		BulkEditValueCurveButton* BitmapButton_Lines_Thickness;
		wxStaticText* StaticText13;
		wxStaticText* StaticText15;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		xlLockButton* BitmapButton_Lines_Objects;
		xlLockButton* BitmapButton_Lines_Segments;
		xlLockButton* BitmapButton_Lines_Trails;
		//*)

	protected:

		//(*Identifiers(LinesPanel)
		static const long ID_STATICTEXT_Lines_Bars;
		static const long ID_SLIDER_Lines_Objects;
		static const long IDD_TEXTCTRL_Lines_Objects;
		static const long ID_BITMAPBUTTON_SLIDER_Lines_Objects;
		static const long ID_STATICTEXT_Lines_Segments;
		static const long ID_SLIDER_Lines_Segments;
		static const long IDD_TEXTCTRL_Lines_Segments;
		static const long ID_BITMAPBUTTON_SLIDER_Lines_Segments;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_Lines_Thickness;
		static const long ID_VALUECURVE_Lines_Thickness;
		static const long IDD_TEXTCTRL_Lines_Thickness;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_Lines_Speed;
		static const long ID_VALUECURVE_Lines_Speed;
		static const long IDD_TEXTCTRL_Lines_Speed;
		static const long ID_STATICTEXT_Lines_Trails;
		static const long ID_SLIDER_Lines_Trails;
		static const long IDD_TEXTCTRL_Lines_Trails;
		static const long ID_BITMAPBUTTON_SLIDER_Lines_Trails;
		static const long ID_CHECKBOX_Lines_FadeTrails;
		//*)

	public:

		//(*Handlers(LinesPanel)
		void OnLockButtonClick(wxCommandEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()

	public:

		void ValidateWindow();
};

#endif
