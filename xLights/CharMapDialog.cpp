/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "CharMapDialog.h"

//(*InternalHeaders(CharMapDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/valnum.h>
#include <wx/settings.h>

#include <log4cpp/Category.hh>

//(*IdInit(CharMapDialog)
const long CharMapDialog::ID_STATICTEXT1 = wxNewId();
const long CharMapDialog::ID_BUTTON1 = wxNewId();
const long CharMapDialog::ID_TEXTCTRL1 = wxNewId();
const long CharMapDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(CharMapDialog,wxDialog)
	//(*EventTable(CharMapDialog)
	//*)
END_EVENT_TABLE()

class KeyActivatedTextCtrl :public wxTextCtrl
{
public:

    void OnKeyDown(wxKeyEvent& event)
    {
        wxCommandEvent e;
        if (event.GetKeyCode() == WXK_UP)
        {
            int v = wxAtoi(GetValue());
            v++;
            SetValue(wxString::Format("%d", v));
        }
        else if (event.GetKeyCode() == WXK_PAGEUP)
        {
            int v = wxAtoi(GetValue());
            v+=10;
            SetValue(wxString::Format("%d", v));
        }
        else if (event.GetKeyCode() == WXK_DOWN)
        {
            int v = wxAtoi(GetValue());
            v--;
            SetValue(wxString::Format("%d", v));
        }
        else if (event.GetKeyCode() == WXK_PAGEDOWN)
        {
            int v = wxAtoi(GetValue());
            v-=10;
            SetValue(wxString::Format("%d", v));
        }
        else
        {
            event.Skip();
        }
    }

    KeyActivatedTextCtrl(wxWindow* parent, wxWindowID id, wxString value = "", 
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, 
        long style = 0, const wxValidator& val = wxDefaultValidator, const wxString& name = "textCtrl") :
        wxTextCtrl(parent, id, value, pos, size, style, val, name)
    {
        Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)& KeyActivatedTextCtrl::OnKeyDown, 0, this);
    }
    virtual ~KeyActivatedTextCtrl() {}
};

CharMapDialog::CharMapDialog(wxWindow* parent, wxFont font, int charCode, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _charCode = charCode;
    _originalCode = charCode;
    _page = _charCode / (CHARMAP_ROWS * CHARMAP_COLS) + 1;

    wxIntegerValidator<unsigned long> val(&_page, wxNUM_VAL_THOUSANDS_SEPARATOR);
    val.SetMin(1);
    val.SetMax(65536);

	//(*Initialize(CharMapDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, id, _("Character Map"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	SetMinSize(wxSize(500,500));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Page:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Button_Down = new wxButton(this, ID_BUTTON1, _("<"), wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Down, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Page = new KeyActivatedTextCtrl(this, ID_TEXTCTRL1, _("1"), wxDefaultPosition, wxDefaultSize, 0, val, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(TextCtrl_Page, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Up = new wxButton(this, ID_BUTTON2, _(">"), wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Up, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(0);
	GridSizer1 = new wxGridSizer(16, 32, 0, 0);
	FlexGridSizer3->Add(GridSizer1, 0, wxEXPAND, 2);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CharMapDialog::OnButton_DownClick);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&CharMapDialog::OnTextCtrl_PageText);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CharMapDialog::OnButton_UpClick);
	Connect(wxEVT_MOTION,(wxObjectEventFunction)&CharMapDialog::OnMouseMove);
	//*)

    GridSizer1->SetRows(CHARMAP_ROWS);
    GridSizer1->SetCols(CHARMAP_COLS);

    SetSize(CHARMAP_COLS * CHARMAP_SIZE * 2, CHARMAP_ROWS * CHARMAP_SIZE * 2);

    Freeze();
    for (int i = 0; i < CHARMAP_ROWS * CHARMAP_COLS; i++)
    {
        int c = (_page-1) * CHARMAP_ROWS * CHARMAP_COLS + i;
        wxString s = wxString(wxUniChar(c));
        auto id = wxNewId();
        auto label = new ClickableStaticText(this, id, s, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, wxString::Format("ID_STATICTEXT%d",i));
        _map[i] = c;
        font.SetPixelSize(wxSize(CHARMAP_SIZE, CHARMAP_SIZE));
        label->SetFont(font);
        Connect(id, wxEVT_COMMAND_LEFT_DCLICK, (wxObjectEventFunction)& CharMapDialog::OnDClick, 0, this);
        Connect(id, wxEVT_MOTION, (wxObjectEventFunction)& CharMapDialog::OnMouseMove, 0, this);
        if (c == _originalCode)
        {
            label->SetBackgroundColour(*wxLIGHT_GREY);
        }
        GridSizer1->Add(label, 1, wxALL | wxEXPAND, 0);
    }
    TextCtrl_Page->SetValue(wxString::Format("%d", _page));
    Thaw();
    Layout();
}

