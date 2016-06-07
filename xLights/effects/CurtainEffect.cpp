#include "CurtainEffect.h"
#include "CurtainPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/curtain-16.xpm"
#include "../../include/curtain-24.xpm"
#include "../../include/curtain-32.xpm"
#include "../../include/curtain-48.xpm"
#include "../../include/curtain-64.xpm"

CurtainEffect::CurtainEffect(int i) : RenderableEffect(i, "Curtain", curtain_16, curtain_24, curtain_32, curtain_48, curtain_64)
{
    //ctor
}

CurtainEffect::~CurtainEffect()
{
    //dtor
}
wxPanel *CurtainEffect::CreatePanel(wxWindow *parent) {
    return new CurtainPanel(parent);
}




static inline int GetCurtainEdge(const std::string &edge) {
    if ("left" == edge) {
        return 0;
    } else if ("center" == edge) {
        return 1;
    } else if ("right" == edge) {
        return 2;
    } else if ("bottom" == edge) {
        return 3;
    } else if ("middle" == edge) {
        return 4;
    } else if ("top" == edge) {
        return 5;
    }
    return 0;
}
static inline int GetCurtainEffect(const std::string &effect) {
    if ("open" == effect) {
        return 0;
    } else if ("close" == effect) {
        return 1;
    } else if ("open then close" == effect) {
        return 2;
    } else if ("close then open" == effect) {
        return 3;
    }
    return 0;
}

typedef enum
{
    //effect: 0=open, 1=close, 2=open then close, 3=close then open
    E_CURTAIN_OPEN =0,
    E_CURTAIN_CLOSE,
    E_CURTAIN_OPEN_CLOSE,
    E_CURTAIN_CLOSE_OPEN
} CURTAIN_EFFECT_e;

class CurtainRenderCache : public EffectRenderCache {
public:
    CurtainRenderCache() : LastCurtainDir(0), LastCurtainLimit(0)  {};
    virtual ~CurtainRenderCache() {};
    
    int LastCurtainDir;
    int LastCurtainLimit;
};


void CurtainEffect::Render(Effect *eff, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    
    float oset = buffer.GetEffectTimeIntervalPosition();
    int swag = GetValueCurveInt("Curtain_Swag", 0, SettingsMap, oset);
    float curtainSpeed = GetValueCurveDouble("Curtain_Speed", 1.0, SettingsMap, oset);

    bool repeat = SettingsMap.GetBool("CHECKBOX_Curtain_Repeat");
    int edge = GetCurtainEdge(SettingsMap["CHOICE_Curtain_Edge"]);
    int effect = GetCurtainEffect(SettingsMap["CHOICE_Curtain_Effect"]);
    
    double a;
    std::vector<int> SwagArray;
    int CurtainDir,xlimit,middle, ylimit;
    int swaglen=buffer.BufferHt > 1 ? swag * buffer.BufferWi / 40 : 0;
    
    if (swaglen > 0) {
        a=double(buffer.BufferHt - 1) / (swaglen * swaglen);
        for (int x=0; x<swaglen; x++) {
            SwagArray.push_back(int(a*x*x));
        }
    }
    
    double position = buffer.GetEffectTimeIntervalPosition(curtainSpeed);
    if (!repeat) {
        position = buffer.GetEffectTimeIntervalPosition() * curtainSpeed;
        if (position > 1.0) {
            position = 1.0;
        }
    }
    CurtainRenderCache *cache = (CurtainRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new CurtainRenderCache();
        buffer.infoCache[id] = cache;
    }
    
    if (effect < E_CURTAIN_OPEN_CLOSE) {
        xlimit=position * buffer.BufferWi;
        ylimit=position * buffer.BufferHt;
    } else {
        xlimit= position <= .5 ? position * 2 * buffer.BufferWi: (position -.5) * 2 * buffer.BufferWi ;
        ylimit= position <= .5 ? position * 2 * buffer.BufferHt: (position -.5) * 2 * buffer.BufferHt ;
    }
    if (buffer.curPeriod == buffer.curEffStartPer || effect < E_CURTAIN_OPEN_CLOSE) {
        CurtainDir=effect % 2;
    } else if (xlimit < cache->LastCurtainLimit) {
        CurtainDir=1-cache->LastCurtainDir;
    } else {
        CurtainDir=cache->LastCurtainDir;
    }
    cache->LastCurtainDir=CurtainDir;
    cache->LastCurtainLimit=xlimit;
    if (CurtainDir==0)
    {
        xlimit=buffer.BufferWi-xlimit-1;
        ylimit=buffer.BufferHt-ylimit-1;
    }
    switch (edge) {
        case 0:
            // left
            DrawCurtain(buffer, true,xlimit,SwagArray);
            break;
        case 1:
            // center
            middle=(xlimit+1)/2;
            DrawCurtain(buffer, true,middle,SwagArray);
            DrawCurtain(buffer, false,middle,SwagArray);
            break;
        case 2:
            // right
            DrawCurtain(buffer, false,xlimit,SwagArray);
            break;
        case 3:
            DrawCurtainVertical(buffer, true, ylimit, SwagArray);
            break;
        case 4:
            middle=(ylimit+1)/2;
            DrawCurtainVertical(buffer, true, middle, SwagArray);
            DrawCurtainVertical(buffer, false, middle, SwagArray);
            break;
        case 5:
            DrawCurtainVertical(buffer, false, ylimit, SwagArray);
            break;
    }

}


void CurtainEffect::DrawCurtain(RenderBuffer & buffer, bool LeftEdge, int xlimit, const std::vector<int> &SwagArray)
{
    int i,x,y;
    xlColor color;
    for (i=0; i<xlimit; i++)
    {
        buffer.GetMultiColorBlend(double(i) / double(buffer.BufferWi), true, color);
        x=LeftEdge ? buffer.BufferWi-i-1 : i;
        for (y=buffer.BufferHt-1; y>=0; y--)
        {
            buffer.SetPixel(x,y,color);
        }
    }
    
    // swag
    for (i=0; i<SwagArray.size(); i++)
    {
        x=xlimit+i;
        buffer.GetMultiColorBlend(double(x) / double(buffer.BufferWi), true, color);
        if (LeftEdge) x=buffer.BufferWi-x-1;
        for (y=buffer.BufferHt-1; y>SwagArray[i]; y--)
        {
            buffer.SetPixel(x,y,color);
        }
    }
}

void CurtainEffect::DrawCurtainVertical(RenderBuffer & buffer, bool topEdge, int ylimit, const std::vector<int> &SwagArray)
{
    int x,y;
    xlColor color;
    for (int i=0; i<ylimit; i++)
    {
        buffer.GetMultiColorBlend(double(i) / double(buffer.BufferHt), true, color);
        y=topEdge ? buffer.BufferHt-i-1 : i;
        for (x=buffer.BufferWi-1; x>=0; x--)
        {
            buffer.SetPixel(x,y,color);
        }
    }
    
    // swag
    for (int i=0; i<SwagArray.size(); i++)
    {
        y=ylimit+i;
        buffer.GetMultiColorBlend(double(y) / double(buffer.BufferHt), true, color);
        if (topEdge) y=buffer.BufferHt-y-1;
        for (x=buffer.BufferWi-1; x>SwagArray[i];x--)
        {
            buffer.SetPixel(x,y,color);
        }
    }
}
