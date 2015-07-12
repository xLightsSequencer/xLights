#include "xlColorPicker.h"


BEGIN_EVENT_TABLE(xlColorPicker,wxPanel)
END_EVENT_TABLE()

xlColorPicker::xlColorPicker(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size, long style, const wxString& name)
: mCurrentColor(xlRED)
{
}

xlColorPicker::~xlColorPicker()
{
}
