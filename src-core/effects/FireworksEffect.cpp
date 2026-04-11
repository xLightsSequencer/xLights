/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "FireworksEffect.h"

#include <spdlog/fmt/fmt.h>

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "media/AudioManager.h"
#include "../models/Model.h"
#include "UtilFunctions.h"
#include "../render/SequenceElements.h"

#include "../../include/fireworks-16.xpm"
#include "../../include/fireworks-24.xpm"
#include "../../include/fireworks-32.xpm"
#include "../../include/fireworks-48.xpm"
#include "../../include/fireworks-64.xpm"

// Fallback defaults (used until OnMetadataLoaded replaces them with Fireworks.json values).
int FireworksEffect::sExplosionsDefault = 16;
int FireworksEffect::sExplosionsMin = 1;
int FireworksEffect::sExplosionsMax = 50;
int FireworksEffect::sCountDefault = 50;
int FireworksEffect::sCountMin = 1;
int FireworksEffect::sCountMax = 100;
double FireworksEffect::sVelocityDefault = 2.0;
double FireworksEffect::sVelocityMin = 1;
double FireworksEffect::sVelocityMax = 10;
int FireworksEffect::sXVelocityDefault = 0;
int FireworksEffect::sXVelocityMin = -100;
int FireworksEffect::sXVelocityMax = 100;
int FireworksEffect::sYVelocityDefault = 0;
int FireworksEffect::sYVelocityMin = -100;
int FireworksEffect::sYVelocityMax = 100;
int FireworksEffect::sXLocationDefault = -1;
int FireworksEffect::sXLocationMin = -1;
int FireworksEffect::sXLocationMax = 100;
int FireworksEffect::sYLocationDefault = -1;
int FireworksEffect::sYLocationMin = -1;
int FireworksEffect::sYLocationMax = 100;
bool FireworksEffect::sHoldColourDefault = true;
bool FireworksEffect::sGravityDefault = true;
int FireworksEffect::sFadeDefault = 50;
int FireworksEffect::sFadeMin = 1;
int FireworksEffect::sFadeMax = 100;
bool FireworksEffect::sUseMusicDefault = false;
int FireworksEffect::sSensitivityDefault = 50;
bool FireworksEffect::sFireTimingDefault = false;
std::string FireworksEffect::sFireTimingTrackDefault = "";

FireworksEffect::FireworksEffect(int id) : RenderableEffect(id, "Fireworks", fireworks_16, fireworks_24, fireworks_32, fireworks_48, fireworks_64)
{
    //ctor
}

FireworksEffect::~FireworksEffect()
{
    //dtor
}

void FireworksEffect::OnMetadataLoaded()
{
    sExplosionsDefault = GetIntDefault("Fireworks_Explosions", sExplosionsDefault);
    sExplosionsMin = (int)GetMinFromMetadata("Fireworks_Explosions", sExplosionsMin);
    sExplosionsMax = (int)GetMaxFromMetadata("Fireworks_Explosions", sExplosionsMax);
    sCountDefault = GetIntDefault("Fireworks_Count", sCountDefault);
    sCountMin = (int)GetMinFromMetadata("Fireworks_Count", sCountMin);
    sCountMax = (int)GetMaxFromMetadata("Fireworks_Count", sCountMax);
    sVelocityDefault = GetDoubleDefault("Fireworks_Velocity", sVelocityDefault);
    sVelocityMin = GetMinFromMetadata("Fireworks_Velocity", sVelocityMin);
    sVelocityMax = GetMaxFromMetadata("Fireworks_Velocity", sVelocityMax);
    sXVelocityDefault = GetIntDefault("Fireworks_XVelocity", sXVelocityDefault);
    sXVelocityMin = (int)GetMinFromMetadata("Fireworks_XVelocity", sXVelocityMin);
    sXVelocityMax = (int)GetMaxFromMetadata("Fireworks_XVelocity", sXVelocityMax);
    sYVelocityDefault = GetIntDefault("Fireworks_YVelocity", sYVelocityDefault);
    sYVelocityMin = (int)GetMinFromMetadata("Fireworks_YVelocity", sYVelocityMin);
    sYVelocityMax = (int)GetMaxFromMetadata("Fireworks_YVelocity", sYVelocityMax);
    sXLocationDefault = GetIntDefault("Fireworks_XLocation", sXLocationDefault);
    sXLocationMin = (int)GetMinFromMetadata("Fireworks_XLocation", sXLocationMin);
    sXLocationMax = (int)GetMaxFromMetadata("Fireworks_XLocation", sXLocationMax);
    sYLocationDefault = GetIntDefault("Fireworks_YLocation", sYLocationDefault);
    sYLocationMin = (int)GetMinFromMetadata("Fireworks_YLocation", sYLocationMin);
    sYLocationMax = (int)GetMaxFromMetadata("Fireworks_YLocation", sYLocationMax);
    sHoldColourDefault = GetBoolDefault("Fireworks_HoldColour", sHoldColourDefault);
    sGravityDefault = GetBoolDefault("Fireworks_Gravity", sGravityDefault);
    sFadeDefault = GetIntDefault("Fireworks_Fade", sFadeDefault);
    sFadeMin = (int)GetMinFromMetadata("Fireworks_Fade", sFadeMin);
    sFadeMax = (int)GetMaxFromMetadata("Fireworks_Fade", sFadeMax);
    sUseMusicDefault = GetBoolDefault("Fireworks_UseMusic", sUseMusicDefault);
    sSensitivityDefault = GetIntDefault("Fireworks_Sensitivity", sSensitivityDefault);
    sFireTimingDefault = GetBoolDefault("FIRETIMING", sFireTimingDefault);
    sFireTimingTrackDefault = GetStringDefault("FIRETIMINGTRACK", sFireTimingTrackDefault);
}

