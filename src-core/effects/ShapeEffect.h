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

#include "Color.h"
#include "../render/TextDrawingContext.h"
#include "../utils/xlPoint.h"

class ShapeRenderCache;

class ShapeEffect : public RenderableEffect
{
public:
    ShapeEffect(int id);
    virtual ~ShapeEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual std::list<std::string> GetFileReferences(Model* model, const SettingsMap& SettingsMap) const override;
    virtual bool CleanupFileLocations(RenderContext* ctx, SettingsMap& SettingsMap) override;
#ifdef LINUX
    virtual bool CanRenderOnBackgroundThread(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override
    {
        return false;
    };
#endif
    static int sStartSizeDefault;
    static int sStartSizeMin;
    static int sStartSizeMax;
    static int sThicknessDefault;
    static int sThicknessMin;
    static int sThicknessMax;
    static int sCountDefault;
    static int sCountMin;
    static int sCountMax;
    static bool sRandomInitialDefault;
    static int sVelocityDefault;
    static int sVelocityMin;
    static int sVelocityMax;
    static int sDirectionDefault;
    static int sDirectionMin;
    static int sDirectionMax;
    static int sLifetimeDefault;
    static int sLifetimeMin;
    static int sLifetimeMax;
    static int sGrowthDefault;
    static int sGrowthMin;
    static int sGrowthMax;
    static int sCentreXDefault;
    static int sCentreXMin;
    static int sCentreXMax;
    static int sCentreYDefault;
    static int sCentreYMin;
    static int sCentreYMax;
    static int sPointsDefault;
    static int sRotationDefault;
    static int sRotationMin;
    static int sRotationMax;
    static bool sRandomLocationDefault;
    static bool sRandomMovementDefault;
    static bool sFadeAwayDefault;
    static bool sHoldColourDefault;
    static bool sUseMusicDefault;
    static int sSensitivityDefault;
    static bool sFireTimingDefault;
    static bool sFilterRegDefault;

protected:
    virtual void OnMetadataLoaded() override;
private:
    static int DecodeShape(const std::string& shape);
    void Drawcircle(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness) const;
    void Drawheart(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness, double rotation) const;
    void Drawstar(RenderBuffer& buffer, int xc, int yc, double radius, int points, xlColor color, int thickness, double rotation = 0) const;
    void Drawpolygon(RenderBuffer& buffer, int xc, int yc, double radius, int sides, xlColor color, int thickness, double rotation = 0) const;
    void Drawsnowflake(RenderBuffer& buffer, int xc, int yc, double radius, int sides, xlColor color, double rotation = 0) const;
    void Drawtree(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness, double rotation) const;
    void Drawcandycane(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness) const;
    void Drawcrucifix(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness, double rotation) const;
    void Drawpresent(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness, double rotation) const;
    void Drawemoji(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int emoji, int emojiTone, TextFontInfo& font) const;
    void Drawellipse(RenderBuffer& buffer, int xc, int yc, double radius, int multipler, xlColor color, int thickness, double rotation = 0) const;
    void DrawSVG(ShapeRenderCache* cache, RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness) const;
};
