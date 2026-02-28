#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"
#include "../RenderBuffer.h"
#include <cmath>

static const int MAX_RGB_BALLS = 20;

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
        dir *= _t < M_PI / 6.0 || _t > (2 * M_PI) / 3 ? -1.0 : 1.0;
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
        if (_x - _radius <= 0) {
            _dx = fabs(_dx);
            if (_dx < 0.2f) _dx = 0.2f;
        }
        if (_x + _radius >= width) {
            _dx = -fabs(_dx);
            if (_dx > -0.2f) _dx = -0.2f;
        }
        if (_y - _radius <= 0) {
            _dy = fabs(_dy);
            if (_dy < 0.2f) _dy = 0.2f;
        }
        if (_y + _radius >= height) {
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
        if ((x == _x) && (y == _y)) return 1;
        return (_radius / (sqrt(pow(x - _x, 2) + pow(y - _y, 2))));
    }
};

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

#define CIRCLES_COUNT_MIN 1
#define CIRCLES_COUNT_MAX 10

#define CIRCLES_SIZE_MIN 1
#define CIRCLES_SIZE_MAX 20

#define CIRCLES_SPEED_MIN 1
#define CIRCLES_SPEED_MAX 30

#define CIRCLES_POS_MIN -50
#define CIRCLES_POS_MAX 50

class CirclesEffect : public RenderableEffect
{
public:
    CirclesEffect(int id);
    virtual ~CirclesEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Circles_Count")
            return CIRCLES_COUNT_MIN;
        if (name == "E_VALUECURVE_Circles_Size")
            return CIRCLES_SIZE_MIN;
        if (name == "E_VALUECURVE_Circles_Speed")
            return CIRCLES_SPEED_MIN;
        if (name == "E_VALUECURVE_Circles_XC")
            return CIRCLES_POS_MIN;
        if (name == "E_VALUECURVE_Circles_YC")
            return CIRCLES_POS_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }
    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Circles_Count")
            return CIRCLES_COUNT_MAX;
        if (name == "E_VALUECURVE_Circles_Size")
            return CIRCLES_SIZE_MAX;
        if (name == "E_VALUECURVE_Circles_Speed")
            return CIRCLES_SPEED_MAX;
        if (name == "E_VALUECURVE_Circles_XC")
            return CIRCLES_POS_MAX;
        if (name == "E_VALUECURVE_Circles_YC")
            return CIRCLES_POS_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;

    // Update ball positions in cache (without drawing). Returns the cache.
    // After this call, cache->balls or cache->metaballs have updated positions.
    CirclesRenderCache* UpdateCacheState(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer);

    // Draw pixels from the already-updated cache state.
    void RenderPixels(const SettingsMap& SettingsMap, RenderBuffer& buffer, CirclesRenderCache* cache,
                      bool plasma, bool fade, bool bubbles, bool bounce, bool collide);

    void RenderMetaBalls(RenderBuffer& buffer, int numBalls, MetaBall* metaballs);
    void RenderRadial(RenderBuffer& buffer, int start_x, int start_y, int radius,
                      int colorCnt, int number, bool radial_3D,
                      const int effectState);
    void RenderCirclesUpdate(RenderBuffer& buffer, int number, RgbBalls* effObjs, int circleSpeed);
};
