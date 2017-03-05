#include <wx/wx.h>

wxString GetBitness()
{
    if (sizeof(size_t) == 4)
    {
        return "32bit";
    }
    else if (sizeof(size_t) == 8)
    {
        return "64bit";
    }

    return "";
}
