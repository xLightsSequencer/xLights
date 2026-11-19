/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowSaveMerge.h"

#include "LayoutObjectId.h"
#include "LayoutRecords.h"
#include "LayoutRenameLog.h"
#include "ShowAutosave.h"

#include <algorithm>
#include <cstdlib>
#include <map>
#include <filesystem>
#include <set>
#include <sstream>
#include <tuple>

#include <spdlog/spdlog.h>

namespace ShowSaveMerge {

using ShowFileMerge::Conflict;
using ShowFileMerge::ConflictKind;
using ShowFileMerge::Merge;
using ShowFileMerge::ReferenceIssue;
using ShowFileMerge::Resolution;

namespace {

std::string FileNameOf(const std::string& path) {
    auto slash = path.find_last_of("/\\");
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

struct File {
    FileVersions v;
    FileStatus status = FileStatus::Unchanged;
    std::string error;
    std::optional<Merge> merge;
    // DeletedWithEffects conflicts, listed after the merge's own.
    std::vector<Conflict> seq;
};

const char* const kSequenceKeyPrefix = "sequence:";

struct LayoutRec {
    pugi::xml_node node;
    std::string section;
    std::string name;
    std::string id;
};

std::vector<LayoutRec> LayoutRecs(pugi::xml_node root) {
    std::vector<LayoutRec> out;
    for (auto [section, tag] : { std::pair<const char*, const char*>{ "models", "model" }, { "modelGroups", "modelGroup" } }) {
        for (pugi::xml_node n : root.child(section).children(tag)) {
            LayoutRec r;
            r.node = n;
            r.section = section;
            r.name = n.attribute("name").value();
            r.id = LayoutObjectId::Read(n);
            if (!r.name.empty()) out.push_back(r);
        }
    }
    return out;
}

// The records of `mine` that `result` no longer has. Matched by layout id
// where both have one, else by name, so a rename is not a delete.
std::vector<LayoutRec> DeletedRecords(pugi::xml_node mine, pugi::xml_node result) {
    std::vector<LayoutRec> out;
    auto res = LayoutRecs(result);
    for (const auto& m : LayoutRecs(mine)) {
        bool found = false;
        for (const auto& r : res) {
            if (r.section != m.section) continue;
            if (!m.id.empty() && !r.id.empty() ? r.id == m.id : r.name == m.name) {
                found = true;
                break;
            }
        }
        if (!found) out.push_back(m);
    }
    return out;
}

std::string IssueKey(const ReferenceIssue& i) {
    return i.section + "\x1f" + i.record + "\x1f" + i.message;
}

std::optional<pugi::xml_document> ParseXml(const std::string& text) {
    if (text.empty()) {
        return std::nullopt;
    }
    std::optional<pugi::xml_document> doc(std::in_place);
    if (!doc->load_string(text.c_str())) {
        return std::nullopt;
    }
    return doc;
}

} // namespace

bool HeldWithLayout(FileKind kind) {
    return kind != FileKind::Presets;
}

HeldSplit SplitHeldWithLayout(const std::vector<FileToSave>& files) {
    HeldSplit out;
    for (const auto& f : files) {
        if (!HeldWithLayout(f.kind)) {
            out.writable.push_back(f);
        } else if (f.write) {
            out.held.push_back(f.path);
        }
    }
    return out;
}

struct Plan::Impl {
    std::vector<File> files;
    ShowFileMerge::Options options;
    SequenceUse use;
    struct Decision {
        Resolution resolution = Resolution::Unresolved;
        std::string target;
    };
    std::map<std::string, Decision> seqDecisions;

