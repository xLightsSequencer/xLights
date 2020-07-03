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
    
    
        virtual void InitializeGLContext();
        void render(wxPaintEvent& event);
    
        void SetDimmingCurve(DimmingCurve *c, int channel);
    
    
        virtual bool UsesVertexTextureAccumulator() {return false;}
        virtual bool UsesVertexColorAccumulator() {return false;}
        virtual bool UsesVertexAccumulator() {return true;}
        virtual bool UsesAddVertex() {return false;}

    protected:
    private:
        DimmingCurve *curve;
        int channel;
    
    
    DECLARE_EVENT_TABLE()
};
