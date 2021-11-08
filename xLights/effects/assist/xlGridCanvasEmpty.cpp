/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "xlGridCanvasEmpty.h"

BEGIN_EVENT_TABLE(xlGridCanvasEmpty, xlGridCanvas)
EVT_PAINT(xlGridCanvasEmpty::render)
END_EVENT_TABLE()

xlGridCanvasEmpty::xlGridCanvasEmpty(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,long style, const wxString &name)
    : xlGridCanvas(parent, id, pos, size, style, name)
{
    mRows = 10;
    mColumns = 10;
}

xlGridCanvasEmpty::~xlGridCanvasEmpty()
{
}

void xlGridCanvasEmpty::SetEffect(Effect* effect_)
{
    mEffect = effect_;
}

void xlGridCanvasEmpty::render( wxPaintEvent& event )
{
    if(!IsShownOnScreen()) return;
    if(!mIsInitialized) {
        PrepareCanvas();
    }

    xlGraphicsContext *ctx = PrepareContextForDrawing();
    if (ctx == nullptr) {
        return;
    }
    ctx->SetViewport(0, 0, mWindowWidth, mWindowHeight);

    if (mEffect != nullptr) {
        DrawBaseGrid(ctx);
        DrawEmptyEffect(ctx);
    }
    FinishDrawing(ctx);
}

void xlGridCanvasEmpty::DrawEmptyEffect(xlGraphicsContext *ctx)
{
    xlVertexAccumulator *va = ctx->createVertexAccumulator();
    va->PreAlloc(8);
    va->AddVertex(0, 0);
    va->AddVertex(mWindowWidth, mWindowHeight);
    va->AddVertex(0, mWindowHeight);
    va->AddVertex(mWindowWidth, 0);
    ctx->enableBlending();
    ctx->drawLines(va, xlRED);
    ctx->disableBlending();
    delete va;
}

