/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "iPadRenderContext.h"

#include "render/Element.h"
#include "render/EffectLayer.h"
#include "render/Effect.h"
#include "render/FSEQFile.h"
#include "render/IRenderJobStatus.h"
#include "render/RenderProgressInfo.h"
#include "render/SequenceMedia.h"
#include "xLightsVersion.h"
#include <map>
#include "effects/ShaderEffect.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/MeshObject.h"
#include "models/ImageObject.h"
#include "models/GridlinesObject.h"
#include "models/TerrainObject.h"
#include "models/RulerObject.h"
#include "models/TwoPointScreenLocation.h"
#include "models/BoxedScreenLocation.h"
#include "models/TerrainScreenLocation.h"
#include "models/MatrixModel.h"
#include "models/ModelScreenLocation.h"
#include "models/Node.h"
#include "render/ValueCurve.h"
#include "utils/Color.h"
#include "utils/ExternalHooks.h"
#include "XmlSerializer/XmlSerializingVisitor.h"
#include "XmlSerializer/XmlSerializer.h"

#include <pugixml.hpp>
#include "utils/FileUtils.h"
#include <log.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <mutex>
#include <string_view>
#include <system_error>
#include <thread>
#include <vector>

iPadRenderContext::iPadRenderContext()
    : _effectManager(FileUtils::GetResourcesDir() + "/effectmetadata"),
      _sequenceElements(this) {
    // Tier 1 memory-pressure mitigation:
    //   * Cap the disk-backed render cache at 50 MB so iPadOS
    //     doesn't balloon its memory-resident frame map by loading
    //     a huge desktop-authored cache at sequence open.
    //   * Cap the undo history at 50 steps — every DeletedEffect /
    //     ModifiedEffect snapshot can be several KB of settings
    //     strings, so 2000-edit sessions on a long show add up.
    //     iPad users don't need desktop-scale undo depth anyway.
    _renderCache.SetMaximumSizeMB(50);
    _sequenceElements.get_undo_mgr().SetMaxSteps(50);
}

iPadRenderContext::~iPadRenderContext() {
    CloseSequence();
}

bool iPadRenderContext::LoadShowFolder(const std::string& showDir) {
    return LoadShowFolder(showDir, {});
}

bool iPadRenderContext::LoadShowFolder(const std::string& showDir,
                                       const std::list<std::string>& mediaFolders) {
    _showDir = showDir;
    _mediaFolders.clear();

    if (!ObtainAccessToURL(showDir, false)) {
        // Stale security-scoped bookmark for the show folder itself —
        // every subsequent file open will fail. Log loudly and continue;
        // the caller will see the load report empty models / settings
        // and surface a re-pick prompt.
        spdlog::warn("iPadRenderContext: ObtainAccessToURL failed for show folder '{}' — bookmark likely stale", showDir);
    }
    for (const auto& folder : mediaFolders) {
        if (ObtainAccessToURL(folder, false)) {
            _mediaFolders.push_back(folder);
        } else {
            // Drop the folder entirely so FileUtils doesn't try to
            // resolve assets through a path it can't actually read.
            // Without this drop, FixFile silently returns broken paths
            // and the user sees missing-media warnings with no clue
            // that the bookmark went stale.
            spdlog::warn("iPadRenderContext: ObtainAccessToURL failed for media folder '{}' — dropping from search list", folder);
        }
    }

    // Wire the show dir + media folders into FileUtils::FixFile so that
    // sequence references (audio, videos, images, 3D meshes, shaders, etc.)
    // that were saved with absolute paths from another machine get re-resolved
    // against the iPad's current show/media locations. Without this,
    // _fixFileSearchDirs stays empty and FixFile has no way to relocate
    // assets — the raw saved paths fall straight through and FileExists fails.
    FileUtils::SetFixFileShowDir(showDir);
    FileUtils::SetFixFileDirectories(_mediaFolders);
    FileUtils::ClearNonExistentFiles();

    // Load network/controller configuration
    if (!_outputManager.Load(showDir)) {
        spdlog::warn("iPadRenderContext: Failed to load xlights_networks.xml from {}", showDir);
    }

    // Create ModelManager + ViewObjectManager
    _modelManager = std::make_unique<ModelManager>(&_outputManager, this);
    _viewObjectManager = std::make_unique<ViewObjectManager>(this);
    _viewsManager.SetModelManager(_modelManager.get());

    // Load models from xlights_rgbeffects.xml
    std::string rgbPath = showDir + "/xlights_rgbeffects.xml";
    ObtainAccessToURL(rgbPath, false);

    spdlog::info("iPadRenderContext: Loading rgbeffects from {}", rgbPath);
    if (FileExists(rgbPath)) {
        spdlog::info("iPadRenderContext: File exists: {}", rgbPath);
    } else {
        spdlog::error("iPadRenderContext: File NOT found: {}", rgbPath);
    }

    pugi::xml_document doc;
    auto result = doc.load_file(rgbPath.c_str());
    if (result) {
        auto xlightsNode = doc.child("xrgb");
        if (!xlightsNode) {
            xlightsNode = doc.child("xlights");
        }
        if (!xlightsNode) {
            spdlog::error("iPadRenderContext: No <xrgb> or <xlights> root element in {}", rgbPath);
        } else {
            // Preview canvas size lives in the <settings> node as
            // <previewWidth value="..."/> / <previewHeight value="..."/>.
            // Desktop falls back to 1280x720 when absent; match that.
            auto settingsNode = xlightsNode.child("settings");
            if (settingsNode) {
                for (auto s = settingsNode.first_child(); s; s = s.next_sibling()) {
                    std::string name = s.name();
                    const char* v = s.attribute("value").as_string();
                    if (name == "previewWidth") {
                        int w = (int)std::strtol(v, nullptr, 10);
                        if (w > 0) _previewWidth = w;
                    } else if (name == "previewHeight") {
                        int h = (int)std::strtol(v, nullptr, 10);
                        if (h > 0) _previewHeight = h;
                    } else if (name == "Display2DCenter0") {
                        _display2DCenter0 = (std::string(v) == "1");
                    } else if (name == "Display2DGrid") {
                        _display2DGrid = (std::string(v) == "1");
                    } else if (name == "Display2DGridSpacing") {
                        long sp = std::strtol(v, nullptr, 10);
                        if (sp > 0) _display2DGridSpacing = sp;
                    } else if (name == "Display2DBoundingBox") {
                        _display2DBoundingBox = (std::string(v) == "1");
                    } else if (name == "LayoutMode3D") {
                        _layoutMode3D = (std::string(v) == "1");
                    } else if (name == "backgroundImage") {
                        _backgroundImage = v;
                    } else if (name == "backgroundBrightness") {
                        int b = (int)std::strtol(v, nullptr, 10);
                        if (b >= 0) _backgroundBrightness = b;
                    } else if (name == "backgroundAlpha") {
                        int a = (int)std::strtol(v, nullptr, 10);
                        if (a >= 0) _backgroundAlpha = a;
                    } else if (name == "scaleImage") {
                        _scaleBackgroundImage = (std::strtol(v, nullptr, 10) > 0);
                    }
                }
            }
            // Resolve the background image against the show folder / media
            // directories. FixFile handles both absolute paths (from a
            // different machine's filesystem) and plain filenames.
            if (!_backgroundImage.empty()) {
                _backgroundImage = FileUtils::FixFile(_showDir, _backgroundImage);
                ObtainAccessToURL(_backgroundImage, false);
                if (!FileExists(_backgroundImage)) {
                    spdlog::warn("iPadRenderContext: background image not found: {}",
                                 _backgroundImage);
                    _backgroundImage.clear();
                }
            }
            spdlog::info("iPadRenderContext: Preview canvas {}x{}, mode3D={}, center2D0={}, bg='{}' bri={} alpha={} scale={}",
                         _previewWidth, _previewHeight, _layoutMode3D,
                         _display2DCenter0, _backgroundImage,
                         _backgroundBrightness, _backgroundAlpha,
                         _scaleBackgroundImage);

            // Named layout groups — each gets its own background stack.
            // Desktop writes these under `<layoutGroups><layoutGroup …/>`
            // with attribute-style values (not `<settings>` children).
            _namedLayoutGroups.clear();
            auto layoutGroupsNode = xlightsNode.child("layoutGroups");
            if (layoutGroupsNode) {
                for (auto lg = layoutGroupsNode.first_child(); lg; lg = lg.next_sibling()) {
                    if (std::string_view(lg.name()) != "layoutGroup") continue;
                    NamedLayoutGroup g;
                    g.name = lg.attribute("name").as_string("");
                    if (g.name.empty()) continue;
                    g.backgroundImage = lg.attribute("backgroundImage").as_string("");
                    g.backgroundBrightness = lg.attribute("backgroundBrightness").as_int(100);
                    g.backgroundAlpha = lg.attribute("backgroundAlpha").as_int(100);
                    g.scaleBackgroundImage = lg.attribute("scaleImage").as_int(0) > 0;
                    if (!g.backgroundImage.empty()) {
                        g.backgroundImage = FileUtils::FixFile(_showDir, g.backgroundImage);
                        ObtainAccessToURL(g.backgroundImage, false);
                        if (!FileExists(g.backgroundImage)) {
                            spdlog::warn("iPadRenderContext: layoutGroup '{}' bg not found: {}",
                                         g.name, g.backgroundImage);
                            g.backgroundImage.clear();
                        }
                    }
                    _namedLayoutGroups.push_back(std::move(g));
                }
                spdlog::info("iPadRenderContext: Loaded {} named layout groups",
                             _namedLayoutGroups.size());
            }

            auto modelsNode = xlightsNode.child("models");
            if (!modelsNode) {
                spdlog::error("iPadRenderContext: No <models> element in {}", rgbPath);
            } else {
                _modelManager->LoadModels(modelsNode, _previewWidth, _previewHeight);
                spdlog::info("iPadRenderContext: Loaded {} models", _modelManager->GetModels().size());

                // Load model groups
                auto groupsNode = xlightsNode.child("modelGroups");
                if (groupsNode) {
                    _modelManager->LoadGroups(groupsNode, _previewWidth, _previewHeight);
                    spdlog::info("iPadRenderContext: Loaded groups, total models now {}",
                                 _modelManager->GetModels().size());
                }

                // Load view objects (house meshes, ground images, gridlines, terrain, rulers)
                auto viewObjectsNode = xlightsNode.child("view_objects");
                if (viewObjectsNode) {
                    _viewObjectManager->LoadViewObjects(viewObjectsNode);
                    spdlog::info("iPadRenderContext: Loaded {} view objects",
                                 _viewObjectManager->size());
                }

                // Load saved views. `SequenceViewManager::GetViews()` always
                // ensures a Master View entry, but the rest (Christmas,
                // Halloween, etc.) come from the <views> node.
                auto viewsNode = xlightsNode.child("views");
                if (viewsNode) {
                    _viewsManager.Load(viewsNode, 0);
                    spdlog::info("iPadRenderContext: Loaded {} views",
                                 _viewsManager.GetViewCount());
                }

                // Viewpoints — saved camera positions (2D/3D separated).
                // Desktop exposes these via the preview right-click menu
                // (ModelPreview context menu). On iPad we surface them
                // through the preview controls overlay.
                auto viewpointsNode = xlightsNode.child("Viewpoints");
                _viewpointMgr.Clear();
                if (viewpointsNode) {
                    _viewpointMgr.Load(viewpointsNode);
                    spdlog::info("iPadRenderContext: Loaded viewpoints (2D={}, 3D={})",
                                 _viewpointMgr.GetNum2DCameras(),
                                 _viewpointMgr.GetNum3DCameras());
                }
            }
        }
    } else {
        spdlog::error("iPadRenderContext: Failed to load {}: {}", rgbPath, result.description());
    }

    // Load the user-customised <colors> palette so brackets / labels /
    // gridlines pick up the same look the user configured on desktop.
    // Re-uses the doc loaded above when possible, falls through to a
    // separate parse if loading failed.
    _palette.clear();
    auto loadPaletteFrom = [this](const pugi::xml_node& root) {
        auto colorsNode = root.child("colors");
        if (!colorsNode) return;
        for (auto c = colorsNode.first_child(); c; c = c.next_sibling()) {
            PaletteColor pc;
            pc.r = (uint8_t)std::clamp(c.attribute("Red").as_int(0), 0, 255);
            pc.g = (uint8_t)std::clamp(c.attribute("Green").as_int(0), 0, 255);
            pc.b = (uint8_t)std::clamp(c.attribute("Blue").as_int(0), 0, 255);
            _palette[c.name()] = pc;
        }
    };
    if (result) {
        auto root = doc.child("xrgb");
        if (!root) root = doc.child("xlights");
        if (root) loadPaletteFrom(root);
    }

    return true;
}

