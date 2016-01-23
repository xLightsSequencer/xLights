#include "DragEffectBitmapButton.h"
#include <wx/dnd.h>
#include "../BitmapCache.h"
#include "../xLightsMain.h"
#include "../effects/RenderableEffect.h"

DragEffectBitmapButton::DragEffectBitmapButton (wxWindow *parent, wxWindowID id, const wxBitmap &bitmap, const wxPoint &pos,
                                const wxSize &size, long style, const wxValidator &validator,
                                const wxString &name)
    : wxBitmapButton (parent, id, bitmap,pos,size,style,validator,name)
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
    int offset = 0;
    #ifdef LINUX
        offset = 12; //Linux puts a 6 pixel border around it
    #endif // LINUX
    SetEffect(mEffect, minW);
    wxBitmapButton::DoSetSizeHints(minW + offset,
                                   minH + offset,
                                   maxW + offset,
                                   maxH + offset,
                                   incW, incH);
}
void DragEffectBitmapButton::SetEffect(RenderableEffect *eff, int sz)
{
    mEffect = eff;
    if (eff != nullptr) {
        SetBitmap(eff->GetEffectIcon(sz, GetContentScaleFactor() < 1.5));
        SetToolTip(eff->ToolTip());
    }
}

void DragEffectBitmapButton::OnMouseLeftDown (wxMouseEvent& event)
{
    if (mEffect == nullptr) {
        return;
    }
    wxString data;
    wxTextDataObject dragData(data);

    //unselect any running effect to make sure the notebook change won't effect it
    wxCommandEvent unselectEffect(EVT_UNSELECTED_EFFECT);
    wxPostEvent(GetParent(), unselectEffect);

    int id = mEffect->GetId();
    
    // Change the Choicebook to correct page
    wxCommandEvent eventEffectChanged(EVT_SELECTED_EFFECT_CHANGED);
    eventEffectChanged.SetInt(id);
    // We are only changing choicebook not populating effect panel with settings
    eventEffectChanged.SetClientData(nullptr);
    wxPostEvent(GetParent(), eventEffectChanged);

#ifdef __linux__
    wxIcon dragCursor;
    dragCursor.CopyFromBitmap(mEffect->GetEffectIcon(16, true));
#else
    wxCursor dragCursor(mEffect->GetEffectIcon(16, true).ConvertToImage());
#endif

    wxDropSource dragSource(this,dragCursor,dragCursor,dragCursor );

    dragSource.SetData( dragData );
    dragSource.DoDragDrop( wxDragMove );
}

void DragEffectBitmapButton::SetBitmap(const wxBitmap &bpm)
{
    wxBitmapButton::SetBitmap(bpm);
    wxBitmapButton::SetBitmapDisabled(bpm.ConvertToDisabled());
}
