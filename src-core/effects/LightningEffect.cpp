/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LightningEffect.h"

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"

#include "../../include/lightning-16.xpm"
#include "../../include/lightning-24.xpm"
#include "../../include/lightning-32.xpm"
#include "../../include/lightning-48.xpm"
#include "../../include/lightning-64.xpm"

#include <algorithm>

namespace
{
    constexpr int DOWN = 0;
    constexpr int UP = 1;
    constexpr int RIGHT = 2;
    constexpr int LEFT = 3;

    inline int GetLightningDirection(const std::string& dir)
    {
        if (dir == "Down") {
            return DOWN;
        } else if (dir == "Up") {
            return UP;
        } else if (dir == "Left") {
            return LEFT;
        } else if (dir == "Right") {
            return RIGHT;
        } else {
            return UP;
        }
    }
    void DrawLightningBolt(RenderBuffer& buffer, int x0, int y0, int x1, int y1, xlColor& color, int width)
    {
        xlColor color2;
        HSVValue hsv = color.asHSV();
        HSVValue hsv2 = color2.asHSV();
        //   if(x0<0 || x1<0 || y1<0 || y0<0) return;

        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = (dx > dy ? dx : -dy) / 2, e2;
        color2 = color;
        // color2.red=color2.green=color2.blue=200;
        //   int frame_startfade = 2*20; // 2 seconds full brightness
        //   int frame_fadedone = 5*20; // 3 seconds to fade o

        color = hsv;

        hsv2.value = hsv.value * 0.90;
        color2 = hsv2;
        color2 = hsv;
        for (;;) {
            buffer.DrawHLine(y0, x0 - width, x0 + width, color);
            if (x0 == x1 && y0 == y1)
                break;
            e2 = err;
            if (e2 > -dx) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dy) {
                err += dx;
                y0 += sy;
            }
        }
    }
}

int LightningEffect::sNumberBoltsDefault = 10;
int LightningEffect::sNumberBoltsMin = 1;
int LightningEffect::sNumberBoltsMax = 50;
int LightningEffect::sNumberSegmentsDefault = 5;
int LightningEffect::sNumberSegmentsMin = 1;
int LightningEffect::sNumberSegmentsMax = 20;
bool LightningEffect::sForkedLightningDefault = false;
int LightningEffect::sTopXDefault = 0;
int LightningEffect::sTopXMin = -50;
int LightningEffect::sTopXMax = 50;
int LightningEffect::sTopYDefault = 0;
int LightningEffect::sTopYMin = 0;
int LightningEffect::sTopYMax = 100;
int LightningEffect::sBotXDefault = 0;
int LightningEffect::sWidthDefault = 1;
std::string LightningEffect::sDirectionDefault = "Up";

LightningEffect::LightningEffect(int id) : RenderableEffect(id, "Lightning", lightning_16, lightning_24, lightning_32, lightning_48, lightning_64)
{
    //ctor
}

LightningEffect::~LightningEffect()
{
    //dtor
}

void LightningEffect::OnMetadataLoaded()
{
    sNumberBoltsDefault = GetIntDefault("Number_Bolts", sNumberBoltsDefault);
    sNumberBoltsMin = (int)GetMinFromMetadata("Number_Bolts", sNumberBoltsMin);
    sNumberBoltsMax = (int)GetMaxFromMetadata("Number_Bolts", sNumberBoltsMax);
    sNumberSegmentsDefault = GetIntDefault("Number_Segments", sNumberSegmentsDefault);
    sNumberSegmentsMin = (int)GetMinFromMetadata("Number_Segments", sNumberSegmentsMin);
    sNumberSegmentsMax = (int)GetMaxFromMetadata("Number_Segments", sNumberSegmentsMax);
    sForkedLightningDefault = GetBoolDefault("ForkedLightning", sForkedLightningDefault);
    sTopXDefault = GetIntDefault("Lightning_TopX", sTopXDefault);
    sTopXMin = (int)GetMinFromMetadata("Lightning_TopX", sTopXMin);
    sTopXMax = (int)GetMaxFromMetadata("Lightning_TopX", sTopXMax);
    sTopYDefault = GetIntDefault("Lightning_TopY", sTopYDefault);
    sTopYMin = (int)GetMinFromMetadata("Lightning_TopY", sTopYMin);
    sTopYMax = (int)GetMaxFromMetadata("Lightning_TopY", sTopYMax);
    sBotXDefault = GetIntDefault("Lightning_BOTX", sBotXDefault);
    sWidthDefault = GetIntDefault("Lightning_WIDTH", sWidthDefault);
    sDirectionDefault = GetStringDefault("Lightning_Direction", sDirectionDefault);
}

void LightningEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    int Number_Bolts = GetValueCurveInt("Number_Bolts", sNumberBoltsDefault, SettingsMap, oset, sNumberBoltsMin, sNumberBoltsMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int Number_Segments = GetValueCurveInt("Number_Segments", sNumberSegmentsDefault, SettingsMap, oset, sNumberSegmentsMin, sNumberSegmentsMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool ForkedLightning = SettingsMap.GetBool("CHECKBOX_ForkedLightning", sForkedLightningDefault);
    int topX = GetValueCurveInt("Lightning_TopX", sTopXDefault, SettingsMap, oset, sTopXMin, sTopXMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int topY = GetValueCurveInt("Lightning_TopY", sTopYDefault, SettingsMap, oset, sTopYMin, sTopYMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int botX = SettingsMap.GetInt("SLIDER_Lightning_BOTX", sBotXDefault);
    int width = SettingsMap.GetInt("SLIDER_Lightning_WIDTH", sWidthDefault);
    int DIRECTION = GetLightningDirection(SettingsMap["CHOICE_Lightning_Direction"]);

    Number_Bolts = std::clamp(Number_Bolts, sNumberBoltsMin, sNumberBoltsMax);
    Number_Segments = std::clamp(Number_Segments, sNumberSegmentsMin, sNumberSegmentsMax);
    width = std::clamp(width, 1, 7);

    int curState = (buffer.curPeriod - buffer.curEffStartPer);
    int xc = buffer.BufferWi / 2;

    int StepSegment = buffer.BufferHt / Number_Bolts;
    int segment = curState%Number_Bolts;
    segment *= 4;
    if (segment>Number_Bolts) segment = Number_Bolts;
    if (curState>Number_Bolts) segment = Number_Bolts;
    xlColor color;
    buffer.palette.GetColor(0, color);
    int x1 = 0;
    int y1 = 0;
    if (DIRECTION == DOWN) {
        x1 = xc + topX;
        y1 = buffer.BufferHt - topY;
    } else if(DIRECTION==UP) {
        x1 = xc + topX;
        y1 = topY; 
    }

    int xoffset = curState * botX / 10.0;
    for(int i = 0; i <= segment; i++) {
        int j = rand() + 1;
        int x2 = 0;
        int y2 = 0;
        if(DIRECTION==UP || DIRECTION==DOWN) {
            if(i % 2 == 0) { // Every even segment will alternate direction
                if (rand() % 2 == 0) // target x is to the left
                    x2 = xc + topX - (j % Number_Segments);
                else // but randomely we reverse direction, also make it a larger jag
                    x2 = xc + topX + (2 * (j % Number_Segments));
            } else { // odd segments will
                if (rand() % 2 == 0) // move to the right
                    x2 = xc + topX + (j % Number_Segments);
                else // but sometimes move 3 units to left.
                    x2 = xc + topX - (3 * (j % Number_Segments));
            }
            if (DIRECTION==DOWN)
                y2 = buffer.BufferHt - (i * StepSegment) - topY;
            else if (DIRECTION == UP)
                y2 = (i * StepSegment) + topY;
        }
        else if (DIRECTION == LEFT || DIRECTION == RIGHT) {

        }

        DrawLightningBolt(buffer, x1 + xoffset, y1, x2 + xoffset, y2, color, width);

        if (ForkedLightning) {
            if (i > (segment / 2)) {
                int x3 = 0;
                if (i % 2 == 1) {
                    if (rand()%2==1)
                        x3 = xc + topX - (j % Number_Segments);
                    else  x3 = xc + topX + (2 * (j % Number_Segments));
                } else {
                    if (rand() % 2 == 1)
                        x3 = xc + topX + (j % Number_Segments);
                    else
                        x3 = xc + topX - (3 * (j % Number_Segments));
                }
                DrawLightningBolt(buffer, x1 + xoffset, y1, x3 + xoffset, y2, color, width);
            }
        }
        x1 = x2;
        y1 = y2;
    }
}

bool LightningEffect::needToAdjustSettings(const std::string& version)
{
    return IsVersionOlder("2023.07", version);
}

void LightningEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults /*true*/)
{
    SettingsMap& settings = effect->GetSettings();
    settings.erase("E_SLIDER_Lightning_BOTY");
}
