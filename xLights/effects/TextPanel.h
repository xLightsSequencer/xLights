#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(TextPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxChoice;
class wxFilePickerCtrl;
class wxFlexGridSizer;
class wxFontPickerCtrl;
class wxGridBagSizer;
class wxNotebook;
class wxNotebookEvent;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include <wx/filepicker.h>
#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"
#include "ExternalHooks.h"

class xlTextFilePickerCtrl : public BulkEditFilePickerCtrl {
public:
    xlTextFilePickerCtrl(wxWindow *parent,
        wxWindowID id,
        const wxString& path = wxEmptyString,
        const wxString& message = wxFileSelectorPromptStr,
        const wxString& wildcard = wxFileSelectorDefaultWildcardStr,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxFLP_DEFAULT_STYLE,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxFilePickerCtrlNameStr)
        : BulkEditFilePickerCtrl(parent, id, path, message, _T("*.txt"), pos, size, style, validator, name) {
    }
    virtual ~xlTextFilePickerCtrl()
    {}

protected:
    void ValidateControl() override;
};


class TextPanel: public xlEffectPanel
{
	public:

		TextPanel(wxWindow* parent);
		virtual ~TextPanel();
		virtual void ValidateWindow() override;

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
		wxChoice* Choice_LyricTrack;
		wxPanel* Panel17;
		wxPanel* Panel_Text1;
		wxStaticText* StaticText107;
		wxStaticText* StaticText162;
		wxStaticText* StaticText186;
		wxStaticText* StaticText1;
		wxStaticText* StaticText211;
		wxStaticText* StaticText212;
		wxStaticText* StaticText213;
		wxStaticText* StaticText214;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText53;
		wxStaticText* StaticText78;
		wxStaticText* StaticText98;
		xlLockButton* BitmapButton1;
		xlLockButton* BitmapButton_TextCount;
		xlLockButton* BitmapButton_TextDir;
		xlLockButton* BitmapButton_TextEffect;
		xlLockButton* BitmapButton_TextFont;
		xlLockButton* BitmapButton_TextToCenter;
		xlLockButton* BitmapButton_Text_Speed;
		xlTextFilePickerCtrl* FilePickerCtrl1;
		//*)


		//(*Identifiers(TextPanel)
		static const long ID_STATICTEXT_Text;
		static const long ID_TEXTCTRL_Text;
		static const long ID_STATICTEXT1;
		static const long ID_FILEPICKERCTRL_Text_File;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE_Text_LyricTrack;
		static const long ID_STATICTEXT_FONTLABEL;
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
		void OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event);
		void OnTextCtrl_TextText(wxCommandEvent& event);
		void OnChoice_LyricTrackSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