CharMapDialog::~CharMapDialog()
{
	//(*Destroy(CharMapDialog)
	//*)
}

void CharMapDialog::OnDClick(wxCommandEvent& event)
{
    for (int i = 0; i < CHARMAP_ROWS * CHARMAP_COLS; i++)
    {
        if (GridSizer1->GetItem(i)->GetWindow() == event.GetEventObject())
        {
            if (!IsValidUnicode(_map[i])) return;
            _charCode = _map[i];
            break;
        }
    }
    EndDialog(wxID_OK);
}

void CharMapDialog::PageChange()
{
    Freeze();
    for (int i = 0; i < CHARMAP_ROWS * CHARMAP_COLS; i++)
    {
        int c = (wxAtoi(TextCtrl_Page->GetValue()) - 1) * CHARMAP_ROWS * CHARMAP_COLS + i;
        wxString s = wxString(wxUniChar(c));
        auto label = GridSizer1->GetItem(i)->GetWindow();
        label->SetLabel(s);
        _map[i] = c;
        if (!IsValidUnicode(c))
        {
            label->SetBackgroundColour(*wxRED);
        }
        else if (c == _originalCode)
        {
            label->SetBackgroundColour(*wxLIGHT_GREY);
        }
        else
        {
            label->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
        }
    }
    Thaw();
    Refresh();
}

void CharMapDialog::OnMouseMove(wxMouseEvent& event)
{
    int newHighlighted = -1;

    for (int i = 0; i < CHARMAP_ROWS * CHARMAP_COLS; i++)
    {
        auto label = GridSizer1->GetItem(i)->GetWindow();
        int c = _map[i];
        if (!IsValidUnicode(c))
        {
            label->SetBackgroundColour(*wxRED);
            label->Refresh();
        }
        else if (label == event.GetEventObject())
        {
            if (c != _highlighted)
            {
                label->SetBackgroundColour(*wxYELLOW);
                label->Refresh();
            }
            newHighlighted = c;
        }
        else if (c == _highlighted)
        {
            if (c == _originalCode)
            {
                label->SetBackgroundColour(*wxLIGHT_GREY);
            }
            else
            {
                label->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
            }
            label->Refresh();
        }
    }
    _highlighted = newHighlighted;
}

void CharMapDialog::OnButton_DownClick(wxCommandEvent& event)
{
    int newPage = wxAtoi(TextCtrl_Page->GetValue()) - 1;
    if (newPage < 1) newPage = 1;
    TextCtrl_Page->SetValue(wxString::Format("%d", newPage));
    PageChange();
}

bool CharMapDialog::IsValidUnicode(int c)
{
    if (c < 32) return false; // while technically valid i dont want the user using them
    if (c > 0x10FFFF) return false;
    if (c >= 0xD800 && c <= 0xDFFF) return false;
    return true;
}

void CharMapDialog::OnButton_UpClick(wxCommandEvent& event)
{
    int newPage = wxAtoi(TextCtrl_Page->GetValue()) + 1;
    if (newPage > GetMaxPage()) newPage = GetMaxPage();
    TextCtrl_Page->SetValue(wxString::Format("%d", newPage));
    PageChange();
}

void CharMapDialog::OnTextCtrl_PageText(wxCommandEvent& event)
{
    auto v = TextCtrl_Page->GetValue();
    if (wxAtoi(TextCtrl_Page->GetValue()) < 1) TextCtrl_Page->SetValue("1");
    if (wxAtoi(TextCtrl_Page->GetValue()) > GetMaxPage())
    {
        TextCtrl_Page->SetValue(wxString::Format("%d", GetMaxPage()));
    }
    PageChange();
}
