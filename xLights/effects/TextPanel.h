#ifndef TEXTPANEL_H
#define TEXTPANEL_H

//(*Headers(TextPanel)
#include <wx/panel.h>
class wxGridBagSizer;
class wxCheckBox;
class wxTextCtrl;
class wxNotebookEvent;
class wxNotebook;
class wxStaticText;
class wxFontPickerCtrl;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

class TextPanel: public wxPanel
{
	public:

		TextPanel(wxWindow* parent);
		virtual ~TextPanel();

		//(*Declarations(TextPanel)
		wxStaticText* StaticText22;
		wxChoice* Choice_Text_Dir1;
		wxStaticText* StaticText53;
		wxBitmapButton* BitmapButton_TextFont3;
		wxTextCtrl* TextCtrl_Text_Line4;
		wxStaticText* StaticText20;
		wxBitmapButton* BitmapButton_TextEffect3;
		wxBitmapButton* BitmapButton_TextToCenter1;
		wxBitmapButton* BitmapButton_TextCount2;
		wxStaticText* StaticText107;
		wxStaticText* StaticText29;
		wxTextCtrl* TextCtrl_Text_Line1;
		wxChoice* Choice_Text_Count4;
		wxStaticText* StaticText78;
		wxBitmapButton* BitmapButton_TextDir3;
		wxBitmapButton* BitmapButton_TextFont2;
		wxChoice* Choice_Text_Dir2;
		wxBitmapButton* BitmapButton_TextCount4;
		wxChoice* Choice_Text_Count2;
		wxBitmapButton* BitmapButton_TextPosition4;
		wxChoice* Choice_Text_Dir3;
		wxSlider* Slider_Text_Position2;
		wxBitmapButton* BitmapButton_TextFont1;
		wxBitmapButton* BitmapButton9;
		wxChoice* Choice_Text_Effect2;
		wxSlider* Slider_Text_Position4;
		wxBitmapButton* BitmapButton10;
		wxStaticText* StaticText42;
		wxStaticText* StaticText19;
		wxStaticText* StaticText18;
		wxStaticText* StaticText186;
		wxStaticText* StaticText188;
		wxBitmapButton* BitmapButton_TextPosition3;
		wxPanel* Panel_Text1;
		wxPanel* Panel_Text2;
		wxStaticText* StaticText54;
		wxStaticText* StaticText44;
		wxChoice* Choice_Text_Effect1;
		wxBitmapButton* BitmapButton12;
		wxBitmapButton* BitmapButton_TextCount1;
		wxPanel* Panel_Text4;
		wxStaticText* StaticText185;
		wxStaticText* StaticText98;
		wxStaticText* StaticText108;
		wxStaticText* StaticText103;
		wxBitmapButton* BitmapButton_TextDir4;
		wxBitmapButton* BitmapButton_TextToCenter3;
		wxBitmapButton* BitmapButton_TextPosition2;
		wxStaticText* StaticText57;
		wxSlider* Slider_Text_Position3;
		wxChoice* Choice_Text_Count3;
		wxBitmapButton* BitmapButton_TextEffect4;
		wxBitmapButton* BitmapButton_TextDir2;
		wxBitmapButton* BitmapButton_TextCount3;
		wxStaticText* StaticText15;
		wxStaticText* StaticText102;
		wxStaticText* StaticText187;
		wxBitmapButton* BitmapButton_TextToCenter2;
		wxChoice* Choice_Text_Dir4;
		wxBitmapButton* BitmapButton_TextEffect2;
		wxTextCtrl* TextCtrl_Text_Line3;
		wxStaticText* StaticText17;
		wxBitmapButton* BitmapButton_TextDir1;
		wxChoice* Choice_Text_Effect3;
		wxBitmapButton* BitmapButton_TextToCenter4;
		wxPanel* Panel17;
		wxChoice* Choice_Text_Count1;
		wxBitmapButton* BitmapButton11;
		wxBitmapButton* BitmapButton_TextFont4;
		wxStaticText* StaticText16;
		wxBitmapButton* BitmapButton_TextEffect1;
		wxPanel* Panel_Text3;
		wxChoice* Choice_Text_Effect4;
		wxTextCtrl* TextCtrl_Text_Line2;
		//*)


