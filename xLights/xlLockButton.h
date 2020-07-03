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
