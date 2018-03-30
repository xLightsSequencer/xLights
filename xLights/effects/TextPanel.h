#ifndef TEXTPANEL_H
#define TEXTPANEL_H

//(*Headers(TextPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxFontPickerCtrl;
class wxGridBagSizer;
class wxNotebook;
class wxNotebookEvent;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class TextPanel: public wxPanel
{
	public:

		TextPanel(wxWindow* parent);
		virtual ~TextPanel();

		//(*Declarations(TextPanel)
		BulkEditCheckBox* CheckBox_Text_PixelOffsets;
		BulkEditChoice* Choice_Text_Count;
		BulkEditChoice* Choice_Text_Dir;
		BulkEditChoice* Choice_Text_Effect;
		BulkEditChoice* Choice_Text_Font;
		BulkEditSlider* Slider_Text_Speed;
		BulkEditSlider* Slider_Text_XEnd;
		BulkEditSlider* Slider_Text_XStart;
		BulkEditSlider* Slider_Text_YEnd;
		BulkEditSlider* Slider_Text_YStart;
		BulkEditTextCtrl* TextCtrl_Text;
		wxBitmapButton* BitmapButton1;
		wxBitmapButton* BitmapButton_TextCount;
		wxBitmapButton* BitmapButton_TextDir;
		wxBitmapButton* BitmapButton_TextEffect;
		wxBitmapButton* BitmapButton_TextFont;
		wxBitmapButton* BitmapButton_TextToCenter;
		wxBitmapButton* BitmapButton_Text_Speed;
		wxPanel* Panel17;
		wxPanel* Panel_Text1;
		wxStaticText* StaticText107;
		wxStaticText* StaticText186;
		wxStaticText* StaticText1;
		wxStaticText* StaticText211;
		wxStaticText* StaticText212;
		wxStaticText* StaticText213;
		wxStaticText* StaticText214;
		wxStaticText* StaticText53;
		wxStaticText* StaticText78;
		wxStaticText* StaticText98;
		//*)


		//(*Identifiers(TextPanel)
		static const long ID_STATICTEXT_Text;
		static const long ID_TEXTCTRL_Text;
		static const long ID_FONTPICKER_Text_Font;
		static const long ID_BITMAPBUTTON_FONTPICKER_Text_Font;
		static const long ID_STATICTEXT_Text_Font;
		static const long ID_CHOICE_Text_Font;
		static const long ID_BITMAPBUTTON1;
		static const long ID_STATICTEXT_Text_Dir;
		static const long ID_CHOICE_Text_Dir;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Dir;
		static const long ID_CHECKBOX_TextToCenter;
		static const long ID_BITMAPBUTTON_TextToCenter;
		static const long ID_STATICTEXT_Text_Speed;
		static const long IDD_SLIDER_Text_Speed;
		static const long ID_TEXTCTRL_Text_Speed;
		static const long ID_BITMAPBUTTON_Text_Speed;
		static const long ID_STATICTEXT_Text_Effect;
		static const long ID_CHOICE_Text_Effect;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Effect;
		static const long ID_STATICTEXT_Text_Count;
		static const long ID_CHOICE_Text_Count;
		static const long ID_BITMAPBUTTON_CHOICE_Text_Count;
		static const long ID_CHECKBOX_Text_PixelOffsets;
		static const long ID_STATICTEXT_Text_XStart;
		static const long ID_SLIDER_Text_XStart;
		static const long IDD_TEXTCTRL_Text_XStart;
		static const long ID_STATICTEXT_Text_YStart;
		static const long IDD_TEXTCTRL_Text_YStart;
		static const long ID_SLIDER_Text_YStart;
		static const long IDD_PANEL6;
		static const long ID_STATICTEXT_Text_XEnd;
		static const long ID_SLIDER_Text_XEnd;
		static const long IDD_TEXTCTRL_Text_XEnd;
		static const long ID_STATICTEXT_Text_YEnd;
		static const long IDD_TEXTCTRL_Text_YEnd;
		static const long ID_SLIDER_Text_YEnd;
		static const long IDD_PANEL17;
		static const long IDD_NOTEBOOK1;
		static const long ID_PANEL_Text1;
		//*)
    protected:

	public:

		//(*Handlers(TextPanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
