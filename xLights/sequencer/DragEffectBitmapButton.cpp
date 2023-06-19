/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "DragEffectBitmapButton.h"
#include <wx/dnd.h>
#include "../BitmapCache.h"
#include "../xLightsMain.h"
#include "../effects/RenderableEffect.h"
#include "../RenderCommandEvent.h"
#include "../UtilFunctions.h"
#include "MainSequencer.h"
#include <log4cpp/Category.hh>

class EffectButtonDropSource : public wxDropSource
{
    xLightsFrame* _frame = nullptr;

public:
#ifdef __linux__
    EffectButtonDropSource(xLightsFrame* frame, wxWindow* window, const wxIcon& cursorCopy = wxNullIcon, const wxIcon& cursorMove = wxNullIcon, const wxIcon& cursorStop = wxNullIcon) :
        wxDropSource(window,
                     cursorCopy,
                     cursorMove,
                     cursorStop)
    {
        _frame = frame;
    }
#else
    EffectButtonDropSource(xLightsFrame* frame, wxWindow* window, const wxCursor& cursorCopy = wxNullCursor, const wxCursor& cursorMove = wxNullCursor, const wxCursor& cursorStop = wxNullCursor) :
        wxDropSource(window,
                     cursorCopy,
                     cursorMove,
                     cursorStop)
    {
        _frame = frame;
    }
#endif

    virtual bool GiveFeedback(wxDragResult effect) override
    {
        EffectsGrid* eg = _frame->GetMainSequencer()->PanelEffectGrid;
        if (eg->CanDropEffect()) {
            eg->UpdateMousePosition(eg->GetDropStartMS());
            return wxDropSource::GiveFeedback(effect);
        } else {
            eg->UpdateMousePosition(-1);
#ifdef __WXMSW__
            eg->SetCursor(wxCURSOR_NO_ENTRY);
            return true;
#else
            return wxDropSource::GiveFeedback(effect);
#endif
        }
    }
};

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
    SetEffect(mEffect, ToDIP(minW));
    wxBitmapButton::DoSetSizeHints(minW,
                                   minH,
                                   maxW,
                                   maxH,
                                   incW, incH);
}
void DragEffectBitmapButton::SetEffect(RenderableEffect *eff, int sz)
{
    mEffect = eff;
    if (eff != nullptr) {
        SetBitmap(eff->GetEffectIcon(sz));
        SetToolTip(eff->ToolTip());
    }
}

void DragEffectBitmapButton::OnMouseLeftDown(wxMouseEvent& event)
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
    dragCursor.CopyFromBitmap(mEffect->GetEffectIcon().GetBitmap(wxSize(16, 16)));
#else
    wxCursor dragCursor(mEffect->GetEffectIcon().GetBitmapFor(this).ConvertToImage());
#endif

    EffectButtonDropSource dragSource((xLightsFrame*)wxTheApp->GetMainTopWindow(), this, dragCursor, dragCursor, dragCursor);

    dragSource.SetData(dragData);
    dragSource.DoDragDrop(wxDragMove);
}

void DragEffectBitmapButton::SetBitmap(const wxBitmapBundle &bpm)
{
    wxBitmapButton::SetBitmap(bpm);
}
