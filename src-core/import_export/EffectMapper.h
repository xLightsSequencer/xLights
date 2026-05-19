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

#include <map>
#include <string>
#include <vector>

class Effect;
class EffectLayer;
class Element;
class SequenceElements;
class SequencePackage;

// Copy effects from a source effect layer to a target effect layer, applying
// import-time fixups (lock removal, duplicate-effect remap, embedded image
// transfer, per-preview camera resolution against the target's RenderContext).
//
// `mapped` collects every src layer touched (used by the caller to mark
// "already imported" sources).
// `mapping` is the source-model-name → target-model-name table — used to
// rewrite Duplicate effect references.
// `mappingModelType` is the source-model-name → ModelType table (used by the
// Element overload to suppress per-model render-style conversion when the
// source is itself a ModelGroup).
//
// `xsqPkg` provides access to source-sequence media (embedded images,
// `IsImportActive`, `FixAndImportMedia`).
void MapXLightsEffects(EffectLayer* target, EffectLayer* src,
                       std::vector<EffectLayer*>& mapped, bool eraseExisting,
                       SequencePackage& xsqPkg, bool lock,
                       const std::map<std::string, std::string>& mapping,
                       bool convertRender,
                       const std::map<std::string, std::string>& mappingModelType);

// Resolve `name` against `layerMap` (or fall through to `seqEl.GetElement`)
// and copy its first layer's effects onto `target`.
void MapXLightsStrandEffects(EffectLayer* target, const std::string& name,
                             std::map<std::string, EffectLayer*>& layerMap,
                             SequenceElements& seqEl,
                             std::vector<EffectLayer*>& mapped, bool eraseExisting,
                             SequencePackage& xsqPkg, bool lock,
                             const std::map<std::string, std::string>& mapping,
                             const std::map<std::string, std::string>& mappingModelType);

// Resolve `name` against `layerMap` / `elementMap` / `seqEl` and copy effects
// across every matching layer onto `target`. Adds layers to `target` if the
// source has more.
void MapXLightsEffects(Element* target, const std::string& name,
                       SequenceElements& seqEl,
                       std::map<std::string, Element*>& elementMap,
                       std::map<std::string, EffectLayer*>& layerMap,
                       std::vector<EffectLayer*>& mapped, bool eraseExisting,
                       SequencePackage& xsqPkg, bool lock,
                       const std::map<std::string, std::string>& mapping,
                       bool convertRender,
                       const std::map<std::string, std::string>& mappingModelType);
