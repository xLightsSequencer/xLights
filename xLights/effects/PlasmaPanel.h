#ifndef PLASMAPANEL_H
#define PLASMAPANEL_H

//(*Headers(PlasmaPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
//*)

#include "../BulkEditControls.h"

class PlasmaPanel: public wxPanel
{
	public:

		PlasmaPanel(wxWindow* parent);
		virtual ~PlasmaPanel();

		//(*Declarations(PlasmaPanel)
		BulkEditSlider* Slider_Plasma_Speed;
		wxStaticText* StaticText204;
		BulkEditValueCurveButton* BitmapButton_Plasma_SpeedVC;
		BulkEditSlider* Slider_Plasma_Line_Density;
		BulkEditChoice* Choice_Plasma_Color;
		BulkEditSlider* Slider_Plasma_Style;
		wxStaticText* StaticText202;
		wxStaticText* StaticText64;
		wxStaticText* StaticText197;
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
