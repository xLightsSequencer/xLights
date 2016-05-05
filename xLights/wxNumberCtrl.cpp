
#ifdef __GNUG__
#pragma implementation "wxNumberCtrl.h"
#endif

#include "wx/wx.h"
#include "wx/textctrl.h"
#include "wxNumberCtrl.h"

IMPLEMENT_CLASS(wxNumberCtrl, wxTextCtrl)

BEGIN_EVENT_TABLE(wxNumberCtrl, wxTextCtrl)
EVT_KEY_DOWN(wxNumberCtrl::OnChar)
EVT_KEY_UP(wxNumberCtrl::OnKeyUp)
EVT_KILL_FOCUS(wxNumberCtrl::OnKillFocus)
END_EVENT_TABLE()


bool wxNumberCtrl::Create(wxWindow* parent, wxWindowID id, const int value,
    const wxPoint& pos, const wxSize& size,
    long style, const wxValidator& validator,
    const wxString& idname)
{
    wxString s;
    if (value != 0)
        s.Printf("%i", value);
    max = INT_MAX;
    min = -(INT_MAX - 1);
    noshow = false;

    if (!wxTextCtrl::Create(parent, id, s, pos, size, style, validator, idname))
        return false;
    else
        return true;


}

void wxNumberCtrl::SetRange(double minimum, double maximum)
{
    double d;
    if (minimum>maximum)
    {
        d = minimum;
        minimum = maximum;
        maximum = d;
    }
    min = minimum;
    max = maximum;

}

void wxNumberCtrl::SetRingBell(bool ring)
{
    ringbell = ring;
}

int wxNumberCtrl::GetInt()
{
    wxString s;
    long val;
    s = GetValue();
    if (noshow)
    {
        if (GetLastPosition() == 0)
            val = noshownumber;
        else
            s.ToLong(&val, 10);
    }
    else
    {
        s.ToLong(&val, 10);
    }
    return (int)val;
}

void wxNumberCtrl::SetInt(int i)
{
    wxString s;
    s.Printf("%i", i);
    SetValue(s);
    if ((noshow) && (i == noshownumber))
        Clear();

}

double wxNumberCtrl::GetDouble()
{
    wxString s;
    s = wxTextCtrl::GetValue();
    double val;
    if (noshow)
    {
        if (GetLastPosition() == 0)
            val = noshownumber;
        else
            s.ToDouble(&val);
    }
    else
    {
        s.ToDouble(&val);
    }
    return val;
}

void wxNumberCtrl::SetDouble(double d, wxString s)
{
    wxString t;
    t.Printf(s, d);
    SetValue(t);
    if ((noshow) && (d == noshownumber))
        Clear();
}

void wxNumberCtrl::OnChar(wxKeyEvent& event)
{
    oldvalue = GetValue();

    switch (event.GetKeyCode())
    {

    case '0':
    case WXK_NUMPAD0:
        if (!((GetDouble() == 0) && (oldvalue.Find('.') == -1))) //keine doppelte Null
            event.Skip();
        break;

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case WXK_NUMPAD1:
    case WXK_NUMPAD2:
    case WXK_NUMPAD3:
    case WXK_NUMPAD4:
    case WXK_NUMPAD5:
    case WXK_NUMPAD6:
    case WXK_NUMPAD7:
    case WXK_NUMPAD8:
    case WXK_NUMPAD9:
        if (GetDouble() == 0) Clear();

    case WXK_DELETE:
    case WXK_RIGHT:
    case WXK_LEFT:
    case WXK_BACK:
    case WXK_TAB:
    case WXK_NUMPAD_END:
    case WXK_NUMPAD_BEGIN:
    case WXK_HOME:
    case WXK_END:
    case WXK_RETURN:
        event.Skip();
        break;

    case WXK_NUMPAD_DECIMAL:
    case '.':
    case ',':
        if (!intonly)
            if (oldvalue.Find('.') == -1) //keine doppelten Punkte zulassen
                WriteText(".");
        break;

    case WXK_NUMPAD_SUBTRACT:
    case '-':
        if (min<0)
            event.Skip();
        break;

    default:
        if (ringbell) wxBell();
        event.Skip(false);
        break;
    }

}

void wxNumberCtrl::OnKeyUp(wxKeyEvent& event)
{
    double d = GetDouble();
    if ((d>max) || (d<min))
    {
        SetValue(oldvalue);
        if (ringbell) wxBell();
    }

}

void wxNumberCtrl::SetNoShow(double ns, bool b)
{
    noshow = b;
    noshownumber = ns;
}


void wxNumberCtrl::OnKillFocus(wxFocusEvent& event)
{
    if (noshow)
    {
        if (GetDouble() == noshownumber)
            Clear();
    }
}

void wxNumberCtrl::AcceptIntOnly(bool b)
{
    intonly = b;
}
