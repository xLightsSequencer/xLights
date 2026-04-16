/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ButterflyEffect.h"

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"

#include "../../include/butterfly-16.xpm"
#include "../../include/butterfly-24.xpm"
#include "../../include/butterfly_32.xpm"
#include "../../include/butterfly-48.xpm"
#include "../../include/butterfly-64.xpm"

#include "Parallel.h"

#include "ispc/ButterflyFunctions.ispc.h"

ButterflyEffect::ButterflyEffect(int i) : RenderableEffect(i, "Butterfly", butterfly_16, butterfly_24, butterfly_32, butterfly_48, butterfly_64)
{
    //ctor
}

std::string ButterflyEffect::sColorsDefault = "Rainbow";
int ButterflyEffect::sStyleDefault = 1;
int ButterflyEffect::sChunksDefault = 1;
int ButterflyEffect::sChunksMin = 1;
int ButterflyEffect::sChunksMax = 10;
int ButterflyEffect::sSkipDefault = 2;
int ButterflyEffect::sSkipMin = 2;
int ButterflyEffect::sSkipMax = 10;
int ButterflyEffect::sSpeedDefault = 10;
int ButterflyEffect::sSpeedMin = 0;
int ButterflyEffect::sSpeedMax = 100;
std::string ButterflyEffect::sDirectionDefault = "Normal";

ButterflyEffect::~ButterflyEffect()
{
    //dtor
}

void ButterflyEffect::OnMetadataLoaded()
{
    sColorsDefault = GetStringDefault("Butterfly_Colors", sColorsDefault);
    sStyleDefault = GetIntDefault("Butterfly_Style", sStyleDefault);
    sChunksDefault = GetIntDefault("Butterfly_Chunks", sChunksDefault);
    sChunksMin = (int)GetMinFromMetadata("Butterfly_Chunks", sChunksMin);
    sChunksMax = (int)GetMaxFromMetadata("Butterfly_Chunks", sChunksMax);
    sSkipDefault = GetIntDefault("Butterfly_Skip", sSkipDefault);
    sSkipMin = (int)GetMinFromMetadata("Butterfly_Skip", sSkipMin);
    sSkipMax = (int)GetMaxFromMetadata("Butterfly_Skip", sSkipMax);
    sSpeedDefault = GetIntDefault("Butterfly_Speed", sSpeedDefault);
    sSpeedMin = (int)GetMinFromMetadata("Butterfly_Speed", sSpeedMin);
    sSpeedMax = (int)GetMaxFromMetadata("Butterfly_Speed", sSpeedMax);
    sDirectionDefault = GetStringDefault("Butterfly_Direction", sDirectionDefault);
}


/*
 01) x*y^3-y*x^3
 (02) (x^2+3*y^2)*e^(-x^2-y^2)
	(03) -x*y*e^(-x^2-y^2)
	(04) -1/(x^2+y^2)
	(05) cos(abs(x)+abs(y))
	(06) cos(abs(x)+abs(y))*(abs(x)+abs(y))
 */

static inline int GetButterflyColorScheme(const std::string &color) {
    if (color == "Rainbow") {
        return 0;
    } else if (color == "Palette") {
        return 1;
    }
    return 0;
}

void ButterflyEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer)
{
    float oset = buffer.GetEffectTimeIntervalPosition();
    const int Chunks = GetValueCurveInt("Butterfly_Chunks", sChunksDefault, SettingsMap, oset, sChunksMin, sChunksMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int Skip = GetValueCurveInt("Butterfly_Skip", sSkipDefault, SettingsMap, oset, sSkipMin, sSkipMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int butterFlySpeed = GetValueCurveInt("Butterfly_Speed", sSpeedDefault, SettingsMap, oset, sSpeedMin, sSpeedMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    const int Style = SettingsMap.GetInt("SLIDER_Butterfly_Style", sStyleDefault);
    int ColorScheme = GetButterflyColorScheme(SettingsMap.Get("CHOICE_Butterfly_Colors", sColorsDefault));
    int ButterflyDirection = SettingsMap.Get("CHOICE_Butterfly_Direction", sDirectionDefault) == "Reverse" ? 1 : 0;

    const int curState = (buffer.curPeriod - buffer.curEffStartPer) * butterFlySpeed * buffer.frameTimeInMs / 50;
    const size_t colorcnt=buffer.GetColorCount();
    const double offset = (ButterflyDirection==1 ? -1 : 1) * double(curState)/200.0;

    ispc::ButterflyData data;
    data.offset = offset;
    data.chunks = Chunks;
    data.skip = Skip;
    data.curState = curState;
    data.colorScheme = ColorScheme;
    data.width = buffer.BufferWi;
    data.height = buffer.BufferHt;
    data.numColors = colorcnt;
    for (int x = 0; x < (int)colorcnt; x++) {
        const xlColor &c = buffer.palette.GetColor(x);
        data.colors[x].v[0] = c.red;
        data.colors[x].v[1] = c.green;
        data.colors[x].v[2] = c.blue;
        data.colors[x].v[3] = c.alpha;
    }
    if (Style > 5) {
        // slightly different setup for "plasmas"
        int state = (buffer.curPeriod - buffer.curEffStartPer); // frames 0 to N
        double Speed_plasma = (Style == 10) ? (101-butterFlySpeed)*3 : (101-butterFlySpeed)*5;
        double time = (state+1.0)/Speed_plasma;
        data.plasmaTime = time;
        data.plasmaStyle = Style;
    }
    
    int max = buffer.BufferHt * buffer.BufferWi;
    constexpr int bfBlockSize = 4096;
    int blocks = max / bfBlockSize + 1;
    parallel_for(0, blocks, [&data, &buffer, max, Style](int y) {
        int start = y * bfBlockSize;
        int end = start + bfBlockSize;
        if (end > max) {
            end = max;
        }
        switch (Style) {
            case 1:
                ButterflyEffectStyle1(data, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
            case 2:
                ButterflyEffectStyle2(data, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
            case 3:
                ButterflyEffectStyle3(data, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
            case 4:
                ButterflyEffectStyle4(data, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
            case 5:
                ButterflyEffectStyle5(data, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
            default:
                ButterflyEffectPlasmaStyles(data, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
        }
    });
}

