#ifndef CURTAINPANEL_H
#define CURTAINPANEL_H

//(*Headers(CurtainPanel)
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

class CurtainPanel: public wxPanel
{
	public:

		CurtainPanel(wxWindow* parent);
		virtual ~CurtainPanel();

		//(*Declarations(CurtainPanel)
		BulkEditValueCurveButton* BitmapButton_Curtain_SpeedVC;
		BulkEditSliderF1* Slider_Curtain_Speed;
		BulkEditCheckBox* CheckBox_Curtain_Repeat;
		wxStaticText* StaticText6;
		wxStaticText* StaticText8;
		wxStaticText* StaticText7;
		wxBitmapButton* BitmapButton_CurtainEffect;
		wxBitmapButton* BitmapButton_CurtainEdge;
		BulkEditSlider* Slider_Curtain_Swag;
		wxStaticText* StaticText178;
		wxBitmapButton* BitmapButton_CurtainRepeat;
		BulkEditChoice* Choice_Curtain_Effect;
		wxBitmapButton* BitmapButton_CurtainSwag;
		BulkEditValueCurveButton* BitmapButton_Curtain_SwagVC;
		BulkEditChoice* Choice_Curtain_Edge;
		//*)

	protected:

		//(*Identifiers(CurtainPanel)
		static const long ID_STATICTEXT_Curtain_Edge;
		static const long ID_CHOICE_Curtain_Edge;
		static const long ID_BITMAPBUTTON_CHOICE_Curtain_Edge;
		static const long ID_STATICTEXT_Curtain_Effect;
		static const long ID_CHOICE_Curtain_Effect;
		static const long ID_BITMAPBUTTON_CHOICE_Curtain_Effect;
		static const long ID_STATICTEXT_Curtain_Swag;
		static const long ID_SLIDER_Curtain_Swag;
		static const long ID_VALUECURVE_Curtain_Swag;
		static const long IDD_TEXTCTRL_Curtain_Swag;
		static const long ID_BITMAPBUTTON_SLIDER_Curtain_Swag;
		static const long ID_STATICTEXT_Curtain_Speed;
		static const long IDD_SLIDER_Curtain_Speed;
		static const long ID_VALUECURVE_Curtain_Speed;
		static const long ID_TEXTCTRL_Curtain_Speed;
		static const long ID_CHECKBOX_Curtain_Repeat;
		static const long ID_BITMAPBUTTON_CHECKBOX_Curtain_Repeat;
		//*)

	public:

		//(*Handlers(CurtainPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};

#endif
