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
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual std::list<std::string> GetFileReferences(Model* model, const SettingsMap& SettingsMap) const override;
    virtual bool CleanupFileLocations(RenderContext* ctx, SettingsMap& SettingsMap) override;
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

    // Cached from Video.json by OnMetadataLoaded(). Video_Speed uses divisor=100
    // so min/max are pre-divisor ticks.
    static double sSpeedDefault;
    static double sSpeedMin;
    static double sSpeedMax;
    static int sSpeedDivisor;
    static int sCropMin;
    static int sCropMax;
    static int sCropLeftDefault;
    static int sCropRightDefault;
    static int sCropTopDefault;
    static int sCropBottomDefault;
    static double sStartTimeDefault;
    static int sSampleSpacingDefault;
    static bool sSyncAudioDefault;
    static bool sAspectRatioDefault;
    static std::string sDurationTreatmentDefault;

protected:
    virtual void OnMetadataLoaded() override;
    virtual bool needToAdjustSettings(const std::string& version) override
    {
        return true;
    };
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
};
