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
#include "../render/RenderBuffer.h"
#include "UtilFunctions.h"

#include <string>
#include <list>

class b2World;
class b2ParticleSystem;

class LiquidEffect : public RenderableEffect
{
public:
    LiquidEffect(int id);
    virtual ~LiquidEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }

    // Cached from Liquid.json by OnMetadataLoaded().
    static bool sTopBarrierDefault;
    static bool sBottomBarrierDefault;
    static bool sLeftBarrierDefault;
    static bool sRightBarrierDefault;
    static bool sHoldColorDefault;
    static bool sMixColorsDefault;
    static std::string sParticleTypeDefault;
    static int sLifeTimeDefault;
    static int sLifeTimeMin;
    static int sLifeTimeMax;
    static int sSizeDefault;
    static int sWarmUpTimeDefault; // hundredths of a second
    static int sDespeckleDefault;
    static double sGravityDefault;
    static double sGravityMin;
    static double sGravityMax;
    static int sGravityDivisor;
    static int sGravityAngleDefault;
    static int sGravityAngleMin;
    static int sGravityAngleMax;
    static bool sEnabled1Default;
    static int sX1Default;
    static int sY1Default;
    static int sDirection1Default;
    static int sVelocity1Default;
    static int sFlow1Default;
    static int sSourceSize1Default;
    static bool sFlowMusic1Default;
    static bool sEnabled2Default;
    static int sX2Default;
    static int sY2Default;
    static int sDirection2Default;
    static int sVelocity2Default;
    static int sFlow2Default;
    static int sSourceSize2Default;
    static bool sFlowMusic2Default;
    static bool sEnabled3Default;
    static int sX3Default;
    static int sY3Default;
    static int sDirection3Default;
    static int sVelocity3Default;
    static int sFlow3Default;
    static int sSourceSize3Default;
    static bool sFlowMusic3Default;
    static bool sEnabled4Default;
    static int sX4Default;
    static int sY4Default;
    static int sDirection4Default;
    static int sVelocity4Default;
    static int sFlow4Default;
    static int sSourceSize4Default;
    static bool sFlowMusic4Default;
    // Shared range values for X/Y/Direction/Velocity/Flow/SourceSize (all sources share the same).
    static int sXMin;
    static int sXMax;
    static int sYMin;
    static int sYMax;
    static int sDirectionMin;
    static int sDirectionMax;
    static int sVelocityMin;
    static int sVelocityMax;
    static int sFlowMin;
    static int sFlowMax;
    static int sSourceSizeMin;
    static int sSourceSizeMax;

protected:
    virtual void OnMetadataLoaded() override;

    void Render(RenderBuffer& buffer,
                bool top, bool bottom, bool left, bool right,
                int lifetime, bool holdcolor, bool mixcolors, int size, int warmUpTime,
                bool enabled1, int direction1, int x1, int y1, int velocity1, int flow1, int sourceSize1, bool flowMusic1,
                bool enabled2, int direction2, int x2, int y2, int velocity2, int flow2, int sourceSize2, bool flowMusic2,
                bool enabled3, int direction3, int x3, int y3, int velocity3, int flow3, int sourceSize3, bool flowMusic3,
                bool enabled4, int direction4, int x4, int y4, int velocity4, int flow4, int sourceSize4, bool flowMusic4,
                const std::string& particleType, int despeckle, float gravity, int gravityAngle);
    void CreateBarrier(b2World* world, float x, float y, float width, float height);
    void Draw(RenderBuffer& buffer, b2ParticleSystem* ps, const xlColor& color, bool mixColors, int despeckle, float gravityX, float gravityY);
    bool LostForever(int x, int y, int w, int h, float gravityX, float gravityY);
    void CreateParticles(b2ParticleSystem* ps, int x, int y, int direction, int velocity, int flow, bool flowMusic, int lifetime, int width, int height, const xlColor& c, const std::string& particleType, bool mixcolors, float audioLevel, int sourceSize, float& flowAccumulator, float dt, int maxParticles);
    void CreateParticleSystem(b2World* world, int lifetime, int size, int maxParticles);
    void Step(b2World* world, RenderBuffer& buffer, bool enabled[], int lifetime, const std::string& particleType, bool mixcolors,
              int x1, int y1, int direction1, int velocity1, int flow1, int sourceSize1, bool flowMusic1,
              int x2, int y2, int direction2, int velocity2, int flow2, int sourceSize2, bool flowMusic2,
              int x3, int y3, int direction3, int velocity3, int flow3, int sourceSize3, bool flowMusic3,
              int x4, int y4, int direction4, int velocity4, int flow4, int sourceSize4, bool flowMusic4, float time,
              float flowAccumulators[4], int maxParticles);
    xlColor GetDespeckleColor(RenderBuffer& buffer, size_t x, size_t y, int despeckle) const;
};
