/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowMergeApply.h"

#include "xLightsShowContext.h"

#include "diagnostics/CheckSequenceReport.h"
#include "diagnostics/SequenceChecker.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/ViewObject.h"
#include "models/ViewObjectManager.h"
#include "outputs/OutputManager.h"
#include "render/Element.h"
#include "render/EffectLayer.h"
#include "render/CommandExecutor.h"
#include "render/SequenceElementCommands.h"
#include "render/SequenceElements.h"
#include "render/UndoManager.h"
#include "utils/LayoutObjectId.h"
#include "utils/LayoutRecords.h"
#include "utils/LayoutRenameLog.h"
#include "utils/ShowFileMerge.h"

#include <algorithm>
#include <map>

#include <spdlog/spdlog.h>

namespace ShowMergeApply {

namespace {

using ShowFileMerge::FileKind;

struct Rec {
    pugi::xml_node node;
    std::string name;
    std::string id;
    bool group = false;
};

std::vector<Rec> Records(pugi::xml_node root, const char* section, const char* tag, bool group) {
    std::vector<Rec> out;
    for (pugi::xml_node n : root.child(section).children(tag)) {
        Rec r;
        r.node = n;
        r.name = LayoutRecords::NameOf(n);
        r.id = LayoutObjectId::Read(n);
        r.group = group;
        if (!r.name.empty()) out.push_back(r);
    }
    return out;
}

// The records of `result` matched to those of `mine`: by id when both have
// one, else by name. -1 where there is no match.
std::vector<int> MatchRecords(const std::vector<Rec>& mine, const std::vector<Rec>& result) {
    std::vector<int> match(mine.size(), -1);
    std::vector<char> taken(result.size(), 0);
    for (size_t i = 0; i < mine.size(); ++i) {
        if (mine[i].id.empty()) continue;
        for (size_t j = 0; j < result.size(); ++j) {
            if (!taken[j] && result[j].id == mine[i].id) {
                match[i] = (int)j;
                taken[j] = 1;
                break;
            }
        }
    }
    for (size_t i = 0; i < mine.size(); ++i) {
        if (match[i] != -1) continue;
        for (size_t j = 0; j < result.size(); ++j) {
            if (taken[j] || result[j].name != mine[i].name) continue;
            if (!mine[i].id.empty() && !result[j].id.empty()) continue; // two different records
            match[i] = (int)j;
            taken[j] = 1;
            break;
        }
    }
    return match;
}

bool HasEffects(xLightsShowContext& ctx, const std::string& name) {
    if (!ctx.IsSequenceLoaded()) return false;
    Element* e = ctx.GetSequenceElements().GetElement(name);
    if (e == nullptr) return false;
    for (size_t i = 0; i < e->GetEffectLayerCount(); ++i) {
        if (e->GetEffectLayer(i)->GetEffectCount() > 0) return true;
    }
    return false;
}

void ApplyModels(xLightsShowContext& ctx, const std::vector<Rec>& mine, const std::vector<Rec>& result, LayoutChanges& changes) {
    ModelManager& mm = ctx.AllModels;
    const int w = mm.GetPreviewWidth();
    const int h = mm.GetPreviewHeight();
    auto match = MatchRecords(mine, result);
    std::vector<char> matched(result.size(), 0);
    for (int j : match) {
        if (j >= 0) matched[j] = 1;
    }

    // Gone in the result.
    for (size_t i = 0; i < mine.size(); ++i) {
        if (match[i] != -1 || mm.GetModel(mine[i].name) == nullptr) continue;
        const bool effects = HasEffects(ctx, mine[i].name);
        if (mm.Delete(mine[i].name, false)) {
            changes.modelsRemoved.push_back(mine[i].name);
            if (effects) changes.removedWithEffects.push_back(mine[i].name);
        }
    }

    // Changed or renamed: models before groups, so a rebuilt group finds its
    // members. A rename to a name another record still holds waits for that
    // record to move first.
    struct Pending {
        size_t mine;
        size_t result;
    };
    for (bool groups : { false, true }) {
        std::vector<Pending> pending;
        for (size_t i = 0; i < mine.size(); ++i) {
            int j = match[i];
            if (j < 0 || mine[i].group != groups) continue;
            const Rec& r = result[j];
            const bool renamed = r.name != mine[i].name;
            if (!renamed && ShowFileMerge::RecordsEqual(FileKind::RgbEffects, mine[i].node, r.node)) continue;
            pending.push_back({ i, (size_t)j });
        }
        while (!pending.empty()) {
            bool progress = false;
            for (auto it = pending.begin(); it != pending.end();) {
                const Rec& m = mine[it->mine];
                const Rec& r = result[it->result];
                const bool renamed = r.name != m.name;
                if (renamed && mm.GetModel(r.name) != nullptr) {
                    ++it;
                    continue;
                }
                Model* nm = mm.CreateModel(r.node, w, h);
                if (nm == nullptr) {
                    spdlog::warn("ShowMergeApply: could not build the merged '{}'", r.name);
                    it = pending.erase(it);
                    progress = true;
                    continue;
                }
                if (renamed) {
                    ctx.GetSequenceElements().RenameModelInViews(m.name, r.name);
                    changes.modelsRenamed.emplace_back(m.name, r.name);
                }
                mm.ReplaceModel(m.name, nm);
                changes.modelsChanged.push_back(r.name);
                it = pending.erase(it);
                progress = true;
            }
            if (!progress) {
                for (const auto& p : pending) {
                    spdlog::warn("ShowMergeApply: cannot rename '{}' to '{}': the name is taken", mine[p.mine].name, result[p.result].name);
                }
                break;
            }
        }
        // New in the result.
        for (size_t j = 0; j < result.size(); ++j) {
            if (matched[j] || result[j].group != groups) continue;
            if (mm.GetModel(result[j].name) != nullptr) {
                Model* nm = mm.CreateModel(result[j].node, w, h);
                if (nm != nullptr) {
                    mm.ReplaceModel(result[j].name, nm);
                    changes.modelsChanged.push_back(result[j].name);
                }
                continue;
            }
            if (mm.createAndAddModel(result[j].node, w, h) != nullptr) {
                changes.modelsAdded.push_back(result[j].name);
            }
        }
    }
}

void ApplyObjects(xLightsShowContext& ctx, const std::vector<Rec>& mine, const std::vector<Rec>& result, LayoutChanges& changes) {
    ViewObjectManager& vm = ctx.AllObjects;
    auto match = MatchRecords(mine, result);
    std::vector<char> matched(result.size(), 0);
    for (int j : match) {
        if (j >= 0) matched[j] = 1;
    }
    for (size_t i = 0; i < mine.size(); ++i) {
        int j = match[i];
        if (j < 0) {
            if (vm.GetViewObject(mine[i].name) != nullptr) {
                vm.Delete(mine[i].name);
                changes.objectsRemoved.push_back(mine[i].name);
            }
            continue;
        }
        const Rec& r = result[j];
        if (r.name == mine[i].name && ShowFileMerge::RecordsEqual(FileKind::RgbEffects, mine[i].node, r.node)) continue;
        ViewObject* obj = vm.CreateObject(r.node);
        if (obj == nullptr) continue;
        if (r.name != mine[i].name && vm.GetViewObject(mine[i].name) != nullptr) {
            vm.Delete(mine[i].name);
        }
        vm.AddViewObject(obj);
        changes.objectsChanged.push_back(r.name);
    }
    for (size_t j = 0; j < result.size(); ++j) {
        if (matched[j]) continue;
        ViewObject* obj = vm.CreateObject(result[j].node);
        if (obj == nullptr) continue;
        vm.AddViewObject(obj);
        changes.objectsChanged.push_back(result[j].name);
    }
}

bool SectionEqual(pugi::xml_node a, pugi::xml_node b) {
    return ShowFileMerge::RecordsEqual(FileKind::RgbEffects, a, b);
}

} // namespace

LayoutChanges ApplyLayout(xLightsShowContext& ctx, const pugi::xml_document& mine, const pugi::xml_document& result) {
    LayoutChanges changes;
    pugi::xml_node rm = mine.document_element();
    pugi::xml_node rr = result.document_element();
    if (!rr) return changes;

    std::vector<Rec> mineRecs = Records(rm, "models", "model", false);
    auto mineGroups = Records(rm, "modelGroups", "modelGroup", true);
    mineRecs.insert(mineRecs.end(), mineGroups.begin(), mineGroups.end());
    std::vector<Rec> resultRecs = Records(rr, "models", "model", false);
    auto resultGroups = Records(rr, "modelGroups", "modelGroup", true);
    resultRecs.insert(resultRecs.end(), resultGroups.begin(), resultGroups.end());
    ApplyModels(ctx, mineRecs, resultRecs, changes);

    ApplyObjects(ctx, Records(rm, "view_objects", "view_object", false), Records(rr, "view_objects", "view_object", false), changes);

    if (!SectionEqual(rm.child("modelSets"), rr.child("modelSets"))) {
        ctx.AllModels.GetSetManager().Load(rr.child("modelSets"));
        changes.modelSetsReloaded = true;
    }
    if (!SectionEqual(rm.child("views"), rr.child("views"))) {
        const int selected = ctx._sequenceViewManager.GetSelectedViewIndex();
        ctx._sequenceViewManager.Reset();
        ctx._sequenceViewManager.Load(rr.child("views"), selected);
        changes.viewsReloaded = true;
    }

    if (!SectionEqual(rm.child(LayoutRenameHints::SECTION), rr.child(LayoutRenameHints::SECTION))) {
        ctx._renameHints.Load(rr.child(LayoutRenameHints::SECTION));
    }

    static const std::set<std::string> handled = { "models", "modelGroups", "view_objects", "modelSets", "views", LayoutRenameHints::SECTION };
    std::set<std::string> sections;
    for (pugi::xml_node n : rm.children()) {
        if (n.type() == pugi::node_element) sections.insert(n.name());
    }
    for (pugi::xml_node n : rr.children()) {
        if (n.type() == pugi::node_element) sections.insert(n.name());
    }
    for (const auto& s : sections) {
        if (handled.count(s)) continue;
        if (!SectionEqual(rm.child(s.c_str()), rr.child(s.c_str()))) {
            changes.otherSections.insert(s);
        }
    }
    if (changes.otherSections.count("Viewpoints") && rr.child("Viewpoints")) {
        ctx.viewpoint_mgr.Load(rr.child("Viewpoints"));
    }

    if (changes.ModelsChanged()) {
        ctx.AllModels.RecalcStartChannels();
    }
    if (changes.Any()) {
        spdlog::info("ShowMergeApply: models +{} ~{} -{}, objects ~{} -{}, views {}, model sets {}, other sections {}",
                     changes.modelsAdded.size(), changes.modelsChanged.size(), changes.modelsRemoved.size(),
                     changes.objectsChanged.size(), changes.objectsRemoved.size(), changes.viewsReloaded, changes.modelSetsReloaded,
                     changes.otherSections.size());
    }
    return changes;
}

namespace {
std::string NameList(const std::vector<std::string>& names) {
    std::string s;
    for (size_t i = 0; i < names.size() && i < 10; ++i) {
        if (i) s += ", ";
        s += names[i];
    }
    if (names.size() > 10) s += ", ... (" + std::to_string(names.size()) + " in all)";
    return s;
}
} // namespace

std::vector<std::string> LayoutChanges::Describe() const {
    std::vector<std::string> out;
    if (!modelsAdded.empty()) out.push_back("Added: " + NameList(modelsAdded));
    if (!modelsChanged.empty()) out.push_back("Changed: " + NameList(modelsChanged));
    if (!modelsRemoved.empty()) out.push_back("Removed: " + NameList(modelsRemoved));
    if (!objectsChanged.empty() || !objectsRemoved.empty()) {
        std::vector<std::string> all = objectsChanged;
        all.insert(all.end(), objectsRemoved.begin(), objectsRemoved.end());
        out.push_back("Objects: " + NameList(all));
    }
    if (viewsReloaded) out.push_back("Sequencer views");
    if (modelSetsReloaded) out.push_back("Model sets");
    return out;
}

std::vector<std::string> LayoutChanges::Warnings() const {
    std::vector<std::string> out;
    if (!removedWithEffects.empty()) {
        out.push_back("Someone else removed " + NameList(removedWithEffects) +
                      " from the layout. Its effects were removed from the open sequence too; close the sequence without saving it to keep them in the sequence file.");
    }
    return out;
}

ShowSaveMerge::SequenceUse SequenceUseOf(xLightsShowContext& ctx) {
    return [&ctx](const std::string& name) {
        ShowSaveMerge::SequenceUseInfo info;
        Element* e = ctx.GetSequenceElements().GetElement(name);
        if (e != nullptr && e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            info.present = true;
            info.effects = e->GetEffectCount();
        }
        return info;
    };
}

namespace {
int CountEffects(const SubElementSnapshot& s) {
    int n = 0;
    for (const auto& l : s.layers) n += (int)l.effects.size();
    for (const auto& [i, effects] : s.nodeEffects) n += (int)effects.size();
    return n;
}
} // namespace

int ApplySequenceChanges(xLightsShowContext& ctx, const std::vector<ShowSaveMerge::SequenceChange>& changes) {
    if (changes.empty()) return 0;
    SequenceElements& seq = ctx.GetSequenceElements();
    int dropped = 0;
    seq.get_undo_mgr().CreateUndoStep();
    ScopedCommandBatch batch(seq, "Layout Merge");
    for (const auto& ch : changes) {
        int idx = -1;
        Element* el = nullptr;
        for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW); ++i) {
            Element* e = seq.GetElement(i, MASTER_VIEW);
            if (e->GetName() == ch.model && e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                idx = (int)i;
                el = e;
                break;
            }
        }
        if (el == nullptr) continue;
        if (!ch.map) {
            ElementEdits::DeleteElement(seq, ch.model);
            spdlog::info("ShowMergeApply: removed '{}' and its {} effect(s) from the open sequence", ch.model, ch.effects);
            continue;
        }
        Model* target = ctx.AllModels.GetModel(ch.target);
        if (target == nullptr || seq.GetElement(ch.target) != nullptr) {
            spdlog::warn("ShowMergeApply: cannot move '{}' to '{}'; leaving the row alone", ch.model, ch.target);
            continue;
        }
        ElementSnapshot snap = ElementSnapshot::Of(*el);
        snap.name = ch.target;
        snap.uid = kNoSeqObjectId;
        // Rows the target has no counterpart for would become rows of a
        // submodel or strand that does not exist.
        std::vector<SubElementSnapshot> kept;
        for (auto& sub : snap.subElements) {
            const bool has = sub.strand ? sub.strandIndex < target->GetNumStrands() : target->GetSubModel(sub.name) != nullptr;
            if (has) {
                kept.push_back(std::move(sub));
            } else {
                dropped += CountEffects(sub);
            }
        }
        snap.subElements = std::move(kept);
        const SeqObjectId after = idx > 0 ? seq.GetElement(idx - 1, MASTER_VIEW)->GetUid() : kNoSeqObjectId;
        ElementEdits::DeleteElement(seq, ch.model);
        auto cmd = std::make_unique<AddElementCommand>(std::move(snap), after, idx);
        if (!seq.GetCommandExecutor().ExecuteInStep(std::move(cmd)).Ok()) {
            spdlog::error("ShowMergeApply: moving '{}' to '{}' failed", ch.model, ch.target);
            continue;
        }
        spdlog::info("ShowMergeApply: moved the {} effect(s) of '{}' to '{}'", ch.effects, ch.model, ch.target);
    }
    seq.PopulateRowInformation();
    return dropped;
}

