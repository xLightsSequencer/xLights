/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <cstdint>
#include <vector>

// Use prefixed nanosvg types to avoid conflicts with wxWidgets' bundled version
#define NSVGimage xl_NSVGimage
#define NSVGrasterizer xl_NSVGrasterizer
struct NSVGimage;
struct NSVGrasterizer;
class RenderBuffer;
class xlColor;

// Rasterize an SVG into a RenderBuffer using nanosvg.
// `rast` and `rasterBuf` are caller-owned (typically cached between frames).
// `scaleBase` is the pre-computed base scale factor for the SVG image.
// Fills the buffer red if `image` is null.
void RasterizeSVGToBuffer(NSVGrasterizer* rast, NSVGimage* image,
                          std::vector<uint8_t>& rasterBuf,
                          RenderBuffer& buffer,
                          int xc, int yc, double radius,
                          float scaleBase, const xlColor& color);
