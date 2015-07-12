#ifndef XLGRIDCANVASEMPTY_H
#define XLGRIDCANVASEMPTY_H

#include "wx/wx.h"
#include "xlGridCanvas.h"
#include "Effect.h"

class xlGridCanvasEmpty : public xlGridCanvas
{
    public:

        xlGridCanvasEmpty(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                          const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);
        virtual ~xlGridCanvasEmpty();

        virtual void SetEffect(Effect* effect_);
        virtual void ForceRefresh();

    protected:
        virtual void InitializeGLCanvas();

    private:

        void render(wxPaintEvent& event);
        void DrawEmptyEffect();

        DECLARE_EVENT_TABLE()
};

#endif // XLGRIDCANVASEMPTY_H
