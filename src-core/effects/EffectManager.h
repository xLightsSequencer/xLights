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

#include "effectPlugin.h"
#include "utils/PluginLoader.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class RenderableEffect;

class EffectManager
{
public:
    //TODO - get rid of this, the internal id's should be irrelevant
    enum RGB_EFFECTS_e {
        eff_OFF,
        eff_ON,
        eff_ADJUST,
        eff_BARS,
        eff_BUTTERFLY,
        eff_CANDLE,
        eff_CIRCLES,
        eff_COLORWASH,
        eff_CURTAIN,
        eff_DMX,
        eff_DUPLICATE,
        eff_FACES,
        eff_FAN,
        eff_FILL,
        eff_FIRE,
        eff_FIREWORKS,
        eff_GALAXY,
        eff_GARLANDS,
        eff_GLEDIATOR,
        eff_GUITAR,
        eff_KALEIDOSCOPE,
        eff_LIFE,
        eff_LIGHTNING,
        eff_LINES,
		eff_LIQUID,
        eff_MARQUEE,
        eff_METEORS,
        eff_MORPH,
        eff_MOVINGHEAD,
        eff_MUSIC,
        eff_PIANO,
        eff_PICTURES,
        eff_PINWHEEL,
        eff_PLASMA,
        eff_RIPPLE,
        eff_SERVO,
        eff_SHADER,
        eff_SHAPE,
        eff_SHIMMER,
        eff_SHOCKWAVE,
        eff_SINGLESTRAND,
        eff_SKETCH,
        eff_SNOWFLAKES,
        eff_SNOWSTORM,
        eff_SPIRALS,
        eff_SPIROGRAPH,
        eff_STATE,
        eff_STROBE,
		eff_TENDRIL,
		eff_TEXT,
		eff_TREE,
		eff_TWINKLE,
		eff_VIDEO,
		eff_VUMETER,
        eff_WARP,
        eff_WAVE,
        eff_LASTEFFECT //Always the last entry
    };


    public:
        // metadataDir is the absolute path to resources/effectmetadata. The
        // UI layer passes this in because resource location is a UI concern.
        // If empty, effects are created without metadata (their legacy
        // hard-coded defaults still apply).
        explicit EffectManager(std::string metadataDir = "");
        virtual ~EffectManager();


        RenderableEffect *GetEffect(int i) const {
            if (i >= (int)size() || i < 0) {
                return nullptr;
            }
            return effects[i];
        }

        RenderableEffect *GetEffect(const std::string &str) const;
        int GetLastEffectId() const { return (int)size() - 1; }

        // One entry per plugin file loadEffectPlugins() found, whether or not
        // it loaded successfully - surfaced to the Plugins Preferences UI so
        // load failures (missing exports, threw, returned null) are visible
        // somewhere other than the log file.
        struct PluginInfo {
            std::string name;   // effect name (empty if load failed before a name was known)
            std::string path;   // plugin file path
            bool loaded = false;
            std::string error;  // human-readable failure reason; empty if loaded
        };

        // Scans pluginDir for effect plugin DLLs (exporting xlCreateEffectPlugin /
        // xlDestroyEffectPlugin) and registers each one, assigning ids starting
        // right after the last built-in RGB_EFFECTS_e value. Safe to call with
        // a non-existent directory.
        void loadEffectPlugins(const std::string& pluginDir);

        const std::vector<PluginInfo>& GetPluginInfo() const { return m_pluginInfo; }

        int GetEffectIndex(const std::string &effectName) const;
        const std::string &GetEffectName(int idx) const;

        std::vector<RenderableEffect*>::const_iterator begin() const;
        std::vector<RenderableEffect*>::const_iterator end() const;
        RenderableEffect *operator[](int i) const {
            return GetEffect(i);
        }
        size_t size() const {
            return effects.size();
        }
    protected:
        RenderableEffect *createEffect(RGB_EFFECTS_e eff);
        void add(RenderableEffect *eff);
        // Loads the JSON metadata file for the given effect (if present) and
        // attaches it via SetMetadata(). Silently no-ops if mMetadataDir is
        // empty or the file does not exist.
        void loadMetadataInto(RenderableEffect *eff);
    private:

        std::string mMetadataDir;
        mutable std::map<std::string, RenderableEffect *> effectsByName;
        std::vector<RenderableEffect *> effects;

        // Declared before m_pluginEffects so it (and the DLL handles it owns)
        // is destroyed AFTER m_pluginEffects (reverse declaration order),
        // guaranteeing the DLLs stay mapped while plugin effect destructors run.
        PluginLoader<RenderableEffect, int> m_pluginLoader{
            {"EffectPlugin", "xlCreateEffectPlugin", "xlDestroyEffectPlugin", "EffectManager"}
        };
        // Owns effects created by loadEffectPlugins(); raw pointers into these
        // also live in `effects` (ids >= eff_LASTEFFECT), but ~EffectManager's
        // delete loop skips those ids since ownership/destruction belongs here.
        std::vector<std::unique_ptr<RenderableEffect, xlDestroyEffectPluginFn>> m_pluginEffects;
        std::vector<PluginInfo> m_pluginInfo;
};
