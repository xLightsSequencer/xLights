#ifndef MORPHPANEL_H
#define MORPHPANEL_H

//(*Headers(MorphPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxNotebookEvent;
class wxNotebook;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

class MorphPanel: public wxPanel
{
	public:

		MorphPanel(wxWindow* parent);
		virtual ~MorphPanel();

		//(*Declarations(MorphPanel)
		wxTextCtrl* TextCtrl_Morph_End_X2;
		wxBitmapButton* BitmapButton_Morph_End_Y1;
		wxBitmapButton* BitmapButton_MorphAccel;
		wxTextCtrl* TextCtrl_Morph_Start_X2;
		wxStaticText* StaticText117;
		wxTextCtrl* TextCtrl_Morph_End_Y1;
		wxStaticText* StaticText121;
		wxStaticText* StaticText120;
		wxCheckBox* CheckBox_Morph_Start_Link;
		wxBitmapButton* BitmapButton_Morph_Repeat_Count;
		wxStaticText* StaticText124;
		wxTextCtrl* TextCtrl_Morph_Start_Y1;
		wxBitmapButton* BitmapButton_MorphStartLength;
		wxStaticText* StaticText3;
		wxBitmapButton* BitmapButton_MorphEndLength;
		wxTextCtrl* TextCtrl_Morph_Start_Y2;
		wxStaticText* StaticText123;
		wxBitmapButton* BitmapButton_Morph_Start_Y2;
		wxBitmapButton* BitmapButton_Morph_Start_X2;
		wxTextCtrl* TextCtrl_Morph_End_X1;
		wxStaticText* StaticText115;
		wxStaticText* StaticText119;
		wxBitmapButton* BitmapButton_Morph_Start_X1;
		wxStaticText* StaticText122;
		wxStaticText* StaticText171;
		wxBitmapButton* BitmapButton_MorphDuration;
		wxNotebook* Notebook_Morph;
		wxStaticText* StaticText172;
		wxCheckBox* CheckBox_Morph_End_Link;
		wxBitmapButton* BitmapButton_Morph_End_X1;
		wxTextCtrl* TextCtrl_Morph_End_Y2;
		wxCheckBox* CheckBox_ShowHeadAtStart;
		wxChoice* Choice_Morph_QuickSet;
		wxTextCtrl* TextCtrl_Morph_Start_X1;
		wxBitmapButton* BitmapButton_ShowHeadAtStart;
		wxBitmapButton* BitmapButton_Morph_End_Y2;
		wxStaticText* StaticText59;
		wxBitmapButton* BitmapButton_Morph_Stagger;
		wxStaticText* StaticText173;
		wxBitmapButton* BitmapButton_Morph_Repeat_Skip;
		wxBitmapButton* BitmapButton_Morph_End_X2;
		wxStaticText* StaticText99;
		wxBitmapButton* BitmapButton_Morph_Start_Y1;
		//*)

	protected:

		//(*Identifiers(MorphPanel)
		static const long ID_SLIDER_Morph_Start_X1;
		static const long IDD_TEXTCTRL_Morph_Start_X1;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Start_X1;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER_Morph_Start_Y1;
		static const long IDD_TEXTCTRL_Morph_Start_Y1;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Start_Y1;
		static const long ID_STATICTEXT61;
		static const long ID_SLIDER_Morph_Start_X2;
		static const long IDD_TEXTCTRL_Morph_Start_X2;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Start_X2;
		static const long ID_STATICTEXT102;
		static const long ID_SLIDER_Morph_Start_Y2;
		static const long IDD_TEXTCTRL_Morph_Start_Y2;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Start_Y2;
		static const long ID_STATICTEXT115;
		static const long ID_SLIDER_MorphStartLength;
		static const long IDD_TEXTCTRL_MorphStartLength;
		static const long ID_BITMAPBUTTON_SLIDER_MorphStartLength;
		static const long ID_CHECKBOX_Morph_Start_Link;
		static const long IDD_CHOICE_Morph_QuickSet;
		static const long ID_PANEL4;
		static const long ID_STATICTEXT116;
		static const long ID_SLIDER_Morph_End_X1;
		static const long IDD_TEXTCTRL_Morph_End_X1;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_End_X1;
		static const long ID_STATICTEXT117;
		static const long ID_SLIDER_Morph_End_Y1;
		static const long IDD_TEXTCTRL_Morph_End_Y1;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_End_Y1;
		static const long ID_STATICTEXT119;
		static const long ID_SLIDER_Morph_End_X2;
		static const long IDD_TEXTCTRL_Morph_End_X2;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_End_X2;
		static const long ID_STATICTEXT121;
		static const long ID_SLIDER_Morph_End_Y2;
		static const long IDD_TEXTCTRL_Morph_End_Y2;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_End_Y2;
		static const long ID_STATICTEXT122;
		static const long ID_SLIDER_MorphEndLength;
		static const long IDD_TEXTCTRL_MorphEndLength;
		static const long ID_BITMAPBUTTON_SLIDER_MorphEndLength;
		static const long ID_CHECKBOX_Morph_End_Link;
		static const long ID_PANEL27;
		static const long ID_STATICTEXT123;
		static const long ID_SLIDER_MorphDuration;
		static const long IDD_TEXTCTRL_MorphDuration;
		static const long ID_BITMAPBUTTON_SLIDER_MorphDuration;
		static const long ID_STATICTEXT124;
		static const long ID_SLIDER_MorphAccel;
		static const long IDD_TEXTCTRL_MorphAccel;
		static const long ID_BITMAPBUTTON_SLIDER_MorphAccel;
		static const long ID_STATICTEXT97;
		static const long ID_SLIDER_Morph_Repeat_Count;
		static const long IDD_TEXTCTRL_Morph_Repeat_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Repeat_Count;
		static const long ID_STATICTEXT156;
		static const long ID_SLIDER_Morph_Repeat_Skip;
		static const long IDD_TEXTCTRL_Morph_Repeat_Skip;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Repeat_Skip;
		static const long ID_STATICTEXT157;
		static const long ID_SLIDER_Morph_Stagger;
		static const long IDD_TEXTCTRL_Morph_Stagger;
		static const long ID_BITMAPBUTTON_SLIDER_Morph_Stagger;
		static const long ID_CHECKBOX_ShowHeadAtStart;
		static const long ID_BITMAPBUTTON_CHECKBOX_ShowHeadAtStart;
		static const long ID_PANEL30;
		static const long ID_NOTEBOOK_Morph;
		//*)

	public:

		//(*Handlers(MorphPanel)
    void UpdateLinkedSliderFloat(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    void UpdateLinkedSlider360(wxCommandEvent& event);
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);
    
    void OnCheckBox_Morph_Start_LinkClick(wxCommandEvent& event);
    void OnCheckBox_Morph_End_LinkClick(wxCommandEvent& event);
    void OnChoice_Morph_QuickSetSelect(wxCommandEvent& event);

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
