#ifndef EFFECTDROPTARGET_H
#define EFFECTDROPTARGET_H
#include "wx/wx.h"
#include <wx/dnd.h>

class EffectsGrid;

class EffectDropTarget: public wxTextDropTarget
{
    public:
        EffectDropTarget(EffectsGrid* parent);
        virtual ~EffectDropTarget();
        virtual bool OnDrop(wxCoord x, wxCoord y) override;
        virtual bool OnDropText(wxCoord x, wxCoord y,const wxString &data ) override;
        virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult) override;
        virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def) override;
    private:
        EffectsGrid* mParent;
};

#endif // EFFECTDROPTARGET_H
