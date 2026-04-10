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

#include <cassert>
#include <string>
#include <nlohmann/json.hpp>
#include "Color.h"
#include "../graphics/xlGraphicsAccumulators.h"

class Model;
class SequenceElements;
class Effect;
class SettingsMap;
class RenderBuffer;
class AudioManager;
class EffectManager;
class EffectLayer;
class RenderContext;

class RenderableEffect
{
public:
    RenderableEffect(int id,
                     std::string n,
                     const char** data16,
                     const char** data24,
                     const char** data32,
                     const char** data48,
                     const char** data64);
    virtual ~RenderableEffect();

    virtual const std::string& Name() const
    {
        return name;
    };

    virtual const std::string& ToolTip() const
    {
        return tooltip;
    };

    const char* const* GetIconData(int sizeIndex) const
    {
        return iconData[sizeIndex];
    }
    virtual int GetId() const
    {
        return id;
    }
    virtual int GetColorSupportedCount() const
    {
        return -1;
    } // -1 is no limit
    virtual bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) const
    {
        return false;
    }
    virtual bool SupportsRadialColorCurves(const SettingsMap& SettingsMap) const
    {
        return false;
    }
    virtual std::list<std::string> GetFileReferences(Model* model, const SettingsMap& SettingsMap) const
    {
        return std::list<std::string>();
    }
    virtual std::list<std::string> GetFacesUsed(const SettingsMap& SettingsMap) const
    {
        return std::list<std::string>();
    }
    virtual bool CleanupFileLocations(RenderContext* ctx, SettingsMap& SettingsMap)
    {
        return false;
    }
    virtual bool AppropriateOnNodes() const
    {
        return true;
    }
    virtual bool CanRenderPartialTimeInterval() const
    {
        return false;
    }
    virtual bool PressButton(const std::string& id, SettingsMap& paletteMap, SettingsMap& settings)
    {
        return false;
    }

    virtual void SetSequenceElements(SequenceElements* els)
    {
        mSequenceElements = els;
    }

    // Methods for rendering the effect
    virtual bool CanRenderOnBackgroundThread(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer)
    {
        return true;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) = 0;
    virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
    {}
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache);

    virtual bool CanBeRandom()
    {
        return true;
    }

    // return 0 if this is completely drawin the effect background
    // return 1 to have the grid place a normal icon
    // return 2 to have the grid place a smaller icon
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2,
                                     xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps);

    virtual bool needToAdjustSettings(const std::string& version);
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true);
    virtual void AdjustSettingsAfterSplit(Effect *first, Effect *second) {}

    static std::string UpgradeValueCurve(EffectManager* effectManager, const std::string& name, const std::string& value, const std::string& effectName);

    // Return dynamic options for a setting. The setting name is the property id
    // (e.g. "SingleStrand_FX"). Returns empty vector if not dynamic.
    virtual std::vector<std::string> GetSettingOptions(const std::string& setting) const { return {}; }

    // JSON metadata describing the effect's properties (defaults, min/max, control
    // types, etc.). Loaded by EffectManager at startup from
    // resources/effectmetadata/<Name>.json. Empty for effects that don't yet have
    // a JSON descriptor (e.g. Moving Head).
    //
    // After storing the JSON, SetMetadata() calls OnMetadataLoaded() so
    // subclasses can extract values into their own cached members. Render()
    // must read from those cached members — never call the Get*Default /
    // Get*FromMetadata helpers during Render, they walk the JSON.
    void SetMetadata(nlohmann::json md);
    const nlohmann::json& GetMetadata() const;
    bool HasMetadata() const;

    // Look up a property by id (e.g. "On_Transparency") within the metadata's
    // "properties" array. Returns nullptr if not found or no metadata is loaded.
    // Walks the JSON — call from OnMetadataLoaded or other cold paths only.
    const nlohmann::json* GetPropertyMetadata(const std::string& id) const;

    // Convenience accessors for reading default/min/max/divisor out of the
    // metadata. Each returns `fallback` if the property or field is missing.
    // Intended for use from OnMetadataLoaded() to populate cached members —
    // NOT for use inside Render() or any hot path (they walk the JSON).
    int GetIntDefault(const std::string& id, int fallback) const;
    double GetDoubleDefault(const std::string& id, double fallback) const;
    bool GetBoolDefault(const std::string& id, bool fallback) const;
    std::string GetStringDefault(const std::string& id, const std::string& fallback) const;
    double GetMinFromMetadata(const std::string& id, double fallback) const;
    double GetMaxFromMetadata(const std::string& id, double fallback) const;
    int GetDivisorFromMetadata(const std::string& id, int fallback) const;

    // Value-curve min/max. Prefer the JSON `vcMin`/`vcMax` fields if present
    // (lets a property expose a narrower VC range than its slider range, e.g.
    // Twinkle_Steps which has slider max 400 but VC max 100). Falls back to
    // `min`/`max`, then to the provided fallback.
    double GetVCMinFromMetadata(const std::string& id, double fallback) const;
    double GetVCMaxFromMetadata(const std::string& id, double fallback) const;

protected:
    // Called by SetMetadata() after storing the JSON, giving subclasses a
    // single point to extract defaults/min/max/divisor into their own
    // cached member variables. Base impl is a no-op. Runs at startup only
    // (once per effect), so walking the JSON via GetIntDefault etc. here is
    // fine — those values end up in cheap plain-member reads at render time.
    virtual void OnMetadataLoaded() {}

    // Default implementations read from metadata (by stripping the
    // "E_VALUECURVE_" prefix from `name` and looking up the property id).
    // Only called from UpgradeValueCurve on sequence load — cold path, so
    // the JSON walk is acceptable. Effects with static JSON-described
    // properties get their min/max for free and do not need to override.
    // Effects with dynamic or non-metadata-described settings (e.g. Shader)
    // still override to intercept those specific names.
    virtual double GetSettingVCMin(const std::string& name) const;
    virtual double GetSettingVCMax(const std::string& name) const;
    virtual int GetSettingVCDivisor(const std::string& name) const;

    double GetValueCurveDouble(const std::string& name, double def, const SettingsMap& SettingsMap, float offset, double min, double max, long startMS, long endMS, int divisor = 1);
    int GetValueCurveInt(const std::string& name, int def, const SettingsMap& SettingsMap, float offset, int min, int max, long startMS, long endMS, int divisor = 1);
    int GetValueCurveIntMax(const std::string& name, int def, const SettingsMap& SettingsMap, int min, int max, int divisor = 1);
    EffectLayer* GetTiming(const std::string& timingtrack) const;
    Effect* GetCurrentTiming(const RenderBuffer& buffer, const std::string& timingtrack) const;
    std::string GetTimingTracks(const int maxLayers = 0, const int absoluteLayers = 0) const;
    bool IsVersionOlder(const std::string& compare, const std::string& version);

    std::string name;
    std::string tooltip;
    int id;
    SequenceElements* mSequenceElements;
    const char** iconData[5];

private:
    nlohmann::json mMetadata;
};