    // The models and groups the merged layout deletes that the open
    // sequence has effects on. One the merge itself still asks about (I
    // changed it, they deleted it) waits for that answer.
    void RefreshSequence(File& f) {
        f.seq.clear();
        if (!use || f.v.kind != FileKind::RgbEffects || !f.v.write || f.status != FileStatus::Merged || !f.merge) return;
        pugi::xml_document mine;
        if (!mine.load_string(f.v.mine.c_str())) return;
        const auto& engine = f.merge->Conflicts();
        for (const auto& rec : DeletedRecords(mine.document_element(), f.merge->ResultXml().document_element())) {
            SequenceUseInfo u = use(rec.name);
            if (u.effects <= 0) continue;
            bool pending = false;
            for (const auto& c : engine) {
                if (c.resolution == Resolution::Unresolved && c.section == rec.section &&
                    ((!rec.id.empty() && c.recordId == rec.id) || c.mineName == rec.name || c.baseName == rec.name)) {
                    pending = true;
                }
            }
            if (pending) continue;
            Conflict c;
            c.key = std::string(kSequenceKeyPrefix) + rec.section + "[" + rec.name + "]";
            c.kind = ConflictKind::DeletedWithEffects;
            c.section = rec.section;
            c.recordId = rec.id;
            c.baseName = rec.name;
            c.mineName = rec.name;
            c.mineValue = std::to_string(u.effects) + (u.effects == 1 ? " effect" : " effects") + " in the open sequence";
            c.mapAllowed = true;
            if (auto d = seqDecisions.find(c.key); d != seqDecisions.end()) {
                c.resolution = d->second.resolution;
                c.mapTarget = d->second.target;
            }
            f.seq.push_back(std::move(c));
        }
    }

    void RefreshSequenceAll() {
        for (auto& f : files) RefreshSequence(f);
    }

    std::vector<std::string> MapTargets(const File& f) const {
        std::vector<std::string> out;
        if (!use) return out;
        // Only models and groups memory already holds: the row moves before
        // the merged layout is brought into memory.
        pugi::xml_document doc;
        pugi::xml_document mine;
        if (!doc.load_string(Result(f).c_str()) || !mine.load_string(f.v.mine.c_str())) return out;
        std::set<std::string> held;
        for (const auto& r : LayoutRecs(mine.document_element())) held.insert(r.name);
        for (const auto& r : LayoutRecs(doc.document_element())) {
            if (held.count(r.name) && !use(r.name).present) out.push_back(r.name);
        }
        return out;
    }

    // Networks first: the layout merge follows the controller renames it finds.
    std::vector<size_t> Order() const {
        std::vector<size_t> order;
        for (size_t i = 0; i < files.size(); ++i) {
            if (files[i].v.kind == FileKind::Networks) order.push_back(i);
        }
        for (size_t i = 0; i < files.size(); ++i) {
            if (files[i].v.kind != FileKind::Networks) order.push_back(i);
        }
        return order;
    }

    ShowFileMerge::NameMaps ControllerMaps() const {
        for (const auto& f : files) {
            if (f.v.kind == FileKind::Networks && f.status == FileStatus::Merged && f.merge) {
                return f.merge->ControllerNameMaps();
            }
        }
        return options.controllerNameMaps;
    }

    void Run(File& f) {
        f.merge.reset();
        f.error.clear();
        if (!f.v.changedOnDisk) {
            f.status = FileStatus::Unchanged;
            return;
        }
        if (!f.v.base) {
            f.status = FileStatus::NoBase;
            f.error = "the version it was loaded from is not known";
            return;
        }
        ShowFileMerge::Options o = options;
        if (f.v.kind == FileKind::RgbEffects) {
            o.controllerNameMaps = ControllerMaps();
        }
        Merge m = Merge::Run(f.v.kind, *f.v.base, f.v.mine, f.v.theirs, o);
        if (!m.Ok()) {
            f.status = FileStatus::Failed;
            f.error = m.Error();
            return;
        }
        f.status = FileStatus::Merged;
        f.merge.emplace(std::move(m));
    }

    void RunAll() {
        for (size_t i : Order()) {
            Run(files[i]);
        }
    }

    static std::vector<std::pair<std::string, Resolution>> Resolutions(const File& f) {
        std::vector<std::pair<std::string, Resolution>> out;
        if (f.merge) {
            for (const auto& c : f.merge->Conflicts()) {
                if (c.resolution != Resolution::Unresolved) out.emplace_back(c.key, c.resolution);
            }
        }
        return out;
    }

