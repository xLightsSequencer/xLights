#include "DragEffectBitmapButton.h"
#include <wx/dnd.h>
#include "../xLightsMain.h"

DragEffectBitmapButton::DragEffectBitmapButton (wxWindow *parent, wxWindowID id, const wxBitmap &bitmap, const wxPoint &pos,
                                const wxSize &size, long style, const wxValidator &validator,
                                const wxString &name)
    : wxBitmapButton (parent, id, bitmap,pos,size,style,validator,name), mDragIconBuffer(&bitmap)
{
    Connect (wxEVT_LEFT_DOWN, wxMouseEventHandler (DragEffectBitmapButton::OnMouseLeftDown));
    mEffectIndex = 0;
}

DragEffectBitmapButton::~DragEffectBitmapButton()
{
}
void DragEffectBitmapButton::DoSetSizeHints(int minW, int minH,
                                            int maxW, int maxH,
                                            int incW, int incH ) {
    wxBitmapButton::DoSetSizeHints(minW, minH, maxW, maxH, incW, incH);
    SetEffectIndex(mEffectIndex, minW);
}
void DragEffectBitmapButton::SetEffectIndex(int index, int sz)
{
    wxString tooltip;
    SetBitmap(xLightsFrame::GetIcon(index, tooltip, sz));
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

    wxCursor dragCursor(mDragIconBuffer->ConvertToImage());

#ifndef __linux__
	// FIXME - This is failing compile on Linux/GTK, not sure why yet
	// http://docs.wxwidgets.org/3.0.2/classwx_drop_source.html
    wxDropSource dragSource(this,dragCursor,dragCursor,dragCursor );

    dragSource.SetData( dragData );
    dragSource.DoDragDrop( wxDragMove );
#endif
}

void DragEffectBitmapButton::SetBitmap(const wxBitmap &bpm)
{
    mDragIconBuffer = &bpm;
    wxBitmapButton::SetBitmap(wxBitmap()); //resets the images
    wxBitmapButton::SetBitmap(bpm);
}