namespace {

bool Reload(xLightsShowContext& ctx, const std::string* text, bool keepRenameLog) {
    OutputManager& om = ctx._outputManager;
    const bool wasOutputting = om.IsOutputting();
    if (wasOutputting) om.StopOutput();
    // Loading drops the controller renames still waiting to reach the layout
    // file's rename hints; keep them first.
    ctx._renameHints.Absorb(LayoutRenameHints::TYPE_CONTROLLER, om.GetRenameLog().TakeUnpersisted());
    std::vector<LayoutRenameLog::Entry> renames;
    if (keepRenameLog) renames = om.GetRenameLog().Entries();
    const bool ok = text ? om.LoadFromText(ctx.showDirectory, *text) : om.Load(ctx.showDirectory);
    for (const auto& [from, to] : renames) om.GetRenameLog().Record(from, to);
    // Re-recording queues them as unsaved hints again; they were kept above.
    (void)om.GetRenameLog().TakeUnpersisted();
    ctx.AllModels.RecalcStartChannels();
    if (wasOutputting) om.StartOutput();
    if (!ok) spdlog::error("ShowMergeApply: reloading the controllers from {} failed", ctx.showDirectory);
    return ok;
}

} // namespace

bool ReloadControllers(xLightsShowContext& ctx) {
    return Reload(ctx, nullptr, false);
}

