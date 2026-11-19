/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// The layout side of a collaboration session: what memory's layout is, and
// bringing it to the layout the session committed. See
// src-core/collab/CollabLayout.h.

#include "iPadRenderContext.h"

#include "models/Model.h"
#include "models/ModelGroup.h"
#include "render/ShowMergeApply.h"
#include "utils/ShowFileGuard.h"

#include <globals.h>
#include <log.h>

#include <sstream>

void iPadRenderContext::SetCollabSession(bool inSession, bool liveLayout, bool host) {
    if (inSession == _collabInSession && liveLayout == _collabLiveLayout && host == _collabHost) return;
    _collabInSession = inSession;
    _collabLiveLayout = inSession && liveLayout;
    _collabHost = inSession && host;
    spdlog::info("iPadRenderContext: collaboration session {}, layout {}, {}", inSession ? "on" : "off",
                 _collabLiveLayout ? "live" : "not live", _collabHost ? "host" : "client");
    if (_layoutLockChanged) _layoutLockChanged();
}

bool iPadRenderContext::RefusesSequenceSettingsWrite(const char* what) const {
    if (xLightsShowContext::RefusesSequenceSettingsWrite(what)) return true;
    if (_collabInSession) {
        spdlog::warn("Sequence settings are frozen during a collaboration session: refusing {}", what);
        return true;
    }
    return false;
}

bool iPadRenderContext::RefusesLayoutFileWrite(const char* what) const {
    if (RefusesLayoutWrite(what)) return true;
    if (InLiveLayoutSession() && !_collabHost) {
        spdlog::warn("Only the session host saves the layout: refusing {}", what);
        return true;
    }
    return false;
}

void iPadRenderContext::NoteLayoutEdit() {
    ++_layoutEditNotes;
}

uint64_t iPadRenderContext::LayoutGeneration() {
    uint64_t recorded = 0;
    if (!showDirectory.empty()) {
        auto& guard = ShowFileGuard::Instance();
        auto mix = [&recorded](const std::optional<ShowFileFingerprint>& f) {
            if (f) recorded = recorded * 1099511628211ULL ^ (f->hash + f->size);
        };
        mix(guard.RecordedFingerprint(showDirectory + "/" + XLIGHTS_RGBEFFECTS_FILE));
        if (!_outputManager.GetNetworksFilePath().empty()) {
            mix(guard.RecordedFingerprint(_outputManager.GetNetworksFilePath()));
        }
    }
    const uint64_t dirt = _layoutDirt.Generation();
    const unsigned int models = AllModels.GetModelGeneration();
    if (dirt != _seenDirtGeneration || _layoutEditNotes != _seenEditNotes || models != _seenModelGeneration ||
        recorded != _seenRecordedHash) {
        _seenDirtGeneration = dirt;
        _seenEditNotes = _layoutEditNotes;
        _seenModelGeneration = models;
        _seenRecordedHash = recorded;
        ++_layoutGeneration;
        _layoutChangedAt = std::chrono::steady_clock::now();
    }
    return _layoutGeneration;
}

bool iPadRenderContext::LayoutEditRecentlyActive(int quietMs) {
    if (_layoutGestureActive) return true;
    LayoutGeneration();
    return std::chrono::steady_clock::now() - _layoutChangedAt < std::chrono::milliseconds(quietMs);
}

std::optional<std::string> iPadRenderContext::MemoryLayoutText() {
    if (showDirectory.empty()) return std::nullopt;
    std::optional<std::string> base = _layoutDirt.Base();
    if (!base) base = ShowFileGuard::Instance().RecordedText(showDirectory + "/" + XLIGHTS_RGBEFFECTS_FILE);
    if (!base || base->empty()) return std::nullopt;
    pugi::xml_document doc;
    if (!doc.load_string(base->c_str()) || !PatchLayoutDocument(doc)) return std::nullopt;
    std::ostringstream out;
    doc.save(out, "  ");
    return out.str();
}

bool iPadRenderContext::SerializeLayoutFiles(std::string& rgbEffects, std::string& networks) {
    auto text = MemoryLayoutText();
    if (!text) return false;
    rgbEffects = std::move(*text);
    networks = _outputManager.SaveToString();
    return true;
}

bool iPadRenderContext::ApplyLiveLayout(const std::string& currentRgb, const std::string& targetRgb, bool rgbDiffers,
                                        const std::string& targetNetworks, bool networksDiffer,
                                        ShowMergeApply::LayoutChanges& changes) {
    if (!rgbDiffers && !networksDiffer) return true;
    if (_layoutGestureActive) {
        // A drag holds the model it moves; the session tries again after.
        return false;
    }
    pugi::xml_document currentDoc;
    pugi::xml_document targetDoc;
    if (rgbDiffers && (!currentDoc.load_string(currentRgb.c_str()) || !targetDoc.load_string(targetRgb.c_str()))) {
        spdlog::error("iPadRenderContext: the session layout does not parse; not applying it");
        return false;
    }
    // On the main thread: wait briefly, and let the session try again rather
    // than hold the UI while a render drains.
    ModelMutationScope mutate(*this, 250);
    if (!mutate.ok()) {
        spdlog::info("iPadRenderContext: the render is busy; the session layout is applied later");
        return false;
    }
    if (rgbDiffers) {
        changes = ShowMergeApply::ApplyLayout(*this, currentDoc, targetDoc);
        pugi::xml_node root = targetDoc.child("xrgb");
        if (!root) root = targetDoc.child("xlights");
        if (root) ReloadHostLayoutSections(root, changes.otherSections);
        // Memory is now the target as a whole; the file still has the old
        // layout, so this stays an edit until the host saves it.
        _layoutDirt.AdoptBase(targetRgb);
    }
    if (networksDiffer) {
        ShowMergeApply::ReloadControllers(*this, targetNetworks, /*keepRenameLog*/ true);
        _layoutDirt.MarkControllers();
    }
    return true;
}

void iPadRenderContext::ReloadHostLayoutSections(pugi::xml_node root, const std::set<std::string>& sections) {
    if (sections.count("settings")) {
        LoadLayoutSettings(root.child("settings"));
    }
    if (sections.count("layoutGroups")) {
        LoadNamedLayoutGroups(root);
        bool found = _activeLayoutGroup == "Default";
        for (const auto& g : _namedLayoutGroups) {
            found |= g.name == _activeLayoutGroup;
        }
        if (!found) _activeLayoutGroup = "Default";
    }
    if (sections.count("colors")) {
        LoadPalette(root);
    }
}

bool iPadRenderContext::DeleteModelMarked(const std::string& name) {
    Model* m = AllModels.GetModel(name);
    if (!m) return false;
    const bool isGroup = m->GetDisplayAs() == DisplayAsType::ModelGroup;
    // The delete also takes the model out of the groups that list it and
    // moves models chained to it onto its start channel; their records
    // change with it.
    std::vector<std::string> dependents;
    const std::string chainedToIt = ">" + name + ":1";
    for (const auto& [otherName, other] : AllModels.GetModels()) {
        if (!other || otherName == name) continue;
        if (other->GetDisplayAs() == DisplayAsType::ModelGroup) {
            if (static_cast<ModelGroup*>(other)->ListsModelOrSubmodelOf(name)) dependents.push_back(otherName);
        } else if (other->ModelStartChannel == chainedToIt) {
            dependents.push_back(otherName);
        }
    }
    if (!AllModels.Delete(name)) return false;
    if (isGroup) {
        MarkGroupDeleted(name);
    } else {
        MarkModelDeleted(name);
    }
    for (const auto& d : dependents) {
        MarkLayoutModelDirty(d);
    }
    return true;
}
