#ifndef CURTAINPANEL_H
#define CURTAINPANEL_H

//(*Headers(CurtainPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class CurtainPanel: public wxPanel
{
	public:

		CurtainPanel(wxWindow* parent);
		virtual ~CurtainPanel();

		//(*Declarations(CurtainPanel)
		BulkEditCheckBox* CheckBox_Curtain_Repeat;
		BulkEditChoice* Choice_Curtain_Edge;
		BulkEditChoice* Choice_Curtain_Effect;
		BulkEditSlider* Slider_Curtain_Swag;
		BulkEditSliderF1* Slider_Curtain_Speed;
		BulkEditValueCurveButton* BitmapButton_Curtain_SpeedVC;
		BulkEditValueCurveButton* BitmapButton_Curtain_SwagVC;
		wxStaticText* StaticText178;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		xlLockButton* BitmapButton_CurtainEdge;
		xlLockButton* BitmapButton_CurtainEffect;
		xlLockButton* BitmapButton_CurtainRepeat;
		xlLockButton* BitmapButton_CurtainSwag;
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
