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

class xlColor;

class PinwheelEffect : public RenderableEffect
{
public:
    PinwheelEffect(int id);
    virtual ~PinwheelEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool SupportsRadialColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }

    // Cached from Pinwheel.json by OnMetadataLoaded(). sSpeedMax is also used
    // as a divisor in the position computation so it must stay in sync with
    // the VC upper bound.
    static int sArmsDefault;
    static int sArmSizeDefault;
    static int sArmSizeMin;
    static int sArmSizeMax;
    static int sTwistDefault;
    static int sTwistMin;
    static int sTwistMax;
    static int sThicknessDefault;
    static int sThicknessMin;
    static int sThicknessMax;
    static int sSpeedDefault;
    static int sSpeedMin;
    static int sSpeedMax;
    static int sOffsetDefault;
    static int sOffsetMin;
    static int sOffsetMax;
    static std::string sStyleDefault;
    static bool sRotationDefault;
    static std::string s3DDefault;
    static int sXCDefault;
    static int sXCMin;
    static int sXCMax;
    static int sYCDefault;
    static int sYCMin;
    static int sYCMax;

protected:
    virtual void OnMetadataLoaded() override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = false) override;

    enum Pinwheel3DType {
        PW_3D_NONE,
        PW_3D,
        PW_3D_Inverted,
        PW_SWEEP
    };
    
    class PinwheelData {
    public:
        PinwheelData(int i) : colorarray(i), colorIsSpacial(i), colorsAsHSV(i), colorsAsColor(i), pinwheel_arms(i) {
            
        }
        std::vector<uint32_t> colorarray;
        std::vector<bool> colorIsSpacial;
        std::vector<HSVValue> colorsAsHSV;
        std::vector<xlColor> colorsAsColor;
        
        bool hasSpacial = false;
        int pinwheel_arms;
        Pinwheel3DType pw3dType;
        int xc_adj = 0;
        int yc_adj = 0;
        float tmax;
        int degrees_per_arm;
        bool pinwheel_rotation;
        int pinwheel_twist;
        int max_radius;
        int poffset;
        float pos;
    };

    void RenderNewMethod(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer);
    virtual void RenderNewArms(RenderBuffer& buffer, PinwheelData &data);
    void RenderOldMethod(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer);
    
    void Draw_arm(RenderBuffer& buffer, int base_degrees, int max_radius, int pinwheel_twist, int xc_adj, int yc_adj, int colorIdx, Pinwheel3DType pinwheel_3d, float round);

    Pinwheel3DType to3dType(const std::string& pinwheel_3d);
    void adjustColor(Pinwheel3DType pw3dType, xlColor& color, HSVValue& hsv, bool allowAlpha, float round);
};
