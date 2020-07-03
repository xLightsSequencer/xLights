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

class FlickerFreeBitmapButton : public wxBitmapButton
{
    public:
         FlickerFreeBitmapButton(wxWindow* parent,
                                 wxWindowID id,
                                 const wxBitmap& bitmap,
                                 const wxPoint& pos,
                                 const wxSize& size = wxDefaultSize,
                                 long style = 0,
                                 const wxValidator& validator = wxDefaultValidator,
                                 const wxString& name = "ButtonName" ):
         wxBitmapButton(parent,id,bitmap,pos,size,style,validator,name){};
        virtual ~FlickerFreeBitmapButton();

    protected:

    void OnEraseBackGround(wxEraseEvent& event) {};

    DECLARE_EVENT_TABLE()
};
