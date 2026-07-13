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

// C ABI interface that effect plugin DLLs must export.
//
// A plugin DLL must export exactly two C functions:
//   RenderableEffect* xlCreateEffectPlugin(int id)     — allocate and return a new effect with the given id
//   void              xlDestroyEffectPlugin(RenderableEffect* effect) — destroy an effect returned by the above
//
// `id` is assigned by EffectManager at load time (starting after the last
// built-in RGB_EFFECTS_e value) and must be passed through to the
// RenderableEffect base constructor unchanged.
//
// Use the XL_EFFECT_PLUGIN_EXPORT(ClassName) macro in your DLL's .cpp to
// generate these, where ClassName's constructor takes a single `int id`.
//
// Example plugin DLL:
//   #include "effectPlugin.h"
//   #include "MyEffect.h"
//   XL_EFFECT_PLUGIN_EXPORT(MyEffect)

#include "utils/pluginExport.h"

class RenderableEffect;

extern "C" {
    typedef RenderableEffect* (*xlCreateEffectPluginFn)(int id);
    typedef void               (*xlDestroyEffectPluginFn)(RenderableEffect* effect);
}

// Place this macro once in a plugin DLL .cpp file to export the required entry points.
#define XL_EFFECT_PLUGIN_EXPORT(ClassName)                                              \
    extern "C" XL_PLUGIN_EXPORT_ATTR RenderableEffect* xlCreateEffectPlugin(int id) {    \
        return new ClassName(id);                                                       \
    }                                                                                    \
    extern "C" XL_PLUGIN_EXPORT_ATTR void xlDestroyEffectPlugin(RenderableEffect* effect) { \
        delete effect;                                                                   \
    }
