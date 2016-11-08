#include "EffectDropTarget.h"
#include <wx/dnd.h>
#include "wx/wx.h"
#include "EffectsGrid.h"



EffectDropTarget::EffectDropTarget(EffectsGrid* parent)
{
    //ctor
    mParent = parent;
}

EffectDropTarget::~EffectDropTarget()
{
    //dtor
}

bool EffectDropTarget::OnDrop(wxCoord x, wxCoord y)
{
    mParent->OnDrop(x,y);
    return true;
}

bool EffectDropTarget::OnDropText(wxCoord x, wxCoord y,const wxString &data )
{
    return true;
}

wxDragResult EffectDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult)
{
    if(mParent->DragOver(x,y)) {
        return wxDragCopy;
    }
    return wxDragNone;
}
