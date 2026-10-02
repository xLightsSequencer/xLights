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

#include <algorithm>
#include <cmath>
#include <vector>

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

int BarsEffect::sBarCountDefault = 1;
int BarsEffect::sBarCountMin = 1;
int BarsEffect::sBarCountMax = 5;
double BarsEffect::sCyclesDefault = 1.0;
double BarsEffect::sCyclesMin = 0;
double BarsEffect::sCyclesMax = 300;
int BarsEffect::sCyclesDivisor = 10;
std::string BarsEffect::sDirectionDefault = "up";
double BarsEffect::sCenterDefault = 0;
double BarsEffect::sCenterMin = -100;
double BarsEffect::sCenterMax = 100;
int BarsEffect::sAngleDefault = 90;
int BarsEffect::sAngleMin = -180;
int BarsEffect::sAngleMax = 180;
bool BarsEffect::sHighlightDefault = false;
bool BarsEffect::sUseFirstColorForHighlightDefault = false;
bool BarsEffect::s3DDefault = false;
bool BarsEffect::sGradientDefault = false;

BarsEffect::~BarsEffect()
{
    // dtor
}

void BarsEffect::OnMetadataLoaded()
{
    sBarCountDefault = GetIntDefault("Bars_BarCount", sBarCountDefault);
    sBarCountMin = (int)GetMinFromMetadata("Bars_BarCount", sBarCountMin);
    sBarCountMax = (int)GetMaxFromMetadata("Bars_BarCount", sBarCountMax);
    sCyclesDefault = GetDoubleDefault("Bars_Cycles", sCyclesDefault);
    sCyclesMin = GetMinFromMetadata("Bars_Cycles", sCyclesMin);
    sCyclesMax = GetMaxFromMetadata("Bars_Cycles", sCyclesMax);
    sCyclesDivisor = GetDivisorFromMetadata("Bars_Cycles", sCyclesDivisor);
    sDirectionDefault = GetStringDefault("Bars_Direction", sDirectionDefault);
    sCenterDefault = GetDoubleDefault("Bars_Center", sCenterDefault);
    sCenterMin = GetMinFromMetadata("Bars_Center", sCenterMin);
    sCenterMax = GetMaxFromMetadata("Bars_Center", sCenterMax);
    sAngleDefault = GetIntDefault("Bars_Angle", sAngleDefault);
    sAngleMin = (int)GetMinFromMetadata("Bars_Angle", sAngleMin);
    sAngleMax = (int)GetMaxFromMetadata("Bars_Angle", sAngleMax);
    sHighlightDefault = GetBoolDefault("Bars_Highlight", sHighlightDefault);
    sUseFirstColorForHighlightDefault = GetBoolDefault("Bars_UseFirstColorForHighlight", sUseFirstColorForHighlightDefault);
    s3DDefault = GetBoolDefault("Bars_3D", s3DDefault);
    sGradientDefault = GetBoolDefault("Bars_Gradient", sGradientDefault);
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
    } else if ("Custom" == DirectionString) {
        return 14;
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
        else break; // Custom Horz / Custom Vert / Custom — fall through to CPU

        float ispcOffset = buffer.GetEffectTimeIntervalPosition();
        int ispcPaletteRepeat = GetValueCurveInt("Bars_BarCount", sBarCountDefault, SettingsMap, ispcOffset, sBarCountMin, sBarCountMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
        double ispcCycles = GetValueCurveDouble("Bars_Cycles", sCyclesDefault, SettingsMap, ispcOffset, sCyclesMin, sCyclesMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sCyclesDivisor);
        double ispcPosition = buffer.GetEffectTimeIntervalPosition(ispcCycles);
        double ispcCenter = GetValueCurveDouble("Bars_Center", sCenterDefault, SettingsMap, ispcPosition, sCenterMin, sCenterMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

        bool ispcHighlight = SettingsMap.GetBool("CHECKBOX_Bars_Highlight", sHighlightDefault);
        bool ispcUseFCFH   = ispcHighlight && SettingsMap.GetBool("CHECKBOX_Bars_UseFirstColorForHighlight", sUseFirstColorForHighlightDefault);
        bool ispcShow3D    = SettingsMap.GetBool("CHECKBOX_Bars_3D", s3DDefault);
        bool ispcGradient  = SettingsMap.GetBool("CHECKBOX_Bars_Gradient", sGradientDefault);

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

        if (buffer.dmx_buffer) {
            // DMX fixtures need the colour routed through SetPixel()
            ispc::uint8_t4 single { { 0, 0, 0, 255 } };
            ispc::BarsEffectISPC(&bdata, 0, 1, &single);
            buffer.SetPixel(0, 0, xlColor(single.v[0], single.v[1], single.v[2], single.v[3]));
            return;
        }

        // Clamp to the real allocation: GetPixelCount() can be < BufferWi*BufferHt
        // for a variable sub-buffer, and the ISPC kernel writes unguarded.
        int max = std::min<int>(buffer.GetPixelCount(), buffer.BufferWi * buffer.BufferHt);
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
    int paletteRepeat = GetValueCurveInt("Bars_BarCount", sBarCountDefault, SettingsMap, offset, sBarCountMin, sBarCountMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    double cycles = GetValueCurveDouble("Bars_Cycles", sCyclesDefault, SettingsMap, offset, sCyclesMin, sCyclesMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sCyclesDivisor);
    double position = buffer.GetEffectTimeIntervalPosition(cycles);
    double center = GetValueCurveDouble("Bars_Center", sCenterDefault, SettingsMap, position, sCenterMin, sCenterMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int direction = GetDirection(SettingsMap["CHOICE_Bars_Direction"]);
    bool highlight = SettingsMap.GetBool("CHECKBOX_Bars_Highlight", sHighlightDefault);
    bool useFirstColorForHighlight = highlight && SettingsMap.GetBool("CHECKBOX_Bars_UseFirstColorForHighlight", sUseFirstColorForHighlightDefault);
    bool show3D = SettingsMap.GetBool("CHECKBOX_Bars_3D", s3DDefault);
    bool gradient = SettingsMap.GetBool("CHECKBOX_Bars_Gradient", sGradientDefault);
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

    if (direction == 14) {
        double angle = GetValueCurveInt("Bars_Angle", sAngleDefault, SettingsMap, offset, sAngleMin, sAngleMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
        RenderCustomAngle(buffer, angle, position, colorcnt, barCount, highlight, useFirstColorForHighlight, show3D, gradient);
    } else if (direction < 4 || direction == 8 || direction == 9) {
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

            // Offset by a multiple of blockHt (not BufferHt) so the modulo phase isn't
            // shifted when BufferHt doesn't divide evenly by colorcnt (blockHt > BufferHt);
            // adding BufferHt directly caused the first row to wrap to the wrong color.
            int n = 4 * blockHt + y + f_offset;
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
            // See the note in the standard horizontal-bars branch above: offset by a
            // multiple of BlockWi (not width) to avoid shifting the modulo phase.
            int n = 4 * BlockWi + x;
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
            // Offset by a multiple of blockWi (not BufferWi) so the modulo phase isn't
            // shifted when BufferWi doesn't divide evenly by colorcnt (blockWi > BufferWi);
            // adding BufferWi directly caused the first column to wrap to the wrong color.
            int n = 4 * blockWi + x + f_offset;
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

void BarsEffect::RenderCustomAngle(RenderBuffer& buffer, double angle, double position, size_t colorcnt, int barCount, bool highlight, bool useFirstColorForHighlight, bool show3D, bool gradient)
{
    const int width = buffer.BufferWi;
    const int height = buffer.BufferHt;
    const double rad = angle * 3.14159265358979323846 / 180.0;
    // cos/sin of a right angle are a hair off 0, which would grow the extent past a whole
    // pixel count and shift every bar; snap them so the axis angles stay exact.
    double dx = std::cos(rad);
    double dy = std::sin(rad);
    if (std::abs(dx) < 1e-9)
        dx = 0.0;
    if (std::abs(dy) < 1e-9)
        dy = 0.0;

    // Distance is measured back from the corner the bars travel toward, so 0/90/180/-90
    // reproduce the Right/up/Left/down pixel phases exactly.
    const double extent = width * std::abs(dx) + height * std::abs(dy);
    int barSize = (int)std::ceil(extent / (double)barCount);
    if (barSize < 1)
        barSize = 1;
    int blockSize = colorcnt * barSize;
    if (blockSize < 1)
        blockSize = 1;
    const int f_offset = position * blockSize;
    const double maxProj = std::max(0.0, (width - 1) * dx) + std::max(0.0, (height - 1) * dy);

    xlColor highlightColor = xlWHITE;
    if (useFirstColorForHighlight) {
        buffer.palette.GetColor(0, highlightColor);
    }

    struct BarColor {
        xlColor color;
        int colorIdx;
        int color2;
        double pct;
        bool spatial;
    };
    std::vector<BarColor> lut(blockSize);
    for (int i = 0; i < blockSize; ++i) {
        BarColor& bc = lut[i];
        bc.colorIdx = i / barSize;
        if (useFirstColorForHighlight) {
            bc.colorIdx += 1;
        }
        bc.color2 = (bc.colorIdx + 1) % colorcnt;
        bc.pct = (double)(i % barSize) / (double)barSize;
        bc.spatial = buffer.palette.IsSpatial(bc.colorIdx);

        xlColor& color = bc.color;
        buffer.palette.GetColor(bc.colorIdx, color);
        if (gradient)
            buffer.Get2ColorBlend(bc.colorIdx, bc.color2, bc.pct, color);
        if (buffer.allowAlpha) {
            if (highlight && i % barSize == 0)
                color = highlightColor;
            if (show3D)
                color.alpha = 255.0 * double(barSize - i % barSize - 1) / (double)barSize;
        } else {
            HSVValue hsv = color.asHSV();
            if (highlight && i % barSize == 0)
                hsv = highlightColor.asHSV();
            if (show3D)
                hsv.value *= double(barSize - i % barSize - 1) / (double)barSize;
            color = hsv;
        }
    }

    auto renderRow = [&](int y) {
        for (int x = 0; x < width; ++x) {
            const double dist = maxProj - (x * dx + y * dy);
            const int n = 4 * blockSize + (int)std::floor(dist) + f_offset;
            const BarColor& bc = lut[n % blockSize];
            if (bc.spatial) {
                xlColor color = bc.color;
                GetSpatialColor(color, bc.colorIdx, (float)x / (float)width, (float)y / (float)height, buffer, gradient, highlightColor, highlight, show3D, barSize, n, bc.pct, bc.color2);
                buffer.SetPixel(x, y, color);
            } else {
                buffer.SetPixel(x, y, bc.color);
            }
        }
    };
    if (buffer.dmx_buffer) {
        for (int y = 0; y < height; ++y) {
            renderRow(y);
        }
    } else {
        parallel_for(0, height, renderRow);
    }
}
