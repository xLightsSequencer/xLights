#include "FireworksEffect.h"
#include "FireworksPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../AudioManager.h"
#include "../models/Model.h"
#include "../SequenceCheck.h"
#include "../sequencer/SequenceElements.h"

#include "../../include/fireworks-16.xpm"
#include "../../include/fireworks-24.xpm"
#include "../../include/fireworks-32.xpm"
#include "../../include/fireworks-48.xpm"
#include "../../include/fireworks-64.xpm"
#include "../UtilFunctions.h"

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

//Max of 50 explosions * 100 particles
static const int maxFlakes = 5000;

class RgbFireworks
{
public:
    //static const float velocity = 2.5;
    static const int maxCycle = 4096;
    static const int maxNewBurstFlakes = 10;
    float _x;
    float _y;
    float _dx;
    float _dy;
    float vel;
    float angle;
    bool _bActive;
    int _cycles;
    int _colorindex;
    int startPeriod;
    float orig_x;
    float orig_y;
    float orig_dx;
    float orig_dy;
    float orig_vel;
    float orig_angle;
    HSVValue _hsv;

    void Reset()
    {
        _x = orig_x;
        _y = orig_y;
        vel = orig_vel;
        angle = orig_angle;
        _dx = orig_dx;
        _dy = orig_dy;
        _cycles = 0;
        _bActive = false;
    }

    void Reset(int x, int y, bool active, float velocity, int colorindex, int start)
    {
        _x       = x;
        orig_x = x;
        _y       = y;
        orig_y = y;
        vel      = (rand()-RAND_MAX/2)*velocity/(RAND_MAX/2);
        orig_vel = vel;
        angle    = 2*M_PI*rand()/RAND_MAX;
        orig_angle = angle;
        _dx      = vel*cos(angle);
        orig_dx = _dx;
        _dy      = vel*sin(angle);
        orig_dy = _dy;
        _cycles  = 0;
        _colorindex = colorindex;
        startPeriod = start;
        _bActive = false;
    }
protected:
private:
};

class FireworksRenderCache : public EffectRenderCache {
public:
    FireworksRenderCache() : fireworkBursts(maxFlakes), sincelasttriggered(0), next(0) {};
    virtual ~FireworksRenderCache() {};
    int next;
    int sincelasttriggered;
    std::vector<RgbFireworks> fireworkBursts;
};

#define REPEATTRIGGER 20

void FireworksEffect::SetDefaultParameters(Model *cls) {
    FireworksPanel *fp = (FireworksPanel*)panel;
    if (fp == nullptr) {
        return;
    }

    SetSliderValue(fp->Slider_Fireworks_Num_Explosions, 16);
    SetSliderValue(fp->Slider_Fireworks_Count, 50);
    SetSliderValue(fp->Slider_Fireworks_Velocity, 2);
    SetSliderValue(fp->Slider_Fireworks_Fade, 50);
    SetSliderValue(fp->Slider_Fireworks_Sensitivity, 50);

    SetCheckBoxValue(fp->CheckBox_Fireworks_UseMusic, false);
    SetCheckBoxValue(fp->CheckBox_FireTiming, false);

    SetPanelTimingTracks();
}

