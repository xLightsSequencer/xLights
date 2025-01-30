/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PlasmaEffect.h"
#include "PlasmaPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/plasma-16.xpm"
#include "../../include/plasma-24.xpm"
#include "../../include/plasma-32.xpm"
#include "../../include/plasma-48.xpm"
#include "../../include/plasma-64.xpm"

#include "../Parallel.h"

#include "ispc/PlasmaFunctions.ispc.h"


PlasmaEffect::PlasmaEffect(int id) : RenderableEffect(id, "Plasma", plasma_16, plasma_24, plasma_32, plasma_48, plasma_64)
{
    //ctor
}

PlasmaEffect::~PlasmaEffect()
{
    //dtor
}
xlEffectPanel *PlasmaEffect::CreatePanel(wxWindow *parent) {
    return new PlasmaPanel(parent);
}

#define PLASMA_NORMAL_COLORS    0
#define PLASMA_PRESET1          1
#define PLASMA_PRESET2          2
#define PLASMA_PRESET3          3
#define PLASMA_PRESET4          4

int PlasmaEffect::GetPlasmaColorScheme(const std::string &ColorSchemeStr) {
    if (ColorSchemeStr == "Preset Colors 1") {
        return PLASMA_PRESET1;
    } else if (ColorSchemeStr == "Preset Colors 2") {
        return PLASMA_PRESET2;
    } else if (ColorSchemeStr == "Preset Colors 3") {
        return PLASMA_PRESET3;
    } else if (ColorSchemeStr == "Preset Colors 4") {
        return PLASMA_PRESET4;
    }
    return PLASMA_NORMAL_COLORS;
}

void PlasmaEffect::SetDefaultParameters() {
    PlasmaPanel *pp = (PlasmaPanel*)panel;
    if (pp == nullptr) {
        return;
    }

    pp->BitmapButton_Plasma_SpeedVC->SetActive(false);

    SetSliderValue(pp->Slider_Plasma_Style, 1);
    SetSliderValue(pp->Slider_Plasma_Line_Density, 1);
    SetSliderValue(pp->Slider_Plasma_Speed, 10);

    SetChoiceValue(pp->Choice_Plasma_Color, "Normal");
}

void PlasmaEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();
    int Style = SettingsMap.GetInt("SLIDER_Plasma_Style", 1);
    int Line_Density = SettingsMap.GetInt("SLIDER_Plasma_Line_Density", 1);
    int PlasmaSpeed = GetValueCurveInt("Plasma_Speed", 10, SettingsMap, oset, PLASMA_SPEED_MIN, PLASMA_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    std::string PlasmaDirectionStr = SettingsMap["CHOICE_Plasma_Direction"];

    int PlasmaDirection = 0; //fixme?
    const int ColorScheme = GetPlasmaColorScheme(SettingsMap["CHOICE_Plasma_Color"]);


    int curState = (buffer.curPeriod - buffer.curEffStartPer) * PlasmaSpeed * buffer.frameTimeInMs / 50;
    double offset=double(curState)/200.0;

    const int state = (buffer.curPeriod - buffer.curEffStartPer); // frames 0 to N
    const double Speed_plasma = (101-PlasmaSpeed)*3; // we want a large number to divide by
    const double time = (state+1.0)/Speed_plasma;

    if (PlasmaDirection==1) offset = -offset;

    const double sin_time_5 = buffer.sin(time / 5);
    const double cos_time_3 = buffer.cos(time / 3);
    const double sin_time_2 = buffer.sin(time / 2);

    ispc::PlasmaData rdata;
    rdata.width = buffer.BufferWi;
    rdata.height = buffer.BufferHt;
    rdata.state = state;
    rdata.Style = Style;
    rdata.Line_Density = Line_Density;
    rdata.sin_time_5 = sin_time_5;
    rdata.cos_time_3 = cos_time_3;
    rdata.sin_time_2 = sin_time_2;
    rdata.time = time;

    rdata.numColors = buffer.palette.Size();
    for (int x = 0; x < rdata.numColors; x++) {
        const xlColor &c = buffer.palette.GetColor(x);
        rdata.colors[x].v[0] = c.red;
        rdata.colors[x].v[1] = c.green;
        rdata.colors[x].v[2] = c.blue;
        rdata.colors[x].v[3] = c.alpha;
    }
    int max = buffer.BufferHt * buffer.BufferWi;
    constexpr int bfBlockSize = 4096;
    int blocks = max / bfBlockSize + 1;
    parallel_for(0, blocks, [&rdata, &buffer, max, ColorScheme](int y) {
        int start = y * bfBlockSize;
        int end = start + bfBlockSize;
        if (end > max) {
            end = max;
        }
        switch (ColorScheme) {
            case 0:
                ispc::PlasmaEffectStyle0(rdata, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
            case 1:
                ispc::PlasmaEffectStyle1(rdata, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
            case 2:
                ispc::PlasmaEffectStyle2(rdata, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
            case 3:
                ispc::PlasmaEffectStyle3(rdata, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
            case 4:
                ispc::PlasmaEffectStyle4(rdata, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
        }
    });
}
