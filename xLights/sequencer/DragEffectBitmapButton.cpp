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
    wxString tooltip;
    SetBitmap(xLightsFrame::GetIconBuffer(index, tooltip));
    SetToolTip(tooltip);
    mEffectIndex = index;
}

void DragEffectBitmapButton::OnMouseLeftDown (wxMouseEvent& event)
{
    wxString data;
    wxTextDataObject dragData(data);

    //unselect any running effect to make sure the notebook change won't effect it
    wxCommandEvent unselectEffect(EVT_UNSELECTED_EFFECT);
    wxPostEvent(GetParent(), unselectEffect);

    // Change the Choicebook to correct page
    wxCommandEvent eventEffectChanged(EVT_SELECTED_EFFECT_CHANGED);
    eventEffectChanged.SetInt(mEffectIndex);
    // We are only changing choicebook not populating effect panel with settings
    eventEffectChanged.SetClientData(nullptr);
    wxPostEvent(GetParent(), eventEffectChanged);


    wxBitmap* bmDrag=new wxBitmap(mDragIconBuffer);
    wxCursor dragCursor(bmDrag->ConvertToImage());

#ifndef __linux__
	// FIXME - This is failing compile on Linux/GTK, not sure why yet
	// http://docs.wxwidgets.org/3.0.2/classwx_drop_source.html
    wxDropSource dragSource(this,dragCursor,dragCursor,dragCursor );

    dragSource.SetData( dragData );
    wxDragResult result = dragSource.DoDragDrop( wxDragMove );
    int i = 0;
#endif
}

void DragEffectBitmapButton::SetBitmap(const char** xpm)
{
    mDragIconBuffer = xpm;
    wxBitmap bm(xpm);
    SetBitmapLabel(bm);
}