		//(*Identifiers(TextPanel)
		static const long ID_STATICTEXT53;
		static const long ID_TEXTCTRL_Text_Line1;
		static const long ID_FONTPICKER_Text_Font1;
		static const long ID_BITMAPBUTTON_FONTPICKER_Text_Font1;
		static const long ID_STATICTEXT79;
		static const long ID_CHOICE_Text_Dir1;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Dir1;
		static const long ID_CHECKBOX_TextToCenter1;
		static const long ID_BITMAPBUTTON_TextToCenter1;
		static const long ID_STATICTEXT28;
		static const long IDD_SLIDER_Text_Speed1;
		static const long ID_TEXTCTRL_Text_Speed1;
		static const long ID_BITMAPBUTTON30;
		static const long ID_STATICTEXT108;
		static const long ID_CHOICE_Text_Effect1;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Effect1;
		static const long ID_STATICTEXT99;
		static const long ID_CHOICE_Text_Count1;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Count1;
		static const long ID_CHECKBOX_Text_PixelOffsets1;
		static const long ID_SLIDER_Text_XStart1;
		static const long IDD_TEXTCTRL_Text_XStart1;
		static const long IDD_TEXTCTRL_Text_YStart1;
		static const long ID_SLIDER_Text_YStart1;
		static const long IDD_PANEL6;
		static const long ID_SLIDER_Text_XEnd1;
		static const long IDD_TEXTCTRL_Text_XEnd1;
		static const long IDD_TEXTCTRL_Text_YEnd1;
		static const long ID_SLIDER_Text_YEnd1;
		static const long IDD_PANEL17;
		static const long IDD_NOTEBOOK1;
		static const long ID_PANEL_Text1;
		static const long ID_STATICTEXT54;
		static const long ID_TEXTCTRL_Text_Line2;
		static const long ID_FONTPICKER_Text_Font2;
		static const long ID_BITMAPBUTTON_FONTPICKER_Text_Font2;
		static const long ID_STATICTEXT57;
		static const long ID_CHOICE_Text_Dir2;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Dir2;
		static const long ID_CHECKBOX_TextToCenter2;
		static const long ID_BITMAPBUTTON_Text2Center2;
		static const long ID_STATICTEXT30;
		static const long IDD_SLIDER_Text_Speed2;
		static const long ID_TEXTCTRL_Text_Speed2;
		static const long ID_BITMAPBUTTON29;
		static const long ID_STATICTEXT109;
		static const long ID_CHOICE_Text_Effect2;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Effect2;
		static const long ID_STATICTEXT104;
		static const long ID_CHOICE_Text_Count2;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Count2;
		static const long ID_STATICTEXT103;
		static const long ID_SLIDER_Text_Position2;
		static const long IDD_TEXTCTRL_Text_Position2;
		static const long ID_BITMAPBUTTON_SLIDER_Text_Position2;
		static const long ID_PANEL_Text2;
		static const long ID_STATICTEXT22;
		static const long ID_TEXTCTRL_Text_Line3;
		static const long ID_FONTPICKER_Text_Font3;
		static const long ID_BITMAPBUTTON_FONTPICKER_Text_Font3;
		static const long ID_STATICTEXT16;
		static const long ID_CHOICE_Text_Dir3;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Dir3;
		static const long ID_CHECKBOX_TextToCenter3;
		static const long ID_BITMAPBUTTON_TextToCenter3;
		static const long ID_STATICTEXT4;
		static const long IDD_SLIDER_Text_Speed3;
		static const long ID_TEXTCTRL_Text_Speed3;
		static const long ID_BITMAPBUTTON27;
		static const long ID_STATICTEXT17;
		static const long ID_CHOICE_Text_Effect3;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Effect3;
		static const long ID_STATICTEXT18;
		static const long ID_CHOICE_Text_Count3;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Count3;
		static const long ID_STATICTEXT19;
		static const long ID_SLIDER_Text_Position3;
		static const long IDD_TEXTCTRL_Text_Position3;
		static const long ID_BITMAPBUTTON_SLIDER_Text_Position3;
		static const long ID_PANEL_Text3;
		static const long ID_STATICTEXT23;
		static const long ID_TEXTCTRL_Text_Line4;
		static const long ID_FONTPICKER_Text_Font4;
		static const long ID_BITMAPBUTTON_FONTPICKER_Text_Font4;
		static const long ID_STATICTEXT24;
		static const long ID_CHOICE_Text_Dir4;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Dir4;
		static const long ID_CHECKBOX_TextToCenter4;
		static const long ID_BITMAPBUTTON_TextToCenter4;
		static const long ID_STATICTEXT31;
		static const long IDD_SLIDER_Text_Speed4;
		static const long ID_TEXTCTRL_Text_Speed4;
		static const long ID_BITMAPBUTTON28;
		static const long ID_STATICTEXT29;
		static const long ID_CHOICE_Text_Effect4;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Effect4;
		static const long ID_STATICTEXT47;
		static const long ID_CHOICE_Text_Count4;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Count4;
		static const long ID_STATICTEXT48;
		static const long ID_SLIDER_Text_Position4;
		static const long IDD_TEXTCTRL_Text_Position4;
		static const long ID_BITMAPBUTTON_SLIDER_Text_Position4;
		static const long ID_PANEL_Text4;
		static const long IDD_NOTEBOOK_Text;
		//*)
    protected:

	public:

		//(*Handlers(TextPanel)
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
