/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowFileLiveMerge.h"

#include "xLightsShowContext.h"

#include "effects/EffectPresetManager.h"
#include "utils/ShowFileMerge.h"
#include "utils/ShowPresence.h"

#include <cctype>
#include <map>
#include <set>
#include <string_view>

#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <spdlog/spdlog.h>

namespace ShowFileLiveMerge {

namespace {

std::optional<std::pair<std::string, ShowFileFingerprint>> ReadWith(const Reader& reader, const std::string& path) {
    return reader ? reader(path) : ShowFileGuard::ReadFile(path);
}

std::string RecordKey(pugi::xml_node n) {
    std::string id = n.attribute("xlId").value();
    if (!id.empty()) {
        return "#" + id;
    }
    const char* name = n.attribute("name") ? n.attribute("name").value() : n.attribute("Name").value();
    return std::string(n.name()) + ":" + name;
}

bool IsNamed(pugi::xml_node n) {
    return n.type() == pugi::node_element && (n.attribute("name") || n.attribute("Name"));
}

bool HasNamedChildren(pugi::xml_node n) {
    for (pugi::xml_node c : n.children()) {
        if (IsNamed(c)) return true;
    }
    return false;
}

size_t CountNodes(FileKind kind, pugi::xml_node a, pugi::xml_node b, int depth) {
    if (!a || !b) {
        return (a || b) ? 1 : 0;
    }
    if (HasNamedChildren(a) || HasNamedChildren(b)) {
        std::map<std::string, pugi::xml_node> left;
        std::map<std::string, pugi::xml_node> right;
        for (pugi::xml_node c : a.children()) {
            if (IsNamed(c)) left[RecordKey(c)] = c;
        }
        for (pugi::xml_node c : b.children()) {
            if (IsNamed(c)) right[RecordKey(c)] = c;
        }
        size_t n = 0;
        for (const auto& [k, node] : left) {
            auto it = right.find(k);
            if (it == right.end() || !ShowFileMerge::RecordsEqual(kind, node, it->second)) ++n;
        }
        for (const auto& [k, node] : right) {
            if (!left.count(k)) ++n;
        }
        return n;
    }
    if (depth == 0) {
        // The layout file's root: its children are sections.
        std::set<std::string> sections;
        for (pugi::xml_node c : a.children()) {
            if (c.type() == pugi::node_element) sections.insert(c.name());
        }
        for (pugi::xml_node c : b.children()) {
            if (c.type() == pugi::node_element) sections.insert(c.name());
        }
        size_t n = 0;
        for (const auto& s : sections) {
            n += CountNodes(kind, a.child(s.c_str()), b.child(s.c_str()), depth + 1);
        }
        return n;
    }
    return ShowFileMerge::RecordsEqual(kind, a, b) ? 0 : 1;
}

const char* KindWord(FileKind kind) {
    switch (kind) {
    case FileKind::RgbEffects: return "Layout";
    case FileKind::Networks: return "Controllers";
    case FileKind::Presets: return "Effect presets";
    }
    return "Show files";
}

} // namespace

size_t CountChangedItems(FileKind kind, const std::string& before, const std::string& after) {
    if (before == after) {
        return 0;
    }
    if (kind == FileKind::Presets) {
        auto a = nlohmann::json::parse(before, nullptr, false);
        auto b = nlohmann::json::parse(after, nullptr, false);
        return (!a.is_discarded() && !b.is_discarded() && a == b) ? 0 : 1;
    }
    pugi::xml_document a;
    pugi::xml_document b;
    if (!a.load_string(before.c_str()) || !b.load_string(after.c_str())) {
        return 1;
    }
    return CountNodes(kind, a.document_element(), b.document_element(), 0);
}

Incoming::Incoming(ShowSaveMerge::Plan plan) :
    _plan(std::move(plan)) {
}

Incoming::Incoming(Incoming&&) noexcept = default;
Incoming& Incoming::operator=(Incoming&&) noexcept = default;
Incoming::~Incoming() = default;

Incoming Incoming::Prepare(const std::vector<LocalFile>& files, const ShowFileMerge::Options& options,
                           const ShowFileGuard& guard, ShowSaveMerge::SequenceUse sequenceUse, const Reader& reader) {
    struct Read {
        std::string theirs;
        std::string base;
        ShowFileFingerprint fingerprint;
        bool changed = false;
        bool edited = false;
    };
    std::map<std::string, Read> reads;
    std::vector<ShowSaveMerge::FileVersions> versions;
    for (const auto& f : files) {
        ShowSaveMerge::FileVersions v;
        v.kind = f.kind;
        v.path = f.path;
        v.write = true;
        v.base = guard.RecordedText(f.path);
        Read r;
        r.edited = f.edited;
        if (auto disk = ReadWith(reader, f.path)) {
            v.theirs = disk->first;
            v.theirsFingerprint = disk->second;
            auto recorded = guard.RecordedFingerprint(f.path);
            v.changedOnDisk = !recorded || !(*recorded == disk->second);
        } else {
            // Unreadable: treat it as unchanged; the watcher reports it again.
            v.theirs = v.base.value_or(std::string());
            v.theirsFingerprint = ShowFileGuard::FingerprintOf(v.theirs);
            v.changedOnDisk = false;
        }
        v.mine = f.edited ? f.mine : v.base.value_or(f.mine);
        r.theirs = v.theirs;
        r.base = v.base.value_or(std::string());
        r.fingerprint = v.theirsFingerprint;
        r.changed = v.changedOnDisk;
        reads[f.path] = std::move(r);
        versions.push_back(std::move(v));
    }
    Incoming in(ShowSaveMerge::Plan::FromVersions(std::move(versions), options, std::move(sequenceUse)));
    for (size_t i = 0; i < in._plan.FileCount(); ++i) {
        Read& r = reads[in._plan.Path(i)];
        in._theirs.push_back(std::move(r.theirs));
        in._base.push_back(std::move(r.base));
        in._theirsFingerprint.push_back(r.fingerprint);
        in._changed.push_back(r.changed);
        in._edited.push_back(r.edited);
    }
    return in;
}

bool Incoming::HasChanges() const {
    for (bool c : _changed) {
        if (c) return true;
    }
    return false;
}

size_t Incoming::ChangedItems() const {
    size_t n = 0;
    for (size_t i = 0; i < _plan.FileCount(); ++i) {
        if (_changed[i]) n += CountChangedItems(_plan.Kind(i), _base[i], _theirs[i]);
    }
    return n;
}

std::vector<std::string> Incoming::ChangedPaths() const {
    std::vector<std::string> out;
    for (size_t i = 0; i < _plan.FileCount(); ++i) {
        if (_changed[i]) out.push_back(_plan.Path(i));
    }
    return out;
}

std::vector<FileKind> Incoming::ChangedKinds() const {
    std::vector<FileKind> out;
    for (size_t i = 0; i < _plan.FileCount(); ++i) {
        if (_changed[i]) out.push_back(_plan.Kind(i));
    }
    return out;
}

bool Incoming::StillOnDisk(const Reader& reader) const {
    for (size_t i = 0; i < _plan.FileCount(); ++i) {
        if (!_changed[i]) continue;
        auto disk = ReadWith(reader, _plan.Path(i));
        if (!disk || !(disk->second == _theirsFingerprint[i])) return false;
    }
    return true;
}

std::vector<std::string> Applied::Describe() const {
    std::vector<std::string> out = layout.Describe();
    if (controllersReloaded) out.push_back("Controllers reloaded");
    if (presetsReloaded) out.push_back("Effect presets reloaded");
    if (droppedEffects > 0) {
        out.push_back(std::to_string(droppedEffects) + " effects on submodel or strand rows the new model does not have were dropped.");
    }
    return out;
}

Applied Apply(xLightsShowContext& ctx, const Incoming& incoming, ShowFileGuard& guard) {
    using ShowSaveMerge::FileKind;
    Applied applied;
    const ShowSaveMerge::Plan& plan = incoming.Plan();
    if (!incoming.CanApply()) {
        spdlog::warn("ShowFileLiveMerge: not applying a merge that has undecided conflicts or does not merge");
        return applied;
    }
    // The sequence side of decided deletions goes first, as one undo step, so
    // the layout's own deletes find no rows left to take effects with.
    if (auto changes = plan.SequenceChanges(); !changes.empty()) {
        applied.droppedEffects = ShowMergeApply::ApplySequenceChanges(ctx, changes);
    }
    for (size_t i = 0; i < plan.FileCount(); ++i) {
        if (!plan.ResultDiffersFromMine(i)) continue;
        const std::string result = plan.Result(i);
        switch (plan.Kind(i)) {
        case FileKind::RgbEffects: {
            pugi::xml_document mineDoc;
            pugi::xml_document resultDoc;
            if (!mineDoc.load_string(plan.Mine(i).c_str()) || !resultDoc.load_string(result.c_str())) {
                spdlog::error("ShowFileLiveMerge: the layout does not parse; not reloading it");
                break;
            }
            applied.layout = ShowMergeApply::ApplyLayout(ctx, mineDoc, resultDoc);
            applied.layoutApplied = true;
            applied.layoutResult = result;
            break;
        }
        case FileKind::Networks:
            applied.controllersReloaded = ShowMergeApply::ReloadControllers(ctx, result, incoming.Edited(i));
            break;
        case FileKind::Presets: {
            nlohmann::json j = nlohmann::json::parse(result, nullptr, false);
            if (j.is_discarded()) {
                spdlog::error("ShowFileLiveMerge: the presets do not parse; not reloading them");
                break;
            }
            ctx._effectPresetManager.LoadFromJson(j);
            applied.presetsReloaded = true;
            break;
        }
        }
    }
    // Memory is now their file plus this process's unsaved edits: their file
    // is what the next save merges against.
    for (size_t i = 0; i < plan.FileCount(); ++i) {
        if (incoming.Changed(i)) {
            guard.Record(plan.Path(i), incoming.Theirs(i));
        }
    }
    return applied;
}

std::string WhoChanged(const std::vector<ShowPresenceEntry>& freshOthers, const std::vector<FileKind>& kinds) {
    if (freshOthers.size() == 1) {
        return freshOthers.front().DisplayName();
    }
    const ShowPresenceEntry* only = nullptr;
    int editing = 0;
    for (const auto& e : freshOthers) {
        bool relevant = false;
        for (auto k : kinds) {
            relevant |= (k == FileKind::Networks) ? e.editingControllers : e.editingLayout;
        }
        if (relevant) {
            only = &e;
            ++editing;
        }
    }
    return editing == 1 ? only->DisplayName() : std::string();
}

std::string BannerText(const std::string& who, const std::vector<FileKind>& kinds, size_t changedItems, size_t decisions) {
    std::vector<FileKind> unique;
    for (auto k : kinds) {
        bool seen = false;
        for (auto u : unique) seen |= u == k;
        if (!seen) unique.push_back(k);
    }
    std::string what;
    if (unique.empty()) {
        what = "Show files";
    } else {
        for (size_t i = 0; i < unique.size(); ++i) {
            std::string w = KindWord(unique[i]);
            if (i > 0) {
                w[0] = (char)std::tolower((unsigned char)w[0]);
                what += (i + 1 == unique.size()) ? " and " : ", ";
            }
            what += w;
        }
    }
    std::string text = what + " updated by " + (who.empty() ? std::string("someone else") : who);
    if (changedItems > 0 || decisions > 0) {
        text += " — ";
        if (changedItems > 0) {
            text += std::to_string(changedItems) + (changedItems == 1 ? " item changed" : " items changed");
        }
        if (decisions > 0) {
            if (changedItems > 0) text += ", ";
            text += std::to_string(decisions) + (decisions == 1 ? " needs your decision" : " need your decision");
        }
    }
    return text;
}

} // namespace ShowFileLiveMerge
