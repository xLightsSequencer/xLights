#include "OnEffect.h"
#include "OnPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
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



void GetOnEffectColors(const Effect *e, xlColor &start, xlColor &end) {
    int starti = e->GetSettings().GetInt("E_TEXTCTRL_Eff_On_Start", 100);
    int endi = e->GetSettings().GetInt("E_TEXTCTRL_Eff_On_End", 100);
    xlColor newcolor;
    newcolor = e->GetPalette()[0];
    if (starti == 100 && endi == 100) {
        start = end = newcolor;
    } else {
        wxImage::HSVValue hsv = newcolor.asHSV();
        hsv.value = (hsv.value * starti) / 100;
        start = hsv;
        hsv = newcolor.asHSV();
        hsv.value = (hsv.value * endi) / 100;
        end = hsv;
    }
}
int OnEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2) {
    if (e->HasBackgroundDisplayList()) {
        DrawGLUtils::DrawDisplayList(x1, y1, x2-x1, y2-y1, e->GetBackgroundDisplayList());
        return e->GetBackgroundDisplayList().iconSize;
    }
    xlColor start;
    xlColor end;
    GetOnEffectColors(e, start, end);
    DrawGLUtils::DrawHBlendedRectangle(start, end, x1, y1, x2, y2);
    return 2;
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