/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "DragColoursBitmapButton.h"
#include <wx/dnd.h>
#include "xLightsMain.h"
#include "UtilFunctions.h"

DragColoursBitmapButton::DragColoursBitmapButton(wxWindow* parent, wxWindowID id, const wxBitmapBundle& bitmap, const wxPoint& pos,
    const wxSize& size, long style, const wxValidator& validator,
    const wxString& name)
    : wxBitmapButton(parent, id, bitmap, pos, size, style, validator, name)
{
    Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(DragColoursBitmapButton::OnMouseLeftDown));
}

DragColoursBitmapButton::~DragColoursBitmapButton()
{
}

void DragColoursBitmapButton::DoSetSizeHints(int minW, int minH,
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

void DragColoursBitmapButton::SetColour(const std::string& colour)
{
    wxSize size = GetSize();
    _c = colour;

    if (ColorCurve::IsColorCurve(colour)) {
        ColorCurve cc(colour);
        const wxBitmap& bbmp = cc.GetImage(size.GetWidth(), size.GetHeight(), false);
        SetBitmap(bbmp);
        UnsetToolTip();
    }
    else {
        wxColour color (colour);
        SetBitmap(ColorCurve::GetSolidColourImage(size.GetWidth(), size.GetHeight(), color));
        SetToolTip(wxString::Format("%s\n%d,%d,%d\n%s", colour, color.Red(), color.Green(), color.Blue(), GetColourName(color)));
    }
}

void DragColoursBitmapButton::OnMouseLeftDown(wxMouseEvent& event)
{
    if (_c == "") {
        return;
    }

    wxString data;
    wxTextDataObject dragData(_c);

#ifdef __linux__
    wxIcon dragCursor;
    dragCursor.CopyFromBitmap(GetBitmap().ConvertToImage());
#else
    wxCursor dragCursor(GetBitmap().ConvertToImage());
#endif

    wxDropSource dragSource(GetParent(), dragCursor, dragCursor, dragCursor);

    dragSource.SetData(dragData);
    dragSource.DoDragDrop(wxDragCopy);
}

void DragColoursBitmapButton::SetBitmap(const wxBitmapBundle& bpm)
{
    wxBitmapButton::SetBitmap(bpm);
}
