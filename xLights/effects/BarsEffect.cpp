/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#define MAX_ISPC_BARS_COLORS 8

#include "BarsEffect.h"

#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "../render/Effect.h"

#include "../../include/bars-16.xpm"
#include "../../include/bars-24.xpm"
#include "../../include/bars-32.xpm"
#include "../../include/bars-48.xpm"
#include "../../include/bars-64.xpm"

#include "ispc/BarsFunctions.ispc.h"
#include "Parallel.h"

BarsEffect::BarsEffect(int i) :
    RenderableEffect(i, "Bars", bars_16, bars_24, bars_32, bars_48, bars_64)
{
    // ctor
}

BarsEffect::~BarsEffect()
{
    // dtor
}

static inline int GetDirection(const std::string& DirectionString)
{
    if ("up" == DirectionString) {
        return 0;
    } else if ("down" == DirectionString) {
        return 1;
    } else if ("expand" == DirectionString) {
        return 2;
    } else if ("compress" == DirectionString) {
        return 3;
    } else if ("Left" == DirectionString) {
        return 4;
    } else if ("Right" == DirectionString) {
        return 5;
    } else if ("H-expand" == DirectionString) {
        return 6;
    } else if ("H-compress" == DirectionString) {
        return 7;
    } else if ("Alternate Up" == DirectionString) {
        return 8;
    } else if ("Alternate Down" == DirectionString) {
        return 9;
    } else if ("Alternate Left" == DirectionString) {
        return 10;
    } else if ("Alternate Right" == DirectionString) {
        return 11;
    } else if ("Custom Horz" == DirectionString) {
        return 12;
    } else if ("Custom Vert" == DirectionString) {
        return 13;
    }
    return 0;
}

void BarsEffect::GetSpatialColor(xlColor& color, size_t colorIndex, float x, float y, RenderBuffer& buffer, bool gradient, const xlColor& highlightColour, bool highlight, bool show3d, int BarHt, int n, float pct, int color2Index) {
    if (buffer.palette.IsSpatial(colorIndex)) {
        buffer.palette.GetSpatialColor(colorIndex, x, y, color);
        xlColor color2;
        buffer.palette.GetSpatialColor(color2Index, x, y, color2);

        if (buffer.allowAlpha) {
            if (gradient)
                buffer.Get2ColorBlend(color, color2, pct);
            if (highlight && n % BarHt == 0)
                color = highlightColour;
            if (show3d)
                color.alpha = 255.0 * double(BarHt - n % BarHt - 1) / BarHt;
        } else {
            if (gradient)
                buffer.Get2ColorBlend(color, color2, pct);
            HSVValue hsv = color.asHSV();
            if (highlight && n % BarHt == 0)
                hsv.saturation = 0.0;
            if (show3d)
                hsv.value *= double(BarHt - n % BarHt - 1) / BarHt;
            color = hsv;
        }
    }
}

void BarsEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    do {
        // ISPC-accelerated path. Custom Horz/Vert need spatial data — skip ISPC.
        const std::string& ispcDirStr = SettingsMap["CHOICE_Bars_Direction"];
        int ispcDir = -1;
        if      (ispcDirStr == "up")              ispcDir = 0;
        else if (ispcDirStr == "down")            ispcDir = 1;
        else if (ispcDirStr == "expand")          ispcDir = 2;
        else if (ispcDirStr == "compress")        ispcDir = 3;
        else if (ispcDirStr == "Left")            ispcDir = 4;
        else if (ispcDirStr == "Right")           ispcDir = 5;
        else if (ispcDirStr == "H-expand")        ispcDir = 6;
        else if (ispcDirStr == "H-compress")      ispcDir = 7;
        else if (ispcDirStr == "Alternate Up")    ispcDir = 8;
        else if (ispcDirStr == "Alternate Down")  ispcDir = 9;
        else if (ispcDirStr == "Alternate Left")  ispcDir = 10;
        else if (ispcDirStr == "Alternate Right") ispcDir = 11;
        else break; // Custom Horz / Custom Vert — fall through to CPU

        float ispcOffset = buffer.GetEffectTimeIntervalPosition();
        int ispcPaletteRepeat = GetValueCurveInt("Bars_BarCount", 1, SettingsMap, ispcOffset, BARCOUNT_MIN, BARCOUNT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
        double ispcCycles = GetValueCurveDouble("Bars_Cycles", 1.0, SettingsMap, ispcOffset, BARCYCLES_MIN, BARCYCLES_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 10);
        double ispcPosition = buffer.GetEffectTimeIntervalPosition(ispcCycles);
        double ispcCenter = GetValueCurveDouble("Bars_Center", 0, SettingsMap, ispcPosition, BARCENTER_MIN, BARCENTER_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

        bool ispcHighlight = SettingsMap.GetBool("CHECKBOX_Bars_Highlight", false);
        bool ispcUseFCFH   = ispcHighlight && SettingsMap.GetBool("CHECKBOX_Bars_UseFirstColorForHighlight", false);
        bool ispcShow3D    = SettingsMap.GetBool("CHECKBOX_Bars_3D", false);
        bool ispcGradient  = SettingsMap.GetBool("CHECKBOX_Bars_Gradient", false);

        size_t ispcColorcnt = buffer.GetColorCount();
        if (ispcColorcnt == 0) ispcColorcnt = 1;
        if (ispcHighlight && ispcUseFCFH) {
            if (ispcColorcnt == 1)
                ispcUseFCFH = false;
            else
                ispcColorcnt -= 1;
        }

        bool hasSpatial = false;
        for (size_t i = 0; i < ispcColorcnt && !hasSpatial; i++)
            hasSpatial = buffer.palette.IsSpatial(i);
        if (hasSpatial) break;

        if ((int)ispcColorcnt > MAX_ISPC_BARS_COLORS) break;

        int ispcBarCount = ispcPaletteRepeat * (int)ispcColorcnt;
        if (ispcBarCount < 1) ispcBarCount = 1;

        ispc::BarsData bdata;
        bdata.width      = buffer.BufferWi;
        bdata.height     = buffer.BufferHt;
        bdata.colorCount = (int)ispcColorcnt;
        bdata.highlight  = ispcHighlight ? 1 : 0;
        bdata.show3D     = ispcShow3D    ? 1 : 0;
        bdata.gradient   = ispcGradient  ? 1 : 0;
        bdata.allowAlpha = buffer.allowAlpha ? 1 : 0;
        bdata.useFirstColorForHighlight = ispcUseFCFH ? 1 : 0;

        if (ispcUseFCFH) {
            xlColor hc;
            buffer.palette.GetColor(0, hc);
            bdata.highlightColor.v[0] = hc.red;
            bdata.highlightColor.v[1] = hc.green;
            bdata.highlightColor.v[2] = hc.blue;
            bdata.highlightColor.v[3] = hc.alpha;
        } else {
            bdata.highlightColor.v[0] = 255;
            bdata.highlightColor.v[1] = 255;
            bdata.highlightColor.v[2] = 255;
            bdata.highlightColor.v[3] = 255;
        }

        int ispcColorOffset = ispcUseFCFH ? 1 : 0;
        for (int i = 0; i < (int)ispcColorcnt; i++) {
            xlColor c;
            buffer.palette.GetColor(i + ispcColorOffset, c);
            bdata.colorsAsRGBA[i].v[0] = c.red;
            bdata.colorsAsRGBA[i].v[1] = c.green;
            bdata.colorsAsRGBA[i].v[2] = c.blue;
            bdata.colorsAsRGBA[i].v[3] = c.alpha;
            HSVValue hsv = c.asHSV();
            bdata.colorsH[i] = (float)hsv.hue;
            bdata.colorsS[i] = (float)hsv.saturation;
            bdata.colorsV[i] = (float)hsv.value;
        }

        if (ispcDir < 4 || ispcDir == 8 || ispcDir == 9) {
            int barHt = (int)std::ceil((float)buffer.BufferHt / (float)ispcBarCount);
            if (barHt < 1) barHt = 1;
            int blockHt = (int)ispcColorcnt * barHt;
            if (blockHt < 1) blockHt = 1;
            int f_offset = (int)(ispcPosition * blockHt);
            if (ispcDir == 8 || ispcDir == 9)
                f_offset = (int)(floor(ispcPosition * ispcBarCount) * barHt);
            int mappedDir = (ispcDir > 4) ? ispcDir - 8 : ispcDir;
            bdata.direction = mappedDir;
            bdata.barSize   = barHt;
            bdata.blockSize = blockHt;
            bdata.f_offset  = f_offset;
            bdata.newCenter = buffer.BufferHt * (100 + (int)ispcCenter) / 200;
        } else {
            int barWi = (int)std::ceil((float)buffer.BufferWi / (float)ispcBarCount);
            if (barWi < 1) barWi = 1;
            int blockWi = (int)ispcColorcnt * barWi;
            if (blockWi < 1) blockWi = 1;
            int f_offset = (int)(ispcPosition * blockWi);
            if (ispcDir > 9)
                f_offset = (int)(floor(ispcPosition * ispcBarCount) * barWi);
            int mappedDir = (ispcDir > 9) ? ispcDir - 6 : ispcDir;
            bdata.direction = mappedDir;
            bdata.barSize   = barWi;
            bdata.blockSize = blockWi;
            bdata.f_offset  = f_offset;
            bdata.newCenter = buffer.BufferWi * (100 + (int)ispcCenter) / 200;
        }

        int max = buffer.BufferWi * buffer.BufferHt;
        constexpr int bfBlockSize = 4096;
        int blocks = max / bfBlockSize + 1;
        parallel_for(0, blocks, [&bdata, &buffer, max](int y) {
            int start = y * bfBlockSize;
            int end = start + bfBlockSize;
            if (end > max) end = max;
            ispc::BarsEffectISPC(&bdata, start, end, (ispc::uint8_t4*)buffer.GetPixels());
        });
        return;
    } while (false);

    float offset = buffer.GetEffectTimeIntervalPosition();
    int paletteRepeat = GetValueCurveInt("Bars_BarCount", 1, SettingsMap, offset, BARCOUNT_MIN, BARCOUNT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    double cycles = GetValueCurveDouble("Bars_Cycles", 1.0, SettingsMap, offset, BARCYCLES_MIN, BARCYCLES_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 10);
    double position = buffer.GetEffectTimeIntervalPosition(cycles);
    double center = GetValueCurveDouble("Bars_Center", 0, SettingsMap, position, BARCENTER_MIN, BARCENTER_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int direction = GetDirection(SettingsMap["CHOICE_Bars_Direction"]);
    bool highlight = SettingsMap.GetBool("CHECKBOX_Bars_Highlight", false);
    bool useFirstColorForHighlight = highlight && SettingsMap.GetBool("CHECKBOX_Bars_UseFirstColorForHighlight", false);
    bool show3D = SettingsMap.GetBool("CHECKBOX_Bars_3D", false);
    bool gradient = SettingsMap.GetBool("CHECKBOX_Bars_Gradient", false);
    xlColor highlightColor;
    
    size_t colorcnt = buffer.GetColorCount();
    if (colorcnt == 0) {
        colorcnt = 1;
    }

    if (highlight && useFirstColorForHighlight) {
        if (colorcnt == 1)
        {
            useFirstColorForHighlight = false;
        }
        else
        {
            colorcnt -= 1;
        }
    }

    int barCount = paletteRepeat * colorcnt;

    if (barCount < 1) {
        barCount = 1;
    }

    xlColor color;


    if (direction < 4 || direction == 8 || direction == 9) {
        int barHt = (int)std::ceil((float)buffer.BufferHt / (float)barCount);
        if (barHt < 1)
            barHt = 1;
        int newCenter = buffer.BufferHt * (100 + center) / 200;
        int blockHt = colorcnt * barHt;
        if (blockHt < 1)
            blockHt = 1;

        int f_offset = position * blockHt;
        if (direction == 8 || direction == 9) {
            f_offset = floor(position * barCount) * barHt;
        }
        direction = direction > 4 ? direction - 8 : direction;

        for (int y = -2 * buffer.BufferHt; y < 2 * buffer.BufferHt; ++y) {

            int n = buffer.BufferHt + y + f_offset;
            int colorIdx = std::abs(n % blockHt) / barHt;
            if (useFirstColorForHighlight) {
                colorIdx += 1;
            }
            int color2 = (colorIdx + 1) % colorcnt;
            double pct = (double)std::abs(n % barHt) / (double)barHt;

            if (useFirstColorForHighlight) {
                buffer.palette.GetColor(0, highlightColor);
            } else {
                highlightColor = xlWHITE;
            }


            if (buffer.allowAlpha) {
                buffer.palette.GetColor(colorIdx, color);
                if (gradient)
                    buffer.Get2ColorBlend(colorIdx, color2, pct, color);
                if (highlight && n % barHt == 0)
                    color = highlightColor;
                if (show3D) {
                    int numerator = barHt - std::abs(n % barHt) - 1;
                    color.alpha = 255.0 * double(numerator) / double(barHt);
                }
            } else {
                buffer.palette.GetColor(colorIdx, color);
                if (gradient)
                    buffer.Get2ColorBlend(colorIdx, color2, pct, color);
                HSVValue hsv = color.asHSV();
                if (highlight && n % barHt == 0)
                    hsv.saturation = 0.0;
                if (show3D) {
                    int numerator = barHt - std::abs(n % barHt) - 1;
                    hsv.value *= double(numerator) / double(barHt);
                }
                color = hsv;
            }

            const bool isSpatialColor = buffer.palette.IsSpatial(colorIdx); // Cache the spatial color check, as it's expensive to do for every pixel

            switch (direction) {
            case 1:
                // down
                for (int x = 0; x < buffer.BufferWi; ++x) {
                    if (isSpatialColor)
                        GetSpatialColor(color, colorIdx, (float)x / (float)buffer.BufferWi, (float)(n % barHt) / (float)barHt, buffer, gradient, highlightColor, highlight, show3D, barHt, n, pct, color2);
                    buffer.SetPixel(x, y, color);
                }
                break;
            case 2:
                // expand
                if (y <= newCenter) {
                    for (int x = 0; x < buffer.BufferWi; ++x) {
                        if (isSpatialColor)
                            GetSpatialColor(color, colorIdx, (float)x / (float)buffer.BufferWi, (float)(n % barHt) / (float)barHt, buffer, gradient, highlightColor, highlight, show3D, barHt, n, pct, color2);
                        buffer.SetPixel(x, y, color);
                        buffer.SetPixel(x, newCenter + (newCenter - y), color);
                    }
                }
                break;
            case 3:
                // compress
                if (y >= newCenter) {
                    for (int x = 0; x < buffer.BufferWi; ++x) {
                        if (isSpatialColor)
                            GetSpatialColor(color, colorIdx, (float)x / (float)buffer.BufferWi, (float)(n % barHt) / (float)barHt, buffer, gradient, highlightColor, highlight, show3D, barHt, n, pct, color2);
                        buffer.SetPixel(x, y, color);
                        buffer.SetPixel(x, newCenter + (newCenter - y), color);
                    }
                }
                break;
            default:
                // up
                for (int x = 0; x < buffer.BufferWi; ++x) {
                    if (isSpatialColor)
                        GetSpatialColor(color, colorIdx, (float)x / (float)buffer.BufferWi, 1.0 - (float)(n % barHt) / (float)barHt, buffer, gradient, highlightColor, highlight, show3D, barHt, n, pct, color2);
                    buffer.SetPixel(x, buffer.BufferHt - y - 1, color);
                }
                break;
            }
        }
    } else if (direction == 12 || direction == 13) {
        int width = buffer.BufferWi;
        int height = buffer.BufferHt;
        if (direction == 13) {
            std::swap(width, height);
        }
        int BarWi = (int)std::ceil((float)width / (float)barCount);
        if (BarWi < 1)
            BarWi = 1;
        int NewCenter = (width * (100.0 + center) / 200.0 - width / 2);
        int BlockWi = colorcnt * BarWi;
        if (BlockWi < 1)
            BlockWi = 1;

        for (int x = -2 * width; x < 2 * width; ++x) {
            int n = width + x;
            int colorIdx = (n % BlockWi) / BarWi;
            if (useFirstColorForHighlight) {
                colorIdx += 1;
            }
            int color2 = (colorIdx + 1) % colorcnt;
            double pct = (double)(n % BarWi) / (double)BarWi;
            if (useFirstColorForHighlight) {
                buffer.palette.GetColor(0, highlightColor);
            } else {
                highlightColor = xlWHITE;
            }

            if (buffer.allowAlpha) {
                buffer.palette.GetColor(colorIdx, color);
                if (gradient)
                    buffer.Get2ColorBlend(colorIdx, color2, pct, color);
                if (highlight && n % BarWi == 0)
                    color = highlightColor;
                if (show3D)
                    color.alpha = 255.0 * double(BarWi - n % BarWi - 1) / BarWi;

            } else {
                buffer.palette.GetColor(colorIdx, color);
                if (gradient)
                    buffer.Get2ColorBlend(colorIdx, color2, pct, color);
                HSVValue hsv = color.asHSV();
                if (highlight && n % BarWi == 0)
                    hsv.saturation = 0.0;
                if (show3D)
                    hsv.value *= double(BarWi - n % BarWi - 1) / BarWi;
                color = hsv;
            }

            int position_x = width - x - 1 + NewCenter;
            for (int y = 0; y < height; ++y) {
                GetSpatialColor(color, colorIdx, 1.0 - pct, (float)y / (float)height, buffer, gradient, highlightColor, highlight, show3D, BarWi, n, pct, color2);
                if (direction == 12) {
                    buffer.SetPixel(position_x, y, color);
                } else {
                    buffer.SetPixel(y, position_x, color);
                }
            }
        }
    } else {
        int barWi = (int)std::ceil((float)buffer.BufferWi / (float)barCount);
        if (barWi < 1)
            barWi = 1;
        int newCenter = buffer.BufferWi * (100 + center) / 200;
        int blockWi = colorcnt * barWi;
        if (blockWi < 1)
            blockWi = 1;
        int f_offset = position * blockWi;
        if (direction > 9) {
            f_offset = floor(position * barCount) * barWi;
        }

        direction = direction > 9 ? direction - 6 : direction;
        for (int x = -2 * buffer.BufferWi; x < 2 * buffer.BufferWi; ++x) {
            int n = buffer.BufferWi + x + f_offset;
            int colorIdx = (n % blockWi) / barWi;
            if (useFirstColorForHighlight) {
                colorIdx += 1;
            }
            int color2 = (colorIdx + 1) % colorcnt;
            double pct = (double)(n % barWi) / (double)barWi;
            if (useFirstColorForHighlight) {
                buffer.palette.GetColor(0, highlightColor);
            } else {
                highlightColor = xlWHITE;
            }
            if (buffer.allowAlpha) {
                buffer.palette.GetColor(colorIdx, color);
                if (gradient)
                    buffer.Get2ColorBlend(colorIdx, color2, pct, color);
                if (highlight && n % barWi == 0)
                    color = highlightColor;
                if (show3D)
                    color.alpha = 255.0 * double(barWi - n % barWi - 1) / (double)barWi;

            } else {
                buffer.palette.GetColor(colorIdx, color);
                if (gradient)
                    buffer.Get2ColorBlend(colorIdx, color2, pct, color);
                HSVValue hsv = color.asHSV();
                if (highlight && n % barWi == 0)
                    hsv = highlightColor.asHSV();
                if (show3D)
                    hsv.value *= double(barWi - n % barWi - 1) / barWi;
                color = hsv;
            }

            const bool isSpatialColor = buffer.palette.IsSpatial(colorIdx); // Cache the spatial color check, as it's expensive to do for every pixel

            switch (direction) {
            case 5:
                // right
                for (int y = 0; y < buffer.BufferHt; ++y) {
                    if (isSpatialColor)
                        GetSpatialColor(color, colorIdx, 1.0 - pct, (double)y / (double)buffer.BufferHt, buffer, gradient, highlightColor, highlight, show3D, barWi, n, pct, color2);
                    buffer.SetPixel(buffer.BufferWi - x - 1, y, color);
                }
                break;
            case 6:
                // H-expand
                if (x <= newCenter) {
                    for (int y = 0; y < buffer.BufferHt; ++y) {
                        if (isSpatialColor)
                            GetSpatialColor(color, colorIdx, pct, (double)y / (double)buffer.BufferHt, buffer, gradient, highlightColor, highlight, show3D, barWi, n, pct, color2);
                        buffer.SetPixel(x, y, color);
                        buffer.SetPixel(newCenter + (newCenter - x), y, color);
                    }
                }
                break;
            case 7:
                // H-compress
                if (x >= newCenter) {
                    for (int y = 0; y < buffer.BufferHt; ++y) {
                        if (isSpatialColor)
                            GetSpatialColor(color, colorIdx, pct, (double)y / (double)buffer.BufferHt, buffer, gradient, highlightColor, highlight, show3D, barWi, n, pct, color2);
                        buffer.SetPixel(x, y, color);
                        buffer.SetPixel(newCenter + (newCenter - x), y, color);
                    }
                }
                break;
            default:
                // left
                for (int y = 0; y < buffer.BufferHt; ++y) {
                    if (isSpatialColor)
                        GetSpatialColor(color, colorIdx, pct, (double)y / (double)buffer.BufferHt, buffer, gradient, highlightColor, highlight, show3D, barWi, n, pct, color2);
                    buffer.SetPixel(x, y, color);
                }
                break;
            }
        }
    }
}
