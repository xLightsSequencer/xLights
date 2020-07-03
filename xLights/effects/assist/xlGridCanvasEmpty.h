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

#include "wx/wx.h"
#include "../../xlGridCanvas.h"
#include "../../sequencer/Effect.h"

class xlGridCanvasEmpty : public xlGridCanvas
{
    public:

        xlGridCanvasEmpty(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                          const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);
        virtual ~xlGridCanvasEmpty();

        virtual void SetEffect(Effect* effect_);
        virtual void ForceRefresh();

    
        virtual bool UsesVertexTextureAccumulator() {return false;}
        virtual bool UsesVertexColorAccumulator() {return false;}
        virtual bool UsesVertexAccumulator() {return true;}
        virtual bool UsesAddVertex() {return false;}
    protected:
        virtual void InitializeGLContext();

    private:

        void render(wxPaintEvent& event);
        void DrawEmptyEffect();

        DECLARE_EVENT_TABLE()
};
