/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "GarlandsEffect.h"

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"

#include "../../include/garlands-16.xpm"
#include "../../include/garlands-24.xpm"
#include "../../include/garlands-32.xpm"
#include "../../include/garlands-48.xpm"
#include "../../include/garlands-64.xpm"

// Fallback defaults (used until OnMetadataLoaded replaces them with Garlands.json values).
int GarlandsEffect::sTypeDefault = 0;
int GarlandsEffect::sTypeMin = 0;
int GarlandsEffect::sTypeMax = 4;
int GarlandsEffect::sSpacingDefault = 10;
int GarlandsEffect::sSpacingMin = 1;
int GarlandsEffect::sSpacingMax = 100;
double GarlandsEffect::sCyclesDefault = 1.0;
double GarlandsEffect::sCyclesMin = 0;
double GarlandsEffect::sCyclesMax = 200;
int GarlandsEffect::sCyclesDivisor = 10;
std::string GarlandsEffect::sDirectionDefault = "Up";

GarlandsEffect::GarlandsEffect(int id) : RenderableEffect(id, "Garlands", garlands_16, garlands_24, garlands_32, garlands_48, garlands_64)
{
    //ctor
}

GarlandsEffect::~GarlandsEffect()
{
    //dtor
}

void GarlandsEffect::OnMetadataLoaded()
{
    sTypeDefault = GetIntDefault("Garlands_Type", sTypeDefault);
    sTypeMin = (int)GetMinFromMetadata("Garlands_Type", sTypeMin);
    sTypeMax = (int)GetMaxFromMetadata("Garlands_Type", sTypeMax);
    sSpacingDefault = GetIntDefault("Garlands_Spacing", sSpacingDefault);
    sSpacingMin = (int)GetMinFromMetadata("Garlands_Spacing", sSpacingMin);
    sSpacingMax = (int)GetMaxFromMetadata("Garlands_Spacing", sSpacingMax);
    sCyclesDefault = GetDoubleDefault("Garlands_Cycles", sCyclesDefault);
    sCyclesMin = GetMinFromMetadata("Garlands_Cycles", sCyclesMin);
    sCyclesMax = GetMaxFromMetadata("Garlands_Cycles", sCyclesMax);
    sCyclesDivisor = GetDivisorFromMetadata("Garlands_Cycles", sCyclesDivisor);
    sDirectionDefault = GetStringDefault("Garlands_Direction", sDirectionDefault);
}

bool GarlandsEffect::needToAdjustSettings(const std::string& version)
{
    return IsVersionOlder("2026.05.2", version);
}

void GarlandsEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults)
{
    if (IsVersionOlder("2026.05.2", version)) {
        // Garlands_Cycles moved from post-divisor min/max (0..20, no divisor)
        // to the JSON convention (pre-divisor 0..200, divisor 10). Active VCs
        // are rescaled automatically by UpgradeValueCurve at load, but any
        // plain slider stored as E_SLIDER_Garlands_Cycles=X (raw int in old
        // [0..20] space) needs to move to E_TEXTCTRL_Garlands_Cycles=X.X as a
        // post-divisor float — same displayed value, just in the new key.
        SettingsMap& settings = effect->GetSettings();
        std::string cycles = settings.Get("E_SLIDER_Garlands_Cycles", "");
        if (!cycles.empty()) {
            settings.erase("E_SLIDER_Garlands_Cycles");
            settings["E_TEXTCTRL_Garlands_Cycles"] = cycles;
        }
    }
}

int GetDirection(const std::string &direction) {
    if ("Up" == direction) {
        return 0;
    } else if ("Down" == direction) {
        return 1;
    } else if ("Left" == direction) {
        return 2;
    } else if ("Right" == direction) {
        return 3;
    } else if ("Up then Down" == direction) {
        return 4;
    } else if ("Down then Up" == direction) {
        return 5;
    } else if ("Left then Right" == direction) {
        return 6;
    } else if ("Right then Left" == direction) {
        return 7;
    }
    return 0;
}

void GarlandsEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    int GarlandType = SettingsMap.GetInt("SLIDER_Garlands_Type", sTypeDefault);
    int Spacing = GetValueCurveInt("Garlands_Spacing", sSpacingDefault, SettingsMap, oset, sSpacingMin, sSpacingMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float cycles = GetValueCurveDouble("Garlands_Cycles", sCyclesDefault, SettingsMap, oset, sCyclesMin, sCyclesMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sCyclesDivisor);

    if (Spacing < 1) {
        Spacing = 1;
    }
    int x,y,yadj,ylimit,ring;
    double ratio;
    xlColor color;
    int dir = GetDirection(SettingsMap.Get("CHOICE_Garlands_Direction", sDirectionDefault));
    double position = buffer.GetEffectTimeIntervalPosition(cycles);
    if (dir > 3) {
        dir -= 4;
        if (position > 0.5) {
            position = (1.0 - position) * 2.0;
        } else {
            position *= 2.0;
        }
    }
    int buffMax = buffer.BufferHt;
    int garlandWid = buffer.BufferWi;
    if (dir > 1) {
        buffMax = buffer.BufferWi;
        garlandWid = buffer.BufferHt;
    }
    double PixelSpacing=Spacing*buffMax/100.0;
    if (PixelSpacing < 2.0) PixelSpacing=2.0;
    
    
    double total = buffMax * PixelSpacing - buffMax + 1;
    double positionOffset = total * position;
    
    for (ring = 0; ring < buffMax; ring++)
    {
        ratio=double(buffMax-ring-1)/double(buffMax);
        buffer.GetMultiColorBlend(ratio, false, color);
        
        y = 1.0 + ring*PixelSpacing - positionOffset;
        
        
        ylimit=ring;
        for (x=0; x<garlandWid; x++)
        {
            yadj=y;
            switch (GarlandType)
            {
                case 1:
                    switch (x%5)
                {
                    case 2:
                        yadj-=2;
                        break;
                    case 1:
                    case 3:
                        yadj-=1;
                        break;
                }
                    break;
                case 2:
                    switch (x%5)
                {
                    case 2:
                        yadj-=4;
                        break;
                    case 1:
                    case 3:
                        yadj-=2;
                        break;
                }
                    break;
                case 3:
                    switch (x%6)
                {
                    case 3:
                        yadj-=6;
                        break;
                    case 2:
                    case 4:
                        yadj-=4;
                        break;
                    case 1:
                    case 5:
                        yadj-=2;
                        break;
                }
                    break;
                case 4:
                    switch (x%5)
                {
                    case 1:
                    case 3:
                        yadj-=2;
                        break;
                }
                    break;
            }
            if (yadj < ylimit) yadj=ylimit;
            if (yadj < buffMax) {
                if (dir == 1 || dir == 2) {
                    yadj = buffMax - yadj - 1;
                }
                if (dir > 1) {
                    buffer.SetPixel(yadj,x,color);
                } else {
                    buffer.SetPixel(x,yadj,color);
                }
            }
        }
    }
}
