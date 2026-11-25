/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowFileMerge.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <set>
#include <sstream>
#include <string_view>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "LayoutObjectId.h"
#include "LayoutRecords.h"
#include "LayoutRenameHints.h"
#include "LayoutRenameLog.h"

namespace ShowFileMerge {

const char* ToString(ConflictKind kind) {
    switch (kind) {
    case ConflictKind::AttributeChanged: return "attribute changed on both sides";
    case ConflictKind::ChildChanged: return "child changed on both sides";
    case ConflictKind::RecordChanged: return "changed on both sides";
    case ConflictKind::DeletedByMine: return "deleted here, changed there";
    case ConflictKind::DeletedByTheirs: return "deleted there, changed here";
    case ConflictKind::BothAdded: return "added on both sides";
    case ConflictKind::BothRenamed: return "renamed on both sides";
    case ConflictKind::OrderChanged: return "reordered on both sides";
    case ConflictKind::DeletedWithEffects: return "deleted there, has effects in the open sequence here";
    }
    return "";
}

namespace {

using OptStr = std::optional<std::string>;

// How a record merges.
enum class Mode {
    Whole,  // replaced as a unit
    Fields, // attributes one by one, children as a keyed list
};

// How a record is identified within its list.
enum class KeyRule {
    Name,    // its name ("name" or "Name" attribute) and layout id; unnamed elements of one tag form one record
    Tag,     // its element name (sections, settings, colours)
    Content, // its content (palettes: a changed palette is a different palette)
};

struct Policy {
    Mode mode = Mode::Whole;
    KeyRule key = KeyRule::Name;
    // A section a writer may simply not write: missing on one side is "not
    // changed there", never "deleted there".
    bool absentIsUnchanged = false;
    // An attribute that holds a comma separated member list.
    const char* listAttr = nullptr;
    // This process's renames of these records (fallback matching).
    const LayoutRenameLog* log = nullptr;
};

// Where a list of children sits.
struct Scope {
    int depth = 0; // 0: children of the document element
    std::string parentTag;
    // The parent is a record both versions say came from the base show folder.
    bool fromBase = false;
    std::string section;
    std::string path;
};

struct Logs {
    const LayoutRenameLog* models = nullptr;
    const LayoutRenameLog* viewObjects = nullptr;
    const LayoutRenameLog* controllers = nullptr;
};

bool IsRgbSection(std::string_view tag) {
    static const std::set<std::string_view> known = { "models", "modelGroups", "view_objects", "layoutGroups", "perspectives", "views",
                                                      "Viewpoints", "palettes", "settings", "colors", "modelSets" };
    return known.count(tag) != 0;
}

Policy Classify(FileKind kind, const Scope& scope, std::string_view tag, const Logs& logs) {
    Policy p;
    switch (kind) {
    case FileKind::RgbEffects:
        if (scope.depth == 0) {
            p.mode = IsRgbSection(tag) ? Mode::Fields : Mode::Whole;
            p.key = KeyRule::Tag;
            p.absentIsUnchanged = true;
        } else if (scope.depth == 1) {
            const std::string& parent = scope.parentTag;
            if (parent == "models" && tag == "model") {
                p.mode = Mode::Fields;
                p.log = logs.models;
            } else if (parent == "modelGroups" && tag == "modelGroup") {
                p.mode = Mode::Fields;
                p.listAttr = "models";
                p.log = logs.models;
            } else if (parent == "view_objects" && tag == "view_object") {
                p.mode = Mode::Fields;
                p.log = logs.viewObjects;
            } else if (parent == "modelSets" && tag == "modelSet") {
                p.mode = Mode::Fields;
                p.listAttr = "models";
            } else if (parent == "palettes") {
                p.key = KeyRule::Content;
            } else if (parent == "settings" || parent == "colors") {
                p.key = KeyRule::Tag;
            }
        }
        break;
    case FileKind::Networks:
        if (scope.depth == 0 && tag == "Controller") {
            p.mode = Mode::Fields;
            p.log = logs.controllers;
        }
        break;
    case FileKind::Presets:
        if (tag == "group") {
            p.mode = Mode::Fields;
        }
        break;
    }
    return p;
}

// A whole-replaced item inside a record (a submodel of a model, the output
// list of a controller) conflicts as a "child"; one in a plain list (a view,
// a preset) as a record.
bool InRecord(FileKind kind, int depth) {
    switch (kind) {
    case FileKind::RgbEffects: return depth >= 2;
    case FileKind::Networks: return depth >= 1;
    case FileKind::Presets: return false;
    }
    return false;
}

// The section a list's children belong to, for conflict reports.
std::string SectionOf(FileKind kind, const Scope& scope, std::string_view tag) {
    if (kind == FileKind::Presets) {
        return "presets";
    }
    return scope.depth == 0 ? std::string(tag) : scope.section;
}

// Values recalculated after load (start channels, an auto-sized output's
// channel count, the saving computer's name). Both sides changing one is not a
// conflict: theirs is kept and a note is reported.
bool IsDerived(FileKind kind, std::string_view element, std::string_view attr) {
    switch (kind) {
    case FileKind::RgbEffects: return element == "model" && attr == "StartChannel";
    case FileKind::Networks: return (element == "Networks" && attr == "computer") || (element == "network" && attr == "MaxChannels");
    case FileKind::Presets: return false;
    }
    return false;
}

// When each version last took updates from the base show folder.
constexpr const char* kRgbSyncStamp = "BaseRgbEffectsSyncedTime";      // a <settings> entry
constexpr const char* kNetworksSyncStamp = "BaseControllersSyncedTime"; // a <Networks> attribute

bool IsBaseSyncStamp(FileKind kind, std::string_view element, std::string_view attr) {
    return kind == FileKind::Networks && element == "Networks" && attr == kNetworksSyncStamp;
}

long long SyncStamp(FileKind kind, pugi::xml_node root) {
    const char* v = kind == FileKind::Networks ? root.attribute(kNetworksSyncStamp).value()
                                               : root.child("settings").child(kRgbSyncStamp).attribute("value").value();
    return std::strtoll(v, nullptr, 10);
}

bool IsBlank(const char* s) {
    for (; *s; ++s) {
        if (*s != ' ' && *s != '\t' && *s != '\n' && *s != '\r') {
            return false;
        }
    }
    return true;
}

bool LooksNumeric(std::string_view s) {
    if (s.empty()) {
        return false;
    }
    bool digit = false;
    for (char c : s) {
        if (c >= '0' && c <= '9') {
            digit = true;
        } else if (c != '-' && c != '+' && c != '.' && c != 'e' && c != 'E') {
            return false;
        }
    }
    return digit;
}

// Equal, or the same number written differently ("6.0" / "6.000000"): the
// writers format floats differently and that is not an edit.
bool ValuesEqual(std::string_view a, std::string_view b) {
    if (a == b) {
        return true;
    }
    if (!LooksNumeric(a) || !LooksNumeric(b)) {
        return false;
    }
    auto integer = [](std::string_view s) { return s.find_first_of(".eE") == std::string_view::npos; };
    std::string sa(a), sb(b);
    if (integer(a) && integer(b)) {
        // channel numbers run into the millions: compare integers exactly
        char* ea = nullptr;
        char* eb = nullptr;
        long long ia = std::strtoll(sa.c_str(), &ea, 10);
        long long ib = std::strtoll(sb.c_str(), &eb, 10);
        return *ea == 0 && *eb == 0 && ia == ib;
    }
    char* ea = nullptr;
    char* eb = nullptr;
    double da = std::strtod(sa.c_str(), &ea);
    double db = std::strtod(sb.c_str(), &eb);
    if (*ea != 0 || *eb != 0) {
        return false;
    }
    double scale = std::max({ 1.0, std::fabs(da), std::fabs(db) });
    return std::fabs(da - db) <= 1e-6 * scale;
}

// ValuesEqual, plus positions written to 4 decimals by older versions.
bool AttrValuesEqual(std::string_view attr, std::string_view a, std::string_view b) {
    return ValuesEqual(a, b) || (LayoutRecords::IsPositionAttribute(attr) && LooksNumeric(a) && LooksNumeric(b) && LayoutRecords::PositionsEqual(a, b));
}

bool OptAttrValuesEqual(std::string_view attr, const OptStr& a, const OptStr& b) {
    if (!a || !b) {
        return !a && !b;
    }
    return AttrValuesEqual(attr, *a, *b);
}

bool IsFromBase(pugi::xml_node n) {
    return std::string_view(n.attribute("FromBase").value()) == "1";
}

OptStr AttrValue(pugi::xml_node n, const char* name) {
    if (!n) {
        return std::nullopt;
    }
    pugi::xml_attribute a = n.attribute(name);
    if (!a) {
        return std::nullopt;
    }
    return std::string(a.value());
}

bool Significant(pugi::xml_node n) {
    if (n.type() == pugi::node_element) {
        return true;
    }
    return (n.type() == pugi::node_pcdata || n.type() == pugi::node_cdata) && !IsBlank(n.value());
}

pugi::xml_node NextSignificant(pugi::xml_node n) {
    while (n && !Significant(n)) {
        n = n.next_sibling();
    }
    return n;
}

// Same content, ignoring layout ids, attribute order and whitespace between
// elements. With `ignoreDerived`, derived values (IsDerived) are skipped too.
bool NodesEqual(FileKind kind, pugi::xml_node a, pugi::xml_node b, bool ignoreDerived) {
    if (!a || !b) {
        return !a && !b;
    }
    if (a.type() != b.type()) {
        return false;
    }
    if (a.type() == pugi::node_pcdata || a.type() == pugi::node_cdata) {
        return std::string_view(a.value()) == b.value();
    }
    if (a.type() == pugi::node_element && std::strcmp(a.name(), b.name()) != 0) {
        return false;
    }
    auto skip = [&](pugi::xml_attribute at, pugi::xml_node owner) {
        return LayoutRecords::IsBookkeepingAttribute(at.name()) || (ignoreDerived && IsDerived(kind, owner.name(), at.name()));
    };
    size_t countA = 0;
    for (pugi::xml_attribute at = a.first_attribute(); at; at = at.next_attribute()) {
        if (skip(at, a)) {
            continue;
        }
        ++countA;
        pugi::xml_attribute other = b.attribute(at.name());
        if (!other || !AttrValuesEqual(at.name(), at.value(), other.value())) {
            return false;
        }
    }
    size_t countB = 0;
    for (pugi::xml_attribute at = b.first_attribute(); at; at = at.next_attribute()) {
        if (!skip(at, b)) {
            ++countB;
        }
    }
    if (countA != countB) {
        return false;
    }
    pugi::xml_node ca = NextSignificant(a.first_child());
    pugi::xml_node cb = NextSignificant(b.first_child());
    while (ca && cb) {
        if (!NodesEqual(kind, ca, cb, ignoreDerived)) {
            return false;
        }
        ca = NextSignificant(ca.next_sibling());
        cb = NextSignificant(cb.next_sibling());
    }
    return !ca && !cb;
}

// A canonical text for a node (sorted attributes, no ids, no whitespace),
// used to key records by content.
void Canon(pugi::xml_node n, std::string& out) {
    if (n.type() == pugi::node_pcdata || n.type() == pugi::node_cdata) {
        out += n.value();
        return;
    }
    out += '<';
    out += n.name();
    std::vector<std::pair<std::string, std::string>> attrs;
    for (pugi::xml_attribute at = n.first_attribute(); at; at = at.next_attribute()) {
        if (!LayoutRecords::IsBookkeepingAttribute(at.name())) {
            attrs.emplace_back(at.name(), at.value());
        }
    }
    std::sort(attrs.begin(), attrs.end());
    for (const auto& [k, v] : attrs) {
        out += ' ';
        out += k;
        out += "=\"";
        out += v;
        out += '"';
    }
    out += '>';
    for (pugi::xml_node c = NextSignificant(n.first_child()); c; c = NextSignificant(c.next_sibling())) {
        Canon(c, out);
    }
    out += "</>";
}

std::string Print(const std::vector<pugi::xml_node>& nodes) {
    std::ostringstream os;
    for (const auto& n : nodes) {
        n.print(os, "", pugi::format_raw);
    }
    return os.str();
}

// One record of a list as one version has it.
struct Item {
    std::string tag;
    std::string name;
    std::string id;
    std::string key;
    std::vector<pugi::xml_node> nodes;
    Policy policy;
};

std::vector<Item> Items(FileKind kind, pugi::xml_node parent, const Scope& scope, const Logs& logs) {
    std::vector<Item> items;
    if (!parent) {
        return items;
    }
    std::unordered_map<std::string, size_t> byKey;
    for (pugi::xml_node c = parent.first_child(); c; c = c.next_sibling()) {
        if (c.type() != pugi::node_element) {
            continue;
        }
        Item it;
        it.tag = c.name();
        it.policy = Classify(kind, scope, it.tag, logs);
        bool aggregate = false;
        switch (it.policy.key) {
        case KeyRule::Tag:
            it.name = it.tag;
            it.key = it.tag;
            aggregate = true;
            break;
        case KeyRule::Content: {
            std::string canon;
            Canon(c, canon);
            it.name = it.tag;
            it.key = it.tag + "=" + canon;
            aggregate = true;
            break;
        }
        case KeyRule::Name:
            it.name = LayoutRecords::NameOf(c);
            if (it.name.empty()) {
                it.key = it.tag;
                aggregate = true;
            } else {
                it.key = it.tag + "[" + it.name + "]";
                it.id = LayoutObjectId::Read(c);
            }
            break;
        }
        auto found = byKey.find(it.key);
        if (found != byKey.end()) {
            if (aggregate) {
                items[found->second].nodes.push_back(c);
                continue;
            }
            // two records with one name: pair them up by occurrence
            int n = 2;
            while (byKey.count(it.key + "#" + std::to_string(n))) {
                ++n;
            }
            it.key += "#" + std::to_string(n);
        }
        it.nodes.push_back(c);
        byKey[it.key] = items.size();
        items.push_back(std::move(it));
    }
    return items;
}

bool ItemsEqual(FileKind kind, const Item* a, const Item* b, bool ignoreDerived) {
    if (!a || !b) {
        return !a && !b;
    }
    if (a->nodes.size() != b->nodes.size()) {
        return false;
    }
    for (size_t i = 0; i < a->nodes.size(); ++i) {
        if (!NodesEqual(kind, a->nodes[i], b->nodes[i], ignoreDerived)) {
            return false;
        }
    }
    return true;
}

// The same record in the three versions (-1 where a version has none).
struct Triple {
    int b = -1;
    int m = -1;
    int t = -1;
};

struct Matching {
    std::vector<Triple> triples;
    std::vector<int> ofB, ofM, ofT; // item index -> triple index
};

std::string KeyWithName(const Item& it, const std::string& name) {
    return it.tag + "[" + name + "]";
}

// Pairs the records of the three versions: by id where both have one, by name
// where either has none (following this process's renames for mine), and
// mine's and theirs' additions by id, then by name.
Matching Match(const std::vector<Item>& B, const std::vector<Item>& M, const std::vector<Item>& T) {
    Matching mt;
    mt.ofB.assign(B.size(), -1);
    mt.ofM.assign(M.size(), -1);
    mt.ofT.assign(T.size(), -1);
    for (size_t i = 0; i < B.size(); ++i) {
        mt.ofB[i] = (int)mt.triples.size();
        mt.triples.push_back({ (int)i, -1, -1 });
    }

    auto link = [&](const std::vector<Item>& side, std::vector<int>& ofSide, int Triple::*member, bool useLog) {
        std::unordered_map<std::string, size_t> baseById;
        for (size_t i = 0; i < B.size(); ++i) {
            if (!B[i].id.empty()) {
                baseById.emplace(B[i].id, i);
            }
        }
        for (size_t j = 0; j < side.size(); ++j) {
            if (side[j].id.empty()) {
                continue;
            }
            auto f = baseById.find(side[j].id);
            if (f != baseById.end()) {
                Triple& tr = mt.triples[mt.ofB[f->second]];
                if (tr.*member < 0 && B[f->second].tag == side[j].tag) {
                    tr.*member = (int)j;
                    ofSide[j] = mt.ofB[f->second];
                }
            }
        }
        std::unordered_map<std::string, size_t> sideByKey;
        for (size_t j = 0; j < side.size(); ++j) {
            if (ofSide[j] < 0) {
                sideByKey.emplace(side[j].key, j);
            }
        }
        auto tryName = [&](size_t i, const std::string& key) {
            auto f = sideByKey.find(key);
            if (f == sideByKey.end() || ofSide[f->second] >= 0) {
                return false;
            }
            // two ids that differ are two records, whatever their names
            if (!B[i].id.empty() && !side[f->second].id.empty()) {
                return false;
            }
            Triple& tr = mt.triples[mt.ofB[i]];
            tr.*member = (int)f->second;
            ofSide[f->second] = mt.ofB[i];
            return true;
        };
        for (size_t i = 0; i < B.size(); ++i) {
            const Triple& tr = mt.triples[mt.ofB[i]];
            if (tr.*member >= 0) {
                continue;
            }
            const LayoutRenameLog* log = useLog ? B[i].policy.log : nullptr;
            if (log != nullptr && B[i].policy.key == KeyRule::Name && !B[i].name.empty()) {
                std::string now = log->CurrentName(B[i].name);
                if (now != B[i].name) {
                    // renamed here: the record of the old name in mine is a new one
                    tryName(i, KeyWithName(B[i], now));
                    continue;
                }
            }
            tryName(i, B[i].key);
        }
    };
    link(M, mt.ofM, &Triple::m, true);
    link(T, mt.ofT, &Triple::t, false);

    // records both added: by id, then by name
    std::unordered_map<std::string, size_t> theirsNewById, theirsNewByKey;
    for (size_t j = 0; j < T.size(); ++j) {
        if (mt.ofT[j] < 0) {
            if (!T[j].id.empty()) {
                theirsNewById.emplace(T[j].id, j);
            }
            theirsNewByKey.emplace(T[j].key, j);
        }
    }
    auto pairNew = [&](size_t i, size_t j) {
        mt.ofM[i] = (int)mt.triples.size();
        mt.ofT[j] = (int)mt.triples.size();
        mt.triples.push_back({ -1, (int)i, (int)j });
    };
    for (size_t i = 0; i < M.size(); ++i) {
        if (mt.ofM[i] >= 0 || M[i].id.empty()) {
            continue;
        }
        auto f = theirsNewById.find(M[i].id);
        if (f != theirsNewById.end() && mt.ofT[f->second] < 0 && T[f->second].tag == M[i].tag) {
            pairNew(i, f->second);
        }
    }
    for (size_t i = 0; i < M.size(); ++i) {
        if (mt.ofM[i] >= 0) {
            continue;
        }
        auto f = theirsNewByKey.find(M[i].key);
        if (f != theirsNewByKey.end() && mt.ofT[f->second] < 0) {
            pairNew(i, f->second);
        }
    }
    for (size_t i = 0; i < M.size(); ++i) {
        if (mt.ofM[i] < 0) {
            mt.ofM[i] = (int)mt.triples.size();
            mt.triples.push_back({ -1, (int)i, -1 });
        }
    }
    for (size_t j = 0; j < T.size(); ++j) {
        if (mt.ofT[j] < 0) {
            mt.ofT[j] = (int)mt.triples.size();
            mt.triples.push_back({ -1, -1, (int)j });
        }
    }
    return mt;
}

// The relative order of the entries all three versions keep.
std::vector<int> CommonOrder(const std::vector<int>& order, const std::vector<char>& common) {
    std::vector<int> out;
    for (int x : order) {
        if (common[x]) {
            out.push_back(x);
        }
    }
    return out;
}

struct Ctx {
    FileKind kind;
    Logs logs;
    const std::map<std::string, Resolution>* resolutions = nullptr;
    std::vector<Conflict> conflicts;
    std::vector<ReferenceIssue> notes;
    std::vector<RecordRename> renames;
    std::set<std::string> usedNames;
    // Mine's last base show folder sync is newer than theirs.
    bool mineNewerBase = false;
    // Options::twoWay: a record only one side has is a conflict.
    bool twoWay = false;

