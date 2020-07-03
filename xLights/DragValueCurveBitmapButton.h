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

#include <wx/wx.h>

class ValueCurve;

class DragValueCurveBitmapButton : public wxBitmapButton
{
public:
    DragValueCurveBitmapButton(wxWindow* parent, wxWindowID id, const wxBitmap& bitmap, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = wxBU_AUTODRAW, const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxButtonNameStr);
    virtual void DoSetSizeHints(int minW, int minH,
        int maxW, int maxH,
        int incW, int incH);
    virtual ~DragValueCurveBitmapButton();
    void OnMouseLeftDown(wxMouseEvent& event);
    void SetBitmap(const wxBitmap& bmp);
    void SetValueCurve(const std::string& _valueCurve);

protected:
private:
    wxString(*GetStringFunction)();
    std::string _vc;
};
