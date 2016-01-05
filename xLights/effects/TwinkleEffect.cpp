#include "TwinkleEffect.h"
#include "TwinklePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

TwinkleEffect::TwinkleEffect(int id) : RenderableEffect(id, "Twinkle")
{
    //ctor
}

TwinkleEffect::~TwinkleEffect()
{
    //dtor
}
wxPanel *TwinkleEffect::CreatePanel(wxWindow *parent) {
    return new TwinklePanel(parent);
}

class StrobeClass
{
public:
    
    int x,y;
    int duration; // How frames strobe light stays on. Will be decremented each frame
    wxImage::HSVValue hsv;
    xlColor color;
};

class TwinkleRenderCache : public EffectRenderCache {
public:
    TwinkleRenderCache() {};
    virtual ~TwinkleRenderCache() {};
    
    std::vector<StrobeClass> strobe;
};


void TwinkleEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    int Count = SettingsMap.GetInt("SLIDER_Twinkle_Count", 1);
    int Steps = SettingsMap.GetInt("SLIDER_Twinkle_Steps", 1);
    bool Strobe = SettingsMap.GetBool("CHECKBOX_Twinkle_Strobe");
    bool reRandomize = SettingsMap.GetBool("CHECKBOX_Twinkle_ReRandom");
    
    int i,ColorIdx;
    int lights = (buffer.BufferHt*buffer.BufferWi)*(Count/100.0); // Count is in range of 1-100 from slider bar
    int step;
    if(lights>0) step=buffer.BufferHt*buffer.BufferWi/lights;
    else step=1;
    int max_modulo;
    max_modulo=Steps;
    if(max_modulo<2) max_modulo=2;  // scm  could we be getting 0 passed in?
    int max_modulo2=max_modulo/2;
    if(max_modulo2<1) max_modulo2=1;
    
    if(step<1) step=1;
    
    
    
    TwinkleRenderCache *cache = (TwinkleRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new TwinkleRenderCache();
        buffer.infoCache[id] = cache;
    }
    std::vector<StrobeClass> &strobe = cache->strobe;
    
    size_t colorcnt=buffer.GetColorCount();
    i = 0;
    if (buffer.needToInit) {
        buffer.needToInit = false;
        strobe.clear();
        for (int y=0; y < buffer.BufferHt; y++) {
            for (int x=0; x < buffer.BufferWi; x++) {
                i++;
                if (i%step==1 || step==1) {
                    int s = strobe.size();
                    strobe.resize(s + 1);
                    strobe[s].duration = rand() % max_modulo;
                    
                    strobe[s].x = x;
                    strobe[s].y = y;
                    
                    ColorIdx=rand()%colorcnt;
                    buffer.palette.GetHSV(ColorIdx, strobe[s].hsv);
                    buffer.palette.GetColor(ColorIdx, strobe[s].color);
                }
            }
        }
    }
    
    for (int x = 0; x < strobe.size(); x++) {
        strobe[x].duration++;
        if (strobe[x].duration < 0) {
            continue;
        }
        if (strobe[x].duration == max_modulo) {
            strobe[x].duration = 0;
            if (reRandomize) {
                strobe[x].duration -= rand() % max_modulo2;
                ColorIdx=rand()%colorcnt;
                buffer.palette.GetHSV(ColorIdx, strobe[x].hsv);
                buffer.palette.GetColor(ColorIdx, strobe[x].color);
            }
        }
        int i7 = strobe[x].duration;
        double v = strobe[x].hsv.value;
        if(i7<=max_modulo2)
        {
            if(max_modulo2>0) v = (1.0*i7)/max_modulo2;
            else v =0;
        }
        else
        {
            if(max_modulo2>0)v = (max_modulo-i7)*1.0/(max_modulo2);
            else v = 0;
        }
        if(v<0.0) v=0.0;
        
        if(Strobe)
        {
            if(i7==max_modulo2)  v = 1.0;
            else v = 0.0;
        }
        if (buffer.allowAlpha) {
            xlColor color(strobe[x].color);
            color.alpha = 255.0 * v;
            buffer.SetPixel(strobe[x].x,strobe[x].y,color); // Turn pixel on
        } else {
            wxImage::HSVValue hsv = strobe[x].hsv;
            //  we left the Hue and Saturation alone, we are just modifiying the Brightness Value
            hsv.value = v;
            buffer.SetPixel(strobe[x].x,strobe[x].y,hsv); // Turn pixel on
        }
    }
}