std::list<std::string> FireworksEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    if (media == nullptr && settings.GetBool("E_CHECKBOX_Fireworks_UseMusic", sUseMusicDefault)) {
        res.push_back(fmt::format("    ERR: Fireworks effect cant grow to music if there is no music. Model '{}', Start {}", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }
    if (settings.GetBool("E_CHECKBOX_FIRETIMING", sFireTimingDefault) && settings.Get("E_CHOICE_FIRETIMINGTRACK", sFireTimingTrackDefault) == "") {
        res.push_back(fmt::format("    ERR: Fireworks effect is meant to fire with timing track but no timing track selected. Model '{}', Start {}", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }

    return res;
}

class FireworkParticle
{
    const int _maxFade = 100;
    double _x;
    double _y;
    double _vx;
    double _vy;
    int _fade;
    bool _gravity;
    HSVValue _startColour;
    int _colourIndex;
    bool _holdColour;
    int _width;
    int _height;
    double _fps;
    int _age = 0;

public:
    FireworkParticle(int x, int y, double vx, double vy, int fade, bool gravity, int colourIndex, bool holdColour, double velocity, int width, int height, int frameMS, const PaletteClass& palette)
    {
        _width = width;
        _height = height;
        _x = x;
        _y = y;
        _fade = fade;
        _gravity = gravity;
        _colourIndex = colourIndex;
        _holdColour = holdColour;

        if (_holdColour)
        {
            palette.GetHSV(_colourIndex, _startColour);
        }

        _fps = 1000.0 / frameMS;

        double explosionVelocity = (rand() - RAND_MAX / 2)*velocity / (RAND_MAX / 2);
        double angle = 2 * M_PI*rand() / RAND_MAX;
        _vx = 3.0 * vx / 100 + explosionVelocity * cos(angle);
        _vy = 3.0 * -vy / 100 + explosionVelocity * sin(angle);
    }

    virtual ~FireworkParticle() {}

    bool Done() const
    {
        return (_fade < _age * 2 || _x < 0 || _y < 0 || _x > _width || (!_gravity && _y > _height));
    }

    void Advance()
    {
        _x += _vx;
        if (_gravity)
        {
            _vy += 0.98 / _fps;
        }
        _y += -_vy;
        _age++;
    }

    int GetX() const { return _x; }
    int GetY() const { return _y; }

    xlColor GetColour(const PaletteClass& palette, bool alpha) const
    {
        double v = ((10.0*_fade) - _age * 20.0) / (10.0*_fade);
        if (v < 0.0) v = 0.0;

        HSVValue cv = _startColour;
        if (_holdColour)
        {
            if (alpha)
            {
                xlColor c(cv);
                c.alpha = 255.0 *v;
                return c;
            }
            else
            {
                cv.value = v;
                return xlColor(cv);
            }
        }
        else
        {
            palette.GetHSV(_colourIndex, cv);
            if (alpha)
            {
                xlColor c(cv);
                c.alpha = 255.0 * v;
                return c;
            }
            else
            {
                cv.value = v;
                return xlColor(cv);
            }
        }
    }
};

class Firework
{
    const int _maxCycles = 500;
    int _cycles = 0;
    mutable bool _done = false;
    std::vector<FireworkParticle> _particles;

public:
    Firework(int particles, int x, int y, double vx, double vy, int fade, bool gravity, int colourIndex, bool holdColour, double velocity, int width, int height, int frameMS, const PaletteClass& palette)
    {
        _cycles = 0;
        for (int i = 0; i < particles; i++)
        {
            _particles.push_back(FireworkParticle(x, y, vx, vy, fade, gravity, colourIndex, holdColour, velocity, width, height, frameMS, palette));
        }
    }

    const std::vector<FireworkParticle>& GetParticles() const {
        return _particles;
    }

    void Advance()
    {
        _cycles++;

        for (auto& it : _particles)
        {
            it.Advance();
        }

        if (Done()) _particles.clear();
    }

    bool AllGone() const
    {
        for (const auto& it : _particles)
        {
            if (!it.Done()) return false;
        }

        _done = true;
        return true;
    }

    bool Done() const
    {
        return _done || _cycles >= _maxCycles || AllGone();
    }
};

class FireworksRenderCache : public EffectRenderCache {
public:
    FireworksRenderCache() {};
    virtual ~FireworksRenderCache() {};
    int _sinceLastTriggered = 0;
    std::list<Firework> _fireworks;
    std::vector<int> _firePeriods;
};

#define REPEATTRIGGER 20

void FireworksEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
{
    std::string timing = effect->GetSettings().Get("E_CHOICE_FIRETIMINGTRACK", "");

    if (timing == oldname)
    {
        effect->GetSettings()["E_CHOICE_FIRETIMINGTRACK"] = newname;
    }

}

std::pair<int,int> FireworksEffect::GetFireworkLocation(int width, int height, int overridex, int overridey)
{
    int startX;
    int startY;
    if (overridex >= 0)
    {
        startX = overridex * width / 100;
    }
    else
    {
        int x25 = static_cast<int>(0.25f * width);
        int x75 = static_cast<int>(0.75f * width);
        if ((x75 - x25) > 0) startX = x25 + rand() % (x75 - x25); else startX = 0;
    }

    if (overridey >= 0)
    {
        startY = overridey * height / 100;
    }
    else
    {
        int y25 = static_cast<int>(0.25f * height);
        int y75 = static_cast<int>(0.75f * height);
        if ((y75 - y25) > 0) startY = y25 + rand() % (y75 - y25); else startY = 0;
    }
    return { startX, startY };
}

void FireworksEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float offset = buffer.GetEffectTimeIntervalPosition();

    int numberOfExplosions = SettingsMap.GetInt("SLIDER_Fireworks_Explosions", sExplosionsDefault);
    int particleCount = GetValueCurveInt("Fireworks_Count", sCountDefault, SettingsMap, offset, sCountMin, sCountMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float particleVelocity = GetValueCurveDouble("Fireworks_Velocity", sVelocityDefault, SettingsMap, offset, sVelocityMin, sVelocityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int fade = GetValueCurveInt("Fireworks_Fade", sFadeDefault, SettingsMap, offset, sFadeMin, sFadeMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int xVelocity = GetValueCurveInt("Fireworks_XVelocity", sXVelocityDefault, SettingsMap, offset, sXVelocityMin, sXVelocityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int yVelocity = GetValueCurveInt("Fireworks_YVelocity", sYVelocityDefault, SettingsMap, offset, sYVelocityMin, sYVelocityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int xLocation = GetValueCurveInt("Fireworks_XLocation", sXLocationDefault, SettingsMap, offset, sXLocationMin, sXLocationMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int yLocation = GetValueCurveInt("Fireworks_YLocation", sYLocationDefault, SettingsMap, offset, sYLocationMin, sYLocationMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool gravity = SettingsMap.GetBool("CHECKBOX_Fireworks_Gravity", sGravityDefault);
    bool holdColour = SettingsMap.GetBool("CHECKBOX_Fireworks_HoldColour", sHoldColourDefault);

    float f = 0.0;
    bool useMusic = SettingsMap.GetBool("CHECKBOX_Fireworks_UseMusic", sUseMusicDefault);
    float sensitivity = static_cast<float>(SettingsMap.GetInt("SLIDER_Fireworks_Sensitivity", sSensitivityDefault)) / 100.0;
    bool useTiming = SettingsMap.GetBool("CHECKBOX_FIRETIMING", sFireTimingDefault);
    std::string timing = SettingsMap.Get("CHOICE_FIRETIMINGTRACK", sFireTimingTrackDefault);
    if (timing == "")
    {
        useTiming = false;
    }
    if (useMusic)
    {
        if (buffer.GetMedia() != nullptr) {
            auto pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, "");
            if (pf != nullptr) {
                f = pf->max;
            }
        }
    }

    FireworksRenderCache *cache = static_cast<FireworksRenderCache*>(buffer.infoCache[id]);
    if (cache == nullptr) {
        cache = new FireworksRenderCache();
        buffer.infoCache[id] = cache;
    }

    auto& sinceLastTriggered = cache->_sinceLastTriggered;
    auto& fireworks = cache->_fireworks;
    auto& firePeriods = cache->_firePeriods;

    size_t colorcnt = buffer.GetColorCount();

    if (buffer.needToInit) {
        buffer.needToInit = false;
        sinceLastTriggered = 0;
        if (!useMusic && !useTiming)
        {
            for (int i = 0; i < numberOfExplosions; i++) {
                firePeriods.push_back(buffer.curEffStartPer + rand01() * (buffer.curEffEndPer - buffer.curEffStartPer));
            }
        }

        if (timing != "")
        {
            effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->AddRenderDependency(timing, buffer.cur_model);
        }
    }

    if (useMusic)
    {
        // only trigger a firework if music is greater than the sensitivity
        if (f > sensitivity)
        {
            // trigger if it was not previously triggered or has been triggered for REPEATTRIGGER frames
            if (sinceLastTriggered == 0 || sinceLastTriggered > REPEATTRIGGER)
            {
                auto location = GetFireworkLocation(buffer.BufferWi, buffer.BufferHt, xLocation, yLocation);
                int colourIndex = rand() % colorcnt;
                fireworks.push_back(Firework(particleCount,
                    location.first, location.second,
                    xVelocity, yVelocity,
                    fade, gravity,
                    colourIndex, holdColour,
                    particleVelocity,
                    buffer.BufferWi, buffer.BufferHt,
                    buffer.frameTimeInMs, buffer.palette));
            }

            // if music is over the trigger level for REPEATTRIGGER frames then we will trigger another firework
            sinceLastTriggered++;
            if (sinceLastTriggered > REPEATTRIGGER)
            {
                sinceLastTriggered = 0;
            }
        }
        else
        {
            // not triggered so clear last triggered counter
            sinceLastTriggered = 0;
        }
    }

    if (useTiming)
    {
        if (mSequenceElements == nullptr)
        {
            // no timing tracks ... this shouldnt happen
        }
        else
        {
            // Load the names of the timing tracks
            EffectLayer* el = GetTiming(timing);

            if (el == nullptr)
            {
                // timing track not found ... this shouldnt happen
            }
            else
            {
                sinceLastTriggered = 0;
                for (int j = 0; j < el->GetEffectCount(); j++)
                {
                    if (buffer.curPeriod == el->GetEffect(j)->GetStartTimeMS() / buffer.frameTimeInMs ||
                        buffer.curPeriod == el->GetEffect(j)->GetEndTimeMS() / buffer.frameTimeInMs)
                    {
                        auto location = GetFireworkLocation(buffer.BufferWi, buffer.BufferHt, xLocation, yLocation);
                        int colourIndex = rand() % colorcnt;
                        fireworks.push_back(Firework(particleCount,
                            location.first, location.second,
                            xVelocity, yVelocity,
                            fade, gravity,
                            colourIndex, holdColour,
                            particleVelocity,
                            buffer.BufferWi, buffer.BufferHt,
                            buffer.frameTimeInMs, buffer.palette));
                        break;
                    }
                }
            }
        }
    }

    if (!useTiming && !useMusic && firePeriods.size() > 0)
    {
        for (const auto& it : firePeriods)
        {
            if (it == buffer.curPeriod)
            {
                auto location = GetFireworkLocation(buffer.BufferWi, buffer.BufferHt, xLocation, yLocation);
                int colourIndex = rand() % colorcnt;
                fireworks.push_back(Firework(particleCount,
                    location.first, location.second,
                    xVelocity, yVelocity,
                    fade, gravity,
                    colourIndex, holdColour,
                    particleVelocity,
                    buffer.BufferWi, buffer.BufferHt,
                    buffer.frameTimeInMs, buffer.palette));
            }
        }
    }

    for (auto& it: fireworks)
    {
        if (!it.Done())
        {
            for (auto p : it.GetParticles())
            {
                buffer.SetPixel(p.GetX(), p.GetY(), p.GetColour(buffer.palette, buffer.allowAlpha));
            }

            it.Advance();
        }
    }
}
