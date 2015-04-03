#include "EffectDropTarget.h"
#include <wx/dnd.h>
#include "wx/wx.h"
#include "EffectsGrid.h"



EffectDropTarget::EffectDropTarget(wxWindow* parent,bool IsEffectsGrid)
{
    //ctor
    mIsEffectsGrid = IsEffectsGrid;
    mParent = parent;
}

EffectDropTarget::~EffectDropTarget()
{
    //dtor
}

bool EffectDropTarget::OnDrop(wxCoord x, wxCoord y)
{
    ((EffectsGrid*)(mParent))->OnDrop(x,y);
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
        if( ((EffectsGrid*)(mParent))->DragOver(x,y) )
            return wxDragCopy;
    }
    return wxDragNone;
}