iPadRenderContext::PaletteColor
iPadRenderContext::GetEffectBracketColor(EffectBracketState state) const {
    // Defaults mirror ColorManager::xLights_color[] in
    // src-ui-wx/color/ColorManager.h. Names match the strings desktop
    // writes into <colors> so a user's customised palette overrides
    // the default.
    const char* key = nullptr;
    PaletteColor fallback;
    switch (state) {
        case EffectBracketState::Default:
            key = "EffectDefault";
            fallback = {192, 192, 192};
            break;
        case EffectBracketState::Selected:
            key = "EffectSelected";
            fallback = {204, 102, 255};
            break;
        case EffectBracketState::Locked:
            key = "LockedEffect";
            fallback = {200, 0, 0};
            break;
        case EffectBracketState::Disabled:
            key = "DisabledEffect";
            fallback = {200, 200, 0};
            break;
    }
    auto it = _palette.find(key);
    return it == _palette.end() ? fallback : it->second;
}

bool iPadRenderContext::SaveViewpoints() {
    if (_showDir.empty()) return false;
    std::string rgbPath = _showDir + "/xlights_rgbeffects.xml";
    if (!ObtainAccessToURL(rgbPath, true)) {
        spdlog::warn("iPadRenderContext::SaveViewpoints: ObtainAccessToURL failed for '{}' — write will likely fail", rgbPath);
    }

    pugi::xml_document doc;
    auto result = doc.load_file(rgbPath.c_str());
    if (!result) {
        spdlog::error("iPadRenderContext::SaveViewpoints: load failed: {}",
                      result.description());
        return false;
    }
    auto root = doc.child("xrgb");
    if (!root) root = doc.child("xlights");
    if (!root) {
        spdlog::error("iPadRenderContext::SaveViewpoints: no root element");
        return false;
    }

    // Remove the old Viewpoints subtree and rewrite from the in-memory
    // ViewpointMgr. ViewpointMgr::Save walks the visitor which creates
    // a fresh <Viewpoints> child under the target node.
    while (auto existing = root.child("Viewpoints")) {
        root.remove_child(existing);
    }
    XmlSerializingVisitor visitor(root);
    _viewpointMgr.Save(visitor);

    if (!doc.save_file(rgbPath.c_str(), "  ")) {
        spdlog::error("iPadRenderContext::SaveViewpoints: write failed for {}",
                      rgbPath);
        return false;
    }
    return true;
}

bool iPadRenderContext::SaveModelStates() {
    if (_dirtyStateModels.empty()) return true;
    if (_showDir.empty()) return false;

    std::string rgbPath = _showDir + "/xlights_rgbeffects.xml";
    if (!ObtainAccessToURL(rgbPath, true)) {
        spdlog::warn("iPadRenderContext::SaveModelStates: ObtainAccessToURL failed for '{}' — write will likely fail", rgbPath);
    }

    pugi::xml_document doc;
    auto result = doc.load_file(rgbPath.c_str());
    if (!result) {
        spdlog::error("iPadRenderContext::SaveModelStates: load failed: {}",
                      result.description());
        return false;
    }
    auto root = doc.child("xrgb");
    if (!root) root = doc.child("xlights");
    if (!root) {
        spdlog::error("iPadRenderContext::SaveModelStates: no root element");
        return false;
    }
    auto modelsNode = root.child("models");
    if (!modelsNode) {
        spdlog::error("iPadRenderContext::SaveModelStates: no <models> element");
        return false;
    }

    for (const auto& modelName : _dirtyStateModels) {
        Model* m = _modelManager ? _modelManager->GetModel(modelName) : nullptr;
        if (!m) {
            spdlog::warn("iPadRenderContext::SaveModelStates: model '{}' not in manager — skipping",
                         modelName);
            continue;
        }
        // Find the matching <model> child by Name attribute.
        pugi::xml_node modelNode;
        for (auto n = modelsNode.first_child(); n; n = n.next_sibling()) {
            if (std::string_view(n.name()) != "model") continue;
            if (modelName == n.attribute("name").as_string()) {
                modelNode = n;
                break;
            }
        }
        if (!modelNode) {
            spdlog::warn("iPadRenderContext::SaveModelStates: <model name='{}'> not found in xml — skipping",
                         modelName);
            continue;
        }
        // Drop existing <stateInfo> children, then rewrite from the live map.
        // WriteStateInfo prepends, so the on-disk order ends up reversed
        // relative to the in-memory map iteration order — same behaviour
        // desktop has, so this matches the canonical file layout.
        while (auto existing = modelNode.child("stateInfo")) {
            modelNode.remove_child(existing);
        }
        Model::WriteStateInfo(modelNode, m->GetStateInfo());
    }

    if (!doc.save_file(rgbPath.c_str(), "  ")) {
        spdlog::error("iPadRenderContext::SaveModelStates: write failed for {}",
                      rgbPath);
        return false;
    }
    _dirtyStateModels.clear();
    return true;
}

