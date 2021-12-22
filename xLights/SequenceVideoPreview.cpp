/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceVideoPreview.h"

#include <wx/dcclient.h>
#include <wx/panel.h>

#include <libavutil/frame.h>

BEGIN_EVENT_TABLE( SequenceVideoPreview, GRAPHICS_BASE_CLASS )
EVT_PAINT( SequenceVideoPreview::paint )
END_EVENT_TABLE()


SequenceVideoPreview::SequenceVideoPreview(wxPanel *parent)
    : GRAPHICS_BASE_CLASS(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "ViewPreview"), _texture(nullptr), _texWidth(0), _texHeight(0)
{

}

SequenceVideoPreview::~SequenceVideoPreview()
{
   deleteTexture();
}
void SequenceVideoPreview::paint( wxPaintEvent& evt )
{
    wxPaintDC( this );
    if(!IsShownOnScreen()) return;
    if(!mIsInitialized) {
        PrepareCanvas();
    }

    xlGraphicsContext *ctx = PrepareContextForDrawing();
    if (ctx == nullptr) {
        return;
    }
    ctx->SetViewport(0, 0, mWindowWidth, mWindowHeight);

    FinishDrawing(ctx);
}

void SequenceVideoPreview::Render( AVFrame *frame )
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

    if (_texture == nullptr || frame->width != _texWidth || frame->height != _texHeight) {
        reinitTexture(ctx, frame->width, frame->height);
    }

    // Upload video frame to texture
#ifndef __WXMSW__
    if (frame->format == AV_PIX_FMT_BGRA) {
        _texture->UpdateData(frame->data[0] , true, true);
    } else
#endif
    if (frame->format == AV_PIX_FMT_RGBA) {
        _texture->UpdateData(frame->data[0] , false, true);
    } else if (frame->format == AV_PIX_FMT_RGB24) {
        _texture->UpdateData(frame->data[0] , false, false);
    } else if (frame->format == AV_PIX_FMT_VIDEOTOOLBOX) {
        _texture->UpdateData(ctx, frame->data[3], "vt");
    }
#ifndef  __WXMSW__
    else if (frame->format == AV_PIX_FMT_BGR24) {
        _texture->UpdateData(frame->data[0] , true, false);
    }
#endif
    ctx->drawTexture(_texture, 0, 0, mWindowWidth, mWindowHeight );
    FinishDrawing(ctx);
}

void SequenceVideoPreview::Clear()
{
    if (_texture == nullptr) {
        // nothing has been displayed yet, delay allocating resources and such
        return;
    }
    xlGraphicsContext *ctx = PrepareContextForDrawing();
    if (ctx == nullptr) {
        return;
    }
    ctx->SetViewport(0, 0, mWindowWidth, mWindowHeight);
    deleteTexture();
    FinishDrawing(ctx);
}

void SequenceVideoPreview::reinitTexture(xlGraphicsContext *ctx, int width, int height )
{
    deleteTexture();

    _texWidth = width;
    _texHeight = height;

    _texture = ctx->createTexture(width, height, true, true);
    _texture->SetName("VideoPreview");
}

void SequenceVideoPreview::deleteTexture()
{
    if (_texture) {
        delete _texture;
        _texture = nullptr;
    }
}
