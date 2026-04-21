/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceVideoPreview.h"

#include <wx/dcclient.h>
#include <wx/panel.h>

BEGIN_EVENT_TABLE( SequenceVideoPreview, GRAPHICS_BASE_CLASS )
EVT_PAINT( SequenceVideoPreview::paint )
END_EVENT_TABLE()


SequenceVideoPreview::SequenceVideoPreview(wxPanel *parent)
    : GRAPHICS_BASE_CLASS(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "VideoPreview"), _texture(nullptr), _texWidth(0), _texHeight(0)
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

void SequenceVideoPreview::Render( VideoFrame *frame )
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
    switch (frame->format) {
        case VideoPixelFormat::BGRA:
            _texture->UpdateData(frame->data, true, true);
            break;
        case VideoPixelFormat::RGBA:
            _texture->UpdateData(frame->data, false, true);
            break;
        case VideoPixelFormat::RGB24:
            _texture->UpdateData(frame->data, false, false);
            break;
        case VideoPixelFormat::BGR24:
            _texture->UpdateData(frame->data, true, false);
            break;
        case VideoPixelFormat::PlatformNative:
            _texture->UpdateData(ctx, frame->nativeHandle, "vt");
            break;
    }
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
