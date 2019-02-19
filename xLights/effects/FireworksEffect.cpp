#include "FireworksEffect.h"
#include "FireworksPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../AudioManager.h"
#include "../models/Model.h"
#include "../UtilFunctions.h"
#include "../sequencer/SequenceElements.h"

#include "../../include/fireworks-16.xpm"
#include "../../include/fireworks-24.xpm"
#include "../../include/fireworks-32.xpm"
#include "../../include/fireworks-48.xpm"
#include "../../include/fireworks-64.xpm"

FireworksEffect::FireworksEffect(int id) : RenderableEffect(id, "Fireworks", fireworks_16, fireworks_24, fireworks_32, fireworks_48, fireworks_64)
{
    //ctor
}

FireworksEffect::~FireworksEffect()
{
    //dtor
}

std::list<std::string> FireworksEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff)
{
    std::list<std::string> res;

    if (media == nullptr && settings.GetBool("E_CHECKBOX_Fireworks_UseMusic", false))
    {
        res.push_back(wxString::Format("    WARN: Fireworks effect cant grow to music if there is no music. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

wxPanel *FireworksEffect::CreatePanel(wxWindow *parent) {
    return new FireworksPanel(parent);
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
        for (auto it : _particles)
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

void FireworksEffect::SetDefaultParameters() {
    FireworksPanel *fp = static_cast<FireworksPanel*>(panel);
    if (fp == nullptr) {
        return;
    }

    fp->BitmapButton_Fireworks_Count->SetActive(false);
    fp->BitmapButton_Fireworks_Velocity->SetActive(false);
    fp->BitmapButton_Fireworks_XVelocity->SetActive(false);
    fp->BitmapButton_Fireworks_YVelocity->SetActive(false);
    fp->BitmapButton_Fireworks_XLocation->SetActive(false);
    fp->BitmapButton_Fireworks_YLocation->SetActive(false);
    fp->BitmapButton_Fireworks_Fade->SetActive(false);

    SetSliderValue(fp->Slider_Fireworks_Num_Explosions, 16);
    SetSliderValue(fp->Slider_Fireworks_Count, 50);
    SetSliderValue(fp->Slider_Fireworks_Velocity, 2);
    SetSliderValue(fp->Slider_Fireworks_Fade, 50);
    SetSliderValue(fp->Slider_Fireworks_Sensitivity, 50);
    SetSliderValue(fp->Slider_Fireworks_XVelocity, 0);
    SetSliderValue(fp->Slider_Fireworks_YVelocity, 0);
    SetSliderValue(fp->Slider_Fireworks_XLocation, -1);
    SetSliderValue(fp->Slider_Fireworks_YLocation, -1);

    SetCheckBoxValue(fp->CheckBox_Fireworks_UseMusic, false);
    SetCheckBoxValue(fp->CheckBox_FireTiming, false);
    SetCheckBoxValue(fp->CheckBox_Fireworks_Gravity, true);
    SetCheckBoxValue(fp->CheckBox_Fireworks_HoldColor, true);

    SetPanelTimingTracks();
}

void FireworksEffect::SetPanelStatus(Model *cls)
{
    SetPanelTimingTracks();
}

bool FireworksEffect::needToAdjustSettings(const std::string &version)
{
    return IsVersionOlder("2019.9", version);
}

void FireworksEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults)
{
    SettingsMap &settings = effect->GetSettings();
    bool gravity = settings.GetBool("E_CHECKBOX_Fireworks_Gravity", false);
    settings["E_CHECKBOX_Fireworks_Gravity"] = gravity ? "1" : "0";

    // also give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }
}

void FireworksEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
{
    wxString timing = effect->GetSettings().Get("E_CHOICE_FIRETIMINGTRACK", "");

    if (timing.ToStdString() == oldname)
    {
        effect->GetSettings()["E_CHOICE_FIRETIMINGTRACK"] = wxString(newname);
    }

    SetPanelTimingTracks();
}

void FireworksEffect::SetPanelTimingTracks() const
{
    FireworksPanel *fp = static_cast<FireworksPanel*>(panel);
    if (fp == nullptr)
    {
        return;
    }

    if (mSequenceElements == nullptr)
    {
        return;
    }

    // Load the names of the timing tracks
    std::string timingtracks = "";
    for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++)
    {
        Element* e = mSequenceElements->GetElement(i);
        if (e->GetEffectLayerCount() == 1 && e->GetType() == ELEMENT_TYPE_TIMING)
        {
            if (timingtracks != "") timingtracks += "|";
            timingtracks += e->GetName();
        }
    }

    wxCommandEvent event(EVT_SETTIMINGTRACKS);
    event.SetString(timingtracks);
    wxPostEvent(fp, event);
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

void FireworksEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float offset = buffer.GetEffectTimeIntervalPosition();
    
    int numberOfExplosions = SettingsMap.GetInt("SLIDER_Fireworks_Explosions", 16);
    int particleCount = GetValueCurveInt("Fireworks_Count", 50, SettingsMap, offset, FIREWORKSCOUNT_MIN, FIREWORKSCOUNT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float particleVelocity = GetValueCurveDouble("Fireworks_Velocity", 2.0, SettingsMap, offset, FIREWORKSVELOCITY_MIN, FIREWORKSVELOCITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int fade = GetValueCurveInt("Fireworks_Fade", 50, SettingsMap, offset, FIREWORKSFADE_MIN, FIREWORKSFADE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int xVelocity = GetValueCurveInt("Fireworks_XVelocity", 0, SettingsMap, offset, FIREWORKSXVELOCITY_MIN, FIREWORKSXVELOCITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int yVelocity = GetValueCurveInt("Fireworks_YVelocity", 0, SettingsMap, offset, FIREWORKSYVELOCITY_MIN, FIREWORKSYVELOCITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int xLocation = GetValueCurveInt("Fireworks_XLocation", -1, SettingsMap, offset, FIREWORKSXLOCATION_MIN, FIREWORKSXLOCATION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int yLocation = GetValueCurveInt("Fireworks_YLocation", -1, SettingsMap, offset, FIREWORKSYLOCATION_MIN, FIREWORKSYLOCATION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool gravity = SettingsMap.GetBool("CHECKBOX_Fireworks_Gravity", false);
    bool holdColour = SettingsMap.GetBool("CHECKBOX_Fireworks_HoldColour", true);

    float f = 0.0;
    bool useMusic = SettingsMap.GetBool("CHECKBOX_Fireworks_UseMusic", false);
    float sensitivity = static_cast<float>(SettingsMap.GetInt("SLIDER_Fireworks_Sensitivity", 50)) / 100.0;
    bool useTiming = SettingsMap.GetBool("CHECKBOX_FIRETIMING", false);
    wxString timing = SettingsMap.Get("CHOICE_FIRETIMINGTRACK", "");
    if (timing == "") useTiming = false;
    if (useMusic)
    {
        if (buffer.GetMedia() != nullptr) {
            std::list<float>* pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
            if (pf != nullptr)
            {
                f = *pf->begin();
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
        SetPanelTimingTracks();
        sinceLastTriggered = 0;
        if (!useMusic)
        {
            for (int i = 0; i < numberOfExplosions; i++) {
                firePeriods.push_back(buffer.curEffStartPer + rand01() * (buffer.curEffEndPer - buffer.curEffStartPer));
            }
        }

        if (timing != "")
        {
            effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->AddRenderDependency(timing.ToStdString(), buffer.cur_model);
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
            Element* t = nullptr;
            for (size_t l = 0; l < mSequenceElements->GetElementCount(); l++)
            {
                Element* e = mSequenceElements->GetElement(l);
                if (e->GetEffectLayerCount() == 1 && e->GetType() == ELEMENT_TYPE_TIMING)
                {
                    if (e->GetName() == timing)
                    {
                        t = e;
                        break;
                    }
                }
            }

            if (t == nullptr)
            {
                // timing track not found ... this shouldnt happen
            }
            else
            {
                sinceLastTriggered = 0;
                EffectLayer* el = t->GetEffectLayer(0);
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
        for (auto it : firePeriods)
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