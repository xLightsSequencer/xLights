#ifndef _WXNUMBERCTRL_H_
#define _WXNUMBERCTRL_H_

#ifdef __GNUG__
#pragma interface "wxNumberCtrl.cpp"
#endif

// includes
#include "wx/wx.h"
#include "wx/textctrl.h"

class wxNumberCtrl : public wxTextCtrl
{
    DECLARE_CLASS(wxNumberCtrl)
public:
    wxNumberCtrl() {};

    wxNumberCtrl(wxWindow* parent, wxWindowID id, const int value = 0,
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = 0, const wxValidator& validator = wxDefaultValidator,
        const wxString& idname = wxTextCtrlNameStr)
    {
        Create(parent, id, value, pos, size, style, validator, idname);
    };

    wxNumberCtrl(wxWindow* parent, wxWindowID id, wxString s,
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = 0, const wxValidator& validator = wxDefaultValidator,
        const wxString& idname = wxTextCtrlNameStr)
    {
        Create(parent, id, 0, pos, size, style, validator, idname);
    };

    /// Creation
    bool Create(wxWindow* parent, wxWindowID id, const int value = 0,
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = 0, const wxValidator& validator = wxDefaultValidator,
        const wxString& idname = wxTextCtrlNameStr);

    // Methoden zum Zugriff auf interne Variablen

    //Lesen
    int GetInt();
    double GetDouble();

    //Setzen

    void SetRingBell(bool ring);
    void SetInt(int i);
    void SetDouble(double d, wxString s = wxT("%0.9g"));
    void SetRange(double minimum, double maximum);
    void SetNoShow(double ns, bool b = true);
    void AcceptIntOnly(bool b);

private:

    DECLARE_EVENT_TABLE()

    void OnKillFocus(wxFocusEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);


    wxString oldvalue;
    int min, max;

    double noshownumber;

    bool ringbell, noshow, intonly;
};
#endif