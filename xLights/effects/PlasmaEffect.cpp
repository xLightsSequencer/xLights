#include "PlasmaEffect.h"
#include "PlasmaPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/plasma-16.xpm"
#include "../../include/plasma-24.xpm"
#include "../../include/plasma-32.xpm"
#include "../../include/plasma-48.xpm"
#include "../../include/plasma-64.xpm"

PlasmaEffect::PlasmaEffect(int id) : RenderableEffect(id, "Plasma", plasma_16, plasma_24, plasma_32, plasma_48, plasma_64)
{
    //ctor
}

PlasmaEffect::~PlasmaEffect()
{
    //dtor
}
wxPanel *PlasmaEffect::CreatePanel(wxWindow *parent) {
    return new PlasmaPanel(parent);
}

#define PLASMA_NORMAL_COLORS    0
#define PLASMA_PRESET1          1
#define PLASMA_PRESET2          2
#define PLASMA_PRESET3          3
#define PLASMA_PRESET4          4

static inline int GetPlasmaColorScheme(const std::string &ColorSchemeStr) {
    if (ColorSchemeStr == "Preset Colors 1") {
        return PLASMA_PRESET1;
    } else if (ColorSchemeStr == "Preset Colors 2") {
        return PLASMA_PRESET2;
    } else if (ColorSchemeStr == "Preset Colors 3") {
        return PLASMA_PRESET3;
    } else if (ColorSchemeStr == "Preset Colors 4") {
        return PLASMA_PRESET4;
    }
    return PLASMA_NORMAL_COLORS;
}

void PlasmaEffect::SetDefaultParameters(Model *cls) {
    PlasmaPanel *pp = (PlasmaPanel*)panel;
    if (pp == nullptr) {
        return;
    }

    pp->BitmapButton_Plasma_SpeedVC->SetActive(false);

    SetSliderValue(pp->Slider_Plasma_Style, 1);
    SetSliderValue(pp->Slider_Plasma_Line_Density, 1);
    SetSliderValue(pp->Slider_Plasma_Speed, 10);

    SetChoiceValue(pp->Choice_Plasma_Color, "Normal");
}

void PlasmaEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();
    int Style = SettingsMap.GetInt("SLIDER_Plasma_Style", 1);
    int Line_Density = SettingsMap.GetInt("SLIDER_Plasma_Line_Density", 1);
    int PlasmaSpeed = GetValueCurveInt("Plasma_Speed", 10, SettingsMap, oset, PLASMA_SPEED_MIN, PLASMA_SPEED_MAX);
    std::string PlasmaDirectionStr = SettingsMap["CHOICE_Plasma_Direction"];

    int PlasmaDirection = 0; //fixme?
    int ColorScheme = GetPlasmaColorScheme(SettingsMap["CHOICE_Plasma_Color"]);

    //  These are for Plasma effect
    static const double pi=3.1415926535897932384626433832;

    int curState = (buffer.curPeriod - buffer.curEffStartPer) * PlasmaSpeed * buffer.frameTimeInMs / 50;
    double offset=double(curState)/200.0;

    int state = (buffer.curPeriod - buffer.curEffStartPer); // frames 0 to N
    double Speed_plasma = (101-PlasmaSpeed)*3; // we want a large number to divide by
    double time = (state+1.0)/Speed_plasma;

    if (PlasmaDirection==1) offset = -offset;

    double sin_time_5 = buffer.sin(time / 5);
    double cos_time_3 = buffer.cos(time / 3);
    double sin_time_2 = buffer.sin(time / 2);
    static double pi3 = pi / 3.0;

    for (int x=0; x<buffer.BufferWi; x++)
    {
        double rx = ((float)x / (buffer.BufferWi - 1)); // rx is now in the range 0.0 to 1.0
        double rx2 = rx * rx;
        double cx = rx + .5*sin_time_5;
        double cx2 = cx*cx;
        double sin_rx_time = buffer.sin(rx + time);

        // 1st equation
        double v1 = buffer.sin(rx * 10 + time);

        for (int y=0; y<buffer.BufferHt; y++)
        {
            // reference: http://www.bidouille.org/prog/plasma

            double ry = ((float)y/(buffer.BufferHt-1)) ;
            double v = v1;

            //  second equation
            v+=buffer.sin (10*(rx*sin_time_2+ry*cos_time_3)+time);

            //  third equation
            double cy=ry+.5*cos_time_3;
            v+=buffer.sin ( sqrt((Style*50)*((cx2)+(cy*cy))+time));

            //    vec2 c = v_coords * u_k - u_k/2.0;
            v += sin_rx_time;
            v += buffer.sin ((ry+time)/2.0);
            v += buffer.sin ((rx+ry+time)/2.0);
            //   c += u_k/2.0 * vec2(buffer.sin (u_time/3.0), buffer.cos (u_time/2.0));
            v += buffer.sin (sqrt(rx2+ry*ry)+time);
            v = v/2.0;
            // vec3 col = vec3(1, buffer.sin (PI*v), buffer.cos (PI*v));
            //   gl_FragColor = vec4(col*.5 + .5, 1);

            double vldpi = v*Line_Density*pi;

            xlColor color;
            switch (ColorScheme)
            {
                case PLASMA_NORMAL_COLORS:
                    {
                        double h = (buffer.sin (vldpi + 2 * pi3) + 1) * 0.5;
                        buffer.GetMultiColorBlend(h,false,color);
                    }
                    break;
                case PLASMA_PRESET1:
                    color.red = (buffer.sin (vldpi) + 1) * 128;
                    color.green = (buffer.cos (vldpi) + 1) * 128;
                    color.blue = 0;
                    break;
                case PLASMA_PRESET2:
                    color.red = 1;
                    color.green = (buffer.cos (vldpi) + 1) * 128;
                    color.blue = (buffer.sin (vldpi) + 1) * 128;
                    break;

                case PLASMA_PRESET3:
                    color.red = (buffer.sin (vldpi) + 1) * 128;
                    color.green = (buffer.sin (vldpi + 2 * pi3) + 1) * 128;
                    color.blue = (buffer.sin (vldpi + 4 * pi3) + 1) * 128;
                    break;
                case PLASMA_PRESET4:
                    color.red=color.green=color.blue = (buffer.sin(vldpi) + 1) * 128;
                    break;
            }
            buffer.SetPixel(x,y,color);
        }
    }
}
