#ifndef PLASMAPANEL_H
#define PLASMAPANEL_H

//(*Headers(PlasmaPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class PlasmaPanel: public wxPanel
{
	public:

		PlasmaPanel(wxWindow* parent);
		virtual ~PlasmaPanel();

		//(*Declarations(PlasmaPanel)
		BulkEditChoice* Choice_Plasma_Color;
		BulkEditSlider* Slider_Plasma_Line_Density;
		BulkEditSlider* Slider_Plasma_Speed;
		BulkEditSlider* Slider_Plasma_Style;
		BulkEditValueCurveButton* BitmapButton_Plasma_SpeedVC;
		wxStaticText* StaticText197;
		wxStaticText* StaticText202;
		wxStaticText* StaticText204;
		wxStaticText* StaticText64;
		//*)

	protected:

		//(*Identifiers(PlasmaPanel)
		static const long ID_STATICTEXT_Plasma_Color;
		static const long ID_CHOICE_Plasma_Color;
		static const long ID_BITMAPBUTTON_CHOICE_Plasma_Color;
		static const long ID_STATICTEXT_Plasma_Style;
		static const long ID_SLIDER_Plasma_Style;
		static const long IDD_TEXTCTRL_Plasma_Style;
		static const long ID_BITMAPBUTTON_SLIDER_Plasma_Style;
		static const long ID_STATICTEXT_Plasma_Line_Density;
		static const long ID_SLIDER_Plasma_Line_Density;
		static const long IDD_TEXTCTRL_Plasma_Line_Density;
		static const long ID_BITMAPBUTTON_SLIDER_Plasma_Line_Density;
		static const long ID_STATICTEXT_Plasma_Speed;
		static const long ID_SLIDER_Plasma_Speed;
		static const long ID_VALUECURVE_Plasma_Speed;
		static const long IDD_TEXTCTRL_Plasma_Speed;
		static const long ID_BITMAPBUTTON_SLIDER_Plasma_Speed;
		//*)

	public:

		//(*Handlers(PlasmaPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
