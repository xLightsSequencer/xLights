/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "DragValueCurveBitmapButton.h"
#include <wx/dnd.h>
#include "xLightsMain.h"
#include "UtilFunctions.h"

DragValueCurveBitmapButton::DragValueCurveBitmapButton(wxWindow* parent, wxWindowID id, const wxBitmap& bitmap, const wxPoint& pos,
    const wxSize& size, long style, const wxValidator& validator,
    const wxString& name)
    : wxBitmapButton(parent, id, bitmap, pos, size, style, validator, name)
{
    Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(DragValueCurveBitmapButton::OnMouseLeftDown));
}

DragValueCurveBitmapButton::~DragValueCurveBitmapButton()
{
}
void DragValueCurveBitmapButton::DoSetSizeHints(int minW, int minH,
    int maxW, int maxH,
    int incW, int incH)
{
    int offset = 0;
#ifdef LINUX
    offset = 12; //Linux puts a 6 pixel border around it
#endif // LINUX
//SetEffect(mEffect, UnScaleWithSystemDPI(minW));
    wxBitmapButton::DoSetSizeHints(minW + offset,
        minH + offset,
        maxW + offset,
        maxH + offset,
        incW, incH);
}

void DragValueCurveBitmapButton::SetValueCurve(const std::string& valueCurve)
{
    _vc = valueCurve;

    ValueCurve vc(valueCurve);

    if (vc.IsOk()) {
        wxSize size = GetSize();
        const wxBitmap& bbmp = vc.GetImage(size.GetWidth(), size.GetHeight(), GetContentScaleFactor());
        SetBitmap(bbmp);
    }
}

void DragValueCurveBitmapButton::OnMouseLeftDown(wxMouseEvent& event)
{
    if (_vc == "") {
        return;
    }

    wxString data;
    wxTextDataObject dragData(_vc);

#ifdef __linux__
    wxIcon dragCursor;
    dragCursor.CopyFromBitmap(GetBitmap().ConvertToImage());
#else
    wxCursor dragCursor(GetBitmap().ConvertToImage());
#endif

    wxDropSource dragSource(this, dragCursor, dragCursor, dragCursor);

    dragSource.SetData(dragData);
    dragSource.DoDragDrop(wxDragCopy);
}

void DragValueCurveBitmapButton::SetBitmap(const wxBitmap& bpm)
{
    wxBitmapButton::SetBitmap(bpm);
    wxBitmapButton::SetBitmapDisabled(bpm.ConvertToDisabled());
}
