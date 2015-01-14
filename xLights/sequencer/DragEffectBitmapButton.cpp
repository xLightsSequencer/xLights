#include "DragEffectBitmapButton.h"
#include <wx/dnd.h>

DragEffectBitmapButton::DragEffectBitmapButton (wxWindow *parent, wxWindowID id, const wxBitmap &bitmap, const wxPoint &pos,
                                const wxSize &size, long style, const wxValidator &validator,
                                const wxString &name): wxBitmapButton (parent, id, bitmap,pos,size,style,validator,name)
{
    Connect (wxEVT_LEFT_DOWN, wxMouseEventHandler (DragEffectBitmapButton::OnMouseLeftDown));
}

DragEffectBitmapButton::~DragEffectBitmapButton()
{
}


void DragEffectBitmapButton::OnMouseLeftDown (wxMouseEvent& event)
{
    std::cout << "dragging" << std::endl;
    wxString data = "hello";
    wxTextDataObject dragData(data);

    wxBitmap* bmDrag=new wxBitmap(mDragIconBuffer);
    wxCursor dragCursor(bmDrag->ConvertToImage());

//    const wxIcon icon(spirals);
    wxDropSource dragSource(this,dragCursor,dragCursor,dragCursor );

    dragSource.SetData( dragData );
    wxDragResult result = dragSource.DoDragDrop( wxDragMove );
    int i = 0;
}

void DragEffectBitmapButton::SetBitmap(const char** xpm)
{
    mDragIconBuffer = xpm;
    wxBitmap bm(xpm);
    SetBitmapLabel(bm);
}
