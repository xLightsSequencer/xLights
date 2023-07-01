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

#include "../Color.h"

#define SHAPE_THICKNESS_MIN 1
#define SHAPE_THICKNESS_MAX 100

#define SHAPE_STARTSIZE_MIN 0
#define SHAPE_STARTSIZE_MAX 100

#define SHAPE_CENTREX_MIN 0
#define SHAPE_CENTREX_MAX 100

#define SHAPE_CENTREY_MIN 0 
#define SHAPE_CENTREY_MAX 100

#define SHAPE_LIFETIME_MIN 1
#define SHAPE_LIFETIME_MAX 100

#define SHAPE_GROWTH_MIN -100
#define SHAPE_GROWTH_MAX 100

#define SHAPE_COUNT_MIN 1
#define SHAPE_COUNT_MAX 100

#define SHAPE_ROTATION_MIN 0
#define SHAPE_ROTATION_MAX 360

#define SHAPE_VELOCITY_MIN 0
#define SHAPE_VELOCITY_MAX 20

#define SHAPE_DIRECTION_MIN 0
#define SHAPE_DIRECTION_MAX 359

class ShapeRenderCache;

class ShapeEffect : public RenderableEffect
{
public:
    ShapeEffect(int id);
    virtual ~ShapeEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual void SetPanelStatus(Model* cls) override;
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
    virtual std::list<std::string> GetFileReferences(Model* model, const SettingsMap& SettingsMap) const override;
    virtual bool CleanupFileLocations(xLightsFrame* frame, SettingsMap& SettingsMap) override;
#ifdef LINUX
    virtual bool CanRenderOnBackgroundThread(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override
    {
        return false;
    };
#endif
    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Shape_Thickness")
            return SHAPE_THICKNESS_MIN;
        if (name == "E_VALUECURVE_Shape_StartSize")
            return SHAPE_STARTSIZE_MIN;
        if (name == "E_VALUECURVE_Shape_CentreX")
            return SHAPE_CENTREX_MIN;
        if (name == "E_VALUECURVE_Shape_CentreY")
            return SHAPE_CENTREY_MIN;
        if (name == "E_VALUECURVE_Shape_Lifetime")
            return SHAPE_LIFETIME_MIN;
        if (name == "E_VALUECURVE_Shape_Growth")
            return SHAPE_GROWTH_MIN;
        if (name == "E_VALUECURVE_Shape_Count")
            return SHAPE_COUNT_MIN;
        if (name == "E_VALUECURVE_Shape_Rotation")
            return SHAPE_ROTATION_MIN;
        if (name == "E_VALUECURVE_Shapes_Velocity")
            return SHAPE_VELOCITY_MIN;
        if (name == "E_VALUECURVE_Shapes_Direction")
            return SHAPE_DIRECTION_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Shape_Thickness")
            return SHAPE_THICKNESS_MAX;
        if (name == "E_VALUECURVE_Shape_StartSize")
            return SHAPE_STARTSIZE_MAX;
        if (name == "E_VALUECURVE_Shape_CentreX")
            return SHAPE_CENTREX_MAX;
        if (name == "E_VALUECURVE_Shape_CentreY")
            return SHAPE_CENTREY_MAX;
        if (name == "E_VALUECURVE_Shape_Lifetime")
            return SHAPE_LIFETIME_MAX;
        if (name == "E_VALUECURVE_Shape_Growth")
            return SHAPE_GROWTH_MAX;
        if (name == "E_VALUECURVE_Shape_Count")
            return SHAPE_COUNT_MAX;
        if (name == "E_VALUECURVE_Shape_Rotation")
            return SHAPE_ROTATION_MAX;
        if (name == "E_VALUECURVE_Shapes_Velocity")
            return SHAPE_VELOCITY_MAX;
        if (name == "E_VALUECURVE_Shapes_Direction")
            return SHAPE_DIRECTION_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;

private:
    static int DecodeShape(const std::string& shape);
    void SetPanelTimingTracks() const;
    void Drawcircle(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness) const;
    void Drawheart(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness, double rotation) const;
    void Drawstar(RenderBuffer& buffer, int xc, int yc, double radius, int points, xlColor color, int thickness, double rotation = 0) const;
    void Drawpolygon(RenderBuffer& buffer, int xc, int yc, double radius, int sides, xlColor color, int thickness, double rotation = 0) const;
    void Drawsnowflake(RenderBuffer& buffer, int xc, int yc, double radius, int sides, xlColor color, double rotation = 0) const;
    void Drawtree(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness, double rotation) const;
    void Drawcandycane(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness) const;
    void Drawcrucifix(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness, double rotation) const;
    void Drawpresent(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness, double rotation) const;
    void Drawemoji(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int emoji, int emojiTone, wxFontInfo& font) const;
    void Drawellipse(RenderBuffer& buffer, int xc, int yc, double radius, int multipler, xlColor color, int thickness, double rotation = 0) const;
    void DrawSVG(ShapeRenderCache* cache, RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness) const;
    bool areSame(double a, double b, float eps) const;
    bool areCollinear(const wxPoint2DDouble& a, const wxPoint2DDouble& b, const wxPoint2DDouble& c, double eps) const;
};
