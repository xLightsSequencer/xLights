#ifndef EFFECTDROPTARGET_H
#define EFFECTDROPTARGET_H
#include "wx/wx.h"
#include <wx/dnd.h>


class EffectDropTarget: public wxTextDropTarget
{
    public:
        EffectDropTarget(wxWindow* parent,bool IsEffectsGrid);
        virtual ~EffectDropTarget();
        virtual bool OnDrop(wxCoord x, wxCoord y);
        virtual bool OnDropText(wxCoord x, wxCoord y,const wxString &data );
        virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult);
    protected:
    private:
        bool mIsEffectsGrid;
        wxWindow* mParent;
};

#endif // EFFECTDROPTARGET_H
