#include "OnEffect.h"
#include "OnPanel.h"

#include "../sequencer/Effect.h"
#include "../RgbEffects.h"
#include "../UtilClasses.h"

static const std::string TEXTCTRL_Eff_On_Start("TEXTCTRL_Eff_On_Start");
static const std::string TEXTCTRL_Eff_On_End("TEXTCTRL_Eff_On_End");
static const std::string CHECKBOX_On_Shimmer("CHECKBOX_On_Shimmer");
static const std::string TEXTCTRL_On_Cycles("TEXTCTRL_On_Cycles");


OnEffect::OnEffect(int i) : RenderableEffect(i, "On")
{
    //ctor
}

OnEffect::~OnEffect()
{
    //dtor
}

wxPanel *OnEffect::CreatePanel(wxWindow *parent) {
    return new OnPanel(parent);
}

void OnEffect::Render(Effect *eff, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    int start = SettingsMap.GetInt(TEXTCTRL_Eff_On_Start, 100);
    int end = SettingsMap.GetInt(TEXTCTRL_Eff_On_End, 100);
    bool shimmer = SettingsMap.GetInt(CHECKBOX_On_Shimmer, 0);
    float cycles = SettingsMap.GetDouble(TEXTCTRL_On_Cycles, 1.0);
    int x,y;
    int cidx = 0;
    if (shimmer) {
        int tot = buffer.curPeriod - buffer.curEffStartPer;
        if (tot % 2) {
            if (buffer.palette.Size() <= 1) {
                return;
            }
            cidx = 1;
        }
    }
    
    double adjust = buffer.GetEffectTimeIntervalPosition(cycles);
    
    xlColor color;
    if (start == 100 && end == 100) {
        buffer.palette.GetColor(cidx, color);
    } else {
        wxImage::HSVValue hsv;
        buffer.palette.GetHSV(cidx,hsv);
        double d = adjust;
        d = start + (end - start) * d;
        d = d / 100.0;
        hsv.value = hsv.value * d;
        color = hsv;
    }
    
    //Every Node set to selected color
    for (x=0; x<buffer.BufferWi; x++)
    {
        for (y=0; y<buffer.BufferHt; y++)
        {
            buffer.SetPixel(x,y,color);
        }
    }
    if (shimmer || cycles != 1.0) {
        wxMutexLocker lock(eff->GetBackgroundDisplayList().lock);
        eff->GetBackgroundDisplayList().resize((buffer.curEffEndPer - buffer.curEffStartPer + 1) * 4);
        buffer.CopyPixelsToDisplayListX(eff, 0, 0, 0);
    } else if (buffer.needToInit) {
        wxMutexLocker lock(eff->GetBackgroundDisplayList().lock);
        eff->GetBackgroundDisplayList().resize(4);
        if (start == 100 && end == 100) {
            buffer.palette.GetColor(0, color);
            buffer.SetDisplayListHRect(eff, 0, 0.0, 0.0, 1.0, 1.0, color, color);
        } else {
            wxImage::HSVValue hsv;
            buffer.palette.GetHSV(cidx,hsv);
            hsv.value = hsv.value * start / 100.0;
            color = hsv;
            
            buffer.palette.GetHSV(cidx,hsv);
            hsv.value = hsv.value * end / 100.0;
            buffer.SetDisplayListHRect(eff, 0, 0.0, 0.0, 1.0, 1.0, color, xlColor(hsv));
        }
        buffer.needToInit = false;
    }
}