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

#define VIDEO_SPEED_MIN -1000
#define VIDEO_SPEED_MAX 1000
#define VIDEO_SPEED_DIVISOR 100

#define VIDEO_CROP_MIN 0
#define VIDEO_CROP_MAX 100

class VideoEffect : public RenderableEffect
{
public:
    VideoEffect(int id);
    virtual ~VideoEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    void Render(RenderBuffer& buffer,
                std::string filename, double starttime, int cropLeft, int cropRight, int cropTop, int cropBottom, bool keepaspectratio, std::string durationTreatment, bool synchroniseAudio, bool transparentBlack, int transparentBlackLevel, double speed, uint32_t sampleSpacing);
    virtual bool CanBeRandom() override
    {
        return false;
    }
    virtual void SetDefaultParameters() override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual std::list<std::string> GetFileReferences(Model* model, const SettingsMap& SettingsMap) const override;
    virtual bool CleanupFileLocations(xLightsFrame* frame, SettingsMap& SettingsMap) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }
    static bool IsVideoFile(std::string filename);

    // Currently not possible but I think changes could be made to make it support partial
    // virtual bool CanRenderPartialTimeInterval() const override { return true; }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Video_Speed")
            return VIDEO_SPEED_MIN;
        if ((name == "E_VALUECURVE_Video_CropLeft") || (name == "E_VALUECURVE_Video_CropRight") || (name == "E_VALUECURVE_Video_CropTop") || (name == "E_VALUECURVE_Video_CropBottom"))
            return VIDEO_CROP_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Video_Speed")
            return VIDEO_SPEED_MAX;
        if ((name == "E_VALUECURVE_Video_CropLeft") || (name == "E_VALUECURVE_Video_CropRight") || (name == "E_VALUECURVE_Video_CropTop") || (name == "E_VALUECURVE_Video_CropBottom"))
            return VIDEO_CROP_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }
    virtual int GetSettingVCDivisor(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Video_Speed")
            return VIDEO_SPEED_DIVISOR;
        return RenderableEffect::GetSettingVCDivisor(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
    virtual bool needToAdjustSettings(const std::string& version) override
    {
        return true;
    };
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
};

