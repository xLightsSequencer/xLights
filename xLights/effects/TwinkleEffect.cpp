#include "TwinkleEffect.h"
#include "TwinklePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/twinkle-16.xpm"
#include "../../include/twinkle-24.xpm"
#include "../../include/twinkle-32.xpm"
#include "../../include/twinkle-48.xpm"
#include "../../include/twinkle-64.xpm"
#include "ValueCurve.h"


TwinkleEffect::TwinkleEffect(int id) : RenderableEffect(id, "Twinkle", twinkle_16, twinkle_24, twinkle_32, twinkle_48, twinkle_64)
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
    int colorindex;
};

class TwinkleRenderCache : public EffectRenderCache {
public:
    TwinkleRenderCache() {};
    virtual ~TwinkleRenderCache() {};
    
    std::vector<StrobeClass> strobe;
};

void TwinkleEffect::SetDefaultParameters(Model *cls)
{
    TwinklePanel *tp = (TwinklePanel*)panel;
    if (tp == nullptr) {
        return;
    }

    SetSliderValue(tp->Slider_Twinkle_Count, 3);
    SetSliderValue(tp->Slider_Twinkle_Steps, 30);
    SetCheckBoxValue(tp->CheckBox_Twinkle_Strobe, false);
    SetCheckBoxValue(tp->CheckBox_Twinkle_ReRandom, false);
}

int TwinkleEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
    DrawGLUtils::xlAccumulator &bg, xlColor* colorMask, bool ramp)
{
    if (ramp)
    {
        float endi;
        float starti;
        std::string vcs = e->GetPaletteMap().Get("C_VALUECURVE_Brightness", "");
        if (vcs == "")
        {
            starti = e->GetSettings().GetInt("C_SLIDER_Brightness", 100);
            endi = starti;
        }
        else
        {
            ValueCurve vc(vcs);
            starti = vc.GetOutputValueAt(0.0);
            endi = vc.GetOutputValueAt(1.0);
        }

        xlColor color = e->GetPalette()[0];
        color.ApplyMask(colorMask);

        int height = y2 - y1;
        float starty = (float)y2 - starti * (float)height / 100.0f;
        float endy = (float)y2 - endi * (float)height / 100.0f;
        float m = float(endy - starty) / float(x2 - x1);

        const int gap = 10;

        bg.Finish(GL_TRIANGLES);
        bg.AddVertex(x1, starty, color);
        bg.AddVertex(x1, y2, color);
        bg.AddVertex(x1, y2, color);
        bg.AddVertex(x2, y2, color);
        bg.AddVertex(x2, y2, color);
        bg.AddVertex(x2, endy, color);
        bg.AddVertex(x2, endy, color);
        bg.AddVertex(x1, starty, color);

        int lastx = x1;
        for (int x = x1 + gap; x < x2; x += gap) {
            float newY = m * (x - x1 - gap) + starty;
            float newY2 = m * (x - x1) + starty;
            bg.AddVertex(x-gap, newY, color);
            bg.AddVertex(x, y2, color);
            bg.AddVertex(x, newY2, color);
            bg.AddVertex(x-gap, y2, color);
            lastx = x;

            // draw a second outline
            float ylow1 = y2 + 1;
            float ylow2 = y2 + 1;
            if (ylow1 > newY) ylow1 = newY;
            if (ylow2 > newY2) ylow2 = newY2;
            bg.AddVertex(x - gap, ylow1, color);
            bg.AddVertex(x, ylow2, color);

            float yhigh1 = newY - 1;
            float yhigh2 = newY2 - 1;
            if (yhigh1 < y2) yhigh1 = y2;
            if (yhigh2 < y2) yhigh2 = y2;
            bg.AddVertex(x - gap, yhigh1, color);
            bg.AddVertex(x, yhigh2, color);
        }

        // fill in the end
        if (lastx != x2)
        {
            float newY = m * (lastx - x1) + starty;
            float newY2 = m * (x2-x1) + starty;
            bg.AddVertex(lastx, newY, color);
            bg.AddVertex(x2, y2, color);
            bg.AddVertex(x2, newY2, color);
            bg.AddVertex(lastx, y2, color);

            // draw a second outline
            float ylow1 = y2 + 1;
            float ylow2 = y2 + 1;
            if (ylow1 > newY) ylow1 = newY;
            if (ylow2 > newY2) ylow2 = newY2;
            bg.AddVertex(lastx, ylow1, color);
            bg.AddVertex(x2, ylow2, color);

            float yhigh1 = newY - 1;
            float yhigh2 = newY2 - 1;
            if (yhigh1 < y2) yhigh1 = y2;
            if (yhigh2 < y2) yhigh2 = y2;
            bg.AddVertex(lastx, yhigh1, color);
            bg.AddVertex(x2, yhigh2, color);
        }

        bg.Finish(GL_LINES);

        return 2;
    }

    return 1;
}

void TwinkleEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    
    int Count = SettingsMap.GetInt("SLIDER_Twinkle_Count", 3);
    int Steps = SettingsMap.GetInt("SLIDER_Twinkle_Steps", 30);
    bool Strobe = SettingsMap.GetBool("CHECKBOX_Twinkle_Strobe", false);
    bool reRandomize = SettingsMap.GetBool("CHECKBOX_Twinkle_ReRandom", false);
    
    int lights = (buffer.BufferHt*buffer.BufferWi)*(Count / 100.0); // Count is in range of 1-100 from slider bar
    int step = 1;
    if (lights > 0) {
        step = buffer.BufferHt*buffer.BufferWi / lights;
    }
    int max_modulo = Steps;
    if (max_modulo<2) 
        max_modulo = 2;  // scm  could we be getting 0 passed in?
    int max_modulo2 = max_modulo / 2;
    if (max_modulo2<1) 
        max_modulo2 = 1;
    
    if(step<1) 
        step=1;
    
    TwinkleRenderCache *cache = (TwinkleRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new TwinkleRenderCache();
        buffer.infoCache[id] = cache;
    }
    std::vector<StrobeClass> &strobe = cache->strobe;
    
    size_t colorcnt=buffer.GetColorCount();

    int i = 0;

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
                    
                    strobe[s].colorindex = rand() % colorcnt;
                }
            }
        }
    }
    
    for (size_t x = 0; x < strobe.size(); x++) {
        strobe[x].duration++;
        if (strobe[x].duration < 0) {
            continue;
        }
        if (strobe[x].duration == max_modulo) {
            strobe[x].duration = 0;
            if (reRandomize) {
                strobe[x].duration -= rand() % max_modulo2;
                strobe[x].colorindex = rand() % colorcnt;
            }
        }
        int i7 = strobe[x].duration;
        HSVValue hsv;
        buffer.palette.GetHSV(strobe[x].colorindex, hsv);
        double v = hsv.value;
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
            xlColor color;
            buffer.palette.GetColor(strobe[x].colorindex, color);
            color.alpha = 255.0 * v;
            buffer.SetPixel(strobe[x].x,strobe[x].y,color); // Turn pixel on
        } else {
            buffer.palette.GetHSV(strobe[x].colorindex, hsv);
            //  we left the Hue and Saturation alone, we are just modifiying the Brightness Value
            hsv.value = v;
            buffer.SetPixel(strobe[x].x,strobe[x].y,hsv); // Turn pixel on
        }
    }
}
