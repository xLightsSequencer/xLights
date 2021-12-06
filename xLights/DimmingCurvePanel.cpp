/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "DimmingCurvePanel.h"

BEGIN_EVENT_TABLE(DimmingCurvePanel, GRAPHICS_BASE_CLASS)
EVT_PAINT(DimmingCurvePanel::render)
END_EVENT_TABLE()

DimmingCurvePanel::DimmingCurvePanel(wxWindow* parent,
                                     wxWindowID id,
                                     const wxPoint& pos,
                                     const wxSize& size,
                                     long style,
                                     const wxString& name)
    : GRAPHICS_BASE_CLASS(parent, id, pos, size, style, "DimmingCurve-" + name), curve(nullptr), channel(0), boxVertices(nullptr), curveVertices(nullptr)
{
}

DimmingCurvePanel::~DimmingCurvePanel()
{
    if (boxVertices) {
        delete boxVertices;
    }
    if (curveVertices) {
        delete curveVertices;
    }
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

void DimmingCurvePanel::render(wxPaintEvent& event) {
    if(!IsShownOnScreen()) return;
    if(!mIsInitialized) { PrepareCanvas(); }

    wxPaintDC(this);

    xlGraphicsContext *ctx = PrepareContextForDrawing();
    if (ctx == nullptr) {
        return;
    }
    ctx->SetViewport(0, 0, mWindowWidth, mWindowHeight);


    if (mWindowResized) {
        if (boxVertices) {
            delete boxVertices;
        }
        if (curveVertices) {
            delete curveVertices;
            curveVertices = nullptr;
        }
        boxVertices = ctx->createVertexAccumulator();
        boxVertices->PreAlloc(6);
        boxVertices->AddVertex(mWindowWidth - 2, 2);
        boxVertices->AddVertex(2.1f, 2);
        boxVertices->AddVertex(2.1f, mWindowHeight - 2);
        boxVertices->AddVertex(mWindowWidth - 2, mWindowHeight - 2);
        boxVertices->AddVertex(mWindowWidth - 2, 2);
        boxVertices->AddVertex(2.1f, mWindowHeight - 2);

        boxVertices->Finalize(false);
        mWindowResized = false;
    }
    ctx->drawLineStrip(boxVertices, xlLIGHT_GREY);

    if (curve != nullptr) {
        if (curveVertices == nullptr) {
            curveVertices = ctx->createVertexAccumulator();
            curveVertices->PreAlloc(256);
            for (int x = 0; x < 255; x++) {
                float xpos = float(x) * float(mWindowWidth - 4.0) / 255.0 + 2.0;
                float ypos = float(x) * float(mWindowHeight - 4.0) / 255.0 + 2.0;
                curveVertices->AddVertex(xpos, ypos, 0);
            }
            curveVertices->Finalize(true);
        }

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
            curveVertices->SetVertex(x, xpos, ypos, 0);
        }
        curveVertices->FlushRange(0, 255);
        ctx->drawLineStrip(curveVertices, xlYELLOW);
    }
    FinishDrawing(ctx);
}
