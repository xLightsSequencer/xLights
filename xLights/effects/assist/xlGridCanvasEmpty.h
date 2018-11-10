#ifndef XLGRIDCANVASEMPTY_H
#define XLGRIDCANVASEMPTY_H

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

#endif // XLGRIDCANVASEMPTY_H