    static void Apply(File& f, const std::vector<std::pair<std::string, Resolution>>& resolutions) {
        if (!f.merge) return;
        // A resolution can change what merges next, so repeat until stable.
        for (int pass = 0; pass < 8; ++pass) {
            bool changed = false;
            for (const auto& [key, r] : resolutions) {
                for (const auto& c : f.merge->Conflicts()) {
                    if (c.key == key && c.resolution != r) {
                        changed |= f.merge->Resolve(key, r);
                        break;
                    }
                }
            }
            if (!changed) break;
        }
    }

    // The controller renames feed the layout merge, so a decision on a
    // controller conflict re-runs the layout merge, keeping its decisions.
    void RemergeLayout() {
        for (auto& f : files) {
            if (f.v.kind == FileKind::RgbEffects && f.status == FileStatus::Merged) {
                auto kept = Resolutions(f);
                Run(f);
                Apply(f, kept);
                RefreshSequence(f);
            }
        }
    }

    std::string Result(const File& f) const {
        if (!(f.status == FileStatus::Merged && f.merge)) {
            return f.v.mine;
        }
        bool keep = false;
        for (const auto& c : f.seq) keep |= c.resolution == Resolution::Mine;
        if (!keep) {
            return f.merge->ResultText();
        }
        // Keep: the result holds this process's record of the model or group.
        pugi::xml_document mine;
        pugi::xml_document out;
        if (!mine.load_string(f.v.mine.c_str())) return f.merge->ResultText();
        out.reset(f.merge->ResultXml());
        pugi::xml_node root = out.document_element();
        for (const auto& c : f.seq) {
            if (c.resolution != Resolution::Mine) continue;
            for (const auto& rec : LayoutRecs(mine.document_element())) {
                if (rec.section == c.section && rec.name == c.mineName) {
                    pugi::xml_node section = root.child(c.section.c_str());
                    if (!section) section = root.append_child(c.section.c_str());
                    section.append_copy(rec.node);
                    break;
                }
            }
        }
        std::ostringstream os;
        out.save(os, "  ", pugi::format_default, pugi::encoding_utf8);
        return os.str();
    }
};

Plan::Plan(std::unique_ptr<Impl> impl) :
    _impl(std::move(impl)) {
}
Plan::Plan(Plan&&) noexcept = default;
Plan& Plan::operator=(Plan&&) noexcept = default;
Plan::~Plan() = default;

Plan Plan::FromVersions(std::vector<FileVersions> files, const ShowFileMerge::Options& options, SequenceUse sequenceUse) {
    auto impl = std::make_unique<Impl>();
    impl->options = options;
    impl->use = std::move(sequenceUse);
    for (auto& v : files) {
        File f;
        f.v = std::move(v);
        impl->files.push_back(std::move(f));
    }
    impl->RunAll();
    impl->RefreshSequenceAll();
    return Plan(std::move(impl));
}

Plan Plan::Prepare(const std::vector<FileToSave>& files, const ShowFileMerge::Options& options, const ShowFileGuard& guard,
                   SequenceUse sequenceUse) {
    std::vector<FileVersions> versions;
    for (const auto& f : files) {
        FileVersions v;
        v.kind = f.kind;
        v.path = f.path;
        v.mine = f.mine;
        v.write = f.write;
        v.base = guard.RecordedText(f.path);
        auto disk = ShowFileGuard::ReadFile(f.path);
        auto recorded = guard.RecordedFingerprint(f.path);
        if (disk) {
            v.theirs = std::move(disk->first);
            v.theirsFingerprint = disk->second;
            v.changedOnDisk = recorded.has_value() && !(*recorded == disk->second);
        } else {
            // Unreadable now: nothing to merge with, and the write would fail
            // anyway; treat it as unchanged like the guard does.
            spdlog::warn("ShowSaveMerge: cannot read '{}' to check it for changes", f.path);
            v.theirsFingerprint = recorded.value_or(ShowFileFingerprint{});
        }
        if (v.changedOnDisk) {
            spdlog::info("ShowSaveMerge: '{}' was changed by someone else since it was loaded or saved here{}", f.path,
                         v.base ? "" : " (its base is not known, so it cannot be merged)");
        }
        versions.push_back(std::move(v));
    }
    return FromVersions(std::move(versions), options, std::move(sequenceUse));
}

size_t Plan::FileCount() const { return _impl->files.size(); }
const std::string& Plan::Path(size_t file) const { return _impl->files.at(file).v.path; }
FileKind Plan::Kind(size_t file) const { return _impl->files.at(file).v.kind; }
bool Plan::Writes(size_t file) const { return _impl->files.at(file).v.write; }
FileStatus Plan::Status(size_t file) const { return _impl->files.at(file).status; }
const std::string& Plan::Error(size_t file) const { return _impl->files.at(file).error; }
const std::string& Plan::Mine(size_t file) const { return _impl->files.at(file).v.mine; }

std::optional<size_t> Plan::Find(FileKind kind, bool writtenOnly) const {
    for (size_t i = 0; i < _impl->files.size(); ++i) {
        const auto& f = _impl->files[i];
        if (f.v.kind == kind && (!writtenOnly || f.v.write)) return i;
    }
    return std::nullopt;
}

bool Plan::ExternalChanges() const {
    for (const auto& f : _impl->files) {
        if (f.v.write && f.v.changedOnDisk) return true;
    }
    return false;
}

bool Plan::CanMerge() const {
    for (const auto& f : _impl->files) {
        if (f.v.write && (f.status == FileStatus::NoBase || f.status == FileStatus::Failed)) return false;
    }
    return true;
}

std::vector<std::string> Plan::ChangedPaths() const {
    std::vector<std::string> out;
    for (const auto& f : _impl->files) {
        if (f.v.write && f.v.changedOnDisk) out.push_back(f.v.path);
    }
    return out;
}

std::vector<ConflictRef> Plan::Conflicts() const {
    std::vector<ConflictRef> out;
    for (size_t i : _impl->Order()) {
        const auto& f = _impl->files[i];
        if (!f.v.write || !f.merge) continue;
        const size_t n = f.merge->Conflicts().size();
        for (size_t c = 0; c < n + f.seq.size(); ++c) out.push_back({ i, c });
    }
    return out;
}

const Conflict& Plan::GetConflict(ConflictRef ref) const {
    const auto& f = _impl->files.at(ref.file);
    const auto& engine = f.merge->Conflicts();
    if (ref.index >= engine.size()) {
        return f.seq.at(ref.index - engine.size());
    }
    return engine.at(ref.index);
}

bool Plan::Resolve(ConflictRef ref, Resolution resolution, const std::string& mapTarget) {
    auto& f = _impl->files.at(ref.file);
    if (!f.merge) return false;
    const size_t n = f.merge->Conflicts().size();
    if (ref.index >= n) {
        if (ref.index - n >= f.seq.size()) return false;
        const Conflict& c = f.seq[ref.index - n];
        if (resolution == Resolution::KeepBoth) return false;
        if (resolution == Resolution::Map) {
            auto targets = _impl->MapTargets(f);
            if (mapTarget.empty() || std::find(targets.begin(), targets.end(), mapTarget) == targets.end()) return false;
        }
        _impl->seqDecisions[c.key] = { resolution, resolution == Resolution::Map ? mapTarget : std::string() };
        _impl->RefreshSequence(f);
        return true;
    }
    if (!f.merge->Resolve(ref.index, resolution)) return false;
    if (f.v.kind == FileKind::Networks) _impl->RemergeLayout();
    _impl->RefreshSequence(f);
    return true;
}

void Plan::ResolveAll(Resolution resolution) {
    if (resolution != Resolution::Mine && resolution != Resolution::Theirs) return;
    for (size_t i : _impl->Order()) {
        auto& f = _impl->files[i];
        if (!f.v.write || !f.merge) continue;
        // The listed deletions get the answer too; one the answer itself
        // brings (theirs deleted a model mine changed) is asked about next.
        for (const auto& c : f.seq) {
            _impl->seqDecisions[c.key] = { resolution, std::string() };
        }
        f.merge->ResolveAll(resolution);
        if (f.v.kind == FileKind::Networks) _impl->RemergeLayout();
        _impl->RefreshSequence(f);
    }
}

bool Plan::AllResolved() const {
    for (const auto& f : _impl->files) {
        if (!f.v.write || !f.merge) continue;
        if (!f.merge->AllResolved()) return false;
        for (const auto& c : f.seq) {
            if (c.resolution == Resolution::Unresolved || (c.resolution == Resolution::Map && c.mapTarget.empty())) return false;
        }
    }
    return true;
}

void Plan::AdoptResolutions(const Plan& previous) {
    for (const auto& [key, d] : previous._impl->seqDecisions) {
        _impl->seqDecisions[key] = d;
    }
    for (size_t i : _impl->Order()) {
        auto& f = _impl->files[i];
        for (const auto& p : previous._impl->files) {
            if (p.v.path == f.v.path && p.v.kind == f.v.kind) {
                Impl::Apply(f, Impl::Resolutions(p));
            }
        }
        if (f.v.kind == FileKind::Networks) {
            // The layout re-runs with the new controller renames; its own
            // decisions are applied on its turn.
            _impl->RemergeLayout();
        }
        _impl->RefreshSequence(f);
    }
}

std::vector<std::string> Plan::MapTargets() const {
    for (const auto& f : _impl->files) {
        if (f.v.kind == FileKind::RgbEffects && f.v.write) return _impl->MapTargets(f);
    }
    return {};
}

std::vector<SequenceChange> Plan::SequenceChanges() const {
    std::vector<SequenceChange> out;
    for (const auto& f : _impl->files) {
        for (const auto& c : f.seq) {
            if (c.resolution != Resolution::Theirs && c.resolution != Resolution::Map) continue;
            SequenceChange ch;
            ch.model = c.mineName;
            ch.effects = (int)std::strtol(c.mineValue.value_or("0").c_str(), nullptr, 10);
            ch.map = c.resolution == Resolution::Map;
            ch.target = c.mapTarget;
            out.push_back(ch);
        }
    }
    return out;
}

std::string Plan::Result(size_t file) const {
    return _impl->Result(_impl->files.at(file));
}

bool Plan::ResultDiffersFromMine(size_t file) const {
    const auto& f = _impl->files.at(file);
    return f.status == FileStatus::Merged && _impl->Result(f) != f.v.mine;
}

WriteResult Plan::Write(ShowFileGuard& guard, std::string* error) const {
    auto fail = [&](WriteResult r, const std::string& msg) {
        if (error) *error = msg;
        return r;
    };
    if (!CanMerge()) {
        return fail(WriteResult::Refused, "a changed file cannot be merged");
    }
    if (!AllResolved()) {
        return fail(WriteResult::Refused, "not every conflict has been resolved");
    }
    for (const auto& f : _impl->files) {
        if (!f.v.write) continue;
        auto now = ShowFileGuard::FingerprintFile(f.v.path);
        if (now && !(*now == f.v.theirsFingerprint)) {
            spdlog::info("ShowSaveMerge: '{}' changed on disk again while saving", f.v.path);
            return fail(WriteResult::ChangedAgain, FileNameOf(f.v.path) + " changed again while saving");
        }
    }
    for (size_t i : _impl->Order()) {
        const auto& f = _impl->files[i];
        if (!f.v.write) continue;
        if (!guard.SaveFile(f.v.path, _impl->Result(f))) {
            return fail(WriteResult::Failed, "unable to write " + f.v.path);
        }
        if (f.status == FileStatus::Merged) {
            spdlog::info("ShowSaveMerge: saved '{}' merged with the changes someone else saved ({} conflict(s) decided)", f.v.path,
                         f.merge ? f.merge->Conflicts().size() : 0);
        }
    }
    return WriteResult::Written;
}

SaveReport Plan::Report() const {
    SaveReport report;
    const File* rgb = nullptr;
    const File* nets = nullptr;
    for (const auto& f : _impl->files) {
        if (f.v.kind == FileKind::RgbEffects && f.v.write) rgb = &f;
        if (f.v.kind == FileKind::Networks && (nets == nullptr || f.v.write)) nets = &f;
        if (!f.v.write || f.status != FileStatus::Merged || !f.merge) continue;
        report.mergedFiles.push_back(FileNameOf(f.v.path));
        for (const auto& r : f.merge->Renames()) {
            const std::string kind = RecordKindName(r.section);
            if (!r.theirsName.empty() && r.theirsName != r.baseName && r.mergedName == r.theirsName) {
                report.renames.push_back(kind + " '" + r.baseName + "' was renamed to '" + r.theirsName + "' by someone else.");
            } else if (!r.mineName.empty() && !r.theirsName.empty() && r.mineName != r.baseName && r.theirsName != r.baseName &&
                       r.mergedName != r.theirsName) {
                report.renames.push_back(kind + " '" + r.baseName + "' was renamed to '" + r.theirsName + "' by someone else; it keeps your name '" +
                                         r.mergedName + "'.");
            }
        }
    }
    if (report.mergedFiles.empty()) {
        return report;
    }
    for (const auto& f : _impl->files) {
        for (const auto& c : f.seq) {
            const std::string what = RecordKindName(c.section) + " '" + c.mineName + "'";
            const long n = std::strtol(c.mineValue.value_or("0").c_str(), nullptr, 10);
            const std::string effects = n == 1 ? std::string("effect") : std::to_string(n) + " effects";
            switch (c.resolution) {
            case Resolution::Mine:
                report.changes.push_back(what + ": someone else deleted it; you kept it and its " + effects + " in the open sequence.");
                break;
            case Resolution::Theirs:
                report.warnings.push_back(what + " was deleted by someone else; its " + effects +
                                          " were removed from the open sequence (Undo brings them back).");
                break;
            case Resolution::Map:
                report.warnings.push_back(what + " was deleted by someone else; its " + effects + " were moved to '" + c.mapTarget +
                                          "' (Undo moves them back).");
                break;
            default:
                break;
            }
        }
    }

    // What the merged files will hold vs what this process wanted to save:
    // only problems the merge brought in are reported.
    std::vector<ReferenceIssue> after;
    std::vector<ReferenceIssue> before;
    for (const auto& f : _impl->files) {
        if (f.v.write && f.status == FileStatus::Merged && f.merge) {
            auto issues = f.merge->ReferenceIssues();
            after.insert(after.end(), issues.begin(), issues.end());
        }
    }
    auto netsOut = nets ? ParseXml(nets->v.write ? _impl->Result(*nets) : nets->v.theirs) : std::nullopt;
    auto netsMine = nets ? ParseXml(nets->v.mine) : std::nullopt;
    if (rgb != nullptr) {
        auto rgbOut = ParseXml(_impl->Result(*rgb));
        auto rgbMine = ParseXml(rgb->v.mine);
        if (rgbOut) {
            auto issues = ShowFileMerge::CheckReferences(*rgbOut, netsOut ? &*netsOut : nullptr);
            after.insert(after.end(), issues.begin(), issues.end());
        }
        if (rgbMine) {
            auto issues = ShowFileMerge::CheckReferences(*rgbMine, netsMine ? &*netsMine : nullptr);
            before.insert(before.end(), issues.begin(), issues.end());
            issues = ShowFileMerge::CheckReferences(*rgbMine, nullptr);
            before.insert(before.end(), issues.begin(), issues.end());
        }
    }
    if (netsMine) {
        pugi::xml_document empty;
        auto issues = ShowFileMerge::CheckReferences(empty, &*netsMine);
        before.insert(before.end(), issues.begin(), issues.end());
    }
    std::set<std::string> seen;
    for (const auto& i : before) seen.insert(IssueKey(i));
    for (const auto& i : after) {
        if (seen.insert(IssueKey(i)).second) report.issues.push_back(i);
    }
    return report;
}

std::string SaveReport::Text() const {
    std::ostringstream os;
    if (!mergedFiles.empty()) {
        os << "Merged your changes with the changes someone else saved in:\n";
        for (const auto& f : mergedFiles) os << "    " << f << "\n";
    }
    if (!changes.empty()) {
        os << "\nBrought in:\n";
        for (const auto& c : changes) os << "    " << c << "\n";
    }
    if (!warnings.empty()) {
        os << "\n";
        for (const auto& w : warnings) os << w << "\n";
    }
    if (!renames.empty()) {
        os << "\nRenamed:\n";
        for (const auto& r : renames) os << "    " << r << "\n";
    }
    if (!issues.empty()) {
        os << "\nPlease check:\n";
        for (const auto& i : issues) {
            os << "    ";
            if (!i.record.empty()) os << RecordKindName(i.section) << " '" << i.record << "': ";
            os << i.message << "\n";
        }
    }
    if (!checks.empty()) {
        os << "\nModel checks:\n";
        for (const auto& c : checks) os << "    " << c << "\n";
    }
    return os.str();
}

bool CanSaveUnattended(const Plan& plan) {
    if (!plan.ExternalChanges()) return true;
    return plan.CanMerge() && !plan.HasConflicts();
}

void ClearRenameLogs(const Plan& plan, LayoutRenameLog* models, LayoutRenameLog* viewObjects, LayoutRenameLog* controllers) {
    if (plan.Find(FileKind::RgbEffects)) {
        if (models) models->Clear();
        if (viewObjects) viewObjects->Clear();
    }
    if (plan.Find(FileKind::Networks) && controllers) {
        controllers->Clear();
    }
}

std::optional<Plan> PrepareAutosaveMerge(FileKind kind, const std::string& realPath, const std::string& backupPath) {
    auto base = ShowAutosave::ReadBaseCopy(backupPath);
    if (!base) return std::nullopt;
    auto mine = ShowFileGuard::ReadFile(backupPath);
    auto theirs = ShowFileGuard::ReadFile(realPath);
    if (!mine || !theirs) return std::nullopt;
    FileVersions v;
    v.kind = kind;
    v.path = realPath;
    v.mine = std::move(mine->first);
    v.base = std::move(*base);
    v.theirs = std::move(theirs->first);
    v.theirsFingerprint = theirs->second;
    v.changedOnDisk = !(ShowFileGuard::FingerprintOf(*v.base) == v.theirsFingerprint) && !(v.base->empty() && !v.theirsFingerprint.exists);
    std::vector<FileVersions> files;
    files.push_back(std::move(v));
    return Plan::FromVersions(std::move(files), {});
}

std::string RecordKindName(const std::string& section) {
    static const std::pair<const char*, const char*> names[] = {
        { "models", "Model" }, { "modelGroups", "Group" }, { "view_objects", "Object" }, { "views", "View" },
        { "perspectives", "Perspective" }, { "layoutGroups", "Layout group" }, { "Viewpoints", "Viewpoint" },
        { "palettes", "Palette" }, { "settings", "Setting" }, { "colors", "Colour" }, { "modelSets", "Model set" },
        { "Controller", "Controller" }, { "presets", "Preset" }, { "Networks", "Controllers file" }, { "xrgb", "Layout file" },
    };
    for (const auto& [s, n] : names) {
        if (section == s) return n;
    }
    return section;
}

namespace {

void DescribeNode(pugi::xml_node n, std::string& out) {
    if (n.type() != pugi::node_element) return;
    std::string part;
    const std::string name = LayoutRecords::NameOf(n);
    part += n.name();
    if (!name.empty()) part += " '" + name + "'";
    std::string attrs;
    for (pugi::xml_attribute a = n.first_attribute(); a; a = a.next_attribute()) {
        std::string_view an = a.name();
        if (an == "name" || an == "Name" || LayoutRecords::IsBookkeepingAttribute(an)) continue;
        if (!attrs.empty()) attrs += ", ";
        std::string v = a.value();
        if (v.size() > 60) v = v.substr(0, 57) + "...";
        attrs += std::string(an) + "=" + v;
    }
    if (!attrs.empty()) part += ": " + attrs;
    if (!out.empty()) out += "\n";
    out += part;
    for (pugi::xml_node c = n.first_child(); c; c = c.next_sibling()) {
        DescribeNode(c, out);
    }
}

} // namespace

std::string ReadableValue(const std::optional<std::string>& value) {
    if (!value) return "(none)";
    const std::string& v = *value;
    size_t first = v.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return v.empty() ? "(empty)" : v;
    if (v[first] != '<') return v;
    pugi::xml_document doc;
    if (!doc.load_string(v.c_str())) return v;
    std::string out;
    for (pugi::xml_node n = doc.first_child(); n; n = n.next_sibling()) {
        DescribeNode(n, out);
    }
    return out.empty() ? v : out;
}

ConflictText Plan::Describe(ConflictRef ref) const {
    const auto& f = _impl->files.at(ref.file);
    const Conflict& c = GetConflict(ref);
    ConflictText t;
    t.fileName = FileNameOf(f.v.path);
    std::string name = !c.mineName.empty() ? c.mineName : (!c.theirsName.empty() ? c.theirsName : c.baseName);
    t.record = RecordKindName(c.section);
    if (!name.empty()) t.record += " '" + name + "'";
    const std::string detail = c.detail;
    switch (c.kind) {
    case ConflictKind::AttributeChanged:
        t.what = detail + ": you and someone else both changed it";
        break;
    case ConflictKind::ChildChanged:
        t.what = detail + ": you and someone else both changed it";
        break;
    case ConflictKind::RecordChanged:
        t.what = "You and someone else both changed it";
        break;
    case ConflictKind::DeletedByMine:
        t.what = "You deleted it; someone else changed it";
        break;
    case ConflictKind::DeletedByTheirs:
        t.what = "Someone else deleted it; you changed it";
        break;
    case ConflictKind::BothAdded:
        t.what = "You and someone else both added one with this name";
        break;
    case ConflictKind::BothRenamed:
        t.what = "You and someone else renamed it differently";
        break;
    case ConflictKind::OrderChanged:
        t.what = "You and someone else both reordered the controllers";
        break;
    case ConflictKind::DeletedWithEffects:
        t.what = "Someone else deleted it; the open sequence has " + c.mineValue.value_or("effects") + " on it";
        break;
    }
    auto side = [&](const std::optional<std::string>& v, bool deleted) {
        if (!v) return std::string(deleted ? "(deleted)" : "(none)");
        return ReadableValue(v);
    };
    t.base = c.baseValue ? ReadableValue(c.baseValue) : std::string();
    t.mine = side(c.mineValue, c.kind == ConflictKind::DeletedByMine || c.kind == ConflictKind::ChildChanged || c.kind == ConflictKind::AttributeChanged);
    t.theirs = side(c.theirsValue, c.kind == ConflictKind::DeletedByTheirs || c.kind == ConflictKind::ChildChanged || c.kind == ConflictKind::AttributeChanged);
    if (c.kind == ConflictKind::BothRenamed) {
        t.mine = c.mineName;
        t.theirs = c.theirsName;
    }
    if (c.kind == ConflictKind::DeletedWithEffects) {
        t.base.clear();
        t.mine = "Keep the model and its effects";
        t.theirs = "Delete the model and its effects from the open sequence";
        if (c.resolution == Resolution::Map) {
            t.mine = "Move its effects to '" + c.mapTarget + "'";
        }
    }
    return t;
}

} // namespace ShowSaveMerge
