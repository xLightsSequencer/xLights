#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
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