bool iPadRenderContext::SaveLayoutChanges() {
    if (_dirtyLayoutModels.empty() &&
        _dirtyLayoutViewObjects.empty() &&
        _createdGroups.empty() &&
        _deletedGroups.empty() &&
        _createdViewObjects.empty() &&
        _deletedViewObjects.empty() &&
        _dirtyBackgroundGroups.empty() &&
        _renamedGroups.empty() &&
        _renamedViewObjects.empty() &&
        _renamedModels.empty()) {
        return true;
    }
    if (_showDir.empty()) return false;

    std::string rgbPath = _showDir + "/xlights_rgbeffects.xml";
    if (!ObtainAccessToURL(rgbPath, true)) {
        spdlog::warn("iPadRenderContext::SaveLayoutChanges: ObtainAccessToURL failed for '{}' — write will likely fail", rgbPath);
    }

    // Always copy the current on-disk file to a single rolling
    // backup before overwriting. The user can `cp` it back if a
    // session of testing turns out badly. The backup intentionally
    // overwrites itself each save so it doesn't accumulate; one
    // step of recovery is the explicit goal.
    if (FileExists(rgbPath)) {
        std::string backupPath = rgbPath + ".iPad-bkp";
        std::error_code ec;
        std::filesystem::copy_file(rgbPath, backupPath,
                                    std::filesystem::copy_options::overwrite_existing,
                                    ec);
        if (ec) {
            spdlog::warn("iPadRenderContext::SaveLayoutChanges: backup copy {} failed: {}",
                         backupPath, ec.message());
        }
    }

    pugi::xml_document doc;
    auto result = doc.load_file(rgbPath.c_str());
    if (!result) {
        spdlog::error("iPadRenderContext::SaveLayoutChanges: load failed: {}",
                      result.description());
        return false;
    }
    auto root = doc.child("xrgb");
    if (!root) root = doc.child("xlights");
    if (!root) {
        spdlog::error("iPadRenderContext::SaveLayoutChanges: no root element");
        return false;
    }
    auto modelsNode = root.child("models");
    if (!modelsNode) {
        spdlog::error("iPadRenderContext::SaveLayoutChanges: no <models> element");
        return false;
    }
    auto modelGroupsNode = root.child("modelGroups");
    // J-7 — if a brand-new group landed before the file ever had
    // any groups, ensure the <modelGroups> container exists. The
    // desktop happily reads xml without a <modelGroups> node, so
    // any show that's never had groups won't have one.
    if (!modelGroupsNode && !_createdGroups.empty()) {
        modelGroupsNode = root.append_child("modelGroups");
    }

    // J-7 (group CRUD) — Pass 0a: drop deleted groups so the
    // subsequent passes don't find stale elements.
    if (modelGroupsNode && !_deletedGroups.empty()) {
        for (const auto& deletedName : _deletedGroups) {
            for (auto n = modelGroupsNode.first_child(); n; ) {
                auto next = n.next_sibling();
                if (std::string_view(n.name()) == "modelGroup" &&
                    deletedName == n.attribute("name").as_string()) {
                    modelGroupsNode.remove_child(n);
                }
                n = next;
            }
        }
    }

    // J-7 — Pass 0b: append a fresh <modelGroup> element for each
    // newly-created group, populated from the live in-memory
    // ModelGroup. Subsequent passes may patch additional attrs if
    // the user edited the group after creating it.
    if (modelGroupsNode && !_createdGroups.empty() && _modelManager) {
        for (const auto& createdName : _createdGroups) {
            Model* m = _modelManager->GetModel(createdName);
            if (!m || m->GetDisplayAs() != DisplayAsType::ModelGroup) {
                spdlog::warn("iPadRenderContext::SaveLayoutChanges: created group '{}' not in manager — skipping",
                             createdName);
                continue;
            }
            auto* g = static_cast<ModelGroup*>(m);
            pugi::xml_node node = modelGroupsNode.append_child("modelGroup");
            node.append_attribute("name")           = createdName.c_str();
            node.append_attribute("LayoutGroup")    = g->GetLayoutGroup().c_str();
            node.append_attribute("layout")         = g->GetLayout().c_str();
            node.append_attribute("DefaultCamera")  = g->GetDefaultCamera().c_str();
            node.append_attribute("GridSize")       = g->GetGridSize();
            node.append_attribute("centreX")        = std::to_string(g->GetCentreX()).c_str();
            node.append_attribute("centreY")        = std::to_string(g->GetCentreY()).c_str();
            node.append_attribute("centreDefined")  = std::to_string(g->GetCentreDefined()).c_str();
            node.append_attribute("selected")       = "0";
            std::string members;
            for (size_t i = 0; i < g->ModelNames().size(); ++i) {
                if (i > 0) members += ",";
                members += g->ModelNames()[i];
            }
            node.append_attribute("models") = members.c_str();
            // Strip from dirty set — we've just written everything
            // we know about this group, no need to patch it too.
            _dirtyLayoutModels.erase(createdName);
        }
    }

    // For each dirty model, serialize the in-memory Model into a fresh
    // pugi::xml_document via the canonical XmlSerializer (same path
    // desktop uses for export). Replace the matching <model> child of
    // the on-disk <models> node with the serialized one. Preserves
    // every attribute the Model owns — transforms, dimensions,
    // rotation, locked, layoutGroup, controllerName, plus model-type-
    // specific attributes the live edit may have side-effected.
    for (const auto& modelName : _dirtyLayoutModels) {
        Model* m = _modelManager ? _modelManager->GetModel(modelName) : nullptr;
        if (!m) {
            spdlog::warn("iPadRenderContext::SaveLayoutChanges: model '{}' not in manager — skipping",
                         modelName);
            continue;
        }

        // ModelGroups live in `<modelGroups>`, not `<models>`, and
        // their on-disk form is a flat attribute list — no nested
        // child elements. Patch attributes in place rather than
        // serializing through XmlSerializer (which targets `<model>`).
        if (m->GetDisplayAs() == DisplayAsType::ModelGroup) {
            if (!modelGroupsNode) {
                spdlog::warn("iPadRenderContext::SaveLayoutChanges: dirty group '{}' but no <modelGroups> element",
                             modelName);
                continue;
            }
            // J-16 — rename support. If this group was renamed in
            // memory, the on-disk `<modelGroup>` still has the OLD
            // name. Look it up via the renames map, find by old
            // name, then update the name attribute below.
            std::string findName = modelName;
            bool renamed = false;
            if (auto it = _renamedGroups.find(modelName); it != _renamedGroups.end()) {
                findName = it->second;
                renamed = true;
            }
            pugi::xml_node existing;
            for (auto n = modelGroupsNode.first_child(); n; n = n.next_sibling()) {
                if (std::string_view(n.name()) != "modelGroup") continue;
                if (findName == n.attribute("name").as_string()) {
                    existing = n;
                    break;
                }
            }
            if (!existing) {
                spdlog::warn("iPadRenderContext::SaveLayoutChanges: <modelGroup name='{}'> not found",
                             findName);
                continue;
            }
            if (renamed) {
                if (existing.attribute("name")) existing.remove_attribute("name");
                existing.append_attribute("name") = modelName.c_str();
            }
            ModelGroup* g = static_cast<ModelGroup*>(m);
            auto setAttr = [&](const char* k, const std::string& v) {
                if (existing.attribute(k)) existing.remove_attribute(k);
                existing.append_attribute(k) = v.c_str();
            };
            auto setAttrInt = [&](const char* k, int v) {
                if (existing.attribute(k)) existing.remove_attribute(k);
                existing.append_attribute(k) = v;
            };
            setAttr("LayoutGroup",    g->GetLayoutGroup());
            setAttr("layout",         g->GetLayout());
            setAttr("DefaultCamera",  g->GetDefaultCamera());
            setAttrInt("GridSize",    g->GetGridSize());
            setAttr("centreX",        std::to_string(g->GetCentreX()));
            setAttr("centreY",        std::to_string(g->GetCentreY()));
            setAttr("centreDefined",  std::to_string(g->GetCentreDefined()));
            // J-7 (group CRUD) — write the comma-delimited member
            // list so add/remove member edits persist.
            std::string members;
            for (size_t i = 0; i < g->ModelNames().size(); ++i) {
                if (i > 0) members += ",";
                members += g->ModelNames()[i];
            }
            setAttr("models", members);
            continue;
        }

        XmlSerializer serializer;
        pugi::xml_document modelDoc = serializer.SerializeModel(m);
        pugi::xml_node serRoot = modelDoc.document_element();
        if (!serRoot) continue;
        pugi::xml_node serModel = serRoot.first_child();
        if (!serModel) continue;

        // J-18 — rename support. If this model was renamed in
        // memory, the on-disk `<model>` still has the OLD name.
        // Find by old name (taken from the renames map keyed by
        // new) and let `insert_copy_before` swap it for the
        // serialized copy (which already carries the new name).
        std::string findName = modelName;
        if (auto it = _renamedModels.find(modelName); it != _renamedModels.end()) {
            findName = it->second;
        }
        pugi::xml_node existing;
        for (auto n = modelsNode.first_child(); n; n = n.next_sibling()) {
            if (std::string_view(n.name()) != "model") continue;
            if (findName == n.attribute("name").as_string()) {
                existing = n;
                break;
            }
        }
        if (!existing) {
            spdlog::warn("iPadRenderContext::SaveLayoutChanges: <model name='{}'> not found in xml — appending",
                         findName);
            modelsNode.append_copy(serModel);
            continue;
        }
        // Replace by inserting the serialized copy before the old node
        // and removing the old node — pugixml has no "replace_child".
        modelsNode.insert_copy_before(serModel, existing);
        modelsNode.remove_child(existing);
    }

    // J-6 — patch `<view_object>` attributes in place for each
    // dirty view object. View-object on-disk form is a flat
    // attribute list, so we patch the screen-location attribs the
    // user can edit (WorldPos / Scale / Rotate / Locked /
    // LayoutGroup). Per-type attributes (Mesh's ObjFile, Image's
    // bitmap path) round-trip untouched.
    // J-12 — view-object create/delete + per-object patch.
    if (!_dirtyLayoutViewObjects.empty() ||
        !_createdViewObjects.empty() ||
        !_deletedViewObjects.empty()) {
        auto viewObjectsNode = root.child("view_objects");
        if (!viewObjectsNode && !_createdViewObjects.empty()) {
            viewObjectsNode = root.append_child("view_objects");
        }
        if (!viewObjectsNode) {
            spdlog::warn("iPadRenderContext::SaveLayoutChanges: dirty view objects but no <view_objects> element — skipping");
        } else {
            // Drop deleted view objects first.
            for (const auto& deletedName : _deletedViewObjects) {
                for (auto n = viewObjectsNode.first_child(); n; ) {
                    auto next = n.next_sibling();
                    if (std::string_view(n.name()) == "view_object" &&
                        deletedName == n.attribute("name").as_string()) {
                        viewObjectsNode.remove_child(n);
                    }
                    n = next;
                }
            }
            // Append fresh elements for created view objects. The
            // common patcher below then writes their per-type
            // attrs (since we add the name to the dirty set
            // before falling through).
            if (_viewObjectManager) {
                for (const auto& createdName : _createdViewObjects) {
                    ViewObject* vo = _viewObjectManager->GetViewObject(createdName);
                    if (!vo) {
                        spdlog::warn("iPadRenderContext::SaveLayoutChanges: created VO '{}' not in manager",
                                     createdName);
                        continue;
                    }
                    pugi::xml_node node = viewObjectsNode.append_child("view_object");
                    node.append_attribute("name") = createdName.c_str();
                    node.append_attribute("DisplayAs") = vo->GetDisplayAsString().c_str();
                    // Defer the rest to the dirty patcher.
                    _dirtyLayoutViewObjects.insert(createdName);
                }
            }
        }
    }
    if (!_dirtyLayoutViewObjects.empty()) {
        auto viewObjectsNode = root.child("view_objects");
        if (!viewObjectsNode) {
            spdlog::warn("iPadRenderContext::SaveLayoutChanges: dirty view objects but no <view_objects> element — skipping");
        } else {
            ViewObjectManager& vm = *_viewObjectManager;
            for (const auto& objName : _dirtyLayoutViewObjects) {
                ViewObject* vo = vm.GetViewObject(objName);
                if (!vo) {
                    spdlog::warn("iPadRenderContext::SaveLayoutChanges: view object '{}' not in manager — skipping",
                                 objName);
                    continue;
                }
                // J-17 — rename support: if this VO was renamed
                // in memory, the on-disk `<view_object>` still
                // has the OLD name. Look up via the renames map
                // and update the name attribute below.
                std::string findName = objName;
                bool renamed = false;
                if (auto it = _renamedViewObjects.find(objName);
                    it != _renamedViewObjects.end()) {
                    findName = it->second;
                    renamed = true;
                }
                pugi::xml_node existing;
                for (auto n = viewObjectsNode.first_child(); n; n = n.next_sibling()) {
                    if (std::string_view(n.name()) != "view_object") continue;
                    if (findName == n.attribute("name").as_string()) {
                        existing = n;
                        break;
                    }
                }
                if (!existing) {
                    spdlog::warn("iPadRenderContext::SaveLayoutChanges: <view_object name='{}'> not found",
                                 findName);
                    continue;
                }
                if (renamed) {
                    if (existing.attribute("name")) existing.remove_attribute("name");
                    existing.append_attribute("name") = objName.c_str();
                }
                auto& loc = vo->GetObjectScreenLocation();
                auto setAttr = [&](const char* k, const std::string& v) {
                    if (existing.attribute(k)) existing.remove_attribute(k);
                    existing.append_attribute(k) = v.c_str();
                };
                auto removeAttr = [&](const char* k) {
                    if (existing.attribute(k)) existing.remove_attribute(k);
                };
                glm::vec3 pos    = loc.GetWorldPosition();
                glm::vec3 scale  = loc.GetScaleMatrix();
                glm::vec3 rotate = loc.GetRotation();
                setAttr("WorldPosX", std::to_string(pos.x));
                setAttr("WorldPosY", std::to_string(pos.y));
                setAttr("WorldPosZ", std::to_string(pos.z));
                setAttr("ScaleX",    std::to_string(scale.x));
                setAttr("ScaleY",    std::to_string(scale.y));
                setAttr("ScaleZ",    std::to_string(scale.z));
                setAttr("RotateX",   std::to_string(rotate.x));
                setAttr("RotateY",   std::to_string(rotate.y));
                setAttr("RotateZ",   std::to_string(rotate.z));
                setAttr("LayoutGroup", vo->GetLayoutGroup());
                if (loc.IsLocked()) {
                    setAttr("Locked", "1");
                } else {
                    removeAttr("Locked");
                }
                if (vo->IsActive()) {
                    removeAttr("Active");
                } else {
                    setAttr("Active", "0");
                }
                auto setInt = [&](const char* k, int v) {
                    if (existing.attribute(k)) existing.remove_attribute(k);
                    existing.append_attribute(k) = v;
                };
                // J-12 — per-type attrs. Names match the
                // XmlNodeKeys constants used by the deserialize
                // factory so round-trip on next launch is clean.
                switch (vo->GetDisplayAs()) {
                    case DisplayAsType::Mesh: {
                        auto* m = dynamic_cast<MeshObject*>(vo);
                        if (m) {
                            setAttr("ObjFile",    m->GetObjFile());
                            setInt ("Brightness", m->GetBrightness());
                            setAttr("MeshOnly",   m->IsMeshOnly() ? "1" : "0");
                        }
                        break;
                    }
                    case DisplayAsType::Image: {
                        auto* i = dynamic_cast<ImageObject*>(vo);
                        if (i) {
                            setAttr("Image",        i->GetImageFile());
                            setInt ("Brightness",   i->GetBrightness());
                            setInt ("Transparency", i->GetTransparency());
                        }
                        break;
                    }
                    case DisplayAsType::Gridlines: {
                        auto* g = dynamic_cast<GridlinesObject*>(vo);
                        if (g) {
                            setInt ("GridLineSpacing", g->GetGridLineSpacing());
                            setInt ("GridWidth",       g->GetGridWidth());
                            setInt ("GridHeight",      g->GetGridHeight());
                            setAttr("GridColor",       g->GetGridColor());
                            setAttr("GridAxis",        g->GetHasAxis() ? "1" : "0");
                            setAttr("PointToFront",    g->GetPointToFront() ? "1" : "0");
                        }
                        break;
                    }
                    case DisplayAsType::Terrain: {
                        auto* t = dynamic_cast<TerrainObject*>(vo);
                        if (t) {
                            setAttr("Image",              t->GetImageFile());
                            setInt ("Brightness",         (int)t->GetBrightness());
                            setInt ("Transparency",       t->GetTransparency());
                            // Desktop typo: "Terrian" not "Terrain"
                            // (deserializer reads both but writes
                            // the legacy spelling).
                            setInt ("TerrianLineSpacing", t->GetSpacing());
                            setInt ("TerrianWidth",       t->GetWidth());
                            setInt ("TerrianDepth",       t->GetDepth());
                            setAttr("HideGrid",  t->IsHideGrid()  ? "1" : "0");
                            setAttr("HideImage", t->IsHideImage() ? "1" : "0");
                            setAttr("GridColor", t->GetGridColor());
                        }
                        break;
                    }
                    case DisplayAsType::Ruler: {
                        auto* r = dynamic_cast<RulerObject*>(vo);
                        if (r) {
                            setInt ("Units",  RulerObject::GetUnits());
                            setAttr("Length", std::to_string(r->GetLength()));
                            // J-14 — TwoPointScreenLocation point-2
                            // offset. WorldPos (X/Y/Z = point 1)
                            // already written by the common patcher.
                            if (auto* tpl = dynamic_cast<TwoPointScreenLocation*>(&loc)) {
                                setAttr("X2", std::to_string(tpl->GetX2()));
                                setAttr("Y2", std::to_string(tpl->GetY2()));
                                setAttr("Z2", std::to_string(tpl->GetZ2()));
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    // J-8 (2D Background pseudo-object) — patch background attrs
    // on the matching target. "Default" maps to top-level
    // `<settings>`; everything else to `<layoutGroups><layoutGroup
    // name="...">`. Path attrs are written using whatever the
    // user picked; the load path FixFile-resolves them on next
    // launch, so absolute / show-relative both round-trip.
    for (const auto& grpName : _dirtyBackgroundGroups) {
        std::string bgPath;
        int bri, alpha;
        bool scale;
        if (grpName == "Default") {
            bgPath = _backgroundImage;
            bri    = _backgroundBrightness;
            alpha  = _backgroundAlpha;
            scale  = _scaleBackgroundImage;
        } else {
            const NamedLayoutGroup* src = nullptr;
            for (const auto& g : _namedLayoutGroups) {
                if (g.name == grpName) { src = &g; break; }
            }
            if (!src) {
                spdlog::warn("iPadRenderContext::SaveLayoutChanges: dirty bg for unknown group '{}'",
                             grpName);
                continue;
            }
            bgPath = src->backgroundImage;
            bri    = src->backgroundBrightness;
            alpha  = src->backgroundAlpha;
            scale  = src->scaleBackgroundImage;
        }

        pugi::xml_node target;
        if (grpName == "Default") {
            target = root.child("settings");
            if (!target) target = root.append_child("settings");
        } else {
            auto layoutGroupsNode = root.child("layoutGroups");
            if (!layoutGroupsNode) {
                layoutGroupsNode = root.append_child("layoutGroups");
            }
            for (auto n = layoutGroupsNode.first_child(); n; n = n.next_sibling()) {
                if (std::string_view(n.name()) != "layoutGroup") continue;
                if (grpName == n.attribute("name").as_string()) {
                    target = n;
                    break;
                }
            }
            if (!target) {
                target = layoutGroupsNode.append_child("layoutGroup");
                target.append_attribute("name") = grpName.c_str();
            }
        }
        auto patch = [&](const char* k, const std::string& v) {
            if (target.attribute(k)) target.remove_attribute(k);
            if (!v.empty()) target.append_attribute(k) = v.c_str();
        };
        auto patchInt = [&](const char* k, int v) {
            if (target.attribute(k)) target.remove_attribute(k);
            target.append_attribute(k) = v;
        };
        patch("backgroundImage", bgPath);
        patchInt("backgroundBrightness", bri);
        patchInt("backgroundAlpha", alpha);
        patchInt("scaleImage", scale ? 1 : 0);
    }

    if (!doc.save_file(rgbPath.c_str(), "  ")) {
        spdlog::error("iPadRenderContext::SaveLayoutChanges: write failed for {}",
                      rgbPath);
        return false;
    }
    _dirtyLayoutModels.clear();
    _dirtyLayoutViewObjects.clear();
    _createdGroups.clear();
    _deletedGroups.clear();
    _dirtyBackgroundGroups.clear();
    _createdViewObjects.clear();
    _deletedViewObjects.clear();
    _renamedGroups.clear();
    _renamedViewObjects.clear();
    _renamedModels.clear();
    return true;
}

void iPadRenderContext::PushLayoutUndoSnapshotForModel(const std::string& modelName) {
    if (modelName.empty() || !_modelManager) return;
    Model* m = _modelManager->GetModel(modelName);
    if (!m) return;
    auto& loc = m->GetModelScreenLocation();
    glm::vec3 rot = loc.GetRotation();
    LayoutUndoEntry e;
    e.target = UndoTarget::Model;
    e.modelName = modelName;
    e.hcenter = loc.GetHcenterPos();
    e.vcenter = loc.GetVcenterPos();
    e.dcenter = loc.GetDcenterPos();
    e.width   = loc.GetMWidth();
    e.height  = loc.GetMHeight();
    e.depth   = loc.GetMDepth();
    e.rotateX = rot.x;
    e.rotateY = rot.y;
    e.rotateZ = rot.z;
    e.locked  = loc.IsLocked();
    e.layoutGroup    = m->GetLayoutGroup();
    e.controllerName = m->GetControllerName();

    _layoutUndoStack.push_back(std::move(e));
    while (_layoutUndoStack.size() > kLayoutUndoMaxDepth) {
        _layoutUndoStack.pop_front();
    }
}

// J-17 — VO common-transform snapshot. ScaleX/Y/Z come from the
// BoxedScreenLocation; objects on other screen-loc types (Ruler
// uses TwoPoint) get all-1 scale and the undo applies just the
// world pos / rotation — close enough for those types.
void iPadRenderContext::PushLayoutUndoSnapshotForViewObject(const std::string& objectName) {
    if (objectName.empty() || !_viewObjectManager) return;
    ViewObject* vo = _viewObjectManager->GetViewObject(objectName);
    if (!vo) return;
    auto& loc = vo->GetObjectScreenLocation();
    glm::vec3 rot = loc.GetRotation();
    LayoutUndoEntry e;
    e.target = UndoTarget::ViewObject;
    e.modelName = objectName;
    e.hcenter = loc.GetHcenterPos();
    e.vcenter = loc.GetVcenterPos();
    e.dcenter = loc.GetDcenterPos();
    if (auto* bsl = dynamic_cast<BoxedScreenLocation*>(&loc)) {
        e.scaleX = bsl->GetScaleX();
        e.scaleY = bsl->GetScaleY();
        e.scaleZ = bsl->GetScaleZ();
    } else {
        glm::vec3 sm = loc.GetScaleMatrix();
        e.scaleX = sm.x;
        e.scaleY = sm.y;
        e.scaleZ = sm.z;
    }
    e.rotateX = rot.x;
    e.rotateY = rot.y;
    e.rotateZ = rot.z;
    e.locked  = loc.IsLocked();
    e.layoutGroup = vo->GetLayoutGroup();
    _layoutUndoStack.push_back(std::move(e));
    while (_layoutUndoStack.size() > kLayoutUndoMaxDepth) {
        _layoutUndoStack.pop_front();
    }
}

void iPadRenderContext::PushTerrainHeightmapUndoSnapshot(const std::string& terrainName) {
    if (terrainName.empty() || !_viewObjectManager) return;
    ViewObject* vo = _viewObjectManager->GetViewObject(terrainName);
    auto* terrain = dynamic_cast<TerrainObject*>(vo);
    if (!terrain) return;
    auto& sloc = dynamic_cast<TerrainScreenLocation&>(terrain->GetBaseObjectScreenLocation());
    LayoutUndoEntry e;
    e.target = UndoTarget::ViewObjectHeightmap;
    e.modelName = terrainName;
    e.pointData = sloc.GetDataAsString();
    _layoutUndoStack.push_back(std::move(e));
    while (_layoutUndoStack.size() > kLayoutUndoMaxDepth) {
        _layoutUndoStack.pop_front();
    }
}

bool iPadRenderContext::UndoLastLayoutChange() {
    if (_layoutUndoStack.empty()) return false;
    LayoutUndoEntry e = _layoutUndoStack.back();
    _layoutUndoStack.pop_back();

    switch (e.target) {
    case UndoTarget::Model: {
        if (!_modelManager) return false;
        Model* m = _modelManager->GetModel(e.modelName);
        if (!m) return false;
        auto& loc = m->GetModelScreenLocation();
        m->SetHcenterPos(e.hcenter);
        m->SetVcenterPos(e.vcenter);
        m->SetDcenterPos(e.dcenter);
        m->SetWidth(e.width);
        m->SetHeight(e.height);
        m->SetDepth(e.depth);
        loc.SetRotateX(e.rotateX);
        loc.SetRotateY(e.rotateY);
        loc.SetRotateZ(e.rotateZ);
        loc.SetLocked(e.locked);
        if (m->GetLayoutGroup() != e.layoutGroup) m->SetLayoutGroup(e.layoutGroup);
        if (m->GetControllerName() != e.controllerName) m->SetControllerName(e.controllerName);
        MarkLayoutModelDirty(e.modelName);
        return true;
    }
    case UndoTarget::ViewObject: {
        if (!_viewObjectManager) return false;
        ViewObject* vo = _viewObjectManager->GetViewObject(e.modelName);
        if (!vo) return false;
        auto& loc = vo->GetObjectScreenLocation();
        vo->SetHcenterPos(e.hcenter);
        vo->SetVcenterPos(e.vcenter);
        vo->SetDcenterPos(e.dcenter);
        if (auto* bsl = dynamic_cast<BoxedScreenLocation*>(&loc)) {
            bsl->SetScaleX(e.scaleX);
            bsl->SetScaleY(e.scaleY);
            bsl->SetScaleZ(e.scaleZ);
        } else {
            loc.SetScaleMatrix(glm::vec3(e.scaleX, e.scaleY, e.scaleZ));
        }
        loc.SetRotateX(e.rotateX);
        loc.SetRotateY(e.rotateY);
        loc.SetRotateZ(e.rotateZ);
        loc.SetLocked(e.locked);
        if (vo->GetLayoutGroup() != e.layoutGroup) vo->SetLayoutGroup(e.layoutGroup);
        vo->IncrementChangeCount();
        vo->ReloadModel();
        MarkLayoutViewObjectDirty(e.modelName);
        return true;
    }
    case UndoTarget::ViewObjectHeightmap: {
        if (!_viewObjectManager) return false;
        ViewObject* vo = _viewObjectManager->GetViewObject(e.modelName);
        auto* terrain = dynamic_cast<TerrainObject*>(vo);
        if (!terrain) return false;
        auto& sloc = dynamic_cast<TerrainScreenLocation&>(terrain->GetBaseObjectScreenLocation());
        sloc.SetDataFromString(e.pointData);
        terrain->IncrementChangeCount();
        terrain->ReloadModel();
        MarkLayoutViewObjectDirty(e.modelName);
        return true;
    }
    }
    return false;
}

void iPadRenderContext::SetActiveLayoutGroup(const std::string& name) {
    if (name == "Default") {
        _activeLayoutGroup = "Default";
        return;
    }
    for (const auto& g : _namedLayoutGroups) {
        if (g.name == name) {
            _activeLayoutGroup = name;
            return;
        }
    }
    // Unknown group — fall back to Default rather than render nothing.
    _activeLayoutGroup = "Default";
}

const std::string& iPadRenderContext::GetActiveBackgroundImage() const {
    if (_activeLayoutGroup == "Default") return _backgroundImage;
    for (const auto& g : _namedLayoutGroups) {
        if (g.name == _activeLayoutGroup) return g.backgroundImage;
    }
    return _backgroundImage;
}

int iPadRenderContext::GetActiveBackgroundBrightness() const {
    if (_activeLayoutGroup == "Default") return _backgroundBrightness;
    for (const auto& g : _namedLayoutGroups) {
        if (g.name == _activeLayoutGroup) return g.backgroundBrightness;
    }
    return _backgroundBrightness;
}

int iPadRenderContext::GetActiveBackgroundAlpha() const {
    if (_activeLayoutGroup == "Default") return _backgroundAlpha;
    for (const auto& g : _namedLayoutGroups) {
        if (g.name == _activeLayoutGroup) return g.backgroundAlpha;
    }
    return _backgroundAlpha;
}

bool iPadRenderContext::GetActiveScaleBackgroundImage() const {
    if (_activeLayoutGroup == "Default") return _scaleBackgroundImage;
    for (const auto& g : _namedLayoutGroups) {
        if (g.name == _activeLayoutGroup) return g.scaleBackgroundImage;
    }
    return _scaleBackgroundImage;
}

// J-8 (2D Background pseudo-object) — setters write through to
// the correct storage (default <settings> vs a named layout
// group) and record the group name in `_dirtyBackgroundGroups`
// so SaveLayoutChanges patches the matching XML attributes.
namespace {
template <typename T>
iPadRenderContext::NamedLayoutGroup* FindNamedGroup(
        std::vector<iPadRenderContext::NamedLayoutGroup>& groups,
        const std::string& name) {
    for (auto& g : groups) {
        if (g.name == name) return &g;
    }
    return nullptr;
}
} // namespace

bool iPadRenderContext::SetActiveBackgroundImage(const std::string& path) {
    if (_activeLayoutGroup == "Default") {
        if (path == _backgroundImage) return false;
        _backgroundImage = path;
    } else {
        auto* g = FindNamedGroup<int>(_namedLayoutGroups, _activeLayoutGroup);
        if (!g) return false;
        if (path == g->backgroundImage) return false;
        g->backgroundImage = path;
    }
    if (!path.empty()) ObtainAccessToURL(path, false);
    _dirtyBackgroundGroups.insert(_activeLayoutGroup);
    return true;
}

bool iPadRenderContext::SetActiveBackgroundBrightness(int brightness) {
    if (brightness < 0) brightness = 0;
    if (brightness > 100) brightness = 100;
    if (_activeLayoutGroup == "Default") {
        if (brightness == _backgroundBrightness) return false;
        _backgroundBrightness = brightness;
    } else {
        auto* g = FindNamedGroup<int>(_namedLayoutGroups, _activeLayoutGroup);
        if (!g) return false;
        if (brightness == g->backgroundBrightness) return false;
        g->backgroundBrightness = brightness;
    }
    _dirtyBackgroundGroups.insert(_activeLayoutGroup);
    return true;
}

bool iPadRenderContext::SetActiveBackgroundAlpha(int alpha) {
    if (alpha < 0) alpha = 0;
    if (alpha > 100) alpha = 100;
    if (_activeLayoutGroup == "Default") {
        if (alpha == _backgroundAlpha) return false;
        _backgroundAlpha = alpha;
    } else {
        auto* g = FindNamedGroup<int>(_namedLayoutGroups, _activeLayoutGroup);
        if (!g) return false;
        if (alpha == g->backgroundAlpha) return false;
        g->backgroundAlpha = alpha;
    }
    _dirtyBackgroundGroups.insert(_activeLayoutGroup);
    return true;
}

bool iPadRenderContext::SetActiveScaleBackgroundImage(bool scale) {
    if (_activeLayoutGroup == "Default") {
        if (scale == _scaleBackgroundImage) return false;
        _scaleBackgroundImage = scale;
    } else {
        auto* g = FindNamedGroup<int>(_namedLayoutGroups, _activeLayoutGroup);
        if (!g) return false;
        if (scale == g->scaleBackgroundImage) return false;
        g->scaleBackgroundImage = scale;
    }
    _dirtyBackgroundGroups.insert(_activeLayoutGroup);
    return true;
}

// Mirrors desktop xLightsFrame::UpdateModelsList (TabSequence.cpp:1209).
// For the Default preview, include models tagged "Default" or
// "All Previews". For a named group, include models tagged with that
// name or "All Previews". Model groups whose own `layout_group` matches
// are expanded to their constituent models, skipping duplicates.
std::vector<Model*> iPadRenderContext::GetModelsForActivePreview() const {
    std::vector<Model*> out;
    if (!_modelManager) return out;

    const std::string& active = _activeLayoutGroup;

    auto matchesActive = [&](const std::string& g) {
        return g == active || g == "All Previews";
    };

    auto addModelIfAbsent = [&](Model* m) {
        if (!m) return;
        if (std::find(out.begin(), out.end(), m) == out.end()) {
            out.push_back(m);
        }
    };

    // Pass 1: individual (non-group) models whose layout_group matches.
    for (const auto& [name, model] : _modelManager->GetModels()) {
        if (!model) continue;
        if (model->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
        if (matchesActive(model->GetLayoutGroup())) {
            addModelIfAbsent(model);
        }
    }
    // Pass 2: ModelGroups tagged for this preview — flatten their
    // children (recursively for nested groups). A model already added
    // in pass 1 is not duplicated.
    std::function<void(ModelGroup*)> expand = [&](ModelGroup* grp) {
        if (!grp) return;
        for (Model* m : grp->Models()) {
            if (!m) continue;
            if (m->GetDisplayAs() == DisplayAsType::ModelGroup) {
                expand(static_cast<ModelGroup*>(m));
            } else {
                addModelIfAbsent(m);
            }
        }
    };
    for (const auto& [name, model] : _modelManager->GetModels()) {
        if (!model) continue;
        if (model->GetDisplayAs() != DisplayAsType::ModelGroup) continue;
        if (matchesActive(model->GetLayoutGroup())) {
            expand(static_cast<ModelGroup*>(model));
        }
    }
    return out;
}

bool iPadRenderContext::OpenSequence(const std::string& path) {
    CloseSequence();

    if (!ObtainAccessToURL(path, false)) {
        spdlog::warn("iPadRenderContext::OpenSequence: ObtainAccessToURL failed for '{}' — bookmark likely stale", path);
    }

    _sequenceFile = std::make_unique<SequenceFile>(path);
    _sequenceDoc = _sequenceFile->Open(_showDir, false, path);

    if (!_sequenceDoc) {
        spdlog::warn("iPadRenderContext: Failed to open sequence {}", path);
        _sequenceFile.reset();
        return false;
    }

    // Must set the views manager BEFORE LoadSequencerFile so SequenceElements
    // can resolve the current view while populating rows. Desktop does the
    // same in `tabSequencer.cpp::NewSequence/OpenSequence`.
    _sequenceElements.SetViewsManager(&_viewsManager);

    if (!_sequenceElements.LoadSequencerFile(*_sequenceFile, *_sequenceDoc, _showDir)) {
        spdlog::warn("iPadRenderContext: Failed to load sequence elements from {}", path);
        _sequenceFile.reset();
        _sequenceDoc.reset();
        return false;
    }

    // PrepareViews grows mAllViews so there is one slot per view in the
    // view manager. Without this only the Master slot exists and switching
    // to any other view crashes inside PopulateRowInformation at
    // `mAllViews[mCurrentView]`. Desktop calls this from tabSequencer.cpp.
    _sequenceElements.PrepareViews(*_sequenceFile);

    _sequenceElements.PopulateRowInformation();

    // Mark the SequenceFile as loaded so subsequent timing-track
    // additions go through the live in-memory path (creating
    // elements + marks immediately) instead of the
    // mPendingTimings queue, which only applies on a future
    // ApplyPendingTimings call. Desktop does this from
    // SeqFileUtilities.cpp after every load; the iPad's
    // sequence-open path is here, so it lands here.
    _sequenceFile->SetSequenceLoaded(true);

    // ValueCurve needs sequence elements for VC expressions referencing timing tracks
    ValueCurve::SetSequenceElements(&_sequenceElements);

    spdlog::info("iPadRenderContext: Row info size: {}, timing rows: {}",
                 _sequenceElements.GetRowInformationSize(),
                 _sequenceElements.GetNumberOfTimingElements());

    // Pre-allocate SequenceData once per sequence. Previously
    // RenderAll called init() on every pass, which meant a
    // Cleanup()+realloc of `_frames` on each render; any concurrent
    // read from a worker thread would OOB. Allocating here makes
    // the buffer stable for the sequence's lifetime — render passes
    // reuse it, and `RenderEngine::Render` already zeros per-frame
    // data when called with `clear=true`.
    EnsureSequenceDataSized();

    spdlog::info("iPadRenderContext: Opened sequence {} ({} elements, {} ms)",
                 path,
                 _sequenceElements.GetElementCount(),
                 _sequenceFile->GetSequenceDurationMS());
    return true;
}

void iPadRenderContext::CloseSequence() {
    // If a background render is in flight, signal abort and wait
    // for the orchestrator + JobPool workers to wind down before
    // destroying the SequenceElements / SequenceData they read.
    // Without this, opening a second sequence while the first one
    // is still rendering races: main thread enters Clear() while
    // the render worker is mid-GetElement(), producing a use-
    // after-free crash deep in StrandElement / ModelElement
    // destruction (observed in TestFlight as a Thread 0 ↔ Thread 17
    // collision when picking a sequence from the picker).
    //
    // The deadline is generous — typical wind-down is tens of ms,
    // but a long-running per-model render can take a couple of
    // seconds to notice the abort. Falling out of the loop after
    // 5 s is a safety net; the worker may still be running when we
    // proceed, but at that point we've given up on a clean
    // shutdown and the user is presumably waiting too long anyway.
    if (_renderEngine) {
        _renderEngine->SignalAbort();
        auto deadline = std::chrono::steady_clock::now()
                      + std::chrono::seconds(5);
        while (!IsRenderDone()
               && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    _sequenceElements.Clear();
    _sequenceData.Cleanup();
    _sequenceDoc.reset();
    _sequenceFile.reset();
}

void iPadRenderContext::EnsureSequenceDataSized() {
    if (!_sequenceFile) return;
    unsigned int numFrames =
        (unsigned int)(_sequenceFile->GetSequenceDurationMS() /
                       _sequenceFile->GetFrameMS());
    unsigned int numChannels = _outputManager.GetTotalChannels();
    if (numChannels == 0) numChannels = 1;
    unsigned int frameTime = (unsigned int)_sequenceFile->GetFrameMS();

    // Only reallocate when the shape has actually changed
    // (sequence duration, frame rate, or channel count). The
    // `init()` path does a full Cleanup + reallocate; skipping it
    // on a matching call saves a full sequence's worth of
    // allocation churn on every render.
    if (_sequenceData.IsValidData()
        && _sequenceData.NumChannels() == numChannels
        && _sequenceData.NumFrames() == numFrames
        && _sequenceData.FrameTime() == frameTime) {
        return;
    }
    _sequenceData.init(numChannels, numFrames, frameTime);
}

bool iPadRenderContext::IsInShowFolder(const std::string& file) const {
    return file.find(_showDir) == 0;
}

bool iPadRenderContext::IsInShowOrMediaFolder(const std::string& file) const {
    if (IsInShowFolder(file)) return true;
    for (const auto& folder : _mediaFolders) {
        if (file.find(folder) == 0) return true;
    }
    return false;
}

std::string iPadRenderContext::MakeRelativePath(const std::string& file) const {
    if (file.find(_showDir) == 0 && file.size() > _showDir.size() + 1) {
        return file.substr(_showDir.size() + 1);
    }
    return file;
}

namespace {
// Byte-equal comparison; matches desktop's wx FilesMatch semantics
// closely enough for the "skip the copy when an identical file already
// exists" branch.
bool FilesMatchBytes(const std::filesystem::path& a,
                     const std::filesystem::path& b) {
    std::error_code ec;
    auto sa = std::filesystem::file_size(a, ec);
    if (ec) return false;
    auto sb = std::filesystem::file_size(b, ec);
    if (ec || sa != sb) return false;
    std::ifstream fa(a, std::ios::binary);
    std::ifstream fb(b, std::ios::binary);
    if (!fa.is_open() || !fb.is_open()) return false;
    constexpr size_t blk = 64 * 1024;
    std::vector<char> bufA(blk), bufB(blk);
    while (fa && fb) {
        fa.read(bufA.data(), blk);
        fb.read(bufB.data(), blk);
        auto ra = fa.gcount();
        auto rb = fb.gcount();
        if (ra != rb) return false;
        if (ra == 0) return true;
        if (std::memcmp(bufA.data(), bufB.data(), (size_t)ra) != 0) return false;
    }
    return fa.eof() && fb.eof();
}

/// Shared routine for MoveToShowFolder / CopyToMediaFolder. Copies
/// `file` into `<destRoot>/<subdir>/<basename>`, appends `_N` on
/// collision unless `reuse` and the existing file matches byte-for-
/// byte. Empty string on any failure. Creates the subdir if missing.
std::string CopyIntoRoot(const std::string& file,
                        const std::string& destRoot,
                        const std::string& subdirectory,
                        bool reuse) {
    if (destRoot.empty() || file.empty()) return "";

    namespace fs = std::filesystem;
    fs::path src(file);
    if (!fs::exists(src)) return "";

    // Normalise subdir: strip leading separator. Desktop's callers
    // pass both "/Images" and "Images"; the trailing concat either way
    // ends up inside destRoot.
    std::string sub = subdirectory;
    while (!sub.empty() && (sub.front() == '/' || sub.front() == '\\')) {
        sub.erase(sub.begin());
    }

    fs::path dir = fs::path(destRoot);
    if (!sub.empty()) dir /= sub;

    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec) {
        spdlog::error("iPadRenderContext: Unable to create media target folder {}: {}",
                      dir.string(), ec.message());
        return "";
    }

    fs::path target = dir / src.filename();
    if (!reuse) {
        // Append _N until we find an unused name (or until an existing
        // file at that slot matches our source byte-for-byte, in which
        // case we reuse it silently).
        std::string stem = src.stem().string();
        std::string ext  = src.extension().string();
        int n = 1;
        while (fs::exists(target) && !FilesMatchBytes(src, target)) {
            target = dir / (stem + "_" + std::to_string(n++) + ext);
        }
    }

    if (!fs::exists(target)) {
        fs::copy_file(src, target, fs::copy_options::none, ec);
        if (ec) {
            spdlog::error("iPadRenderContext: Copy {} -> {} failed: {}",
                          src.string(), target.string(), ec.message());
            return "";
        }
    }
    return target.string();
}
} // namespace

std::string iPadRenderContext::MoveToShowFolder(const std::string& file,
                                                  const std::string& subdirectory,
                                                  bool reuse) {
    if (_showDir.empty()) return "";
    return CopyIntoRoot(file, _showDir, subdirectory, reuse);
}

std::string iPadRenderContext::CopyToMediaFolder(const std::string& file,
                                                  const std::string& mediaFolderPath,
                                                  const std::string& subdirectory) {
    // Refuse unknown media folders — writing outside the configured
    // set would break the "media must be in show or media folder"
    // invariant iPad enforces.
    bool known = false;
    for (const auto& mf : _mediaFolders) {
        if (mf == mediaFolderPath) { known = true; break; }
    }
    if (!known) return "";
    return CopyIntoRoot(file, mediaFolderPath, subdirectory, /*reuse*/ false);
}

AudioManager* iPadRenderContext::GetCurrentMediaManager() const {
    return _sequenceFile ? _sequenceFile->GetMedia() : nullptr;
}

void iPadRenderContext::SetWaveformTrackIndex(int idx) {
    if (!_sequenceFile) {
        _waveformTrackIndex = -1;
        return;
    }
    if (idx < -1) idx = -1;
    if (idx >= _sequenceFile->GetAltTrackCount()) idx = -1;
    _waveformTrackIndex = idx;
}

AudioManager* iPadRenderContext::GetWaveformMedia() const {
    if (!_sequenceFile) return nullptr;
    if (_waveformTrackIndex < 0) return _sequenceFile->GetMedia();
    AudioManager* alt = _sequenceFile->GetAltTrackMedia(_waveformTrackIndex);
    return alt ? alt : _sequenceFile->GetMedia();
}

int iPadRenderContext::GetAltTrackCount() const {
    return _sequenceFile ? _sequenceFile->GetAltTrackCount() : 0;
}

std::string iPadRenderContext::GetAltTrackDisplayName(int idx) const {
    if (!_sequenceFile) return "";
    if (idx < 0 || idx >= _sequenceFile->GetAltTrackCount()) return "";
    return _sequenceFile->GetAltTrackDisplayName(idx);
}

const std::string& iPadRenderContext::GetHeaderInfo(HEADER_INFO_TYPES type) const {
    if (_sequenceFile) {
        return _sequenceFile->GetHeaderInfo(type);
    }
    static const std::string empty;
    return empty;
}

Model* iPadRenderContext::GetModel(const std::string& name) const {
    if (_modelManager) {
        if (Model* m = _modelManager->GetModel(name)) return m;
    }
    // Preset model lives in its own ModelManager so it isn't visible
    // on the real sequence / layout. Desktop's xLightsFrame::GetModel
    // short-circuits on the preset name too
    // (`tabSequencer.cpp:335-336`); without this, the render engine
    // resolves the preset name to nullptr, skips `ModelElement::Init`,
    // and every rendered frame comes back black.
    if (_presetModel != nullptr && name == _presetModel->GetName()) {
        return _presetModel;
    }
    if (_presetModelManager) {
        if (Model* m = _presetModelManager->GetModel(name)) return m;
    }
    return nullptr;
}

PhonemeDictionary& iPadRenderContext::GetPhonemeDictionary() {
    if (!_phonemeDict) {
        _phonemeDict = std::make_unique<PhonemeDictionary>();
    }
    // Always call LoadDictionaries — the method short-circuits if
    // the map is already populated, so repeat calls are cheap.
    // Search dirs: show folder first (so user-override
    // `user_dictionary` files win), then the app bundle's
    // `dictionaries/` folder (where the shipped corpus lives).
    std::vector<std::string> searchDirs;
    if (!_showDir.empty()) {
        searchDirs.push_back(_showDir);
    }
    std::string res = FileUtils::GetResourcesDir();
    if (!res.empty()) {
        searchDirs.push_back(res + "/dictionaries");
    }
    _phonemeDict->LoadDictionaries(searchDirs);
    return *_phonemeDict;
}

TimingElement* iPadRenderContext::AddTimingElement(const std::string& name,
                                                    const std::string& subType) {
    // Mirrors xLightsFrame::AddTimingElement (tabSequencer.cpp:3502).
    // Makes the name unique by appending _N suffixes, deactivates all
    // existing timing elements so the new one is the only active
    // track, adds the element + a single default effect layer, and
    // registers it with the current sequence view.
    std::string n = name;
    int nn = 1;
    while (_sequenceElements.GetElement(n) != nullptr) {
        n = name + "_" + std::to_string(nn++);
    }
    _sequenceElements.DeactivateAllTimingElements();
    int timingCount = _sequenceElements.GetNumberOfTimingElements();
    Element* raw = _sequenceElements.AddElement(timingCount, n, "timing",
                                                 /*visible*/ true,
                                                 /*collapsed*/ false,
                                                 /*active*/ true,
                                                 /*selected*/ false,
                                                 /*renderDisabled*/ false);
    TimingElement* e = dynamic_cast<TimingElement*>(raw);
    if (!e) return nullptr;
    e->SetSubType(subType);
    e->AddEffectLayer();
    _sequenceElements.AddTimingToCurrentView(n);
    _sequenceElements.PopulateRowInformation();
    return e;
}

bool iPadRenderContext::AbortRender(int maxTimeMs) {
    // Mirror the desktop's xLightsFrame::AbortRender contract: signal
    // every in-flight render job to bail and then BLOCK until they
    // actually finish (or the timeout elapses). Callers depend on
    // this — once AbortRender returns, layout-mutation code is free
    // to rewrite Model state without racing the render thread.
    if (!_renderEngine) return true;
    if (IsRenderDone()) return true;
    _renderEngine->SignalAbort();
    if (maxTimeMs <= 0) maxTimeMs = 60000;
    int loops = maxTimeMs / 10;
    int i = 0;
    while (!IsRenderDone() && i < loops) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ++i;
    }
    return IsRenderDone();
}

bool iPadRenderContext::WasRenderAborted() const {
    return _renderEngine && _renderEngine->GetAbortedRenderJobs() > 0;
}

void iPadRenderContext::RenderEffectForModel(const std::string& model,
                                              int startms, int endms, bool clear) {
    if (_renderEngine && _sequenceData.IsValidData()) {
        _renderEngine->RenderEffectForModel(model, startms, endms,
                                             _sequenceElements, _sequenceData,
                                             false, _modelsChangeCount, clear);
    }
}

void iPadRenderContext::EnsureRenderEngine() {
    if (!_jobPool) {
        _jobPool = std::make_unique<JobPool>("RenderPool");
        // RenderEngine workers can block waiting on frames from other models;
        // with too few threads a contended sequence deadlocks. Oversubscribe
        // well past core count so a blocked worker never exhausts the pool.
        size_t hw = std::thread::hardware_concurrency();
        size_t poolThreads = std::max<size_t>(24, hw * 2);
        _jobPool->Start(poolThreads);
    }
    if (!_renderEngine) {
        _renderEngine = std::make_unique<RenderEngine>(*this, *_jobPool, _renderCache);
    }
}

void iPadRenderContext::RenderAll() {
    if (!_sequenceFile || !_modelManager) return;

    // SequenceData is normally allocated in OpenSequence and reused
    // across every render pass — avoids the allocation churn (and
    // the concurrent-render OOB crash) that came from re-`init()`
    // on every RenderAll. This call only reallocates when something
    // changed (duration / frame rate / channel count); otherwise
    // it's a no-op. `RenderEngine::Render(..., clear=true)` zeros
    // the existing frames per-range so we don't need to ourselves.
    EnsureSequenceDataSized();

    unsigned int numFrames = _sequenceData.NumFrames();
    unsigned int numChannels = _sequenceData.NumChannels();

    EnsureRenderEngine();

    _renderEngine->BuildRenderTree(_sequenceElements, _modelsChangeCount);

    auto models = _renderEngine->GetRenderTree().GetModels();
    std::list<Model*> empty;

    _renderEngine->Render(_sequenceElements, _sequenceData,
                           models, empty,
                           0, (int)numFrames - 1,
                           nullptr, true,
                           [](bool) {});

    spdlog::info("iPadRenderContext: RenderAll started for {} frames, {} channels",
                 numFrames, numChannels);
}

void iPadRenderContext::HandleMemoryWarning() {
    // Lighter-touch response: try to free significant memory WITHOUT
    // interrupting active work. Callers may fire this repeatedly as
    // the pressure source stays elevated.
    //
    //   * Signal the render engine to abort at the next safe point
    //     (actual abort is cooperative; workers notice between
    //     frames).
    //   * Drop the memory-resident render cache frames. The
    //     on-disk cache stays — it rebuilds its mmap window on
    //     next GetFrame.
    //   * Drop UI-side derivatives from every media entry:
    //     preview thumbnails and scaled-image variants. Those
    //     rebuild lazily on the next media-picker or effect-panel
    //     access, usually sub-second.
    if (_renderEngine) {
        _renderEngine->SignalAbort();
    }
    _renderCache.Purge(nullptr, false);
    _sequenceElements.GetSequenceMedia().PurgePreviewCaches();
    spdlog::warn("iPadRenderContext: memory warning handled "
                 "(render abort signalled, cache + media previews purged)");
}

void iPadRenderContext::HandleMemoryCritical() {
    // Strictly more aggressive than Warning — the lighter response
    // wasn't enough, so we stop pretending and tear down optional
    // state even at the cost of user work. Called when the OS is
    // threatening jetsam.
    HandleMemoryWarning();

    // Block until render workers actually exit so the buffers they
    // hold can be reclaimed. Warning just signals; Critical waits.
    if (_renderEngine) {
        AbortRender(3000);
    }

    // Free the preset render scaffolding (shader preview rendering,
    // effect thumbnail generation). Reconstructed on next
    // `EnsurePresetModel`.
    _presetSequenceData.Cleanup();
    if (_presetModel) {
        _presetSequenceElements.Clear();
        _presetModel = nullptr;
        _presetModelManager.reset();
    }

    // Drop media entries that aren't referenced by any current
    // effect. `MarkAllUnused` + render path would normally flag
    // what's in use, but by the time we're critical we can't
    // afford another render pass — walk every effect's settings
    // and palette maps to build the "in-use" set, then remove the
    // rest. `RemoveUnusedMedia` does exactly this on the
    // caller's behalf.
    _sequenceElements.GetSequenceMedia().RemoveUnusedMedia();

    spdlog::error("iPadRenderContext: memory critical handled "
                  "(render aborted, preset data freed, unused media removed)");
}

bool iPadRenderContext::IsRenderDone() {
    // No render ever kicked off (or already torn down) — treat as
    // "done" so abort-and-wait short-circuits cleanly.
    if (!_renderEngine) return true;
    if (!_sequenceData.IsValidData()) return false;
    // Each RenderProgressInfo flips its `completed` atomic when its last
    // RenderJob signals via FinishNotifier (covers normal, aborted, and
    // early-bail exits). We both check completion and lazily drain finished
    // entries here -- called from the main thread, so cleanup + callback run
    // safely off the render workers. Any pending rpi keeps the result false.
    auto& list = _renderEngine->GetRenderProgressInfo();
    bool allDone = true;
    for (auto it = list.begin(); it != list.end();) {
        RenderProgressInfo* rpi = *it;
        if (rpi->completed.load()) {
            rpi->CleanupJobs();
            if (rpi->callback) rpi->callback(_renderEngine->GetAbortedRenderJobs() > 0);
            delete rpi;
            it = list.erase(it);
        } else {
            allDone = false;
            ++it;
        }
    }
    return allDone;
}

float iPadRenderContext::GetRenderProgressFraction() const {
    if (!_renderEngine) return 1.0f;
    auto& list = const_cast<RenderEngine*>(_renderEngine.get())->GetRenderProgressInfo();
    if (list.empty()) return 1.0f;

    uint64_t totalDone = 0;
    uint64_t totalWork = 0;
    for (auto* rpi : list) {
        const int totalFrames = rpi->endFrame - rpi->startFrame + 1;
        if (totalFrames <= 0 || !rpi->jobs) continue;
        for (int i = 0; i < rpi->numRows; ++i) {
            IRenderJobStatus* job = rpi->jobs[i];
            if (!job) continue;
            const int cur = job->GetCurrentFrame();
            int done;
            if (cur == END_OF_RENDER_FRAME) {
                done = totalFrames;
            } else {
                int rel = cur - rpi->startFrame;
                if (rel < 0) rel = 0;
                if (rel > totalFrames) rel = totalFrames;
                done = rel;
            }
            totalDone += static_cast<uint64_t>(done);
            totalWork += static_cast<uint64_t>(totalFrames);
        }
    }
    if (totalWork == 0) return 1.0f;
    return static_cast<float>(totalDone) / static_cast<float>(totalWork);
}

void iPadRenderContext::SetModelColors(int frameMS) {
    if (!_sequenceData.IsValidData() || !_modelManager) return;

    int frame = frameMS / _sequenceData.FrameTime();
    if (frame < 0 || (unsigned int)frame >= _sequenceData.NumFrames()) return;

    auto& fd = _sequenceData[frame];
    auto models = _modelManager->GetModels();
    for (auto& [name, model] : models) {
        int chansPerNode = model->GetChanCountPerNode();
        for (size_t n = 0; n < model->GetNodeCount(); n++) {
            int32_t startChan = model->NodeStartChannel(n);
            if (startChan >= 0 && (unsigned int)startChan + chansPerNode <= _sequenceData.NumChannels()) {
                model->SetNodeChannelValues(n, &fd[startChan]);
            }
        }
    }
}

std::vector<iPadRenderContext::PixelData> iPadRenderContext::GetModelPixels(
    const std::string& modelName, int frameMS) {

    std::vector<PixelData> pixels;
    Model* model = GetModel(modelName);
    if (!model) return pixels;

    SetModelColors(frameMS);

    for (size_t n = 0; n < model->GetNodeCount(); n++) {
        xlColor color = model->GetNodeColor(n);
        std::vector<std::tuple<float, float, float>> pts;
        model->GetNode3DScreenCoords(n, pts);
        for (const auto& [sx, sy, sz] : pts) {
            pixels.push_back({sx, sy, color.red, color.green, color.blue});
        }
    }
    return pixels;
}

std::vector<iPadRenderContext::PixelData> iPadRenderContext::GetAllModelPixels(int frameMS) {
    std::vector<PixelData> allPixels;
    if (!_modelManager) return allPixels;

    SetModelColors(frameMS);

    static bool loggedOnce = false;
    auto models = _modelManager->GetModels();
    for (auto& [name, model] : models) {
        for (size_t n = 0; n < model->GetNodeCount(); n++) {
            xlColor color = model->GetNodeColor(n);
            std::vector<std::tuple<float, float, float>> pts;
            model->GetNode3DScreenCoords(n, pts);
            for (const auto& [sx, sy, sz] : pts) {
                allPixels.push_back({sx, sy, color.red, color.green, color.blue});
            }
        }
        if (!loggedOnce && model->GetNodeCount() > 0) {
            std::vector<std::tuple<float, float, float>> firstPts;
            model->GetNode3DScreenCoords(0, firstPts);
            if (!firstPts.empty()) {
                auto [fx, fy, fz] = firstPts[0];
                spdlog::info("Preview: model '{}' node0 screen=({},{},{}), color=({},{},{}), nodes={}",
                             name, fx, fy, fz,
                             model->GetNodeColor(0).red,
                             model->GetNodeColor(0).green,
                             model->GetNodeColor(0).blue,
                             model->GetNodeCount());
            }
        }
    }
    if (!loggedOnce && !allPixels.empty()) {
        spdlog::info("Preview: total {} pixels from {} models", allPixels.size(), models.size());
        loggedOnce = true;
    }
    return allPixels;
}

// === Preset model + preview-render helpers ==============================
// Ports `xLightsFrame::EnsurePresetModel` +
// `xLightsFrame::RenderEffectToFrames` from
// `src-ui-wx/app-shell/TabConvert.cpp`. Kept iPad-local so we don't drag
// wx into core; reuses everything else (MatrixModel, ModelManager,
// RenderEngine) from src-core.

namespace {

#define PRESET_MODEL_NAME "PRESET_Matrix_XYZZY"
constexpr int PRESET_ICON_SIZE = 64;

// Raster a single model's node colours into the provided xlImage at
// (x, y) offset. Mirrors `RenderModelOnXlImage` from
// `src-ui-wx/app-shell/TabConvert.cpp:795`.
void RenderModelOnXlImagePreset(xlImage& image, Model* model,
                                 uint8_t* framedata, int startAddr,
                                 int x, int y, bool invert) {
    int outheight = image.GetHeight();
    int outwidth = image.GetWidth();
    uint8_t* imagedata = image.GetData();

    int chs = model->GetChanCountPerNode();
    uint8_t* ps = framedata + startAddr;

    char r = model->GetChannelColorLetter(0);
    int rr = 0, gg = 1, bb = 2;
    if (r == 'G') gg = 0;
    else if (r == 'B') bb = 0;
    char g = model->GetChannelColorLetter(1);
    if (g == 'R') rr = 1;
    else if (g == 'B') bb = 1;
    char b = model->GetChannelColorLetter(2);
    if (b == 'R') rr = 2;
    else if (b == 'G') gg = 2;

    for (size_t i = 0; i < model->GetNodeCount(); i++) {
        xlColor c = model->GetNodeColor(i);
        std::vector<xlPoint> pts;
        model->GetNodeCoords(i, pts);

        for (const auto& it : pts) {
            int xx = x + it.x;
            int yy = y + it.y;
            if (invert) yy = outheight - yy - 1;

            if (xx >= 0 && xx < outwidth && yy >= 0 && yy < outheight) {
                uint8_t* p = imagedata + (yy * outwidth + xx) * 4; // RGBA
                if (chs == 1) {
                    p[0] = c.Red();
                    p[1] = c.Green();
                    p[2] = c.Blue();
                } else {
                    p[0] = *(ps + rr);
                    p[1] = *(ps + gg);
                    p[2] = *(ps + bb);
                }
                p[3] = 255; // fully opaque
            }
        }
        ps += chs;
    }
}

// Handle ModelGroups by iterating members, exactly as
// `FillXlImage` does on desktop.
void FillXlImagePreset(xlImage& image, Model* model,
                       uint8_t* framedata, int startAddr, bool invert) {
    if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
        auto* mg = static_cast<ModelGroup*>(model);
        for (auto it = mg->Models().begin(); it != mg->Models().end(); ++it) {
            int start = (*it)->GetFirstChannel() - startAddr;
            RenderModelOnXlImagePreset(image, *it, framedata, start, 0, 0, invert);
        }
    } else {
        RenderModelOnXlImagePreset(image, model, framedata,
                                    model->GetFirstChannel() - startAddr + 1,
                                    0, 0, invert);
    }
}

} // namespace

void iPadRenderContext::EnsurePresetModel() {
    if (_presetModel != nullptr) return;

    // Preset lives in its own ModelManager so it isn't visible on the
    // real sequence's preview / layout.
    _presetModelManager = std::make_unique<ModelManager>(nullptr, this);

    auto* matrixModel = new MatrixModel(*_presetModelManager);
    _presetModel = matrixModel;

    matrixModel->SetStringType("RGB Nodes");
    matrixModel->SetPixelStyle(Model::PIXEL_STYLE::PIXEL_STYLE_SMOOTH);
    matrixModel->SetPixelSize(2);
    matrixModel->SetTransparency(0);
    matrixModel->SetNumMatrixStrings(PRESET_ICON_SIZE);
    matrixModel->SetNodesPerString(PRESET_ICON_SIZE);
    matrixModel->SetStrandsPerString(1);
    matrixModel->SetVertical(false);
    matrixModel->SetDirection("L");
    matrixModel->SetStartSide("T");

    auto& screenLoc = matrixModel->GetModelScreenLocation();
    screenLoc.SetWorldPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    auto& boxedLoc = dynamic_cast<BoxedScreenLocation&>(screenLoc);
    boxedLoc.SetScale(1.0f, 1.0f);
    boxedLoc.SetScaleZ(1.0f);
    screenLoc.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

    matrixModel->SetLayoutGroup("Unassigned");
    matrixModel->SetName(PRESET_MODEL_NAME);
    matrixModel->SetStartChannel("1");
    matrixModel->Setup();

    _presetSequenceElements.AddElement(_presetModel->GetName(), "Model",
                                        true, false, false, false, false);
}

std::vector<std::shared_ptr<xlImage>> iPadRenderContext::RenderEffectToFrames(
    Model* matrixModel, SequenceData& seqData, SequenceElements& seqElements,
    size_t numFrames, int frameTimeMs) {
    std::vector<std::shared_ptr<xlImage>> result;
    if (!matrixModel || numFrames == 0) return result;

    int width = 0, height = 0;
    matrixModel->GetBufferSize("Default", "2D", "None", width, height, 0);
    if (width <= 0 || height <= 0) return result;

    size_t channels = (size_t)width * (size_t)height * 3;
    seqData.init(channels, numFrames, frameTimeMs, true);

    EnsureRenderEngine();

    std::atomic<bool> renderComplete{false};
    _renderEngine->Render(seqElements, seqData,
                           { matrixModel }, { matrixModel },
                           0, (int)numFrames - 1,
                           nullptr, true,
                           [&renderComplete](bool) { renderComplete = true; });

    while (!renderComplete) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    for (size_t i = 0; i < numFrames; i++) {
        auto img = std::make_shared<xlImage>(width, height);
        FillXlImagePreset(*img, matrixModel,
                           (uint8_t*)&seqData[i][0], 1, true);
        result.push_back(img);
    }

    // Tier 1 #6: release the preset SequenceData allocation once
    // we've copied frames out to xlImages. For a 64×64 preset at
    // 100 frames that's ~8 MB; for shader preview sweeps with
    // hundreds of frames it balloons much further. Callers that
    // need it again (the next shader preview, the next thumbnail
    // batch) re-init via the standard path — allocation is cheap.
    seqData.Cleanup();

    return result;
}

void iPadRenderContext::GenerateShaderPreview(ShaderMediaCacheEntry* entry) {
    if (!entry || entry->GetShaderSource().empty()) return;
    if (entry->HasPreview()) return;

    ShaderConfig* config = entry->GetShaderConfig(&_presetSequenceElements);
    if (!config) {
        // Try the main sequence's elements if the preset doesn't know
        // about this shader yet. ShaderConfig creation only needs a
        // `SequenceElements*` handle for logging.
        config = entry->GetShaderConfig(&_sequenceElements);
        if (!config) return;
    }

    // Serialize concurrent shader-preview requests. The single preset
    // scaffolding (shared `_presetSequenceElements` / `_presetSequenceData`
    // / `_presetModel`) can only host one render at a time; a second
    // caller racing through here would reset the effect layer out
    // from under the first render. Swift kicks off one thumbnail
    // request per shader on sheet open, so we need to queue them —
    // the earlier CAS-based guard *dropped* concurrent requests
    // instead of queueing, which is why only one more preview
    // populated per sheet open.
    static std::mutex s_previewMutex;
    std::scoped_lock lock(s_previewMutex);
    // Re-check after locking — a caller ahead of us may have already
    // rendered this exact entry.
    if (entry->HasPreview()) return;

    EnsurePresetModel();

    // Build the default settings string: file + global shader
    // parameters + per-uniform defaults. Mirrors
    // `GenerateShaderPreview` in ShaderPreviewGenerator.cpp.
    std::string settings = "E_0FILEPICKERCTRL_IFS=" + entry->GetFilePath();
    settings += ",E_SLIDER_Shader_Speed=100";
    settings += ",E_TEXTCTRL_Shader_Offset_X=0,E_TEXTCTRL_Shader_Offset_Y=0";
    settings += ",E_TEXTCTRL_Shader_Zoom=0,E_TEXTCTRL_Shader_LeadIn=0";

    for (const auto& parm : config->GetParms()) {
        if (!parm.ShowParm()) continue;
        switch (parm._type) {
            case ShaderParmType::SHADER_PARM_FLOAT: {
                std::string key = "E_TEXTCTRL_" +
                    parm.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_TEXTCTRL);
                char buf[64];
                snprintf(buf, sizeof(buf), "%.4f", parm._default);
                settings += "," + key + "=" + buf;
                break;
            }
            case ShaderParmType::SHADER_PARM_BOOL: {
                std::string key = "E_CHECKBOX_" +
                    parm.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_CHECKBOX);
                settings += "," + key + "=" + (parm._default != 0.0 ? "1" : "0");
                break;
            }
            case ShaderParmType::SHADER_PARM_LONGCHOICE: {
                std::string key = "E_CHOICE_" +
                    parm.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_CHOICE);
                auto choices = parm.GetChoices();
                if (!choices.empty()) {
                    int idx = (int)parm._default;
                    std::string choiceStr = choices[0];
                    for (const auto& [val, str] : parm._valueOptions) {
                        if (val == idx) { choiceStr = str; break; }
                    }
                    settings += "," + key + "=" + choiceStr;
                }
                break;
            }
            case ShaderParmType::SHADER_PARM_POINT2D: {
                std::string keyBase = parm.GetUndecoratedId(
                    ShaderCtrlType::SHADER_CTRL_TEXTCTRL);
                char bufX[64], bufY[64];
                snprintf(bufX, sizeof(bufX), "%.4f", parm._defaultPt.x);
                snprintf(bufY, sizeof(bufY), "%.4f", parm._defaultPt.y);
                settings += ",E_TEXTCTRL_" + keyBase + "X=" + bufX;
                settings += ",E_TEXTCTRL_" + keyBase + "Y=" + bufY;
                break;
            }
            default:
                break;
        }
    }

    const std::string palette =
        "C_BUTTON_Palette1=#FFFFFF,C_BUTTON_Palette2=#FF0000,"
        "C_CHECKBOX_Palette1=1,C_CHECKBOX_Palette2=1";

    Element* elem = _presetSequenceElements.GetElement(_presetModel->GetName());
    if (!elem) return;

    for (const auto& it : elem->GetEffectLayers()) {
        it->DeleteAllEffects();
    }
    if (elem->GetEffectLayerCount() == 0) {
        elem->AddEffectLayer();
    }

    EffectLayer* el = elem->GetEffectLayer(0);
    el->AddEffect(0, "Shader", settings, palette, 0, 1000,
                   false, false, true);

    constexpr int frameTimeMs = 50;
    constexpr size_t numFrames = 20; // 1 second at 50 ms — matches desktop

    auto frames = RenderEffectToFrames(_presetModel, _presetSequenceData,
                                        _presetSequenceElements,
                                        numFrames, frameTimeMs);
    if (!frames.empty()) {
        entry->SetPreviewFrames(std::move(frames), frameTimeMs);
    }
}

// ----------------------------------------------------------------------------
// FSEQ write/read — mirrors the v2/zstd/sparse path of
// `xLightsFrame::WriteFalconPiFile` (TabConvert.cpp:1565) +
// `FileConverter::WriteFalconPiFile` (FileConverter.cpp:1488). The iPad
// produces fseq files with the same shape so that FPP / Falcon controllers
// playing back an iPad-saved show see no difference from a desktop save.
// ----------------------------------------------------------------------------

namespace {

// Recursive port of `addRanges` in src-ui-wx/app-shell/TabConvert.cpp:1552.
// ModelGroups expand to their member models so the per-model channel ranges
// — not the group's amalgamated range — are what we record.
void addModelRanges(Model* m, std::map<uint32_t, uint32_t>& ranges) {
    if (!m) return;
    if (auto* grp = dynamic_cast<ModelGroup*>(m)) {
        for (auto* m2 : grp->Models()) {
            addModelRanges(m2, ranges);
        }
    } else {
        uint32_t cur = ranges[m->GetFirstChannel()];
        ranges[m->GetFirstChannel()] = std::max(m->GetChanCount(), cur);
    }
}

// Walk the master view (every ELEMENT_TYPE_MODEL element in `_sequenceElements`)
// and produce a collapsed list of (startChannel, length) sparse ranges that
// covers every model's channels. Mirrors the loop at TabConvert.cpp:1580–1622
// with the same gapEliminate=0 behavior — adjacent/overlapping ranges merge
// but otherwise gaps are preserved.
std::vector<std::pair<uint32_t, uint32_t>>
ComputeMasterViewSparseRanges(SequenceElements& seqElements,
                              iPadRenderContext& ctx) {
    std::map<uint32_t, uint32_t> ranges;
    int n = seqElements.GetElementCount();
    for (int i = 0; i < n; ++i) {
        Element* element = seqElements.GetElement(i);
        if (!element) continue;
        if (element->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
        Model* m = ctx.GetModel(element->GetModelName());
        addModelRanges(m, ranges);
    }

    std::vector<std::pair<uint32_t, uint32_t>> result;
    constexpr uint32_t kSentinel = UINT32_MAX;
    constexpr uint32_t gapEliminate = 0;
    std::pair<uint32_t, uint32_t> cur(kSentinel, kSentinel);
    for (auto& a : ranges) {
        if (cur.first == kSentinel) {
            cur.first = a.first;
            cur.second = a.second;
        } else if (a.first <= (cur.first + cur.second + gapEliminate)) {
            uint32_t maxEnd = std::max(cur.first + cur.second - 1,
                                       a.first + a.second - 1);
            cur.second = maxEnd - cur.first + 1;
        } else {
            result.push_back(cur);
            cur.first = a.first;
            cur.second = a.second;
        }
    }
    if (cur.first != kSentinel) {
        result.push_back(cur);
    }
    return result;
}

// Build one FE / FC variable header for an FPP timing track. Matches the
// layout in FileConverter.cpp:1576–1632: 1-byte version, 4-byte command count,
// null-terminated host list (empty), then per-command name + count + (start,
// end) frame pairs converted from MS via stepTime.
FSEQFile::VariableHeader BuildFppCommandHeader(TimingElement* te, int stepTime) {
    std::map<std::string, std::vector<std::pair<uint32_t, uint32_t>>> commands;
    for (int l = 0; l < (int)te->GetEffectLayerCount(); ++l) {
        EffectLayer* layer = te->GetEffectLayer(l);
        if (!layer) continue;
        for (auto& eff : layer->GetAllEffects()) {
            if (!eff) continue;
            commands[eff->GetEffectName()].push_back(
                std::make_pair(eff->GetStartTimeMS(), eff->GetEndTimeMS()));
        }
    }

    int totalLen = 3; // 1 byte ver, 2 byte count (legacy layout — see desktop)
    const std::string fppInstances; // null-terminated host list, currently empty
    totalLen += fppInstances.size() + 1;
    for (auto& a : commands) {
        totalLen += a.first.length() + 1 + 4;
        totalLen += a.second.size() * 8;
    }

    FSEQFile::VariableHeader header;
    header.extendedData = true;
    header.code[0] = 'F';
    header.code[1] = (te->GetSubType() == "FPP Effects") ? 'E' : 'C';
    header.data.resize(totalLen);

    uint8_t* data = &header.data[0];
    data[0] = 1;
    uint32_t* t2 = reinterpret_cast<uint32_t*>(&data[1]);
    *t2 = static_cast<uint32_t>(commands.size());
    std::memcpy(&data[5], fppInstances.c_str(), fppInstances.size() + 1);
    data += 6 + fppInstances.size();
    for (auto& a : commands) {
        const std::string& c = a.first;
        uint32_t count = static_cast<uint32_t>(a.second.size());
        std::memcpy(data, c.c_str(), c.length() + 1);
        data += c.length() + 1;
        uint32_t* t = reinterpret_cast<uint32_t*>(data);
        *t = count;
        data += 4;
        ++t;
        for (size_t x = 0; x < count; ++x) {
            uint32_t sframe = a.second[x].first / stepTime;
            uint32_t eframe = a.second[x].second / stepTime;
            *t = sframe; ++t;
            *t = eframe; ++t;
        }
        data += count * 8;
    }
    return header;
}

inline uint32_t roundTo4(uint32_t n) { return (n + 3) & ~uint32_t(3); }

} // namespace

bool iPadRenderContext::WriteFseq(const std::string& path) {
    if (!IsSequenceLoaded()) return false;
    if (path.empty()) return false;

    if (_sequenceData.NumChannels() == 0 || _sequenceData.NumFrames() == 0) {
        spdlog::warn("WriteFseq: sequence data is empty (no render run yet?). Skipping.");
        return false;
    }

    std::unique_ptr<FSEQFile> file(FSEQFile::createFSEQFile(
        path, 2, FSEQFile::CompressionType::zstd, 2));
    if (!file) {
        spdlog::error("WriteFseq: failed to create FSEQ file at {}", path);
        return false;
    }

    file->enableMinorVersionFeatures(2);
    const uint32_t stepSize = roundTo4(static_cast<uint32_t>(_sequenceData.NumChannels()));
    file->setChannelCount(stepSize);
    file->setStepTime(_sequenceData.FrameTime());
    file->setNumFrames(static_cast<uint32_t>(_sequenceData.NumFrames()));

    if (_sequenceFile) {
        const std::string& mediaFile = _sequenceFile->GetMediaFile();
        if (!mediaFile.empty()) {
            FSEQFile::VariableHeader mf;
            mf.code[0] = 'm';
            mf.code[1] = 'f';
            mf.data.assign(mediaFile.begin(), mediaFile.end());
            mf.data.push_back('\0');
            file->addVariableHeader(mf);
        }
    }

    {
        FSEQFile::VariableHeader sp;
        sp.code[0] = 's';
        sp.code[1] = 'p';
        const std::string ver = "xLights iPadOS " + xlights_version_string;
        sp.data.assign(ver.begin(), ver.end());
        sp.data.push_back('\0');
        file->addVariableHeader(sp);
    }

    auto sparse = ComputeMasterViewSparseRanges(_sequenceElements, *this);
    auto* v2 = dynamic_cast<V2FSEQFile*>(file.get());
    if (v2 && !sparse.empty()) {
        for (auto& r : sparse) {
            v2->m_sparseRanges.push_back(r);
            spdlog::info("WriteFseq sparse range start={} end={} size={}",
                         r.first + 1, r.first + r.second, r.second);
        }
    }

    for (int x = 0; x < _sequenceElements.GetNumberOfTimingElements(); ++x) {
        TimingElement* te = _sequenceElements.GetTimingElement(x);
        if (!te) continue;
        const std::string& sub = te->GetSubType();
        if (sub == "FPP Commands" || sub == "FPP Effects") {
            file->addVariableHeader(BuildFppCommandHeader(te, _sequenceData.FrameTime()));
        }
    }

    file->writeHeader();
    const uint32_t numFrames = static_cast<uint32_t>(_sequenceData.NumFrames());
    for (uint32_t fr = 0; fr < numFrames; ++fr) {
        file->addFrame(fr, &_sequenceData[fr][0]);
    }
    file->finalize();
    spdlog::info("WriteFseq: wrote {} frames × {} channels (stepSize={}) to {}",
                 numFrames, _sequenceData.NumChannels(), stepSize, path);
    return true;
}

bool iPadRenderContext::TryLoadFseq(const std::string& fseqPath,
                                     const std::string& xsqPath) {
    if (!IsSequenceLoaded()) return false;
    if (fseqPath.empty()) return false;

    std::error_code ec;
    if (!std::filesystem::exists(fseqPath, ec) || ec) {
        return false;
    }

    if (!xsqPath.empty()) {
        auto fseqMtime = std::filesystem::last_write_time(fseqPath, ec);
        if (ec) return false;
        auto xsqMtime = std::filesystem::last_write_time(xsqPath, ec);
        if (ec) return false;
        if (fseqMtime < xsqMtime) {
            spdlog::info("TryLoadFseq: fseq is older than xsq; will render.");
            return false;
        }
    }

    std::unique_ptr<FSEQFile> file(FSEQFile::openFSEQFile(fseqPath));
    if (!file) {
        spdlog::warn("TryLoadFseq: openFSEQFile failed for {}", fseqPath);
        return false;
    }

    if (file->getVersionMajor() != 2) {
        spdlog::info("TryLoadFseq: not a v2 fseq (got v{}); will render.",
                     file->getVersionMajor());
        return false;
    }

    const uint32_t fileFrames = static_cast<uint32_t>(file->getNumFrames());
    const int fileStep = file->getStepTime();

    const uint32_t expectedFrames = static_cast<uint32_t>(_sequenceData.NumFrames());
    const int expectedStep = _sequenceData.FrameTime();

    if (fileFrames != expectedFrames || fileStep != expectedStep) {
        spdlog::info("TryLoadFseq: shape mismatch (frames {} vs {}, step {} vs {}); will render.",
                     fileFrames, expectedFrames, fileStep, expectedStep);
        return false;
    }

    // Sparse-range comparison is the real validity check. The on-disk channel
    // count for a sparse fseq is the SUM of sparse-range lengths (see
    // V2FSEQFile::writeHeader's recalculation), not the max channel address —
    // so comparing it directly to _sequenceData.NumChannels() is meaningless.
    // What we actually need is: do the file's sparse ranges match the ranges
    // we'd compute from today's master view? If yes, the fseq covers exactly
    // the channels we'd render and we can use it; if not (added/removed
    // models, view change, channel reshuffling), we render.
    auto* v2 = dynamic_cast<V2FSEQFile*>(file.get());
    if (!v2) {
        spdlog::warn("TryLoadFseq: failed to cast to V2FSEQFile; will render.");
        return false;
    }

    auto expectedRanges = ComputeMasterViewSparseRanges(_sequenceElements, *this);
    if (expectedRanges.size() != v2->m_sparseRanges.size()) {
        spdlog::info("TryLoadFseq: sparse-range count mismatch ({} vs {}); will render.",
                     v2->m_sparseRanges.size(), expectedRanges.size());
        return false;
    }
    for (size_t i = 0; i < expectedRanges.size(); ++i) {
        if (expectedRanges[i] != v2->m_sparseRanges[i]) {
            spdlog::info("TryLoadFseq: sparse range #{} mismatch (file {}+{} vs expected {}+{}); will render.",
                         i, v2->m_sparseRanges[i].first, v2->m_sparseRanges[i].second,
                         expectedRanges[i].first, expectedRanges[i].second);
            return false;
        }
    }

    // Pass the actual sparse ranges to prepareRead so V2 doesn't trip its
    // "requested range outside read ranges" warning and so we read only the
    // bytes that are actually stored. `readFrame` then uses the same range
    // list to scatter each compressed-frame chunk back to its absolute
    // channel offset in `_sequenceData`.
    file->prepareRead(expectedRanges, 0);

    const uint32_t maxChan = static_cast<uint32_t>(_sequenceData.NumChannels());
    for (uint32_t fr = 0; fr < fileFrames; ++fr) {
        std::unique_ptr<FSEQFile::FrameData> fd(file->getFrame(fr));
        if (!fd) {
            spdlog::warn("TryLoadFseq: getFrame({}) returned null; falling back to render.", fr);
            return false;
        }
        if (!fd->readFrame(&_sequenceData[fr][0], maxChan)) {
            spdlog::warn("TryLoadFseq: readFrame({}) failed; falling back to render.", fr);
            return false;
        }
    }

    spdlog::info("TryLoadFseq: loaded {} frames over {} sparse range(s) from {}",
                 fileFrames, expectedRanges.size(), fseqPath);
    return true;
}
