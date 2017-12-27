#ifndef PLASMAPANEL_H
#define PLASMAPANEL_H

//(*Headers(PlasmaPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../BulkEditControls.h"

class PlasmaPanel: public wxPanel
{
	public:

		PlasmaPanel(wxWindow* parent);
		virtual ~PlasmaPanel();

		//(*Declarations(PlasmaPanel)
		BulkEditChoice* Choice_Plasma_Color;
		wxStaticText* StaticText204;
		wxStaticText* StaticText202;
		wxStaticText* StaticText64;
		BulkEditSlider* Slider_Plasma_Style;
		wxStaticText* StaticText197;
		BulkEditSlider* Slider_Plasma_Speed;
		BulkEditSlider* Slider_Plasma_Line_Density;
		BulkEditValueCurveButton* BitmapButton_Plasma_SpeedVC;
		//*)

	protected:

		//(*Identifiers(PlasmaPanel)
		static const long ID_STATICTEXT_Plasma_Color;
		static const long ID_CHOICE_Plasma_Color;
		static const long ID_BITMAPBUTTON35;
		static const long ID_STATICTEXT_Plasma_Style;
		static const long ID_SLIDER_Plasma_Style;
		static const long IDD_TEXTCTRL_Plasma_Style;
		static const long ID_BITMAPBUTTON36;
		static const long ID_STATICTEXT_Plasma_Line_Density;
		static const long ID_SLIDER_Plasma_Line_Density;
		static const long IDD_TEXTCTRL_Plasma_Line_Density;
		static const long ID_BITMAPBUTTON37;
		static const long ID_STATICTEXT_Plasma_Speed;
		static const long ID_SLIDER_Plasma_Speed;
		static const long ID_VALUECURVE_Plasma_Speed;
		static const long IDD_TEXTCTRL_Plasma_Speed;
		static const long ID_BITMAPBUTTON39;
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
