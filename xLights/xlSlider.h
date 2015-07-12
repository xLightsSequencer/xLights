#ifndef XLSLIDER_H
#define XLSLIDER_H

#include "wx/wx.h"

class xlSlider : public wxSlider
{
    public:

        xlSlider(wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize,
                long style=wxSL_HORIZONTAL, const wxValidator &validator=wxDefaultValidator, const wxString &name=wxSliderNameStr);
        virtual ~xlSlider();
    protected:
    private:
};

#endif // XLSLIDER_H
