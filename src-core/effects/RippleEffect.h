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

class RippleEffect : public RenderableEffect
{
public:
    RippleEffect(int id);
    virtual ~RippleEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    //virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }
    virtual std::list<std::string> GetFileReferences(Model* model, const SettingsMap& SettingsMap) const override;
    virtual bool CleanupFileLocations(RenderContext* ctx, SettingsMap& SettingsMap) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;

    virtual bool SupportsRadialColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
        // Not sure this updates dynamically as one would want
        //std::string ds = SettingsMap.Get("CHOICE_Ripple_Draw_Style", "Old");
        //return ds != "Old";
    }

    // Cached from Ripple.json by OnMetadataLoaded().
    static std::string sDrawStyleDefault;
    static std::string sObjectToDrawDefault;
    static std::string sMovementDefault;
    static int sScaleDefault;
    static int sScaleMin;
    static int sScaleMax;
    static double sOutlineDefault;
    static double sOutlineMin;
    static double sOutlineMax;
    static int sOutlineDivisor;
    static int sThicknessDefault;
    static int sThicknessMin;
    static int sThicknessMax;
    static double sSpacingDefault;
    static double sSpacingMin;
    static double sSpacingMax;
    static int sSpacingDivisor;
    static double sCyclesDefault;
    static double sCyclesMin;
    static double sCyclesMax;
    static int sCyclesDivisor;
    static int sPointsDefault;
    static int sRotationDefault;
    static int sRotationMin;
    static int sRotationMax;
    static double sTwistDefault;
    static double sTwistMin;
    static double sTwistMax;
    static int sTwistDivisor;
    static int sXCDefault;
    static int sXCMin;
    static int sXCMax;
    static int sYCDefault;
    static int sYCMin;
    static int sYCMax;
    static double sVelocityDefault;
    static double sVelocityMin;
    static double sVelocityMax;
    static int sVelocityDivisor;
    static int sDirectionDefault;
    static int sDirectionMin;
    static int sDirectionMax;
    static bool s3DDefault;

protected:
    virtual void OnMetadataLoaded() override;

private:

    // Old-style draw functions
    void Drawcircle(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
    void Drawsquare(RenderBuffer& buffer, int Movement, int x1, int x2, int y1, int y2, int Ripple_Thickness, int CheckBox_Ripple3D, HSVValue& hsv);
    void Drawtriangle(RenderBuffer& buffer, int Movement, int xc, int yc, double side, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
    void Drawstar(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, int points, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D, float rotation);
    void Drawheart(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
    void Drawpolygon(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, int points, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D, float rotation);
    void Drawsnowflake(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, int points, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D, double rotation);
    void Drawtree(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
    void Drawcandycane(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
    void Drawcrucifix(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
    void Drawpresent(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
};
