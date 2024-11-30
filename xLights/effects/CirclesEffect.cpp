/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CirclesEffect.h"
#include "CirclesPanel.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/circles-16.xpm"
#include "../../include/circles-24.xpm"
#include "../../include/circles-32.xpm"
#include "../../include/circles-48.xpm"
#include "../../include/circles-64.xpm"

CirclesEffect::CirclesEffect(int i) : RenderableEffect(i, "Circles", circles_16, circles_24, circles_32, circles_48, circles_64)
{
    //ctor
}

CirclesEffect::~CirclesEffect()
{
    //dtor
}

xlEffectPanel *CirclesEffect::CreatePanel(wxWindow *parent) {
    return new CirclesPanel(parent);
}

class RgbBalls
{
public:
    float _x;
    float _y;
    float _dx;
    float _dy;
    float _radius;
    float _t;
    float dir;
    float _angle;
    float _spd;
    int _colorindex;

    void Reset(float x, float y, float speed, float angle, float radius, int colorindex)
    {
        _angle = angle;
        _spd = speed;
        _x = x;
        _y = y;
        _dx = speed * cos(angle);
        _dy = speed * sin(angle);
        _radius = radius;
        _colorindex = colorindex;
        _t = (float)M_PI / 6.0;
        dir = 1.0f;
    }
    void updatePositionArc(int x, int y, int r)
    {
        _x = x + r * cos(_t);
        _y = y + r * sin(_t);
        _t += dir * (M_PI / 9.0);
        dir *= _t < M_PI / 6.0 || _t >(2 * M_PI) / 3 ? -1.0 : 1.0;
    }
    void updatePosition(float incr, int width, int height)
    {
        _x += _dx * incr;
        _x = _x > width ? 0 : _x;
        _x = _x < 0 ? width : _x;
        _y += _dy * incr;
        _y = _y > height ? 0 : _y;
        _y = _y < 0 ? height : _y;
    }

    void Bounce(int width, int height)
    {
        if (_x - _radius <= 0)
        {
            _dx = fabs(_dx);
            if (_dx < 0.2f) _dx = 0.2f;
        }
        if (_x + _radius >= width)
        {
            _dx = -fabs(_dx);
            if (_dx > -0.2f) _dx = -0.2f;
        }
        if (_y - _radius <= 0)
        {
            _dy = fabs(_dy);
            if (_dy < 0.2f) _dy = 0.2f;
        }
        if (_y + _radius >= height)
        {
            _dy = -fabs(_dy);
            if (_dy > -0.2f) _dy = -0.2f;
        }
    }
};

class MetaBall : public RgbBalls
{
public:
    float Equation(float x, float y)
    {
        //            if(x==_x || y==_y) return 1; //this is incorrect
        if ((x == _x) && (y == _y)) return 1; //only return 1 if *both* coordinates match; else gives extraneous horiz or vert lines -DJ
        return (_radius / (sqrt(pow(x - _x, 2) + pow(y - _y, 2))));
    }
};

static const int MAX_RGB_BALLS = 20;

class CirclesRenderCache : public EffectRenderCache {
public:
    CirclesRenderCache() : numBalls(0), metaType(false) {
        balls = new RgbBalls[MAX_RGB_BALLS];
        metaballs = new MetaBall[MAX_RGB_BALLS];
    };
    virtual ~CirclesRenderCache() {
        delete [] balls;
        delete [] metaballs;
    };
    bool metaType;
    int numBalls;
    RgbBalls *balls;
    MetaBall *metaballs;
};

void CirclesEffect::SetDefaultParameters() {
    CirclesPanel *cp = (CirclesPanel*)panel;
    if (cp == nullptr) {
        return;
    }

    cp->BitmapButton_Circles_Count->SetActive(false);
    cp->BitmapButton_Circles_Size->SetActive(false);
    cp->BitmapButton_Circles_Speed->SetActive(false);

    SetSliderValue(cp->Slider_Circles_Count, 3);
    SetSliderValue(cp->Slider_Circles_Size, 5);
    SetSliderValue(cp->Slider_Circles_Speed, 10);
    SetSliderValue(cp->Slider_Circles_XC, 0);
    SetSliderValue(cp->Slider_Circles_YC, 0);

    SetCheckBoxValue(cp->CheckBox_Circles_Bounce, false);
    SetCheckBoxValue(cp->CheckBox_Circles_Radial, false);
    SetCheckBoxValue(cp->CheckBox_Circles_Plasma, false);
    SetCheckBoxValue(cp->CheckBox_Circles_Radial_3D, false);
    SetCheckBoxValue(cp->CheckBox_Circles_Bubbles, false);
    SetCheckBoxValue(cp->CheckBox_Circles_Collide, false);
    SetCheckBoxValue(cp->CheckBox_Circles_Random_m, false);
    SetCheckBoxValue(cp->CheckBox_Circles_Linear_Fade, false);
}

void CirclesEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();
    int number = GetValueCurveInt("Circles_Count", 3, SettingsMap, oset, CIRCLES_COUNT_MIN, CIRCLES_COUNT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int circleSpeed = GetValueCurveInt("Circles_Speed", 10, SettingsMap, oset, CIRCLES_SPEED_MIN, CIRCLES_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int radius = GetValueCurveInt("Circles_Size", 5, SettingsMap, oset, CIRCLES_SIZE_MIN, CIRCLES_SIZE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    bool plasma = SettingsMap.GetBool("CHECKBOX_Circles_Plasma", false);
    bool radial = SettingsMap.GetBool("CHECKBOX_Circles_Radial", false);
    bool radial_3D = SettingsMap.GetBool("CHECKBOX_Circles_Radial_3D", false);
    int start_x = buffer.BufferWi / 2;
    int start_y = buffer.BufferHt / 2;
    bool fade = SettingsMap.GetBool("CHECKBOX_Circles_Linear_Fade", false);
    bool bubbles = SettingsMap.GetBool("CHECKBOX_Circles_Bubbles", false);
    //bool random = SettingsMap.GetBool("CHECKBOX_Circles_Random_m", false);
    bool collide = SettingsMap.GetBool("CHECKBOX_Circles_Collide", false);
    bool bounce = SettingsMap.GetBool("CHECKBOX_Circles_Bounce", false);

    CirclesRenderCache* cache = (CirclesRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new CirclesRenderCache();
        buffer.infoCache[id] = cache;
    }

    size_t colorCnt = buffer.GetColorCount();

    RgbBalls* effectObjects = plasma ? cache->metaballs : cache->balls;
    if (number > MAX_RGB_BALLS) {
        number = MAX_RGB_BALLS;
    }
    int effectState = (buffer.curPeriod - buffer.curEffStartPer) * circleSpeed * buffer.frameTimeInMs / 50;

    if (radial || radial_3D)
    {
        start_x = start_x + (GetValueCurveInt("Circles_XC", 0, SettingsMap, oset, CIRCLES_POS_MIN, CIRCLES_POS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) / float(CIRCLES_POS_MAX) * start_x);
        start_y = start_y + (GetValueCurveInt("Circles_YC", 0, SettingsMap, oset, CIRCLES_POS_MIN, CIRCLES_POS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) / float(CIRCLES_POS_MAX) * start_y);
        RenderRadial(buffer, start_x, start_y, radius, colorCnt, number, radial_3D, effectState);
        return; //radial is the easiest case so just get out.
    }

    if (buffer.needToInit || radius != effectObjects[0]._radius || number != cache->numBalls || cache->metaType != plasma)
    {
        for (int ii = 0; ii < number; ii++)
        {
            int colorIdx = 0;
            float angle;
            float spd;
            if (ii >= cache->numBalls || buffer.needToInit)
            {
                start_x = rand() % (buffer.BufferWi);
                start_y = rand() % (buffer.BufferHt);
                colorIdx = ii % colorCnt;
                angle = rand() % 2 ? rand() % 90 : -rand() % 90;
                spd = rand() % 3 + 1;
            }
            else
            {
                start_x = effectObjects[ii]._x;
                start_y = effectObjects[ii]._y;
                colorIdx = effectObjects[ii]._colorindex;
                angle = effectObjects[ii]._angle;
                spd = effectObjects[ii]._spd;
            }
            effectObjects[ii].Reset((float)start_x, (float)start_y, spd, angle, (float)radius, colorIdx);
            if (bubbles) //keep bubbles going mostly up
            {
                // This looks odd ... rand() is 0-1 so % 45 is going to be rand()
                angle = 90 + rand() % 45 - 22.5f; //+/- 22.5 degrees from 90 degrees
                angle *= 2.0f * (float)M_PI / 180.0f;
                effectObjects[ii]._dx = spd * cos(angle);
                effectObjects[ii]._dy = spd * sin(angle);
            }
        }
        cache->numBalls = number;
        cache->metaType = plasma;
        buffer.needToInit = false;
    }
    else
    {
        RenderCirclesUpdate(buffer, number, effectObjects, circleSpeed);
    }

    if (bounce)
    {
        //update position in case something hit a wall
        for (int ii = 0; ii < number; ii++)
        {
            effectObjects[ii].Bounce(buffer.BufferWi, buffer.BufferHt);
        }
    }
    if (collide)
    {
        //update position if two balls collided
    }

    if (plasma)
    {
        RenderMetaBalls(buffer, cache->numBalls, cache->metaballs);
    }
    else
    {
        for (int ii = 0; ii < number; ii++)
        {
            HSVValue hsv;
            buffer.palette.GetHSV(cache->balls[ii]._colorindex, hsv);
            if (fade)
            {
                buffer.DrawFadingCircle(cache->balls[ii]._x, cache->balls[ii]._y, cache->balls[ii]._radius, hsv, !bounce && !collide);
            }
            else
            {
                buffer.DrawCircle(cache->balls[ii]._x, cache->balls[ii]._y, cache->balls[ii]._radius, hsv, !bubbles, !bounce && !collide);
            }
        }
    }
}

void CirclesEffect::RenderCirclesUpdate(RenderBuffer& buffer, int ballCnt, RgbBalls* effObjs, int circleSpeed)
{
    for (int ii = 0; ii < ballCnt; ii++)
    {
        effObjs[ii].updatePosition((float)circleSpeed * (float)buffer.frameTimeInMs / 200.0, buffer.BufferWi, buffer.BufferHt);
    }
}

void CirclesEffect::RenderRadial(RenderBuffer& buffer, int x, int y, int thickness, int colorCnt, int number, bool radial_3D, const int effectState)
{
    int barht = buffer.BufferHt / (thickness + 1);
    if (barht < 1) barht = 1;
    int maxRadius = effectState > buffer.BufferHt ? buffer.BufferHt : effectState / 2 + thickness;
    int blockHt = colorCnt * barht;
    int f_offset = effectState / 4 % (blockHt + 1);

    barht = barht > 0 ? barht : 1;
    HSVValue hsv;
    buffer.palette.GetHSV(0, hsv);

    xlColor lastColor = xlBLACK;
    for (int ii = maxRadius; ii >= 0; ii--)
    {
        int n = ii - f_offset + blockHt;
        int colorIdx = (n) % blockHt / barht;
        buffer.palette.GetHSV(colorIdx, hsv);

        if (radial_3D)
        {
            hsv.hue = (float)(ii + effectState) / ((float)maxRadius / (float)number);
            if (hsv.hue > 1.0) hsv.hue = hsv.hue - (long)hsv.hue;
            hsv.saturation = 1.0;
            hsv.value = 1.0;
        }
        xlColor color(hsv);
        if (lastColor != color) {
            buffer.DrawCircle(x, y, ii, hsv, true);
            lastColor = color;
        }
    }
}

void CirclesEffect::RenderMetaBalls(RenderBuffer& buffer, int numBalls, MetaBall* metaballs)
{
    for (int row = 0; row < buffer.BufferHt; row++)
    {
        for (int col = 0; col < buffer.BufferWi; col++)
        {
            float sum = 0.0f;
            HSVValue hsv;
            hsv.hue = 0.0f;
            hsv.saturation = 0.0f;
            hsv.value = 0.0f;

            for (int ii = 0; ii < numBalls; ii++)
            {
                float val = metaballs[ii].Equation((float)col, (float)row);
                sum += val;
                HSVValue temp;
                buffer.palette.GetHSV(metaballs[ii]._colorindex, temp);
                if (val > 0.30f)
                {
                    temp.value = val > 1.0f ? 1.0f : val;
                    hsv = buffer.Get2ColorAdditive(hsv, temp);
                }
            }
            if (sum >= 0.90f)
            {
                buffer.SetPixel(col, row, hsv);
            }
        }
    }
}
