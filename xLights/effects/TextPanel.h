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
		wxSlider* Slider_Text_YEnd;
		wxStaticText* StaticText53;
		wxSlider* Slider_Text_YStart;
		wxSlider* Slider_Text_Speed;
		wxStaticText* StaticText107;
		wxStaticText* StaticText78;
		wxCheckBox* CheckBox_Text_PixelOffsets;
		wxBitmapButton* BitmapButton_TextToCenter;
		wxStaticText* StaticText186;
		wxTextCtrl* TextCtrl_Text;
		wxPanel* Panel_Text1;
		wxStaticText* StaticText1;
		wxBitmapButton* BitmapButton1;
		wxChoice* Choice_Text_Font;
		wxSlider* Slider_Text_XStart;
		wxStaticText* StaticText98;
		wxChoice* Choice_Text_Effect;
		wxBitmapButton* BitmapButton_TextDir;
		wxSlider* Slider_Text_XEnd;
		wxBitmapButton* BitmapButton_TextCount;
		wxChoice* Choice_Text_Count;
		wxBitmapButton* BitmapButton_TextEffect;
		wxBitmapButton* BitmapButton_Text_Speed;
		wxPanel* Panel17;
		wxBitmapButton* BitmapButton_TextFont;
		wxChoice* Choice_Text_Dir;
		//*)


		//(*Identifiers(TextPanel)
		static const long ID_STATICTEXT53;
		static const long ID_TEXTCTRL_Text;
		static const long ID_FONTPICKER_Text_Font;
		static const long ID_BITMAPBUTTON_FONTPICKER_Text_Font;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_Text_Font;
		static const long ID_BITMAPBUTTON1;
		static const long ID_STATICTEXT79;
		static const long ID_CHOICE_Text_Dir;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Dir;
		static const long ID_CHECKBOX_TextToCenter;
		static const long ID_BITMAPBUTTON_TextToCenter;
		static const long ID_STATICTEXT28;
		static const long IDD_SLIDER_Text_Speed;
		static const long ID_TEXTCTRL_Text_Speed;
		static const long ID_BITMAPBUTTON_Text_Speed;
		static const long ID_STATICTEXT108;
		static const long ID_CHOICE_Text_Effect;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Effect;
		static const long ID_STATICTEXT99;
		static const long ID_CHOICE_Text_Count;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Count;
		static const long ID_CHECKBOX_Text_PixelOffsets;
		static const long ID_SLIDER_Text_XStart;
		static const long IDD_TEXTCTRL_Text_XStart;
		static const long IDD_TEXTCTRL_Text_YStart;
		static const long ID_SLIDER_Text_YStart;
		static const long IDD_PANEL6;
		static const long ID_SLIDER_Text_XEnd;
		static const long IDD_TEXTCTRL_Text_XEnd;
		static const long IDD_TEXTCTRL_Text_YEnd;
		static const long ID_SLIDER_Text_YEnd;
		static const long IDD_PANEL17;
		static const long IDD_NOTEBOOK1;
		static const long ID_PANEL_Text1;
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
		void OnVCChanged(wxCommandEvent& event);
        void UpdateLinkedSliderFloat2(wxCommandEvent& event);
        void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
