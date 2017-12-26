#ifndef ButterfLYPANEL_H
#define ButterfLYPANEL_H

//(*Headers(ButterflyPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../BulkEditControls.h"

class ButterflyPanel: public wxPanel
{
	public:

		ButterflyPanel(wxWindow* parent);
		virtual ~ButterflyPanel();

		//(*Declarations(ButterflyPanel)
		wxStaticText* StaticText175;
		BulkEditValueCurveButton* BitmapButton_Butterfly_Skip;
		BulkEditSlider* Slider_Butterfly_Style;
		wxStaticText* StaticText26;
		BulkEditChoice* Choice_Butterfly_Direction;
		wxStaticText* StaticText58;
		wxStaticText* StaticText27;
		BulkEditValueCurveButton* BitmapButton_Butterfly_Speed;
		wxStaticText* StaticText28;
		BulkEditSlider* Slider_Butterfly_Skip;
		BulkEditSlider* Slider_Butterfly_Chunks;
		BulkEditValueCurveButton* BitmapButton_Butterfly_Chunks;
		wxStaticText* StaticText25;
		BulkEditSlider* Slider_Butterfly_Speed;
		BulkEditChoice* Choice_Butterfly_Colors;
		//*)

	protected:

		//(*Identifiers(ButterflyPanel)
		static const long ID_STATICTEXT_Butterfly_Colors;
		static const long ID_CHOICE_Butterfly_Colors;
		static const long ID_BITMAPBUTTON_CHOICE_Butterfly_Color;
		static const long ID_STATICTEXT_Butterfly_Style;
		static const long ID_SLIDER_Butterfly_Style;
		static const long IDD_TEXTCTRL_Butterfly_Style;
		static const long ID_BITMAPBUTTON_SLIDER_Butterfly_Style;
		static const long ID_STATICTEXT_Butterfly_Chunks;
		static const long ID_SLIDER_Butterfly_Chunks;
		static const long ID_VALUECURVE_Butterfly_Chunks;
		static const long IDD_TEXTCTRL_Butterfly_Chunks;
		static const long ID_BITMAPBUTTON_SLIDER_Butterfly_Chunks;
		static const long ID_STATICTEXT_Butterfly_Skip;
		static const long ID_SLIDER_Butterfly_Skip;
		static const long ID_VALUECURVE_Butterfly_Skip;
		static const long IDD_TEXTCTRL_Butterfly_Skip;
		static const long ID_BITMAPBUTTON15;
		static const long ID_STATICTEXT_Butterfly_Speed;
		static const long ID_SLIDER_Butterfly_Speed;
		static const long ID_VALUECURVE_Butterfly_Speed;
		static const long IDD_TEXTCTRL_Butterfly_Speed;
		static const long ID_BITMAPBUTTON_SLIDER_Butterfly_Skip;
		static const long ID_STATICTEXT_Butterfly_Direction;
		static const long ID_CHOICE_Butterfly_Direction;
		static const long ID_BITMAPBUTTON9;
		//*)

	public:

		//(*Handlers(ButterflyPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};

#endif
