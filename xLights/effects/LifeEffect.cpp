#include "LifeEffect.h"
#include "LifePanel.h"


#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/life-16.xpm"
#include "../../include/life-24.xpm"
#include "../../include/life-32.xpm"
#include "../../include/life-48.xpm"
#include "../../include/life-64.xpm"
#include "../UtilFunctions.h"

LifeEffect::LifeEffect(int id) : RenderableEffect(id, "Life", life_16, life_24, life_32, life_48, life_48)
{
    //ctor
}

LifeEffect::~LifeEffect()
{
    //dtor
}


wxPanel *LifeEffect::CreatePanel(wxWindow *parent) {
    return new LifePanel(parent);
}

bool LifeEffect::SetLifePixel(RenderBuffer& buffer, int &x, int &y, xlColor &color)
{
    ///////////////////////////////////////////// DMX Model Support //////////////////////////////////////////
    // if the model is a DMX model this will write the color into the proper red, green, and blue channels. //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (RenderDMXModel(buffer, color)) {
        return true;
    }

    buffer.SetPixel(x, y, color);
    return false;
}

static int Life_CountNeighbors(RenderBuffer &buffer, int x0, int y0)
{
    //     2   3   4
    //     1   X   5
    //     0   7   6
    static int n_x[] = {-1,-1,-1,0,1,1,1,0};
    static int n_y[] = {-1,0,1,1,1,0,-1,-1};
    int x,y,cnt=0;
    for (int i=0; i < 8; i++)
    {
        x=(x0+n_x[i]) % buffer.BufferWi;
        y=(y0+n_y[i]) % buffer.BufferHt;
        if (x < 0) x+=buffer.BufferWi;
        if (y < 0) y+=buffer.BufferHt;
        if (buffer.GetTempPixelRGB(x,y) != xlBLACK) cnt++;
    }
    return cnt;
}

class LifeRenderCache : public EffectRenderCache {
public:
    LifeRenderCache() : LastLifeCount(0), LastLifeType(0), LastLifeState(0) {};
    virtual ~LifeRenderCache() {};
    int LastLifeCount;
    int LastLifeType;
    int LastLifeState;
};

void LifeEffect::SetDefaultParameters() {
    LifePanel *lp = (LifePanel*)panel;
    if (lp == nullptr) {
        return;
    }

    SetSliderValue(lp->Slider_Life_Count, 50);
    SetSliderValue(lp->Slider_Life_Seed, 0);
    SetSliderValue(lp->Slider_Life_Speed, 10);
}

void LifeEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    int Count = SettingsMap.GetInt("SLIDER_Life_Count", 50);
    int Type = SettingsMap.GetInt("SLIDER_Life_Seed", 0);
    int lspeed = SettingsMap.GetInt("SLIDER_Life_Speed", 10);
        
    LifeRenderCache *cache = (LifeRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new LifeRenderCache();
        buffer.infoCache[id] = cache;
    }
    
    int i,x,y,cnt;
    bool isLive;
    xlColor color;
    
    int BufferHt = buffer.BufferHt;
    int BufferWi = buffer.BufferWi;
    
    if(BufferHt<1) BufferHt = 1;
    Count=BufferWi * BufferHt * Count / 200 + 1;
    if (buffer.needToInit || Count != cache->LastLifeCount || Type != cache->LastLifeType)
    {
        buffer.needToInit = false;
        // seed tempbuf
        cache->LastLifeCount=Count;
        cache->LastLifeType=Type;
        buffer.ClearTempBuf();
        for(i=0; i<Count; i++)
        {
            x=rand() % BufferWi;
            y=rand() % BufferHt;
            buffer.GetMultiColorBlend(rand01(),false,color);
            buffer.SetTempPixel(x,y,color);
        }
    }
    int effectState = (buffer.curPeriod-buffer.curEffStartPer) * lspeed * buffer.frameTimeInMs / 50;
    
    long TempState=effectState % 400 / 20;
    if (TempState == cache->LastLifeState)
    {
        if (!SupportsDMXModel(buffer)) {
            buffer.pixels = buffer.tempbuf;
        }   
        return;
    }
    else
    {
        cache->LastLifeState=TempState;
    }
    for (x=0; x < BufferWi; x++)
    {
        for (y=0; y < BufferHt; y++)
        {
            buffer.GetTempPixel(x,y,color);
            isLive = color != xlBLACK;
            cnt=Life_CountNeighbors(buffer, x, y);
            switch (Type)
            {
                case 0:
                    // B3/S23
                    /*
                     Any live cell with fewer than two live neighbours dies, as if caused by under-population.
                     Any live cell with two or three live neighbours lives on to the next generation.
                     Any live cell with more than three live neighbours dies, as if by overcrowding.
                     Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
                     */
                    if (isLive && cnt >= 2 && cnt <= 3)
                    {
                        if (SetLifePixel(buffer, x, y, color)) return;  // exit function if a DMX Model
                    }
                    else if (!isLive && cnt == 3)
                    {
                        buffer.GetMultiColorBlend(rand01(),false,color);
                        if (SetLifePixel(buffer, x, y, color)) return;  // exit function if a DMX Model
                    }
                    break;
                case 1:
                    // B35/S236
                    if (isLive && (cnt == 2 || cnt == 3 || cnt == 6))
                    {
                        if (SetLifePixel(buffer, x, y, color)) return;  // exit function if a DMX Model
                    }
                    else if (!isLive && (cnt == 3 || cnt == 5))
                    {
                        buffer.GetMultiColorBlend(rand01(),false,color);
                        if (SetLifePixel(buffer, x, y, color)) return;  // exit function if a DMX Model
                    }
                    break;
                case 2:
                    // B357/S1358
                    if (isLive && (cnt == 1 || cnt == 3 || cnt == 5 || cnt == 8))
                    {
                        if (SetLifePixel(buffer, x, y, color)) return;  // exit function if a DMX Model
                    }
                    else if (!isLive && (cnt == 3 || cnt == 5 || cnt == 7))
                    {
                        buffer.GetMultiColorBlend(rand01(),false,color);
                        if (SetLifePixel(buffer, x, y, color)) return;  // exit function if a DMX Model
                    }
                    break;
                case 3:
                    // B378/S235678
                    if (isLive && (cnt == 2 || cnt == 3 || cnt >= 5))
                    {
                        if (SetLifePixel(buffer, x, y, color)) return;  // exit function if a DMX Model
                    }
                    else if (!isLive && (cnt == 3 || cnt == 7 || cnt == 8))
                    {
                        buffer.GetMultiColorBlend(rand01(),false,color);
                        if (SetLifePixel(buffer, x, y, color)) return;  // exit function if a DMX Model
                    }
                    break;
                case 4:
                    // B25678/S5678
                    if (isLive && (cnt >= 5))
                    {
                        if (SetLifePixel(buffer, x, y, color)) return;  // exit function if a DMX Model
                    }
                    else if (!isLive && (cnt == 2 || cnt >= 5))
                    {
                        buffer.GetMultiColorBlend(rand01(),false,color);
                        if (SetLifePixel(buffer, x, y, color)) return;  // exit function if a DMX Model
                    }
                    break;
            }
        }
    }
    // copy new life state to tempbuf
    buffer.tempbuf=buffer.pixels;
}
