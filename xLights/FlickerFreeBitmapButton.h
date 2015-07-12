#ifndef FLICKERFREEBITMAPBUTTON_H
#define FLICKERFREEBITMAPBUTTON_H

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

#endif // FLICKERFREEBITMAPBUTTON_H