void FireworksEffect::SetPanelStatus(Model *cls)
{
    SetPanelTimingTracks();
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

void FireworksEffect::SetPanelTimingTracks()
{
    FireworksPanel *fp = (FireworksPanel*)panel;
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

void FireworksEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    int Number_Explosions = SettingsMap.GetInt("SLIDER_Fireworks_Explosions", 16);
    int Count = SettingsMap.GetInt("SLIDER_Fireworks_Count", 50);
    float Velocity = SettingsMap.GetDouble("SLIDER_Fireworks_Velocity", 2.0f);
    int Fade = SettingsMap.GetInt("SLIDER_Fireworks_Fade", 50);

    float f = 0.0;
    bool useMusic = SettingsMap.GetBool("CHECKBOX_Fireworks_UseMusic", false);
    float sensitivity = (float)SettingsMap.GetInt("SLIDER_Fireworks_Sensitivity", 50) / 100.0;
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

    FireworksRenderCache *cache = (FireworksRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new FireworksRenderCache();
        buffer.infoCache[id] = cache;
    }
    
    int& next = cache->next;
    int& sincelasttriggered = cache->sincelasttriggered;
    int x25,x75,y25,y75;
    //float velocity = 3.5;
    int ColorIdx;
    float v;
    HSVValue hsv;
    size_t colorcnt = buffer.GetColorCount();
    
    if (buffer.needToInit) {
        SetPanelTimingTracks();
        cache->sincelasttriggered = 0;
        cache->next = 0;
        buffer.needToInit = false;
        for(int i=0; i<maxFlakes; i++) {
            cache->fireworkBursts[i]._bActive = false;
        }
        for (int x = 0; x < Number_Explosions; x++) {
            double start = -1;
            if (!useMusic)
            {
                start = buffer.curEffStartPer + rand01() * (buffer.curEffEndPer - buffer.curEffStartPer);
            }
            x25=(int)buffer.BufferWi*0.25;
            x75=(int)buffer.BufferWi*0.75;
            y25=(int)buffer.BufferHt*0.25;
            y75=(int)buffer.BufferHt*0.75;
            int startX;
            int startY;
            if((x75-x25)>0) startX = x25 + rand()%(x75-x25); else startX=0;
            if((y75-y25)>0) startY = y25 + rand()%(y75-y25); else startY=0;
            
            // Create a new burst
            ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
            for(int i=0; i<Count; i++) {
                cache->fireworkBursts[x * Count + i].Reset(startX, startY, false, Velocity, ColorIdx, start);
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
            if (sincelasttriggered == 0 || sincelasttriggered > REPEATTRIGGER)
            {
                // activate all the particles in the next firework
                for (int j = 0; j < Count; j++)
                {
                    cache->fireworkBursts[Count*next + j]._bActive = true;
                    buffer.palette.GetHSV(cache->fireworkBursts[Count*next + j]._colorindex, hsv); // Now go and get the hsv value for this ColorIdx
                    cache->fireworkBursts[Count*next + j]._hsv = hsv;
                }

                // use the next firework next time
                next++;
                if (next == Number_Explosions)
                {
                    next = 0;
                }
            }

            // if music is over the trigger level for REPEATTRIGGER frames then we will trigger another firework
            sincelasttriggered++;
            if (sincelasttriggered > REPEATTRIGGER)
            {
                sincelasttriggered = 0;
            }
        }
        else
        {
            // not triggered so clear last triggered counter
            sincelasttriggered = 0;
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
                sincelasttriggered = 0;
                EffectLayer* el = t->GetEffectLayer(0);
                for (int j = 0; j < el->GetEffectCount(); j++)
                {
                    if (buffer.curPeriod == el->GetEffect(j)->GetStartTimeMS() / buffer.frameTimeInMs ||
                        buffer.curPeriod == el->GetEffect(j)->GetEndTimeMS() / buffer.frameTimeInMs)
                    {
                        // activate all the particles in the next firework
                        for (int k = 0; k < Count; k++)
                        {
                            cache->fireworkBursts[Count*next + k]._bActive = true;
                            buffer.palette.GetHSV(cache->fireworkBursts[Count*next + k]._colorindex, hsv); // Now go and get the hsv value for this ColorIdx
                            cache->fireworkBursts[Count*next + k]._hsv = hsv;
                        }

                        // use the next firework next time
                        next++;
                        if (next == Number_Explosions)
                        {
                            next = 0;
                        }
                        break;
                    }
                }
            }
        }
    }

    for (int i=0; i<(Count*Number_Explosions); i++) {
        if (!useMusic && !useTiming)
        {
            if (cache->fireworkBursts[i].startPeriod == buffer.curPeriod) {
                cache->fireworkBursts[i]._bActive = true;
                buffer.palette.GetHSV(cache->fireworkBursts[i]._colorindex, hsv); // Now go and get the hsv value for this ColorIdx
                cache->fireworkBursts[i]._hsv = hsv;
            }
        }

        // ... active flakes:
        if (cache->fireworkBursts[i]._bActive)
        {
            // Update position
            cache->fireworkBursts[i]._x += cache->fireworkBursts[i]._dx;
            cache->fireworkBursts[i]._y += (-cache->fireworkBursts[i]._dy - cache->fireworkBursts[i]._cycles*cache->fireworkBursts[i]._cycles/10000000.0);
            // If this flake run for more than maxCycle or this flake is out of bounds, time to switch it off
            cache->fireworkBursts[i]._cycles+=20;
            if (cache->fireworkBursts[i]._cycles >= 10000 || cache->fireworkBursts[i]._y >= buffer.BufferHt || cache->fireworkBursts[i]._y < 0 ||
                cache->fireworkBursts[i]._x < 0. || cache->fireworkBursts[i]._x >= buffer.BufferWi)
            {
                cache->fireworkBursts[i]._bActive = false;
                if (useMusic)
                {
                    cache->fireworkBursts[i].Reset();
                }
                continue;
            }
        }
        if(cache->fireworkBursts[i]._bActive == true)
        {
            v = ((Fade*10.0)-cache->fireworkBursts[i]._cycles)/(Fade*10.0);
            if (v<0) v=0.0;
            if (buffer.allowAlpha) {
                xlColor c(cache->fireworkBursts[i]._hsv);
                c.alpha = 255.0 * v;
                buffer.SetPixel(cache->fireworkBursts[i]._x, cache->fireworkBursts[i]._y, c);
            } else {
                hsv=cache->fireworkBursts[i]._hsv;
                hsv.value=v;
                buffer.SetPixel(cache->fireworkBursts[i]._x, cache->fireworkBursts[i]._y, hsv);
            }
        }
    }
}