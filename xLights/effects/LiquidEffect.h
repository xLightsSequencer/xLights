#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"
#include "../RenderBuffer.h"
#include "../UtilFunctions.h"

#include <string>
#include <list>

class wxString;
class b2World;
class b2ParticleSystem;

#define LIQUID_LIFETIME_MIN 0
#define LIQUID_LIFETIME_MAX 1000

#define LIQUID_DIRECTION_MIN 0
#define LIQUID_DIRECTION_MAX 360

#define LIQUID_FLOW_MIN 0
#define LIQUID_FLOW_MAX 1000

#define LIQUID_VELOCITY_MIN 0
#define LIQUID_VELOCITY_MAX 1000

#define LIQUID_X_MIN 0
#define LIQUID_X_MAX 100

#define LIQUID_Y_MIN 0
#define LIQUID_Y_MAX 100

#define LIQUID_SOURCESIZE_MIN 0
#define LIQUID_SOURCESIZE_MAX 100

#define LIQUID_GRAVITY_MIN -1000
#define LIQUID_GRAVITY_MAX 1000
#define LIQUID_GRAVITY_DIVISOR 10

#define LIQUID_GRAVITYANGLE_MIN 0
#define LIQUID_GRAVITYANGLE_MAX 360

class LiquidEffect : public RenderableEffect
{
public:
    LiquidEffect(int id);
    virtual ~LiquidEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_LifeTime")
            return LIQUID_LIFETIME_MIN;
        if (StartsWith(name, "E_VALUECURVE_Direction"))
            return LIQUID_DIRECTION_MIN;
        if (StartsWith(name, "E_VALUECURVE_Flow"))
            return LIQUID_FLOW_MIN;
        if (StartsWith(name, "E_VALUECURVE_Velocity"))
            return LIQUID_VELOCITY_MIN;
        if (StartsWith(name, "E_VALUECURVE_X"))
            return LIQUID_X_MIN;
        if (StartsWith(name, "E_VALUECURVE_Y"))
            return LIQUID_Y_MIN;
        if (StartsWith(name, "E_VALUECURVE_Liquid_SourceSize"))
            return LIQUID_SOURCESIZE_MIN;
        if (name == "E_VALUECURVE_Liquid_Gravity")
            return LIQUID_GRAVITY_MIN;
        if (name == "E_VALUECURVE_Liquid_GravityAngle")
            return LIQUID_GRAVITYANGLE_MIN;

        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_LifeTime")
            return LIQUID_LIFETIME_MAX;
        if (StartsWith(name, "E_VALUECURVE_Direction"))
            return LIQUID_DIRECTION_MAX;
        if (StartsWith(name, "E_VALUECURVE_Flow"))
            return LIQUID_FLOW_MAX;
        if (StartsWith(name, "E_VALUECURVE_Velocity"))
            return LIQUID_VELOCITY_MAX;
        if (StartsWith(name, "E_VALUECURVE_X"))
            return LIQUID_X_MAX;
        if (StartsWith(name, "E_VALUECURVE_Y"))
            return LIQUID_Y_MAX;
        if (StartsWith(name, "E_VALUECURVE_Liquid_SourceSize"))
            return LIQUID_SOURCESIZE_MAX;
        if (name == "E_VALUECURVE_Liquid_Gravity")
            return LIQUID_GRAVITY_MAX;
        if (name == "E_VALUECURVE_Liquid_GravityAngle")
            return LIQUID_GRAVITYANGLE_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }
    virtual int GetSettingVCDivisor(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Liquid_Gravity")
            return LIQUID_GRAVITY_DIVISOR;
        return RenderableEffect::GetSettingVCDivisor(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
    void Render(RenderBuffer& buffer,
                bool top, bool bottom, bool left, bool right,
                int lifetime, bool holdcolor, bool mixcolors, int size, int warmUpFrames,
                int direction1, int x1, int y1, int velocity1, int flow1, int sourceSize1, bool flowMusic1,
                bool enabled2, int direction2, int x2, int y2, int velocity2, int flow2, int sourceSize2, bool flowMusic2,
                bool enabled3, int direction3, int x3, int y3, int velocity3, int flow3, int sourceSize3, bool flowMusic3,
                bool enabled4, int direction4, int x4, int y4, int velocity4, int flow4, int sourceSize4, bool flowMusic4,
                const std::string& particleType, int despeckle, float gravity, int gravityAngle);
    void CreateBarrier(b2World* world, float x, float y, float width, float height);
    void Draw(RenderBuffer& buffer, b2ParticleSystem* ps, const xlColor& color, bool mixColors, int despeckle, float gravityX, float gravityY);
    bool LostForever(int x, int y, int w, int h, float gravityX, float gravityY);
    void CreateParticles(b2ParticleSystem* ps, int x, int y, int direction, int velocity, int flow, bool flowMusic, int lifetime, int width, int height, const xlColor& c, const std::string& particleType, bool mixcolors, float audioLevel, int sourceSize);
    void CreateParticleSystem(b2World* world, int lifetime, int size);
    void Step(b2World* world, RenderBuffer& buffer, bool enabled[], int lifetime, const std::string& particleType, bool mixcolors,
              int x1, int y1, int direction1, int velocity1, int flow1, int sourceSize1, bool flowMusic1,
              int x2, int y2, int direction2, int velocity2, int flow2, int sourceSize2, bool flowMusic2,
              int x3, int y3, int direction3, int velocity3, int flow3, int sourceSize3, bool flowMusic3,
              int x4, int y4, int direction4, int velocity4, int flow4, int sourceSize4, bool flowMusic4, float time);
    xlColor GetDespeckleColor(RenderBuffer& buffer, size_t x, size_t y, int despeckle) const;
};
