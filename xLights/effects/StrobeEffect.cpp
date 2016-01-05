#include "StrobeEffect.h"
#include "StrobePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

StrobeEffect::StrobeEffect(int id) : RenderableEffect(id, "Strobe")
{
    //ctor
}

StrobeEffect::~StrobeEffect()
{
    //dtor
}

wxPanel *StrobeEffect::CreatePanel(wxWindow *parent) {
    return new StrobePanel(parent);
}


class StrobeClass
{
public:
    
    int x,y;
    int duration; // How frames strobe light stays on. Will be decremented each frame
    wxImage::HSVValue hsv;
    xlColor color;
};

class StrobeRenderCache : public EffectRenderCache {
public:
    StrobeRenderCache() {};
    virtual ~StrobeRenderCache() {};
    
    std::vector<StrobeClass> strobe;
};


void StrobeEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    int Number_Strobes = SettingsMap.GetInt("SLIDER_Number_Strobes", 1);
    int StrobeDuration = SettingsMap.GetInt("SLIDER_Strobe_Duration", 1);
    int Strobe_Type = SettingsMap.GetInt("SLIDER_Strobe_Type", 0);
    
    StrobeRenderCache *cache = (StrobeRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new StrobeRenderCache();
        buffer.infoCache[id] = cache;
    }
    std::vector<StrobeClass> &strobe = cache->strobe;

    
    if (StrobeDuration == 0) {
        StrobeDuration = 1;
    }
    if (buffer.needToInit) {
        buffer.needToInit = false;
        strobe.resize(StrobeDuration * Number_Strobes);
        for (int x = 0; x < strobe.size(); x++) {
            strobe[x].duration = -1;
        }
    }
    int offset = ((buffer.curPeriod-buffer.curEffStartPer) % StrobeDuration) * Number_Strobes;
    
    int ColorIdx;
    StrobeClass m;
    wxImage::HSVValue hsv;
    size_t colorcnt=buffer.GetColorCount();
    
    // create new strobe, randomly place a strobe
    
    for(int i = 0; i < Number_Strobes; i++)
    {
        strobe[offset + i].x =rand() % buffer.BufferWi; // randomly pick a x,y location for strobe to fire
        strobe[offset + i].y =rand() % buffer.BufferHt;
        strobe[offset + i].duration = StrobeDuration;
        
        ColorIdx=rand()%colorcnt;
        buffer.palette.GetHSV(ColorIdx, strobe[offset + i].hsv); // take first checked color as color of flash
        buffer.palette.GetColor(ColorIdx, strobe[offset + i].color); // take first checked color as color of flash
    }
    
    // render strobe, we go through all storbes and decide if they should be turned on
    
    int x,y,n=0;
    
    for (std::vector<StrobeClass>::iterator it=strobe.begin(); it!=strobe.end(); ++it) {
        n++;
        hsv=it->hsv;
        xlColor color(it->color);
        x=it->x;
        y=it->y;
        if(it->duration > 0)
        {
            buffer.SetPixel(x,y,color);
        }
        
        double v = 1.0;
        if(it->duration==1)
        {
            v = 0.5;
        }
        else if(it->duration==2)
        {
            v = 0.75;
        }
        if (buffer.allowAlpha) {
            color.alpha = 255.0 * v;
        } else {
            hsv.value *= v;
            color = hsv;
        }
        
        if(Strobe_Type==2)
        {
            int r = rand()%2;
            if(r==0)
            {
                buffer.SetPixel(x,y-1,color);
                buffer.SetPixel(x,y+1,color);
            }
            else
            {
                buffer.SetPixel(x-1,y,color);
                buffer.SetPixel(x+1,y,color);
            }
            
        }
        if(Strobe_Type==3)
        {
            buffer.SetPixel(x,y-1,color);
            buffer.SetPixel(x,y+1,color);
            buffer.SetPixel(x-1,y,color);
            buffer.SetPixel(x+1,y,color);
        }
        if(Strobe_Type==4)
        {
            int r = rand()%2;
            if(r==0)
            {
                buffer.SetPixel(x,y-1,color);
                buffer.SetPixel(x,y+1,color);
                buffer.SetPixel(x-1,y,color);
                buffer.SetPixel(x+1,y,color);
            }
            else
            {
                buffer.SetPixel(x+1,y-1,color);
                buffer.SetPixel(x+1,y+1,color);
                buffer.SetPixel(x-1,y-1,color);
                buffer.SetPixel(x-1,y+1,color);
            }
        }
        
        it->duration--;  // decrease the frame counter on this strobe, when it gets to zero we no longer will turn it on
    }

}