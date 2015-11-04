#ifndef DIMMINGCURVEPANEL_H
#define DIMMINGCURVEPANEL_H

#include "wx/wx.h"
#include "xlGLCanvas.h"
#include "DimmingCurve.h"


class DimmingCurvePanel : public xlGLCanvas
{
    public:
        DimmingCurvePanel(wxWindow* parent,
                          wxWindowID id = wxID_ANY,
                          const wxPoint& pos = wxDefaultPosition,
                          const wxSize& size = wxDefaultSize,
                          long style = 0,
                          const wxString& name = wxGLCanvasName,
                          int glFlags[] = nullptr
                          );
        virtual ~DimmingCurvePanel();
    
    
        virtual void InitializeGLCanvas();
        void render(wxPaintEvent& event);
    
        void SetDimmingCurve(DimmingCurve *c, int channel);
    protected:
    private:
        DimmingCurve *curve;
        int channel;
    
    
    DECLARE_EVENT_TABLE()
};

#endif // DIMMINGCURVEPANEL_H
