#include "DragEffectBitmapButton.h"
#include <wx/dnd.h>
#include "../BitmapCache.h"
#include "../xLightsMain.h"
#include "../effects/RenderableEffect.h"
#include "../RenderCommandEvent.h"
#include "../UtilFunctions.h"

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
    SetEffect(mEffect, UnScaleWithSystemDPI(minW));
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
        const wxBitmap &bbmp = eff->GetEffectIcon(sz);

        int ns = GetContentScaleFactor() * sz;
        if (ns != bbmp.GetScaledWidth()) {
            const wxBitmap &bbmp2 = eff->GetEffectIcon(ScaleWithSystemDPI(sz), true);
            if (ns != bbmp2.GetScaledWidth()) {
                wxImage img = eff->GetEffectIcon(64, true).ConvertToImage();
                wxImage scaled = img.Scale(ns, ns, wxIMAGE_QUALITY_HIGH);
                SetBitmap(wxBitmap(scaled));
            } else {
                SetBitmap(bbmp2);
            }
        } else {
            SetBitmap(bbmp);
        }
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
    SelectedEffectChangedEvent eventEffectChanged(nullptr, false, true);
    // We are only changing choicebook not populating effect panel with settings
    eventEffectChanged.SetInt(id);
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
