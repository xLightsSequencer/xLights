#include "DimmingCurvePanel.h"
#include "DrawGLUtils.h"


BEGIN_EVENT_TABLE(DimmingCurvePanel, xlGLCanvas)
EVT_PAINT(DimmingCurvePanel::render)
END_EVENT_TABLE()

DimmingCurvePanel::DimmingCurvePanel(wxWindow* parent,
                                     wxWindowID id,
                                     const wxPoint& pos,
                                     const wxSize& size,
                                     long style,
                                     const wxString& name,
                                     int glFlags[])
    : xlGLCanvas(parent, id, pos, size, style, name, true), curve(nullptr), channel(0)
{
}

DimmingCurvePanel::~DimmingCurvePanel()
{
    if (curve != nullptr) {
        delete curve;
    }
}

void DimmingCurvePanel::SetDimmingCurve(DimmingCurve *c, int ch) {
    if (curve != nullptr) {
        delete curve;
    }
    curve = c;
    channel = ch;
    Refresh();
    Update();
}

void DimmingCurvePanel::InitializeGLCanvas() {
    if(!IsShownOnScreen()) return;
    SetCurrentGLContext();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Black Background
    glDisable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    mIsInitialized = true;
}
void DimmingCurvePanel::render(wxPaintEvent& event) {
    if(!mIsInitialized) { InitializeGLCanvas(); }
    if(!IsShownOnScreen()) return;
    SetCurrentGLContext();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    wxPaintDC(this);
    if( mWindowResized )
    {
        prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    }

    DrawGLUtils::DrawLine(xlLIGHT_GREY,255,2,2,mWindowWidth-2,2,0.5);
    DrawGLUtils::DrawLine(xlLIGHT_GREY,255,2,2,2,mWindowHeight-2,0.5);
    DrawGLUtils::DrawLine(xlLIGHT_GREY,255,mWindowWidth-2,2,mWindowWidth-2,mWindowHeight-2,0.5);
    DrawGLUtils::DrawLine(xlLIGHT_GREY,255,2,mWindowHeight-2,mWindowWidth-2,mWindowHeight-2,0.5);


    DrawGLUtils::DrawLine(xlLIGHT_GREY,255,mWindowWidth-2,2,2,mWindowHeight-2,0.5);

    if (curve != nullptr) {
        DrawGLUtils::PreAlloc(256);

        for (int x = 0; x < 255; x++) {
            xlColor c(x,x,x);
            curve->apply(c);

            float xpos = float(x) * float(mWindowWidth - 4.0) / 255.0 + 2.0;
            float ypos = c.Red();
            switch (channel) {
                case 0:
                    ypos = c.Red();
                    break;
                case 1:
                    ypos = c.Green();
                    break;
                case 2:
                    ypos = c.Blue();
                    break;
            }

            ypos = mWindowHeight - 2 - ypos * float(mWindowHeight - 4.0) / 255.0;

            DrawGLUtils::AddVertex(xpos, ypos, xlYELLOW);
        }
        DrawGLUtils::End(GL_LINE_STRIP);
    }
    SwapBuffers();
}
