#include "MorphEffect.h"
#include "MorphPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "assist/AssistPanel.h"
#include "assist/xlGridCanvasMorph.h"
#include "../models/Model.h"

#include "../../include/morph-16.xpm"
#include "../../include/morph-64.xpm"
#include "../UtilFunctions.h"


MorphEffect::MorphEffect(int id) : RenderableEffect(id, "Morph", morph_16, morph_64, morph_64, morph_64, morph_64)
{
    //ctor
}

MorphEffect::~MorphEffect()
{
    //dtor
}

wxPanel *MorphEffect::CreatePanel(wxWindow *parent) {
    return new MorphPanel(parent);
}

AssistPanel *MorphEffect::GetAssistPanel(wxWindow *parent, xLightsFrame* xl_frame) {
    AssistPanel *assist_panel = new AssistPanel(parent);
    xlGridCanvas* grid = new xlGridCanvasMorph(assist_panel->GetCanvasParent(), wxNewId(), wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("MorphGrid"));
    assist_panel->SetGridCanvas(grid);
    return assist_panel;
}

std::list<std::string> MorphEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff)
{
    std::list<std::string> res;

    if (settings.Get("E_VALUECURVE_Morph_Start_X1", "").find("Active=TRUE") != std::string::npos ||
        settings.Get("E_VALUECURVE_Morph_Start_X2", "").find("Active=TRUE") != std::string::npos ||
        settings.Get("E_VALUECURVE_Morph_Start_Y1", "").find("Active=TRUE") != std::string::npos ||
        settings.Get("E_VALUECURVE_Morph_Start_Y2", "").find("Active=TRUE") != std::string::npos ||
        settings.Get("E_VALUECURVE_Morph_End_X1", "").find("Active=TRUE") != std::string::npos ||
        settings.Get("E_VALUECURVE_Morph_End_X2", "").find("Active=TRUE") != std::string::npos ||
        settings.Get("E_VALUECURVE_Morph_End_Y1", "").find("Active=TRUE") != std::string::npos ||
        settings.Get("E_VALUECURVE_Morph_End_Y2", "").find("Active=TRUE") != std::string::npos ||
        settings.Get("E_VALUECURVE_MorphRepeat_Count", "").find("Active=TRUE") != std::string::npos ||
        settings.Get("E_VALUECURVE_MorphRepeat_Skip", "").find("Active=TRUE") != std::string::npos
        )
    {
        // we cant validate a value curve
    }
    else
    {
        int startx = std::max(1,std::abs(settings.GetInt("E_SLIDER_Morph_Start_X1", 0) - settings.GetInt("E_SLIDER_Morph_Start_X2", 0)) * model->GetDefaultBufferWi() / 80);
        int endx = std::max(1, std::abs(settings.GetInt("E_SLIDER_Morph_End_X1", 0) - settings.GetInt("E_SLIDER_Morph_End_X2", 0)) * model->GetDefaultBufferWi() / 80);
        int starty = std::max(1, std::abs(settings.GetInt("E_SLIDER_Morph_Start_Y1", 0) - settings.GetInt("E_SLIDER_Morph_Start_Y2", 0)) * model->GetDefaultBufferWi() / 80);
        int endy = std::max(1, std::abs(settings.GetInt("E_SLIDER_Morph_End_Y1", 0) - settings.GetInt("E_SLIDER_Morph_End_Y2", 0)) * model->GetDefaultBufferWi() / 80);

        int minmorph = std::min(startx, std::min(starty, std::min(endx, endy)));
        int repeat_count = settings.GetInt("E_SLIDER_Morph_Repeat_Count", 0);
        int repeat_skip = settings.GetInt("E_SLIDER_Morph_Repeat_Skip", 0);
        int maxmodel = std::max(model->GetDefaultBufferWi(), model->GetDefaultBufferHt());

        if ((minmorph + repeat_skip) * repeat_count > 2 * maxmodel)
        {
            res.push_back(wxString::Format("    WARN: Morph effect with repeat count and skip which are larger than necessary. This may lead to slow render times. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
    }

    return res;
}

void MorphEffect::SetDefaultParameters() {
    MorphPanel *mp = (MorphPanel*)panel;
    if (mp == nullptr) {
        return;
    }

    mp->BitmapButton_MorphAccel->SetActive(false);
    mp->BitmapButton_MorphDuration->SetActive(false);
    mp->BitmapButton_MorphEndLength->SetActive(false);
    mp->BitmapButton_MorphStartLength->SetActive(false);
    mp->BitmapButton_Morph_End_X1->SetActive(false);
    mp->BitmapButton_Morph_End_Y1->SetActive(false);
    mp->BitmapButton_Morph_End_X2->SetActive(false);
    mp->BitmapButton_Morph_End_Y2->SetActive(false);
    mp->BitmapButton_Morph_Start_X1->SetActive(false);
    mp->BitmapButton_Morph_Start_Y1->SetActive(false);
    mp->BitmapButton_Morph_Start_X2->SetActive(false);
    mp->BitmapButton_Morph_Start_Y2->SetActive(false);
    mp->BitmapButton_Morph_Stagger->SetActive(false);
    mp->BitmapButton_Morph_Repeat_Count->SetActive(false);
    mp->BitmapButton_Morph_Repeat_Skip->SetActive(false);

    SetSliderValue(mp->Slider_MorphAccel, 0);
    SetSliderValue(mp->Slider_MorphDuration, 20);
    SetSliderValue(mp->Slider_MorphEndLength, 1);
    SetSliderValue(mp->Slider_MorphStartLength, 1);
    SetSliderValue(mp->Slider_Morph_End_X1, 0);
    SetSliderValue(mp->Slider_Morph_End_X2, 100);
    SetSliderValue(mp->Slider_Morph_End_Y1, 100);
    SetSliderValue(mp->Slider_Morph_End_Y2, 100);
    SetSliderValue(mp->Slider_Morph_Repeat_Count, 0);
    SetSliderValue(mp->Slider_Morph_Repeat_Skip, 1);
    SetSliderValue(mp->Slider_Morph_Stagger, 0);
    SetSliderValue(mp->Slider_Morph_Start_X1, 0);
    SetSliderValue(mp->Slider_Morph_Start_X2, 100);
    SetSliderValue(mp->Slider_Morph_Start_Y1, 0);
    SetSliderValue(mp->Slider_Morph_Start_Y2, 0);

    SetCheckBoxValue(mp->CheckBox_Morph_End_Link, false);
    SetCheckBoxValue(mp->CheckBox_Morph_Start_Link, false);
    SetCheckBoxValue(mp->CheckBox_ShowHeadAtStart, false);
}

void GetMorphEffectColors(const Effect *e, xlColor &start_h, xlColor &end_h, xlColor &start_t, xlColor &end_t) {
    int useHeadStart = e->GetSettings().GetInt("E_CHECKBOX_MorphUseHeadStartColor", 0);
    int useTailStart = e->GetSettings().GetInt("E_CHECKBOX_MorphUseHeadEndColor", 0);

    int hcols = 0, hcole = 1;
    int tcols = 2, tcole = 3;
    switch (e->GetPaletteSize()) {
        case 1:  //one color selected, use it for all
            hcols = hcole = tcols = tcole = 0;
            break;
        case 2: //two colors, head/tail
            hcols = hcole = 0;
            tcols = tcole = 1;
            break;
        case 3: //three colors, head /tail start/end
            hcols = hcole = 0;
            tcols = 1;
            tcole = 2;
            break;
    }

    if( useHeadStart > 0 )
    {
        tcols = hcols;
    }

    if( useTailStart > 0 )
    {
        tcole = hcole;
    }

    start_h = e->GetPalette()[hcols];
    end_h = e->GetPalette()[hcole];
    start_t = e->GetPalette()[tcols];
    end_t = e->GetPalette()[tcole];
}
int MorphEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2, DrawGLUtils::xlAccumulator &backgrounds, xlColor* colorMask, bool ramps) {
    int head_duration = e->GetSettings().GetInt("E_SLIDER_MorphDuration", 20);
    xlColor start_h;
    xlColor end_h;
    xlColor start_t;
    xlColor end_t;
    GetMorphEffectColors(e, start_h, end_h, start_t, end_t);
    start_h.ApplyMask(colorMask);
    end_h.ApplyMask(colorMask);
    start_t.ApplyMask(colorMask);
    end_t.ApplyMask(colorMask);
    int x_mid = (int)((float)(x2-x1) * (float)head_duration / 100.0) + x1;
    backgrounds.AddHBlendedRectangle(start_h, end_h, x1, y1+1, x_mid, y2-1);
    if(e->GetPaletteSize() <= 4) {
        backgrounds.AddHBlendedRectangle(start_t, end_t, x_mid, y1+4, x2, y2-4);
    }
    else {
        backgrounds.AddHBlendedRectangle(e->GetPalette(), x_mid, y1+4, x2, y2-4, colorMask, 2);
    }
    return 0;

}

static void StoreLine( const int x0_, const int y0_, const int x1_, const int y1_, std::vector<int> *vx,  std::vector<int> *vy)
{
    int x0 = x0_;
    int x1 = x1_;
    int y0 = y0_;
    int y1 = y1_;

    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;

    for(;;){
        vx->push_back(x0);
        vy->push_back(y0);
        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

static int calcPosition(int value, int base)
{
    if( value == 100 ) return (base - 1);
    double band = 100.0 / (double)base;
    return (int)((double)value / band);
}

void MorphEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {

    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    int start_x1 = GetValueCurveInt("Morph_Start_X1", 0, SettingsMap, eff_pos, MORPH_X_MIN, MORPH_X_MAX);
    int start_y1 = GetValueCurveInt("Morph_Start_Y1", 0, SettingsMap, eff_pos, MORPH_Y_MIN, MORPH_Y_MAX);
    int start_x2 = GetValueCurveInt("Morph_Start_X2", 0, SettingsMap, eff_pos, MORPH_X_MIN, MORPH_X_MAX);
    int start_y2 = GetValueCurveInt("Morph_Start_Y2", 0, SettingsMap, eff_pos, MORPH_Y_MIN, MORPH_Y_MAX);
    int end_x1 = GetValueCurveInt("Morph_End_X1", 0, SettingsMap, eff_pos, MORPH_X_MIN, MORPH_X_MAX);
    int end_y1 = GetValueCurveInt("Morph_End_Y1", 0, SettingsMap, eff_pos, MORPH_Y_MIN, MORPH_Y_MAX);
    int end_x2 = GetValueCurveInt("Morph_End_X2", 0, SettingsMap, eff_pos, MORPH_X_MIN, MORPH_X_MAX);
    int end_y2 = GetValueCurveInt("Morph_End_Y2", 0, SettingsMap, eff_pos, MORPH_Y_MIN, MORPH_Y_MAX);
    int start_length = GetValueCurveInt("MorphStartLength", 0, SettingsMap, eff_pos, MORPH_STARTLENGTH_MIN, MORPH_STARTLENGTH_MAX);
    int end_length = GetValueCurveInt("MorphEndLength", 0, SettingsMap, eff_pos, MORPH_ENDLENGTH_MIN, MORPH_ENDLENGTH_MAX);
    int duration = GetValueCurveInt("MorphDuration", 0, SettingsMap, eff_pos, MORPH_DURATION_MIN, MORPH_DURATION_MAX);
    int acceleration = GetValueCurveInt("MorphAccel", 0, SettingsMap, eff_pos, MORPH_ACCEL_MIN, MORPH_ACCEL_MAX);
    int repeat_count = GetValueCurveInt("Morph_Repeat_Count", 0, SettingsMap, eff_pos, MORPH_REPEAT_MIN, MORPH_REPEAT_MAX);
    int repeat_skip = GetValueCurveInt("Morph_Repeat_Skip", 0, SettingsMap, eff_pos, MORPH_REPEATSKIP_MIN, MORPH_REPEATSKIP_MAX);
    int stagger = GetValueCurveInt("Morph_Stagger", 0, SettingsMap, eff_pos, MORPH_STAGGER_MIN, MORPH_STAGGER_MAX);
    bool start_linked = SettingsMap.GetBool("CHECKBOX_Morph_Start_Link");
    bool end_linked = SettingsMap.GetBool("CHECKBOX_Morph_End_Link");
    bool showEntireHeadAtStart = SettingsMap.GetBool("CHECKBOX_ShowHeadAtStart");

    double step_size = 0.1;

    int hcols = 0, hcole = 1;
    int tcols = 2, tcole = 3;
    int num_tail_colors = 2;
    switch (buffer.palette.Size()) {
        case 1:  //one color selected, use it for all
            hcols = hcole = tcols = tcole = 0;
            break;
        case 2: //two colors, head/tail
            hcols = hcole = 0;
            tcols = tcole = 1;
            break;
        case 3: //three colors, head /tail start/end
            hcols = hcole = 0;
            tcols = 1;
            tcole = 2;
            break;
        default:
            num_tail_colors = buffer.palette.Size() - 2;
            break;
    }

    int x1a = calcPosition(start_x1, buffer.BufferWi);
    int y1a = calcPosition(start_y1, buffer.BufferHt);
    int x2a = calcPosition(end_x1, buffer.BufferWi);
    int y2a = calcPosition(end_y1, buffer.BufferHt);

    int x1b, x2b, y1b, y2b;

    if( start_linked )
    {
        x1b = x1a;
        y1b = y1a;
    }
    else
    {
        x1b = calcPosition(start_x2, buffer.BufferWi);
        y1b = calcPosition(start_y2, buffer.BufferHt);
    }

    if( end_linked )
    {
        x2b = x2a;
        y2b = y2a;
    }
    else
    {
        x2b = calcPosition(end_x2, buffer.BufferWi);
        y2b = calcPosition(end_y2, buffer.BufferHt);
    }

    xlColor head_color, tail_color, test_color;

    // compute direction
    int delta_xa = x2a - x1a;
    int delta_xb = x2b - x1b;
    int delta_ya = y2a - y1a;
    int delta_yb = y2b - y1b;
    int direction = delta_xa + delta_xb + delta_ya + delta_yb;
    int repeat_x = 0;
    int repeat_y = 0;
    double effect_pct = 1.0;
    double stagger_pct = 0.0;
    if( repeat_count > 0 )
    {
        if( (std::abs((float)delta_xa) + std::abs((float)delta_xb)) < (std::abs((float)delta_ya) + std::abs((float)delta_yb)) )
        {
            repeat_x = repeat_skip;
        }
        else
        {
            repeat_y = repeat_skip;
        }
        double stagger_val = (double)(std::abs((double)stagger))/200.0;
        effect_pct = 1.0 / (1 + stagger_val * repeat_count);
        stagger_pct = effect_pct * stagger_val;
    }

    std::vector<int> v_ax;
    std::vector<int> v_ay;
    std::vector<int> v_bx;
    std::vector<int> v_by;

    StoreLine(x1a, y1a, x2a, y2a, &v_ax, &v_ay);  // store side a
    StoreLine(x1b, y1b, x2b, y2b, &v_bx, &v_by);  // store side b

    int size_a = v_ax.size();
    int size_b = v_bx.size();

    std::vector<int> *v_lngx;  // pointer to longest vector x
    std::vector<int> *v_lngy;  // pointer to longest vector y
    std::vector<int> *v_shtx;  // pointer to shorter vector x
    std::vector<int> *v_shty;  // pointer to shorter vector y

    if( size_a > size_b )
    {
        v_lngx = &v_ax;
        v_lngy = &v_ay;
        v_shtx = &v_bx;
        v_shty = &v_by;
    }
    else
    {
        v_lngx = &v_bx;
        v_lngy = &v_by;
        v_shtx = &v_ax;
        v_shty = &v_ay;
    }

    double pos_a, pos_b;
    double total_tail_length, alpha_pct;
    double total_length = v_lngx->size();     // total length of longest vector
    double head_duration = duration/100.0;    // time the head is in the frame
    double head_end_of_head_pos = total_length + 1;
    double tail_end_of_head_pos = total_length + 1;
    double head_end_of_tail_pos = -1;
    double tail_end_of_tail_pos = -1;

    for( int repeat = 0; repeat <= repeat_count; repeat++ )
    {
        double eff_pos_adj = buffer.calcAccel(eff_pos, acceleration);
        double eff_start_pct = (stagger >= 0) ? stagger_pct*repeat : stagger_pct*(repeat_count-repeat);
        double eff_end_pct = eff_start_pct + effect_pct;
        eff_pos_adj = (eff_pos_adj - eff_start_pct) / (eff_end_pct - eff_start_pct);
        if( eff_pos_adj < 0.0 )
        {
            head_end_of_head_pos = -1;
            tail_end_of_head_pos = -1;
            head_end_of_tail_pos = -1;
            tail_end_of_tail_pos = -1;
            total_tail_length = 1.0;
            if( showEntireHeadAtStart )
            {
                head_end_of_head_pos = start_length;
            }
        }
        else
        {
            if( head_duration > 0.0 )
            {
                double head_loc_pct = eff_pos_adj / head_duration;
                head_end_of_head_pos = total_length * head_loc_pct;
                double current_total_head_length = end_length * head_loc_pct + start_length * (1.0 - head_loc_pct);  // adjusted head length excluding clipping
                head_end_of_head_pos += current_total_head_length * head_loc_pct * head_duration;
                total_tail_length = total_length * ( 1 / head_duration - 1.0);
                if( showEntireHeadAtStart )
                {
                    head_end_of_head_pos += current_total_head_length * (1.0 - eff_pos_adj);
                }
                tail_end_of_head_pos = head_end_of_head_pos - current_total_head_length;
                head_end_of_tail_pos = tail_end_of_head_pos - step_size;
                tail_end_of_tail_pos = head_end_of_tail_pos - total_tail_length;
                buffer.Get2ColorBlend(hcols, hcole, std::min( head_loc_pct, 1.0), head_color);
            }
            else
            {
                total_tail_length = total_length;
                head_end_of_tail_pos = total_length * 2 * eff_pos_adj;
                tail_end_of_tail_pos = head_end_of_tail_pos - total_tail_length;
            }
        }

        // draw the tail
        for( double i = std::min(head_end_of_tail_pos, total_length-1); i >= tail_end_of_tail_pos && i >= 0.0; i -= step_size )
        {
            double pct = ((total_length == 0) ? 0.0 : i / total_length);
            pos_a = i;
            pos_b = v_shtx->size() * pct;
            double tail_color_pct = (i-tail_end_of_tail_pos) / total_tail_length;
            if( num_tail_colors > 2 )
            {
                double color_index = ((double)num_tail_colors - 1.0) * (1.0 - tail_color_pct);
                tail_color_pct = color_index - (double)((int)color_index);
                tcols = (int)color_index + 2;
                tcole = tcols + 1;
                if( tcole == num_tail_colors+1 )
                {
                    alpha_pct = (1.0 - tail_color_pct);
                }
                else
                {
                    alpha_pct = 1.0;
                }
                buffer.Get2ColorBlend(tcols, tcole, tail_color_pct, tail_color);
            }
            else
            {
                if( tail_color_pct > 0.5 )
                {
                    alpha_pct = 1.0;
                }
                else
                {
                    alpha_pct = tail_color_pct / 0.5;
                }
                buffer.Get2ColorBlend(tcole, tcols, tail_color_pct, tail_color);
            }
            if( buffer.allowAlpha ) {
                tail_color.alpha = 255 * alpha_pct;
            }
            buffer.DrawThickLine( (*v_lngx)[pos_a]+(repeat_x*repeat), (*v_lngy)[pos_a]+(repeat_y*repeat), (*v_shtx)[pos_b]+(repeat_x*repeat), (*v_shty)[pos_b]+(repeat_y*repeat), tail_color, direction >= 0);
        }

        // draw the head
        for( double i = std::max(tail_end_of_head_pos, 0.0); i <= head_end_of_head_pos && i < total_length; i += step_size )
        {
            double pct = ((total_length == 0) ? 0.0 : i / total_length);
            pos_a = i;
            pos_b = v_shtx->size() * pct;
            buffer.DrawThickLine( (*v_lngx)[pos_a]+(repeat_x*repeat), (*v_lngy)[pos_a]+(repeat_y*repeat), (*v_shtx)[pos_b]+(repeat_x*repeat), (*v_shty)[pos_b]+(repeat_y*repeat), head_color, direction >= 0);
        }
    }
}

