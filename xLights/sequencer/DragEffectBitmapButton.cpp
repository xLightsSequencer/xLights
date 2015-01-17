#include "DragEffectBitmapButton.h"
#include <wx/dnd.h>
#include "../xLightsMain.h"

DragEffectBitmapButton::DragEffectBitmapButton (wxWindow *parent, wxWindowID id, const wxBitmap &bitmap, const wxPoint &pos,
                                const wxSize &size, long style, const wxValidator &validator,
                                const wxString &name): wxBitmapButton (parent, id, bitmap,pos,size,style,validator,name)
{
    Connect (wxEVT_LEFT_DOWN, wxMouseEventHandler (DragEffectBitmapButton::OnMouseLeftDown));
}

DragEffectBitmapButton::~DragEffectBitmapButton()
{
}

void DragEffectBitmapButton::SetEffectIndex(int index)
{
    SetBitmap(xLightsFrame::GetIconBuffer(index));
    mEffectIndex = index;
}

void DragEffectBitmapButton::OnMouseLeftDown (wxMouseEvent& event)
{
    wxString data;
    wxTextDataObject dragData(data);

    // Change the Choicebook to correct page
    wxCommandEvent eventEffectChanged(EVT_SELECTED_EFFECT_CHANGED);
    eventEffectChanged.SetInt(mEffectIndex);
    // We are only changing choicebook not populating effect panel with settings
    eventEffectChanged.SetClientData(nullptr);
    wxPostEvent(GetParent(), eventEffectChanged);


    wxBitmap* bmDrag=new wxBitmap(mDragIconBuffer);
    wxCursor dragCursor(bmDrag->ConvertToImage());

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
