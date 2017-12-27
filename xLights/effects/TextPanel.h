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

#include "../BulkEditControls.h"

class TextPanel: public wxPanel
{
	public:

		TextPanel(wxWindow* parent);
		virtual ~TextPanel();

		//(*Declarations(TextPanel)
		wxStaticText* StaticText53;
		wxStaticText* StaticText213;
		BulkEditSlider* Slider_Text_XStart;
		wxStaticText* StaticText107;
		BulkEditChoice* Choice_Text_Dir;
		wxStaticText* StaticText78;
		BulkEditChoice* Choice_Text_Font;
		BulkEditTextCtrl* TextCtrl_Text;
		wxBitmapButton* BitmapButton_TextToCenter;
		wxStaticText* StaticText186;
		wxPanel* Panel_Text1;
		wxStaticText* StaticText1;
		BulkEditSlider* Slider_Text_YStart;
		wxBitmapButton* BitmapButton1;
		wxStaticText* StaticText98;
		wxStaticText* StaticText211;
		wxBitmapButton* BitmapButton_TextDir;
		wxStaticText* StaticText212;
		wxBitmapButton* BitmapButton_TextCount;
		BulkEditChoice* Choice_Text_Count;
		BulkEditSlider* Slider_Text_XEnd;
		wxBitmapButton* BitmapButton_TextEffect;
		BulkEditSlider* Slider_Text_YEnd;
		wxBitmapButton* BitmapButton_Text_Speed;
		wxStaticText* StaticText214;
		wxPanel* Panel17;
		wxBitmapButton* BitmapButton_TextFont;
		BulkEditCheckBox* CheckBox_Text_PixelOffsets;
		BulkEditChoice* Choice_Text_Effect;
		BulkEditSlider* Slider_Text_Speed;
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