bool ReloadControllers(xLightsShowContext& ctx, const std::string& text, bool keepRenameLog) {
    return Reload(ctx, &text, keepRenameLog);
}

std::vector<std::string> ModelCheckMessages(xLightsShowContext& ctx) {
    CheckSequenceReport report;
    for (const auto& s : CheckSequenceReport::REPORT_SECTIONS) {
        report.AddSection(s);
    }
    SequenceChecker checker(ctx._sequenceElements, ctx.AllModels, ctx._outputManager, nullptr, ctx.showDirectory);
    checker.RunModelChecks(report);
    std::vector<std::string> out;
    for (const auto& s : report.GetSections()) {
        for (const auto& i : s.issues) {
            if (i.type == CheckSequenceReport::ReportIssue::INFO) continue;
            std::string m = i.message;
            size_t first = m.find_first_not_of(' ');
            out.push_back(first == std::string::npos ? m : m.substr(first));
        }
    }
    return out;
}

std::vector<std::string> NewMessages(const std::vector<std::string>& before, const std::vector<std::string>& after) {
    std::vector<std::string> out;
    std::set<std::string> seen(before.begin(), before.end());
    for (const auto& a : after) {
        if (seen.insert(a).second) out.push_back(a);
    }
    return out;
}

} // namespace ShowMergeApply
