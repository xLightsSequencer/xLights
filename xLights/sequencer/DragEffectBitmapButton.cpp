#include "DragEffectBitmapButton.h"
#include <wx/dnd.h>
#include "../BitmapCache.h"
#include "../xLightsMain.h"
#include "../effects/RenderableEffect.h"

DragEffectBitmapButton::DragEffectBitmapButton (wxWindow *parent, wxWindowID id, const wxBitmap &bitmap, const wxPoint &pos,
                                const wxSize &size, long style, const wxValidator &validator,
                                const wxString &name)
    : wxBitmapButton (parent, id, bitmap,pos,size,style,validator,name), mDragIconBuffer(&bitmap)
{
    Connect (wxEVT_LEFT_DOWN, wxMouseEventHandler (DragEffectBitmapButton::OnMouseLeftDown));
    mEffect = nullptr;
}

DragEffectBitmapButton::~DragEffectBitmapButton()
{
}
void DragEffectBitmapButton::DoSetSizeHints(int minW, int minH,
                                            int maxW, int maxH,
                                            int incW, int incH ) {
    wxBitmapButton::DoSetSizeHints(minW, minH, maxW, maxH, incW, incH);
    SetEffect(mEffect, minW);
}
void DragEffectBitmapButton::SetEffect(RenderableEffect *eff, int sz)
{
    mEffect = eff;
    if (eff != nullptr) {
        SetBitmap(eff->GetEffectIcon(sz));
        SetToolTip(eff->Name());
    }
}

void DragEffectBitmapButton::OnMouseLeftDown (wxMouseEvent& event)
{
    wxString data;
    wxTextDataObject dragData(data);

    //unselect any running effect to make sure the notebook change won't effect it
    wxCommandEvent unselectEffect(EVT_UNSELECTED_EFFECT);
    wxPostEvent(GetParent(), unselectEffect);

    int id = 0;
    if (mEffect != nullptr) {
        id = mEffect->GetId();
    }
    // Change the Choicebook to correct page
    wxCommandEvent eventEffectChanged(EVT_SELECTED_EFFECT_CHANGED);
    eventEffectChanged.SetInt(id);
    // We are only changing choicebook not populating effect panel with settings
    eventEffectChanged.SetClientData(nullptr);
    wxPostEvent(GetParent(), eventEffectChanged);

#ifdef __linux__
    wxIcon dragCursor;
    dragCursor.CopyFromBitmap(*mDragIconBuffer);
#else
    wxCursor dragCursor(mDragIconBuffer->ConvertToImage());
#endif

    wxDropSource dragSource(this,dragCursor,dragCursor,dragCursor );

    dragSource.SetData( dragData );
    dragSource.DoDragDrop( wxDragMove );
}

void DragEffectBitmapButton::SetBitmap(const wxBitmap &bpm)
{
    mDragIconBuffer = &bpm;
    wxBitmapButton::SetBitmap(bpm);
    wxBitmapButton::SetBitmapDisabled(bpm.ConvertToDisabled());
}