    Resolution Lookup(const std::string& key) const {
        auto f = resolutions->find(key);
        return f == resolutions->end() ? Resolution::Unresolved : f->second;
    }
    Resolution Add(Conflict c) {
        c.resolution = Lookup(c.key);
        Resolution r = c.resolution;
        conflicts.push_back(std::move(c));
        return r;
    }
};

struct RecordView {
    const Item* b = nullptr;
    const Item* m = nullptr;
    const Item* t = nullptr;

    const Item* Any() const { return t ? t : (m ? m : b); }
    std::string Id() const {
        for (const Item* i : { m, t, b }) {
            if (i && !i->id.empty()) {
                return i->id;
            }
        }
        return {};
    }
    // Stable across runs: the id, else the key the record had in base.
    std::string Ident() const {
        const Item* any = b ? b : (m ? m : t);
        std::string id = Id();
        return id.empty() ? any->key : any->tag + "{" + id + "}";
    }
};

Conflict MakeConflict(const RecordView& r, ConflictKind kind, const std::string& section, const std::string& key) {
    Conflict c;
    c.key = key;
    c.kind = kind;
    c.section = section;
    c.recordId = r.Id();
    if (r.b) c.baseName = r.b->name;
    if (r.m) c.mineName = r.m->name;
    if (r.t) c.theirsName = r.t->name;
    return c;
}

OptStr ItemText(const Item* i) {
    return i ? OptStr(Print(i->nodes)) : std::nullopt;
}

std::string UniqueName(Ctx& ctx, const std::string& name) {
    for (int n = 2;; ++n) {
        std::string candidate = name + "-" + std::to_string(n);
        if (ctx.usedNames.insert(candidate).second) {
            return candidate;
        }
    }
}

void MergeChildren(Ctx& ctx, pugi::xml_node out, pugi::xml_node b, pugi::xml_node m, pugi::xml_node t, const Scope& scope);

// Merges a member list attribute: adds and removes from both sides apply.
std::string MergeMemberList(const OptStr& vb, const OptStr& vm, const OptStr& vt) {
    return LayoutRecords::MergeMembers(vb ? &*vb : nullptr, vm ? &*vm : nullptr, vt ? &*vt : nullptr);
}

// Merges one record attribute by attribute, then its children.
void MergeFields(Ctx& ctx, pugi::xml_node out, const RecordView& r, const Policy& policy, const std::string& section,
                 const std::string& path, const Scope& childScope) {
    pugi::xml_node nb = r.b ? r.b->nodes.front() : pugi::xml_node();
    pugi::xml_node nm = r.m ? r.m->nodes.front() : pugi::xml_node();
    pugi::xml_node nt = r.t ? r.t->nodes.front() : pugi::xml_node();
    pugi::xml_node any = nt ? nt : nm;
    pugi::xml_node node = out.append_child(any.name());

    std::vector<std::string> names;
    std::set<std::string> seen;
    for (pugi::xml_node n : { nt, nm, nb }) {
        if (!n) continue;
        for (pugi::xml_attribute a = n.first_attribute(); a; a = a.next_attribute()) {
            if (!LayoutObjectId::IsIdAttribute(a.name()) && seen.insert(a.name()).second) {
                names.push_back(a.name());
            }
        }
    }
    const std::string nameAttr = LayoutRecords::NameAttributeOf(any);
    // Both versions took this record from the base show folder: what the base
    // owns follows the newer base sync rather than asking.
    const bool baseOwned = nm && nt && IsFromBase(nm) && IsFromBase(nt);
    for (const auto& attr : names) {
        OptStr vb = AttrValue(nb, attr.c_str());
        OptStr vm = AttrValue(nm, attr.c_str());
        OptStr vt = AttrValue(nt, attr.c_str());
        OptStr chosen;
        if (OptAttrValuesEqual(attr, vm, vb)) {
            chosen = vt;
        } else if (OptAttrValuesEqual(attr, vt, vb)) {
            chosen = vm;
        } else if (OptAttrValuesEqual(attr, vm, vt)) {
            chosen = vt;
        } else if (policy.listAttr != nullptr && attr == policy.listAttr) {
            chosen = MergeMemberList(vb, vm, vt);
        } else if (LayoutRecords::IsBookkeepingAttribute(attr)) {
            chosen = vm;
        } else if (IsBaseSyncStamp(ctx.kind, any.name(), attr)) {
            chosen = ctx.mineNewerBase ? vm : vt;
        } else if (baseOwned && !LayoutRecords::IsShowOwnedAttribute(any.name(), attr) && attr != nameAttr) {
            chosen = ctx.mineNewerBase ? vm : vt;
            ctx.notes.push_back({ section, r.Any()->name,
                                  attr + " comes from the base show folder and was updated on both sides; kept the version from the newer base sync" });
        } else if (IsDerived(ctx.kind, any.name(), attr)) {
            chosen = vt;
            ctx.notes.push_back({ section, r.Any()->name,
                                  attr + " was changed on both sides; kept '" + vt.value_or("") + "' (it is recalculated on load), not '" +
                                      vm.value_or("") + "'" });
        } else {
            bool isName = attr == nameAttr && policy.key == KeyRule::Name && r.b != nullptr;
            Conflict c = MakeConflict(r, isName ? ConflictKind::BothRenamed : ConflictKind::AttributeChanged, section, path + "@" + attr);
            c.detail = attr;
            c.baseValue = vb;
            c.mineValue = vm;
            c.theirsValue = vt;
            chosen = ctx.Add(std::move(c)) == Resolution::Mine ? vm : vt;
        }
        if (chosen) {
            node.append_attribute(attr.c_str()) = chosen->c_str();
        }
    }
    std::string id = r.Id();
    if (!id.empty()) {
        node.append_attribute(LayoutObjectId::ATTRIBUTE) = id.c_str();
    }
    MergeChildren(ctx, node, nb, nm, nt, childScope);
}

enum class Emit { Drop, Mine, Theirs, Merged, Both };

// Copies one side's version of a record. A version written without layout
// ids gets the record's id from the other versions.
void EmitCopy(pugi::xml_node out, const RecordView& r, const Item* side) {
    for (const auto& nd : side->nodes) {
        pugi::xml_node copy = out.append_copy(nd);
        if (side->policy.key == KeyRule::Name && !side->name.empty() && side->id.empty()) {
            std::string id = r.Id();
            if (!id.empty()) {
                LayoutObjectId::Write(copy, id);
            }
        }
    }
}

void MergeChildren(Ctx& ctx, pugi::xml_node out, pugi::xml_node b, pugi::xml_node m, pugi::xml_node t, const Scope& scope) {
    std::vector<Item> B = Items(ctx.kind, b, scope, ctx.logs);
    std::vector<Item> M = Items(ctx.kind, m, scope, ctx.logs);
    std::vector<Item> T = Items(ctx.kind, t, scope, ctx.logs);
    Matching mt = Match(B, M, T);
    const size_t n = mt.triples.size();
    const bool inRecord = InRecord(ctx.kind, scope.depth);

    std::vector<Emit> emit(n, Emit::Drop);
    std::vector<RecordView> views(n);
    for (size_t i = 0; i < n; ++i) {
        const Triple& tr = mt.triples[i];
        RecordView& r = views[i];
        r.b = tr.b >= 0 ? &B[tr.b] : nullptr;
        r.m = tr.m >= 0 ? &M[tr.m] : nullptr;
        r.t = tr.t >= 0 ? &T[tr.t] : nullptr;
        const Policy& policy = r.Any()->policy;
        const std::string section = SectionOf(ctx.kind, scope, r.Any()->tag);
        const std::string path = scope.path + "/" + r.Ident();
        auto conflict = [&](ConflictKind kind) {
            Conflict c = MakeConflict(r, kind, section, path);
            if (inRecord) {
                c.detail = r.Any()->key;
            }
            c.baseValue = ItemText(r.b);
            c.mineValue = ItemText(r.m);
            c.theirsValue = ItemText(r.t);
            c.keepBothAllowed = kind == ConflictKind::BothAdded && policy.key == KeyRule::Name && !r.m->name.empty();
            return ctx.Add(std::move(c));
        };
        auto bothChanged = [&]() {
            if ((scope.fromBase && !LayoutRecords::IsShowOwnedChild(scope.parentTag, r.Any()->tag)) ||
                (ctx.kind == FileKind::RgbEffects && scope.parentTag == "settings" && r.Any()->tag == kRgbSyncStamp)) {
                if (scope.fromBase) {
                    ctx.notes.push_back({ section, r.Any()->name, "comes from the base show folder and was updated on both sides; kept the version from the newer base sync" });
                }
                return ctx.mineNewerBase ? Emit::Mine : Emit::Theirs;
            }
            if (ItemsEqual(ctx.kind, r.m, r.t, true)) {
                ctx.notes.push_back({ section, r.Any()->name, "changed on both sides in values that are recalculated on load; kept theirs" });
                return Emit::Theirs;
            }
            return conflict(inRecord ? ConflictKind::ChildChanged : ConflictKind::RecordChanged) == Resolution::Mine ? Emit::Mine : Emit::Theirs;
        };

        if (!r.b) {
            if (r.m && r.t) {
                if (ItemsEqual(ctx.kind, r.m, r.t, false)) {
                    emit[i] = Emit::Theirs;
                } else if (policy.mode == Mode::Fields && policy.absentIsUnchanged) {
                    emit[i] = Emit::Merged;
                } else if (IsFromBase(r.m->nodes.front()) && IsFromBase(r.t->nodes.front())) {
                    // both took it from the base show folder
                    emit[i] = ctx.mineNewerBase ? Emit::Mine : Emit::Theirs;
                } else {
                    switch (conflict(ConflictKind::BothAdded)) {
                    case Resolution::Mine: emit[i] = Emit::Mine; break;
                    case Resolution::KeepBoth: emit[i] = Emit::Both; break;
                    default: emit[i] = Emit::Theirs; break;
                    }
                }
            } else if (ctx.twoWay && !policy.absentIsUnchanged) {
                // No base: the side without it may have deleted it or the other added it.
                if (r.m) {
                    emit[i] = conflict(ConflictKind::DeletedByTheirs) == Resolution::Mine ? Emit::Mine : Emit::Drop;
                } else {
                    emit[i] = conflict(ConflictKind::DeletedByMine) == Resolution::Mine ? Emit::Drop : Emit::Theirs;
                }
            } else {
                emit[i] = r.m ? Emit::Mine : Emit::Theirs;
            }
        } else if (!r.m && !r.t) {
            emit[i] = Emit::Drop;
        } else if (!r.m) {
            if (policy.absentIsUnchanged || ItemsEqual(ctx.kind, r.b, r.t, false)) {
                emit[i] = policy.absentIsUnchanged ? Emit::Theirs : Emit::Drop;
            } else {
                emit[i] = conflict(ConflictKind::DeletedByMine) == Resolution::Mine ? Emit::Drop : Emit::Theirs;
            }
        } else if (!r.t) {
            if (policy.absentIsUnchanged || ItemsEqual(ctx.kind, r.b, r.m, false)) {
                emit[i] = policy.absentIsUnchanged ? Emit::Mine : Emit::Drop;
            } else {
                emit[i] = conflict(ConflictKind::DeletedByTheirs) == Resolution::Mine ? Emit::Mine : Emit::Drop;
            }
        } else if (ItemsEqual(ctx.kind, r.b, r.m, false)) {
            emit[i] = Emit::Theirs;
        } else if (ItemsEqual(ctx.kind, r.b, r.t, false)) {
            emit[i] = Emit::Mine;
        } else if (ItemsEqual(ctx.kind, r.m, r.t, false)) {
            emit[i] = Emit::Theirs;
        } else if (policy.mode == Mode::Fields) {
            emit[i] = Emit::Merged;
        } else {
            emit[i] = bothChanged();
        }
    }

    // order
    std::vector<int> orderB, orderM, orderT;
    for (size_t i = 0; i < B.size(); ++i) orderB.push_back(mt.ofB[i]);
    for (size_t i = 0; i < M.size(); ++i) orderM.push_back(mt.ofM[i]);
    for (size_t i = 0; i < T.size(); ++i) orderT.push_back(mt.ofT[i]);
    std::vector<char> keep(n, 0), common(n, 0);
    for (size_t i = 0; i < n; ++i) {
        keep[i] = emit[i] != Emit::Drop;
        const Triple& tr = mt.triples[i];
        common[i] = keep[i] && tr.b >= 0 && tr.m >= 0 && tr.t >= 0;
    }
    std::vector<int> cb = CommonOrder(orderB, common), cm = CommonOrder(orderM, common), ct = CommonOrder(orderT, common);
    bool mineMoved = cm != cb;
    bool theirsMoved = ct != cb;
    bool preferMine = mineMoved && !theirsMoved;
    // The controller order drives start channels: two different reorders conflict.
    if (mineMoved && theirsMoved && cm != ct && ctx.kind == FileKind::Networks && scope.depth == 0) {
        auto names = [&](const std::vector<int>& order) {
            std::string s;
            for (int x : order) {
                if (!s.empty()) s += ", ";
                s += views[x].Any()->name;
            }
            return s;
        };
        Conflict c;
        c.key = scope.path + "#order";
        c.kind = ConflictKind::OrderChanged;
        c.section = "Controller";
        c.detail = "order";
        c.baseValue = names(cb);
        c.mineValue = names(cm);
        c.theirsValue = names(ct);
        preferMine = ctx.Add(std::move(c)) == Resolution::Mine;
    }
    std::vector<int> order = preferMine ? LayoutRecords::MergeOrder(orderM, orderT, orderB, keep) : LayoutRecords::MergeOrder(orderT, orderM, orderB, keep);

    for (int x : order) {
        const RecordView& r = views[x];
        switch (emit[x]) {
        case Emit::Drop:
            break;
        case Emit::Mine:
            EmitCopy(out, r, r.m);
            break;
        case Emit::Theirs:
            EmitCopy(out, r, r.t);
            break;
        case Emit::Both: {
            for (auto& nd : r.t->nodes) out.append_copy(nd);
            pugi::xml_node copy = out.append_copy(r.m->nodes.front());
            copy.attribute(LayoutRecords::NameAttributeOf(copy)).set_value(UniqueName(ctx, r.m->name).c_str());
            break;
        }
        case Emit::Merged: {
            const Item* any = r.Any();
            Scope child;
            child.depth = scope.depth + 1;
            child.parentTag = any->tag;
            child.section = SectionOf(ctx.kind, scope, any->tag);
            child.path = scope.path + "/" + r.Ident();
            child.fromBase = r.m && r.t && IsFromBase(r.m->nodes.front()) && IsFromBase(r.t->nodes.front());
            MergeFields(ctx, out, r, any->policy, child.section, child.path, child);
            break;
        }
        }
    }
}

// --- renames and references -------------------------------------------------

std::string MapName(const std::map<std::string, std::string>& map, const std::string& name) {
    auto f = map.find(name);
    return f == map.end() ? name : f->second;
}

// ">Model:1", "@Model:1", "<Model:1": a start channel chained to a model.
bool ChainedTarget(const std::string& sc, std::string& model, size_t& colon) {
    if (sc.size() < 2 || (sc[0] != '>' && sc[0] != '@' && sc[0] != '<')) {
        return false;
    }
    colon = sc.find(':', 1);
    if (colon == std::string::npos) {
        return false;
    }
    model = sc.substr(1, colon - 1);
    return true;
}

bool IsStartChannelAttr(std::string_view name) {
    if (name == "StartChannel") {
        return true;
    }
    if (name.size() > 6 && name.substr(0, 6) == "String") {
        for (char c : name.substr(6)) {
            if (c < '0' || c > '9') return false;
        }
        return true;
    }
    return false;
}

void SetIfChanged(pugi::xml_attribute a, const std::string& value) {
    if (value != a.value()) {
        a.set_value(value.c_str());
    }
}

std::string RewriteMemberList(const std::string& list, const std::map<std::string, std::string>& map) {
    std::vector<std::string> items = LayoutRecords::SplitMembers(list);
    bool changed = false;
    for (auto& s : items) {
        std::string mapped = MapName(map, s);
        if (mapped == s) {
            size_t slash = s.find('/');
            if (slash != std::string::npos) {
                std::string owner = MapName(map, s.substr(0, slash));
                if (owner != s.substr(0, slash)) {
                    mapped = owner + s.substr(slash);
                }
            }
        }
        if (mapped != s) {
            s = mapped;
            changed = true;
        }
    }
    return changed ? LayoutRecords::JoinMembers(items) : list;
}

// Points every reference to a model or group in `root` at its merged name.
void RewriteModelRefs(pugi::xml_node root, const std::map<std::string, std::string>& map) {
    if (map.empty() || !root) {
        return;
    }
    for (pugi::xml_node model : root.child("models").children("model")) {
        for (pugi::xml_attribute a = model.first_attribute(); a; a = a.next_attribute()) {
            if (IsStartChannelAttr(a.name())) {
                std::string sc = a.value(), target;
                size_t colon = 0;
                if (ChainedTarget(sc, target, colon)) {
                    std::string mapped = MapName(map, target);
                    if (mapped != target) {
                        SetIfChanged(a, sc.substr(0, 1) + mapped + sc.substr(colon));
                    }
                }
            }
        }
        if (pugi::xml_attribute a = model.attribute("ShadowModelFor")) {
            SetIfChanged(a, MapName(map, a.value()));
        }
        if (pugi::xml_attribute a = model.attribute("MotionParent")) {
            SetIfChanged(a, MapName(map, a.value()));
        }
        if (pugi::xml_attribute a = model.attribute("ModelChain")) {
            std::string chain = a.value();
            if (chain.size() > 1 && chain[0] == '>') {
                SetIfChanged(a, ">" + MapName(map, chain.substr(1)));
            }
        }
    }
    for (const auto& [section, record] : { std::pair{ "modelGroups", "modelGroup" }, std::pair{ "views", "view" }, std::pair{ "modelSets", "modelSet" } }) {
        for (pugi::xml_node n : root.child(section).children(record)) {
            if (pugi::xml_attribute a = n.attribute("models")) {
                SetIfChanged(a, RewriteMemberList(a.value(), map));
            }
        }
    }
}

// Points every reference to a controller in `root` (rgbeffects) at its merged name.
void RewriteControllerRefs(pugi::xml_node root, const std::map<std::string, std::string>& map) {
    if (map.empty() || !root) {
        return;
    }
    for (pugi::xml_node model : root.child("models").children("model")) {
        if (pugi::xml_attribute a = model.attribute("Controller")) {
            SetIfChanged(a, MapName(map, a.value()));
        }
        for (pugi::xml_attribute a = model.first_attribute(); a; a = a.next_attribute()) {
            std::string sc = a.value();
            if (IsStartChannelAttr(a.name()) && sc.size() > 1 && sc[0] == '!') {
                size_t colon = sc.find(':');
                std::string ctrl = sc.substr(1, colon == std::string::npos ? std::string::npos : colon - 1);
                std::string mapped = MapName(map, ctrl);
                if (mapped != ctrl) {
                    SetIfChanged(a, "!" + mapped + (colon == std::string::npos ? std::string() : sc.substr(colon)));
                }
            }
        }
    }
    for (pugi::xml_node obj : root.child("view_objects").children("view_object")) {
        if (pugi::xml_attribute a = obj.attribute("Controller")) {
            SetIfChanged(a, MapName(map, a.value()));
        }
    }
}

// Finds the records of one list renamed on either side, decides each one's
// merged name (a BothRenamed resolution picks it) and adds the renames to `maps`.
void DetectRenames(Ctx& ctx, pugi::xml_node b, pugi::xml_node m, pugi::xml_node t, const Scope& scope, NameMaps& maps) {
    std::vector<Item> B = Items(ctx.kind, b, scope, ctx.logs);
    std::vector<Item> M = Items(ctx.kind, m, scope, ctx.logs);
    std::vector<Item> T = Items(ctx.kind, t, scope, ctx.logs);
    Matching mt = Match(B, M, T);
    for (const Triple& tr : mt.triples) {
        RecordView r;
        r.b = tr.b >= 0 ? &B[tr.b] : nullptr;
        r.m = tr.m >= 0 ? &M[tr.m] : nullptr;
        r.t = tr.t >= 0 ? &T[tr.t] : nullptr;
        if (!r.b || r.b->policy.key != KeyRule::Name || r.b->name.empty() || r.b->policy.mode != Mode::Fields) {
            continue;
        }
        const std::string& nb = r.b->name;
        bool mineRenamed = r.m && r.m->name != nb;
        bool theirsRenamed = r.t && r.t->name != nb;
        if (!mineRenamed && !theirsRenamed) {
            continue;
        }
        std::string merged;
        if (mineRenamed && theirsRenamed && r.m->name != r.t->name) {
            std::string key = scope.path + "/" + r.Ident() + "@" + LayoutRecords::NameAttributeOf(r.b->nodes.front());
            merged = ctx.Lookup(key) == Resolution::Mine ? r.m->name : r.t->name;
        } else {
            merged = mineRenamed ? r.m->name : r.t->name;
        }
        maps.base[nb] = merged;
        if (r.m && r.m->name != merged) maps.mine[r.m->name] = merged;
        if (!r.m) maps.mine[nb] = merged;
        if (r.t && r.t->name != merged) maps.theirs[r.t->name] = merged;
        if (!r.t) maps.theirs[nb] = merged;
        ctx.renames.push_back({ SectionOf(ctx.kind, scope, r.b->tag), r.Id(), nb, r.m ? r.m->name : std::string(),
                                r.t ? r.t->name : std::string(), merged });
    }
}

void CollectNames(pugi::xml_node n, int depth, std::set<std::string>& names) {
    for (pugi::xml_node c = n.first_child(); c; c = c.next_sibling()) {
        if (c.type() != pugi::node_element) continue;
        std::string name = LayoutRecords::NameOf(c);
        if (!name.empty()) names.insert(name);
        if (depth > 0) CollectNames(c, depth - 1, names);
    }
}

// --- presets JSON <-> XML -------------------------------------------------

void PresetsToXml(const nlohmann::json& j, pugi::xml_node node) {
    for (auto it = j.begin(); it != j.end(); ++it) {
        if (it.key() == "children" || (it.key() == "type" && node.name() != std::string("item"))) {
            continue;
        }
        if (it.value().is_string()) {
            node.append_attribute(it.key().c_str()) = it.value().get<std::string>().c_str();
        } else {
            node.append_attribute(("json." + it.key()).c_str()) = it.value().dump().c_str();
        }
    }
    auto children = j.find("children");
    if (children != j.end() && children->is_array()) {
        for (const auto& c : *children) {
            if (!c.is_object()) continue;
            std::string type = c.value("type", "");
            const char* tag = (type == "group" || type == "effect") ? type.c_str() : "item";
            PresetsToXml(c, node.append_child(tag));
        }
    }
}

nlohmann::json PresetsFromXml(pugi::xml_node node, bool root) {
    nlohmann::json j = nlohmann::json::object();
    std::string tag = node.name();
    if (!root && (tag == "group" || tag == "effect")) {
        j["type"] = tag;
    }
    for (pugi::xml_attribute a = node.first_attribute(); a; a = a.next_attribute()) {
        std::string name = a.name();
        if (name.rfind("json.", 0) == 0) {
            j[name.substr(5)] = nlohmann::json::parse(a.value(), nullptr, false);
        } else {
            j[name] = a.value();
        }
    }
    if (root || tag == "group") {
        nlohmann::json children = nlohmann::json::array();
        for (pugi::xml_node c : node.children()) {
            if (c.type() == pugi::node_element) {
                children.push_back(PresetsFromXml(c, false));
            }
        }
        j["children"] = children;
    }
    return j;
}

bool LoadPresets(const std::string& text, pugi::xml_document& doc, std::string& error) {
    doc.reset();
    if (text.empty()) {
        return true;
    }
    nlohmann::json j = nlohmann::json::parse(text, nullptr, false);
    if (j.is_discarded() || !j.is_object()) {
        error = "not a presets file";
        return false;
    }
    PresetsToXml(j, doc.append_child("presets"));
    return true;
}

bool LoadXml(const std::string& text, pugi::xml_document& doc, std::string& error) {
    doc.reset();
    if (text.empty()) {
        return true;
    }
    pugi::xml_parse_result r = doc.load_buffer(text.data(), text.size(), pugi::parse_default | pugi::parse_declaration);
    if (!r) {
        error = r.description();
        return false;
    }
    return true;
}

} // namespace

// --- Merge ------------------------------------------------------------------

struct Merge::Impl {
    FileKind kind = FileKind::RgbEffects;
    bool ok = true;
    std::string error;
    bool textInput = false;
    std::string mineText;
    std::string theirsText;
    pugi::xml_document base;
    pugi::xml_document mine;
    pugi::xml_document theirs;
    LayoutRenameLog modelLog;
    LayoutRenameLog viewObjectLog;
    LayoutRenameLog controllerLog;
    Logs logs;
    NameMaps controllerMaps; // from the options (rgbeffects)
    bool twoWay = false;
    bool theirsUnchanged = false;
    bool mineUnchanged = false;
    std::map<std::string, Resolution> resolutions;

