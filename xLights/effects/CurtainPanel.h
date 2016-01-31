#ifndef CURTAINPANEL_H
#define CURTAINPANEL_H

//(*Headers(CurtainPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
//*)

class CurtainPanel: public wxPanel
{
	public:

		CurtainPanel(wxWindow* parent);
		virtual ~CurtainPanel();

		//(*Declarations(CurtainPanel)
		wxBitmapButton* BitmapButton_CurtainEdge;
		wxChoice* Choice_Curtain_Edge;
		wxStaticText* StaticText8;
		wxStaticText* StaticText7;
		wxBitmapButton* BitmapButton_CurtainEffect;
		wxCheckBox* CheckBox_Curtain_Repeat;
		wxStaticText* StaticText6;
		wxBitmapButton* BitmapButton_CurtainRepeat;
		wxBitmapButton* BitmapButton_CurtainSwag;
		wxChoice* Choice_Curtain_Effect;
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
		static const long IDD_TEXTCTRL_Curtain_Swag;
		static const long ID_BITMAPBUTTON_SLIDER_Curtain_Swag;
		static const long IDD_SLIDER_Curtain_Speed;
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

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
