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

#define METEORS_COUNT_MIN 1
#define METEORS_COUNT_MAX 100

#define METEORS_LENGTH_MIN 1
#define METEORS_LENGTH_MAX 100

#define METEORS_SWIRL_MIN 0
#define METEORS_SWIRL_MAX 20

#define METEORS_SPEED_MIN 0
#define METEORS_SPEED_MAX 50

#define METEORS_XOFFSET_MIN -100
#define METEORS_XOFFSET_MAX 100

#define METEORS_YOFFSET_MIN -100
#define METEORS_YOFFSET_MAX 100

class MeteorsEffect : public RenderableEffect
{
public:
    MeteorsEffect(int id);
    virtual ~MeteorsEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Meteors_Count")
            return METEORS_COUNT_MIN;
        if (name == "E_VALUECURVE_Meteors_Length")
            return METEORS_LENGTH_MIN;
        if (name == "E_VALUECURVE_Meteors_Swirl_Intensity")
            return METEORS_SWIRL_MIN;
        if (name == "E_VALUECURVE_Meteors_Speed")
            return METEORS_SPEED_MIN;
        if (name == "E_VALUECURVE_Meteors_XOffset")
            return METEORS_XOFFSET_MIN;
        if (name == "E_VALUECURVE_Meteors_YOffset")
            return METEORS_YOFFSET_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Meteors_Count")
            return METEORS_COUNT_MAX;
        if (name == "E_VALUECURVE_Meteors_Length")
            return METEORS_LENGTH_MAX;
        if (name == "E_VALUECURVE_Meteors_Swirl_Intensity")
            return METEORS_SWIRL_MAX;
        if (name == "E_VALUECURVE_Meteors_Speed")
            return METEORS_SPEED_MAX;
        if (name == "E_VALUECURVE_Meteors_XOffset")
            return METEORS_XOFFSET_MAX;
        if (name == "E_VALUECURVE_Meteors_YOffset")
            return METEORS_YOFFSET_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;

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
};
