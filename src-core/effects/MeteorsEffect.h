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

class MeteorsEffect : public RenderableEffect
{
public:
    MeteorsEffect(int id);
    virtual ~MeteorsEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }

    // Cached from Meteors.json by OnMetadataLoaded().
    static std::string sTypeDefault;
    static std::string sEffectDefault;
    static int sCountDefault;
    static int sCountMin;
    static int sCountMax;
    static int sLengthDefault;
    static int sLengthMin;
    static int sLengthMax;
    static int sSwirlDefault;
    static int sSwirlMin;
    static int sSwirlMax;
    static int sSpeedDefault;
    static int sSpeedMin;
    static int sSpeedMax;
    static int sWarmupFramesDefault;
    static int sXOffsetDefault;
    static int sXOffsetMin;
    static int sXOffsetMax;
    static int sYOffsetDefault;
    static int sYOffsetMin;
    static int sYOffsetMax;
    static bool sUseMusicDefault;
    static bool sFadeWithDistanceDefault;

protected:
    virtual void OnMetadataLoaded() override;

private:
    void RenderMeteorsVertical(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed, int warmupFrames);
        void VerticalAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count);
        void VerticalMoveMeteors(RenderBuffer& buffer, int speed);
        void VerticalRemoveMeteors(RenderBuffer& buffer, int Length);
    void RenderMeteorsHorizontal(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed, int warmupFrames);
        void HorizontalAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count);
        void HorizontalMoveMeteors(RenderBuffer& buffer, int speed);
        void HorizontalRemoveMeteors(RenderBuffer& buffer, int Length);
    void RenderMeteorsImplode(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int SwirlIntensity, int mspeed, int xoffset, int yoffset, bool fadeWithDistance, int warmupFrames);
        void ImplodeAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int xoffset, int yoffset);
        void ImplodeMoveMeteors(RenderBuffer& buffer, int speed, int xoffset, int yoffset, bool fadeWithDistance);
        void ImplodeRemoveMeteors(RenderBuffer& buffer, int xoffset, int yoffset);
    void RenderIcicleDrip(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mSpeed, int warmupFrames);
        void IcicleAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count);
        void IcicleMoveMeteors(RenderBuffer& buffer, int mSpeed);
        void IcicleRemoveMeteors(RenderBuffer& buffer);
    void RenderMeteorsExplode(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int SwirlIntensity, int mSpeed, int xoffset, int yoffset, bool fadeWithDistance, int warmupFrames);
        void ExplodeAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count, int xoffset, int yoffset);
        void ExplodeMoveMeteors(RenderBuffer& buffer, int speed, int xoffset, int yoffset, bool fadeWithDistance);
        void ExplodeRemoveMeteors(RenderBuffer& buffer);

    float calcEffectStateOffset(int mSpeed, RenderBuffer& buffer);
};
