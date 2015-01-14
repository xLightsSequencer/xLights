#include "EffectDropTarget.h"
#include <wx/dnd.h>


EffectDropTarget::EffectDropTarget(bool IsEffectsGrid)
{
    //ctor
    mIsEffectsGrid = IsEffectsGrid;
}

EffectDropTarget::~EffectDropTarget()
{
    //dtor
}

bool EffectDropTarget::OnDrop(wxCoord x, wxCoord y)
{
    return true;
}

bool EffectDropTarget::OnDropText(wxCoord x, wxCoord y,const wxString &data )
{
    if(mIsEffectsGrid)
    {
        return true;
    }
    else
    {
        return false;
    }
}

wxDragResult EffectDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult)
{
    if(mIsEffectsGrid)
    {
        return wxDragCopy;
    }
    else
    {
        return wxDragNone;
    }
}
