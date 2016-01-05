#include "FireworksEffect.h"
#include "FireworksPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

FireworksEffect::FireworksEffect(int id) : RenderableEffect(id, "Fireworks")
{
    //ctor
}

FireworksEffect::~FireworksEffect()
{
    //dtor
}

wxPanel *FireworksEffect::CreatePanel(wxWindow *parent) {
    return new FireworksPanel(parent);
}

//Max of 50 explosions * 100 particals
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
    wxImage::HSVValue _hsv;
    int startPeriod;
    
    void Reset(int x, int y, bool active, float velocity, wxImage::HSVValue hsv, int start)
    {
        _x       = x;
        _y       = y;
        vel      = (rand()-RAND_MAX/2)*velocity/(RAND_MAX/2);
        angle    = 2*M_PI*rand()/RAND_MAX;
        _dx      = vel*cos(angle);
        _dy      = vel*sin(angle);
        _bActive = false;
        _cycles  = 0;
        _hsv     = hsv;
        startPeriod = start;
    }
protected:
private:
};

class FireworksRenderCache : public EffectRenderCache {
public:
    FireworksRenderCache() : fireworkBursts(maxFlakes) {};
    virtual ~FireworksRenderCache() {};
    
    std::vector<RgbFireworks> fireworkBursts;
};


void FireworksEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    int Number_Explosions = SettingsMap.GetInt("SLIDER_Fireworks_Explosions", 0);
    int Count = SettingsMap.GetInt("SLIDER_Fireworks_Count", 0);
    float Velocity = SettingsMap.GetDouble("SLIDER_Fireworks_Velocity", 0.0f);
    int Fade = SettingsMap.GetInt("SLIDER_Fireworks_Fade", 0);

    FireworksRenderCache *cache = (FireworksRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new FireworksRenderCache();
        buffer.infoCache[id] = cache;
    }
    
    int i=0;
    int x25,x75,y25,y75;
    //float velocity = 3.5;
    int startX;
    int startY,ColorIdx;
    float v;
    wxImage::HSVValue hsv;
    size_t colorcnt = buffer.GetColorCount();
    
    if (buffer.needToInit) {
        buffer.needToInit = false;
        for(i=0; i<maxFlakes; i++) {
            cache->fireworkBursts[i]._bActive = false;
        }
        for (int x = 0; x < Number_Explosions; x++) {
            double start = buffer.curEffStartPer + buffer.rand01() * (buffer.curEffEndPer - buffer.curEffStartPer);
            
            x25=(int)buffer.BufferWi*0.25;
            x75=(int)buffer.BufferWi*0.75;
            y25=(int)buffer.BufferHt*0.25;
            y75=(int)buffer.BufferHt*0.75;
            startX=(int)buffer.BufferWi/2;
            startY=(int)buffer.BufferHt/2;
            if((x75-x25)>0) startX = x25 + rand()%(x75-x25); else startX=0;
            if((y75-y25)>0) startY = y25 + rand()%(y75-y25); else startY=0;
            
            // Create a new burst
            ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
            buffer.palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
            for(i=0; i<Count; i++) {
                cache->fireworkBursts[x * Count + i].Reset(startX, startY, false, Velocity, hsv, start);
            }
        }
    }
    
    for (i=0; i<(Count*Number_Explosions); i++) {
        if (cache->fireworkBursts[i].startPeriod == buffer.curPeriod) {
            cache->fireworkBursts[i]._bActive = true;
        }
        
        // ... active flakes:
        if (cache->fireworkBursts[i]._bActive)
        {
            // Update position
            cache->fireworkBursts[i]._x += cache->fireworkBursts[i]._dx;
            cache->fireworkBursts[i]._y += (-cache->fireworkBursts[i]._dy - cache->fireworkBursts[i]._cycles*cache->fireworkBursts[i]._cycles/10000000.0);
            // If this flake run for more than maxCycle or this flake is out of bounds, time to switch it off
            cache->fireworkBursts[i]._cycles+=20;
            if (cache->fireworkBursts[i]._cycles >= 10000 || cache->fireworkBursts[i]._y >= buffer.BufferHt ||
                cache->fireworkBursts[i]._x < 0. || cache->fireworkBursts[i]._x >= buffer.BufferWi)
            {
                cache->fireworkBursts[i]._bActive = false;
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