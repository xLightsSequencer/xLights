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

 //(*Headers(CharMapDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class KeyActivatedTextCtrl;

class ClickableStaticText : public wxStaticText
{
public:
    void OnLeftDClick(wxMouseEvent& event)
    {
        wxCommandEvent e(wxEVT_COMMAND_LEFT_DCLICK, GetId());
        e.SetEventObject(this);
        wxPostEvent(GetParent(), e);
    }
    void OnMotion(wxMouseEvent& event)
    {
        wxPostEvent(GetParent(), event);
    }
    ClickableStaticText(wxWindow* parent, wxWindowID id, const wxString& label,
        const wxPoint& pos = wxDefaultPosition, const wxSize
        & size = wxDefaultSize, int style = 0, const wxString& name =
        "staticText") : wxStaticText(parent, id, label, pos, size, style, name)
    {
        Connect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)& ClickableStaticText::OnLeftDClick, 0, this);
        Connect(wxEVT_MOTION, (wxObjectEventFunction)& ClickableStaticText::OnMotion, 0, this);
    }
    virtual ~ClickableStaticText() {}
};

#define CHARMAP_COLS 16
#define CHARMAP_ROWS 8
#define CHARMAP_SIZE 30

class CharMapDialog: public wxDialog
{
    unsigned long _page;
    int _map[CHARMAP_COLS * CHARMAP_ROWS];
    int _charCode;
    int _originalCode;
    int _highlighted = -1;

	public:

		CharMapDialog(wxWindow* parent, wxFont f, int charChode, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~CharMapDialog();
        int GetCharCode() const { return _charCode; }
        void OnDClick(wxCommandEvent& event);
        void PageChange();
        bool IsValidUnicode(int c);
        int GetMaxPage()
        {
            return 0x10FFFF / (CHARMAP_COLS * CHARMAP_ROWS) + 1;
        }

		//(*Declarations(CharMapDialog)
		KeyActivatedTextCtrl* TextCtrl_Page;
		wxButton* Button_Down;
		wxButton* Button_Up;
		wxGridSizer* GridSizer1;
		wxStaticText* StaticText1;
		//*)

	protected:

		//(*Identifiers(CharMapDialog)
		static const long ID_STATICTEXT1;
		static const long ID_BUTTON1;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(CharMapDialog)
		void OnMouseMove(wxMouseEvent& event);
		void OnButton_DownClick(wxCommandEvent& event);
		void OnButton_UpClick(wxCommandEvent& event);
		void OnTextCtrl_PageText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
