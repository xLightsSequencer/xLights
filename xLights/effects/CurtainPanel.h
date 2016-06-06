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

#include "../ValueCurveButton.h"

class CurtainPanel: public wxPanel
{
	public:

		CurtainPanel(wxWindow* parent);
		virtual ~CurtainPanel();

		//(*Declarations(CurtainPanel)
		wxChoice* Choice_Curtain_Edge;
		wxStaticText* StaticText6;
		wxStaticText* StaticText8;
		ValueCurveButton* BitmapButton_Curtain_SpeedVC;
		wxChoice* Choice_Curtain_Effect;
		wxCheckBox* CheckBox_Curtain_Repeat;
		wxStaticText* StaticText7;
		wxBitmapButton* BitmapButton_CurtainEffect;
		wxBitmapButton* BitmapButton_CurtainEdge;
		wxBitmapButton* BitmapButton_CurtainRepeat;
		ValueCurveButton* BitmapButton_Curtain_SwagVC;
		wxBitmapButton* BitmapButton_CurtainSwag;
		//*)

	protected:

		//(*Identifiers(CurtainPanel)
		static const long ID_STATICTEXT6;
		static const long ID_CHOICE_Curtain_Edge;
		static const long ID_BITMAPBUTTON_CHOICE_Curtain_Edge;
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE_Curtain_Effect;
		static const long ID_BITMAPBUTTON_CHOICE_Curtain_Effect;
		static const long ID_STATICTEXT7;
		static const long ID_SLIDER_Curtain_Swag;
		static const long ID_VALUECURVE_Curtain_Swag;
		static const long IDD_TEXTCTRL_Curtain_Swag;
		static const long ID_BITMAPBUTTON_SLIDER_Curtain_Swag;
		static const long IDD_SLIDER_Curtain_Speed;
		static const long ID_VALUECURVE_Curtain_Speed;
		static const long ID_TEXTCTRL_Curtain_Speed;
		static const long ID_CHECKBOX_Curtain_Repeat;
		static const long ID_BITMAPBUTTON_CHECKBOX_Curtain_Repeat;
		//*)

	public:

		//(*Handlers(CurtainPanel)
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
