/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// NanoSVG implementation is provided by wxWidgets (wxmsw33ud_core.lib via bmpsvg.obj)
// This file provides the wrapper function RasterizeSVGToBuffer for use in effects.
// Do NOT define NANOSVGRAST_IMPLEMENTATION here as it would conflict with wxWidgets.

#include "nanosvg/src/nanosvg.h"
#include "nanosvg/src/nanosvgrast.h"

#include "nanosvgrast_impl.h"
#include "../render/RenderBuffer.h"
#include "../Color.h"

#include <cstring>

void RasterizeSVGToBuffer(NSVGrasterizer* rast, NSVGimage* image,
                          std::vector<uint8_t>& rasterBuf,
                          RenderBuffer& buffer,
                          int xc, int yc, double radius,
                          float scaleBase, const xlColor& color)
{
    if (image == nullptr) {
        for (int x = 0; x < buffer.BufferWi; ++x) {
            for (int y = 0; y < buffer.BufferHt; ++y) {
                buffer.SetPixel(x, y, xlRED);
            }
        }
        return;
    }

    if (rast == nullptr) return;

    // Compute scale and translation so the SVG center maps to (xc, yc) in buffer coords.
    // nsvgRasterize uses screen coords (y=0 at top); buffer uses y=0 at bottom, so y is flipped.
    float scale = scaleBase * (float)radius * 10.0f;
    float tx = (float)xc - (image->width * 0.5f) * scale;
    float ty = (float)(buffer.BufferHt - 1 - yc) - (image->height * 0.5f) * scale;

    size_t bufSize = (size_t)buffer.BufferWi * buffer.BufferHt * 4;
    rasterBuf.resize(bufSize);
    std::memset(rasterBuf.data(), 0, bufSize);
    nsvgRasterize(rast, image, tx, ty, scale,
                  rasterBuf.data(), buffer.BufferWi, buffer.BufferHt, buffer.BufferWi * 4);

    // Composite into RenderBuffer with y-flip.
    // nsvgRasterize outputs premultiplied-alpha RGBA — only scale by color.alpha.
    for (int y = 0; y < buffer.BufferHt; ++y) {
        int srcY = buffer.BufferHt - 1 - y;  // flip: raster top → buffer bottom
        const uint8_t* row = &rasterBuf[srcY * buffer.BufferWi * 4];
        for (int x = 0; x < buffer.BufferWi; ++x) {
            const uint8_t* p = &row[x * 4];
            if (p[3] > 0) {
                uint8_t a = (uint8_t)((int)p[3] * color.alpha / 255);
                xlColor src((int)p[0] * color.alpha / 255,
                            (int)p[1] * color.alpha / 255,
                            (int)p[2] * color.alpha / 255, a);
                buffer.SetPixel(x, y, src, false, true);
            }
        }
    }
}