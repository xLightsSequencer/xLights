#ifndef XLLOCKBUTTON_H
#define XLLOCKBUTTON_H

#include <wx/bmpbuttn.h>


class xlSizedBitmapButton : public wxBitmapButton
{
public:
    xlSizedBitmapButton(wxWindow *parent, wxWindowID id, const wxBitmap &bitmap, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=wxBU_AUTODRAW, const wxValidator &validator=wxDefaultValidator, const wxString &name=wxButtonNameStr);
    virtual ~xlSizedBitmapButton();
    
protected:
    
private:
};



class xlLockButton : public xlSizedBitmapButton
{
    public:
        xlLockButton(wxWindow *parent, wxWindowID id, const wxBitmap &bitmap, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=wxBU_AUTODRAW, const wxValidator &validator=wxDefaultValidator, const wxString &name=wxButtonNameStr);
        virtual ~xlLockButton();

    protected:

    private:
};

#endif // XLLOCKBUTTON_H