    mutable bool dirty = true;
    mutable pugi::xml_document result;
    mutable std::vector<Conflict> conflicts;
    mutable std::vector<RecordRename> renames;
    mutable std::vector<ReferenceIssue> notes;
    mutable NameMaps detectedControllerMaps;

    void Init(const Options& options) {
        auto copyLog = [](const LayoutRenameLog* from, LayoutRenameLog& to) -> const LayoutRenameLog* {
            if (from == nullptr) return nullptr;
            for (const auto& [o, n] : from->Entries()) to.Record(o, n);
            return &to;
        };
        logs.models = copyLog(options.modelRenames, modelLog);
        logs.viewObjects = copyLog(options.viewObjectRenames, viewObjectLog);
        logs.controllers = copyLog(options.controllerRenames, controllerLog);
        controllerMaps = options.controllerNameMaps;
        twoWay = options.twoWay;
        theirsUnchanged = NodesEqual(kind, base.document_element(), theirs.document_element(), false);
        mineUnchanged = !theirsUnchanged && NodesEqual(kind, base.document_element(), mine.document_element(), false);
    }

    void Compute() const {
        if (!dirty) return;
        dirty = false;
        result.reset();
        conflicts.clear();
        renames.clear();
        notes.clear();
        detectedControllerMaps = NameMaps();

        Ctx ctx;
        ctx.kind = kind;
        ctx.logs = logs;
        ctx.resolutions = &resolutions;
        ctx.mineNewerBase = SyncStamp(kind, mine.document_element()) > SyncStamp(kind, theirs.document_element());
        ctx.twoWay = twoWay;

        pugi::xml_document b, m, t;
        b.reset(base);
        m.reset(mine);
        t.reset(theirs);
        pugi::xml_node rb = b.document_element(), rm = m.document_element(), rt = t.document_element();

        // The rename hints are a set, merged on their own: never a conflict.
        std::vector<LayoutRenameHints::Entry> hints;
        if (kind == FileKind::RgbEffects) {
            // A side without the section (written by a version that drops
            // it) did not change it.
            auto read = [&rb](pugi::xml_node root) {
                pugi::xml_node n = root.child(LayoutRenameHints::SECTION);
                return LayoutRenameHints::Read(n ? n : rb.child(LayoutRenameHints::SECTION));
            };
            hints = LayoutRenameHints::Merge(read(rb), read(rm), read(rt));
            for (pugi::xml_node root : { rb, rm, rt }) {
                while (pugi::xml_node n = root.child(LayoutRenameHints::SECTION)) {
                    root.remove_child(n);
                }
            }
        }

        Scope root;
        root.depth = 0;
        root.parentTag = (rt ? rt : rm).name();
        if (kind == FileKind::RgbEffects) {
            NameMaps modelMaps, objectMaps;
            for (const char* section : { "models", "modelGroups" }) {
                Scope s;
                s.depth = 1;
                s.parentTag = section;
                s.section = section;
                s.path = std::string("/") + section;
                DetectRenames(ctx, rb.child(section), rm.child(section), rt.child(section), s, modelMaps);
            }
            Scope s;
            s.depth = 1;
            s.parentTag = "view_objects";
            s.section = "view_objects";
            s.path = "/view_objects";
            DetectRenames(ctx, rb.child("view_objects"), rm.child("view_objects"), rt.child("view_objects"), s, objectMaps);
            RewriteModelRefs(rb, modelMaps.base);
            RewriteModelRefs(rm, modelMaps.mine);
            RewriteModelRefs(rt, modelMaps.theirs);
            RewriteControllerRefs(rb, controllerMaps.base);
            RewriteControllerRefs(rm, controllerMaps.mine);
            RewriteControllerRefs(rt, controllerMaps.theirs);
        } else if (kind == FileKind::Networks) {
            DetectRenames(ctx, rb, rm, rt, root, detectedControllerMaps);
        }
        renames = ctx.renames;

        if (theirsUnchanged) {
            result.reset(mine);
            return;
        }
        if (mineUnchanged) {
            result.reset(theirs);
            return;
        }

        for (const pugi::xml_document* d : { &b, &m, &t }) {
            CollectNames(d->document_element(), 1, ctx.usedNames);
        }
        for (pugi::xml_node d = (rt ? t : m).first_child(); d; d = d.next_sibling()) {
            if (d.type() == pugi::node_declaration) {
                result.append_copy(d);
                break;
            }
        }
        if (!rm && !rt) {
            return;
        }
        RecordView rootView;
        Item ib, im, it;
        auto fill = [&](Item& i, pugi::xml_node n) {
            i.tag = n.name();
            i.key = i.tag;
            i.policy.mode = Mode::Fields;
            i.policy.key = KeyRule::Tag;
            i.nodes.push_back(n);
        };
        if (rb) { fill(ib, rb); rootView.b = &ib; }
        if (rm) { fill(im, rm); rootView.m = &im; }
        if (rt) { fill(it, rt); rootView.t = &it; }
        MergeFields(ctx, result, rootView, rootView.Any()->policy, root.parentTag, "", root);
        if (!hints.empty()) {
            LayoutRenameHints merged;
            for (const auto& e : hints) {
                merged.Add(e);
            }
            merged.WriteTo(result.document_element());
        }
        conflicts = std::move(ctx.conflicts);
        notes = std::move(ctx.notes);
    }
};

Merge::Merge(std::unique_ptr<Impl> impl) :
    _impl(std::move(impl)) {
}
Merge::Merge(Merge&&) noexcept = default;
Merge& Merge::operator=(Merge&&) noexcept = default;
Merge::~Merge() = default;

Merge Merge::Run(FileKind kind, const std::string& base, const std::string& mine, const std::string& theirs, const Options& options) {
    auto impl = std::make_unique<Impl>();
    impl->kind = kind;
    impl->textInput = true;
    impl->mineText = mine;
    impl->theirsText = theirs;
    const std::pair<const std::string*, pugi::xml_document*> inputs[] = { { &base, &impl->base }, { &mine, &impl->mine }, { &theirs, &impl->theirs } };
    const char* labels[] = { "base", "mine", "theirs" };
    for (int i = 0; i < 3; ++i) {
        std::string error;
        bool loaded = kind == FileKind::Presets ? LoadPresets(*inputs[i].first, *inputs[i].second, error)
                                                : LoadXml(*inputs[i].first, *inputs[i].second, error);
        if (!loaded) {
            impl->ok = false;
            impl->error = std::string(labels[i]) + ": " + error;
            return Merge(std::move(impl));
        }
    }
    impl->Init(options);
    impl->Compute();
    return Merge(std::move(impl));
}

Merge Merge::Run(FileKind kind, const pugi::xml_document& base, const pugi::xml_document& mine, const pugi::xml_document& theirs,
                 const Options& options) {
    auto impl = std::make_unique<Impl>();
    impl->kind = kind;
    impl->base.reset(base);
    impl->mine.reset(mine);
    impl->theirs.reset(theirs);
    if (kind == FileKind::Presets) {
        impl->ok = false;
        impl->error = "presets are merged from their JSON text";
        return Merge(std::move(impl));
    }
    impl->Init(options);
    impl->Compute();
    return Merge(std::move(impl));
}

bool Merge::Ok() const { return _impl->ok; }
const std::string& Merge::Error() const { return _impl->error; }
FileKind Merge::Kind() const { return _impl->kind; }
bool Merge::TheirsUnchanged() const { return _impl->ok && _impl->theirsUnchanged; }
bool Merge::MineUnchanged() const { return _impl->ok && _impl->mineUnchanged; }

const std::vector<Conflict>& Merge::Conflicts() const {
    _impl->Compute();
    return _impl->conflicts;
}

bool Merge::AllResolved() const {
    for (const auto& c : Conflicts()) {
        if (c.resolution == Resolution::Unresolved) {
            return false;
        }
    }
    return true;
}

bool Merge::Resolve(const std::string& key, Resolution resolution) {
    for (const auto& c : Conflicts()) {
        if (c.key == key) {
            if ((resolution == Resolution::KeepBoth && !c.keepBothAllowed) || resolution == Resolution::Map) {
                return false;
            }
            _impl->resolutions[key] = resolution;
            _impl->dirty = true;
            return true;
        }
    }
    return false;
}

bool Merge::Resolve(size_t index, Resolution resolution) {
    const auto& list = Conflicts();
    if (index >= list.size()) {
        return false;
    }
    std::string key = list[index].key;
    return Resolve(key, resolution);
}

void Merge::ResolveAll(Resolution resolution) {
    if (resolution == Resolution::KeepBoth || resolution == Resolution::Map) {
        return;
    }
    // A resolution can change what merges next (a rename), so repeat until
    // every conflict has one.
    for (int pass = 0; pass < 8; ++pass) {
        bool changed = false;
        for (const auto& c : Conflicts()) {
            auto f = _impl->resolutions.find(c.key);
            if (f == _impl->resolutions.end() || f->second != resolution) {
                _impl->resolutions[c.key] = resolution;
                changed = true;
            }
        }
        if (!changed) {
            break;
        }
        _impl->dirty = true;
    }
}

std::string Merge::ResultText() const {
    if (!_impl->ok) {
        return {};
    }
    if (_impl->textInput && _impl->theirsUnchanged) {
        return _impl->mineText;
    }
    if (_impl->textInput && _impl->mineUnchanged) {
        return _impl->theirsText;
    }
    _impl->Compute();
    if (_impl->kind == FileKind::Presets) {
        pugi::xml_node root = _impl->result.document_element();
        if (!root) {
            return {};
        }
        return PresetsFromXml(root, true).dump(2);
    }
    std::ostringstream os;
    _impl->result.save(os, "  ", pugi::format_default, pugi::encoding_utf8);
    return os.str();
}

const pugi::xml_document& Merge::ResultXml() const {
    _impl->Compute();
    return _impl->result;
}

const std::vector<RecordRename>& Merge::Renames() const {
    _impl->Compute();
    return _impl->renames;
}

NameMaps Merge::ControllerNameMaps() const {
    _impl->Compute();
    return _impl->detectedControllerMaps;
}

std::vector<ReferenceIssue> Merge::ReferenceIssues() const {
    _impl->Compute();
    std::vector<ReferenceIssue> issues = _impl->notes;
    if (_impl->kind == FileKind::RgbEffects) {
        auto more = CheckReferences(_impl->result, nullptr);
        issues.insert(issues.end(), more.begin(), more.end());
    } else if (_impl->kind == FileKind::Networks) {
        pugi::xml_document empty;
        auto more = CheckReferences(empty, &_impl->result);
        issues.insert(issues.end(), more.begin(), more.end());
    }
    return issues;
}

bool RecordsEqual(FileKind kind, pugi::xml_node a, pugi::xml_node b) {
    return NodesEqual(kind, a, b, false);
}

std::vector<ReferenceIssue> CheckReferences(const pugi::xml_document& rgbEffects, const pugi::xml_document* networks) {
    std::vector<ReferenceIssue> issues;
    pugi::xml_node root = rgbEffects.document_element();

    std::map<std::string, int> names;
    std::set<std::string> submodels;
    for (pugi::xml_node model : root.child("models").children("model")) {
        std::string name = model.attribute("name").value();
        ++names[name];
        for (pugi::xml_node sm : model.children("subModel")) {
            submodels.insert(name + "/" + sm.attribute("name").value());
        }
    }
    for (pugi::xml_node group : root.child("modelGroups").children("modelGroup")) {
        ++names[group.attribute("name").value()];
    }
    for (const auto& [name, count] : names) {
        if (count > 1) {
            issues.push_back({ "models", name, "more than one model or group is called '" + name + "'" });
        }
    }

    std::set<std::string> controllers;
    if (networks != nullptr) {
        std::map<std::string, int> ctrlNames, ctrlIds;
        for (pugi::xml_node c : networks->document_element().children("Controller")) {
            std::string name = c.attribute("Name").value();
            controllers.insert(name);
            ++ctrlNames[name];
            std::string id = c.attribute("Id").value();
            if (!id.empty()) ++ctrlIds[id];
        }
        for (const auto& [name, count] : ctrlNames) {
            if (count > 1) issues.push_back({ "Controller", name, "more than one controller is called '" + name + "'" });
        }
        for (const auto& [id, count] : ctrlIds) {
            if (count > 1) issues.push_back({ "Controller", id, "more than one controller has id " + id });
        }
    }
    auto knownController = [&](const std::string& c) {
        return networks == nullptr || c.empty() || c == "No Controller" || c == "Use Start Channel" || controllers.count(c) != 0;
    };

    for (pugi::xml_node model : root.child("models").children("model")) {
        std::string name = model.attribute("name").value();
        for (pugi::xml_attribute a = model.first_attribute(); a; a = a.next_attribute()) {
            if (!IsStartChannelAttr(a.name())) continue;
            std::string sc = a.value(), target;
            size_t colon = 0;
            if (ChainedTarget(sc, target, colon) && target != name && !names.count(target)) {
                issues.push_back({ "models", name, "start channel '" + sc + "' refers to missing model '" + target + "'" });
            } else if (sc.size() > 1 && sc[0] == '!') {
                std::string ctrl = sc.substr(1, sc.find(':') == std::string::npos ? std::string::npos : sc.find(':') - 1);
                if (!knownController(ctrl)) {
                    issues.push_back({ "models", name, "start channel '" + sc + "' refers to missing controller '" + ctrl + "'" });
                }
            }
        }
        std::string shadow = model.attribute("ShadowModelFor").value();
        if (!shadow.empty() && !names.count(shadow)) {
            issues.push_back({ "models", name, "shadows missing model '" + shadow + "'" });
        }
        std::string chain = model.attribute("ModelChain").value();
        if (chain.size() > 1 && chain[0] == '>' && !names.count(chain.substr(1))) {
            issues.push_back({ "models", name, "is chained to missing model '" + chain.substr(1) + "'" });
        }
        std::string ctrl = model.attribute("Controller").value();
        if (!knownController(ctrl)) {
            issues.push_back({ "models", name, "is on missing controller '" + ctrl + "'" });
        }
    }
    for (pugi::xml_node group : root.child("modelGroups").children("modelGroup")) {
        std::string name = group.attribute("name").value();
        for (const auto& member : LayoutRecords::SplitMembers(group.attribute("models").value())) {
            if (!names.count(member) && !submodels.count(member)) {
                issues.push_back({ "modelGroups", name, "member '" + member + "' does not exist" });
            }
        }
    }
    for (pugi::xml_node obj : root.child("view_objects").children("view_object")) {
        std::string ctrl = obj.attribute("Controller").value();
        if (obj.attribute("Controller") && !knownController(ctrl)) {
            issues.push_back({ "view_objects", obj.attribute("name").value(), "is bound to missing controller '" + ctrl + "'" });
        }
    }
    return issues;
}

} // namespace ShowFileMerge
