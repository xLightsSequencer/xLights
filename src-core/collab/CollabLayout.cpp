/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CollabLayout.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <sstream>
#include <string_view>

#include <pugixml.hpp>

#include "CollabMessages.h"
#include "utils/LayoutObjectId.h"
#include "utils/LayoutRecords.h"
#include "utils/ShowFileMerge.h"

using nlohmann::json;
using ShowFileMerge::FileKind;

namespace Collab {

namespace {

// --- the record view of a file ------------------------------------------------

// rgbeffects sections whose children are records (matched one by one); every
// other top-level element of a layout file is one whole item.
bool IsRecordSection(FileKind kind, std::string_view tag) {
    if (kind != FileKind::RgbEffects) {
        return false;
    }
    static const std::set<std::string_view> sections = { "models", "modelGroups", "view_objects", "modelSets",
                                                         "views",  "perspectives", "layoutGroups" };
    return sections.count(tag) != 0;
}

// Record order means something: controllers (start channels follow it) and the
// lists a user orders by hand.
bool OrderCounts(FileKind kind, std::string_view section) {
    return kind == FileKind::Networks || section == "views" || section == "perspectives" || section == "layoutGroups";
}

// Root attributes that never make two versions differ: ids, the saving
// version, and the networks file's saving computer.
bool IgnoredAttribute(FileKind kind, pugi::xml_node owner, std::string_view name) {
    if (LayoutRecords::IsBookkeepingAttribute(name)) {
        return true;
    }
    return kind == FileKind::Networks && std::string_view(owner.name()) == "Networks" && name == "computer";
}

struct Rec {
    pugi::xml_node node;
    std::string id;
    std::string tag;
    std::string name;
    std::string key;
};

// The record children of `container`: the elements of a record section, or
// a networks root's controllers.
std::vector<Rec> RecordsOf(FileKind kind, pugi::xml_node container) {
    std::vector<Rec> out;
    for (pugi::xml_node n = container.first_child(); n; n = n.next_sibling()) {
        if (n.type() != pugi::node_element) {
            continue;
        }
        if (kind == FileKind::Networks && std::string_view(n.name()) != "Controller") {
            continue;
        }
        Rec r;
        r.node = n;
        r.id = LayoutObjectId::Read(n);
        r.tag = n.name();
        r.name = LayoutRecords::NameOf(n);
        r.key = r.id.empty() ? "n:" + r.tag + ":" + r.name : "i:" + r.id;
        out.push_back(std::move(r));
    }
    return out;
}

bool UniqueKeys(const std::vector<Rec>& recs) {
    std::set<std::string> seen;
    for (const auto& r : recs) {
        if (!seen.insert(r.key).second) {
            return false;
        }
    }
    return true;
}

// For each record of `from`, the record of `to` it is (by id, else by name
// when one of them has no id); -1 when it has none.
std::vector<int> Match(const std::vector<Rec>& from, const std::vector<Rec>& to) {
    std::vector<int> match(from.size(), -1);
    std::vector<char> taken(to.size(), 0);
    for (size_t i = 0; i < from.size(); ++i) {
        if (from[i].id.empty()) {
            continue;
        }
        for (size_t j = 0; j < to.size(); ++j) {
            if (!taken[j] && to[j].id == from[i].id) {
                match[i] = (int)j;
                taken[j] = 1;
                break;
            }
        }
    }
    for (size_t i = 0; i < from.size(); ++i) {
        if (match[i] != -1) {
            continue;
        }
        for (size_t j = 0; j < to.size(); ++j) {
            if (taken[j] || to[j].tag != from[i].tag || to[j].name != from[i].name) {
                continue;
            }
            if (!from[i].id.empty() && !to[j].id.empty()) {
                continue; // two different records with one name
            }
            match[i] = (int)j;
            taken[j] = 1;
            break;
        }
    }
    return match;
}

// Whole items: top-level elements that are not record containers, keyed by
// tag and occurrence.
std::vector<std::pair<std::string, pugi::xml_node>> WholeItems(FileKind kind, pugi::xml_node root) {
    std::vector<std::pair<std::string, pugi::xml_node>> out;
    std::map<std::string, int> seen;
    for (pugi::xml_node n = root.first_child(); n; n = n.next_sibling()) {
        if (n.type() != pugi::node_element) {
            continue;
        }
        std::string tag = n.name();
        if (IsRecordSection(kind, tag) || (kind == FileKind::Networks && tag == "Controller")) {
            continue;
        }
        int idx = seen[tag]++;
        out.push_back({ tag + "#" + std::to_string(idx), n });
    }
    return out;
}

// Record sections present in `root`: tag -> node (the first of each tag).
std::map<std::string, pugi::xml_node> Sections(FileKind kind, pugi::xml_node root) {
    std::map<std::string, pugi::xml_node> out;
    for (pugi::xml_node n = root.first_child(); n; n = n.next_sibling()) {
        if (n.type() == pugi::node_element && IsRecordSection(kind, n.name()) && !out.count(n.name())) {
            out[n.name()] = n;
        }
    }
    return out;
}

std::string Xml(pugi::xml_node n) {
    std::ostringstream os;
    n.print(os, "", pugi::format_raw);
    return os.str();
}

// The attributes that count, as a map.
json AttrMap(FileKind kind, pugi::xml_node n) {
    json a = json::object();
    for (pugi::xml_attribute at = n.first_attribute(); at; at = at.next_attribute()) {
        if (!IgnoredAttribute(kind, n, at.name())) {
            a[at.name()] = at.value();
        }
    }
    return a;
}

bool AttrsEqual(FileKind kind, pugi::xml_node a, pugi::xml_node b) {
    pugi::xml_document da;
    pugi::xml_document db;
    pugi::xml_node ea = da.append_child(a.name());
    pugi::xml_node eb = db.append_child(b.name());
    for (pugi::xml_attribute at = a.first_attribute(); at; at = at.next_attribute()) {
        if (!IgnoredAttribute(kind, a, at.name())) {
            ea.append_attribute(at.name()) = at.value();
        }
    }
    for (pugi::xml_attribute at = b.first_attribute(); at; at = at.next_attribute()) {
        if (!IgnoredAttribute(kind, b, at.name())) {
            eb.append_attribute(at.name()) = at.value();
        }
    }
    return ShowFileMerge::RecordsEqual(kind, ea, eb);
}

bool Parse(const std::string& text, pugi::xml_document& doc) {
    if (text.empty()) {
        return false;
    }
    return doc.load_buffer(text.data(), text.size(), pugi::parse_default | pugi::parse_declaration) && doc.document_element();
}

// --- Diff -------------------------------------------------------------------

json DiffContainer(FileKind kind, const std::string& section, pugi::xml_node from, pugi::xml_node to, bool& wholeInstead) {
    std::vector<Rec> f = RecordsOf(kind, from);
    std::vector<Rec> t = RecordsOf(kind, to);
    if (!UniqueKeys(f) || !UniqueKeys(t)) {
        wholeInstead = true;
        return nullptr;
    }
    std::vector<int> match = Match(f, t);
    std::vector<char> matched(t.size(), 0);
    json add = json::array();
    json chg = json::array();
    json del = json::array();
    for (size_t i = 0; i < f.size(); ++i) {
        int j = match[i];
        if (j < 0) {
            del.push_back(json{ { "k", f[i].key }, { "n", f[i].name }, { "t", f[i].tag } });
            continue;
        }
        matched[j] = 1;
        if (!ShowFileMerge::RecordsEqual(kind, f[i].node, t[j].node)) {
            chg.push_back(json{ { "k", f[i].key }, { "x", Xml(t[j].node) } });
        }
    }
    for (size_t j = 0; j < t.size(); ++j) {
        if (!matched[j]) {
            add.push_back(json{ { "k", t[j].key }, { "x", Xml(t[j].node) } });
        }
    }
    json out = json::object();
    if (!section.empty()) {
        out["s"] = section;
    }
    if (!add.empty()) {
        out["add"] = std::move(add);
    }
    if (!chg.empty()) {
        out["chg"] = std::move(chg);
    }
    if (!del.empty()) {
        out["del"] = std::move(del);
    }
    if (!section.empty() && !AttrsEqual(kind, from, to)) {
        out["a"] = AttrMap(kind, to);
    }
    if (OrderCounts(kind, section)) {
        // What Apply would leave: kept records in their old order (with their
        // new keys), then the added ones.
        std::vector<std::string> after;
        for (size_t i = 0; i < f.size(); ++i) {
            if (match[i] >= 0) {
                after.push_back(t[match[i]].key);
            }
        }
        for (size_t j = 0; j < t.size(); ++j) {
            if (!matched[j]) {
                after.push_back(t[j].key);
            }
        }
        std::vector<std::string> want;
        for (const auto& r : t) {
            want.push_back(r.key);
        }
        if (after != want) {
            out["order"] = want;
        }
    }
    bool any = out.contains("add") || out.contains("chg") || out.contains("del") || out.contains("a") || out.contains("order");
    return any ? out : json(nullptr);
}

json DiffFile(FileKind kind, const std::string& fromText, const std::string& toText) {
    if (fromText == toText) {
        return nullptr;
    }
    pugi::xml_document from;
    pugi::xml_document to;
    bool okFrom = Parse(fromText, from);
    bool okTo = Parse(toText, to);
    if (!okFrom || !okTo || std::strcmp(from.document_element().name(), to.document_element().name()) != 0) {
        if (!okFrom && !okTo && fromText.empty() == toText.empty() && fromText.empty()) {
            return nullptr;
        }
        return json{ { "file", toText } };
    }
    pugi::xml_node rf = from.document_element();
    pugi::xml_node rt = to.document_element();
    json out = json::object();
    if (!AttrsEqual(kind, rf, rt)) {
        out["root"] = AttrMap(kind, rt);
    }
    json secs = json::array();
    json whole = json::array();
    auto wholeItem = [&](const std::string& key, pugi::xml_node a, pugi::xml_node b) {
        if (a && b && ShowFileMerge::RecordsEqual(kind, a, b)) {
            return;
        }
        if (!b) {
            whole.push_back(json{ { "k", key }, { "del", true } });
        } else {
            whole.push_back(json{ { "k", key }, { "x", Xml(b) } });
        }
    };
    if (kind == FileKind::Networks) {
        bool wholeInstead = false;
        json c = DiffContainer(kind, "", rf, rt, wholeInstead);
        if (wholeInstead) {
            return json{ { "file", toText } };
        }
        if (!c.is_null()) {
            secs.push_back(std::move(c));
        }
    } else {
        auto sf = Sections(kind, rf);
        auto st = Sections(kind, rt);
        std::set<std::string> tags;
        for (const auto& [t, n] : sf) {
            tags.insert(t);
        }
        for (const auto& [t, n] : st) {
            tags.insert(t);
        }
        for (const auto& tag : tags) {
            auto a = sf.find(tag);
            auto b = st.find(tag);
            if (b == st.end()) {
                secs.push_back(json{ { "s", tag }, { "drop", true } });
                continue;
            }
            if (a == sf.end()) {
                secs.push_back(json{ { "s", tag }, { "new", Xml(b->second) } });
                continue;
            }
            bool wholeInstead = false;
            json c = DiffContainer(kind, tag, a->second, b->second, wholeInstead);
            if (wholeInstead) {
                if (!ShowFileMerge::RecordsEqual(kind, a->second, b->second)) {
                    secs.push_back(json{ { "s", tag }, { "new", Xml(b->second) } });
                }
                continue;
            }
            if (!c.is_null()) {
                secs.push_back(std::move(c));
            }
        }
    }
    auto wf = WholeItems(kind, rf);
    auto wt = WholeItems(kind, rt);
    std::map<std::string, pugi::xml_node> mf(wf.begin(), wf.end());
    std::map<std::string, pugi::xml_node> mt(wt.begin(), wt.end());
    for (const auto& [k, n] : mf) {
        auto b = mt.find(k);
        wholeItem(k, n, b == mt.end() ? pugi::xml_node() : b->second);
    }
    for (const auto& [k, n] : mt) {
        if (!mf.count(k)) {
            wholeItem(k, pugi::xml_node(), n);
        }
    }
    if (!secs.empty()) {
        out["s"] = std::move(secs);
    }
    if (!whole.empty()) {
        out["w"] = std::move(whole);
    }
    return out.empty() ? json(nullptr) : out;
}

// --- Apply ------------------------------------------------------------------

bool IsStr(const json& j, const char* key) {
    auto it = j.find(key);
    return it != j.end() && it->is_string();
}

const std::string& Str(const json& j, const char* key) {
    return j.find(key)->get_ref<const std::string&>();
}

// Parses an element fragment; false when it is not exactly one element.
bool Fragment(const std::string& xml, pugi::xml_document& doc) {
    if (!doc.load_buffer(xml.data(), xml.size(), pugi::parse_default)) {
        return false;
    }
    int elements = 0;
    for (pugi::xml_node n = doc.first_child(); n; n = n.next_sibling()) {
        if (n.type() == pugi::node_element) {
            ++elements;
        }
    }
    return elements == 1;
}

bool SetAttrs(FileKind kind, pugi::xml_node n, const json& attrs) {
    if (!attrs.is_object()) {
        return false;
    }
    for (const auto& [k, v] : attrs.items()) {
        if (!v.is_string() || k.empty()) {
            return false;
        }
    }
    std::vector<std::string> remove;
    for (pugi::xml_attribute at = n.first_attribute(); at; at = at.next_attribute()) {
        if (!IgnoredAttribute(kind, n, at.name())) {
            remove.push_back(at.name());
        }
    }
    for (const auto& r : remove) {
        n.remove_attribute(r.c_str());
    }
    for (const auto& [k, v] : attrs.items()) {
        pugi::xml_attribute at = n.attribute(k.c_str());
        if (!at) {
            at = n.append_attribute(k.c_str());
        }
        at.set_value(v.get_ref<const std::string&>().c_str());
    }
    return true;
}

pugi::xml_node FindRec(FileKind kind, pugi::xml_node container, const std::string& key) {
    for (const auto& r : RecordsOf(kind, container)) {
        if (r.key == key) {
            return r.node;
        }
    }
    return {};
}

bool ApplyContainer(FileKind kind, pugi::xml_node container, const json& c) {
    std::vector<Rec> before = RecordsOf(kind, container);
    if (!UniqueKeys(before)) {
        return false;
    }
    if (auto it = c.find("del"); it != c.end()) {
        if (!it->is_array()) {
            return false;
        }
        for (const auto& d : *it) {
            if (!d.is_object() || !IsStr(d, "k")) {
                return false;
            }
            pugi::xml_node n = FindRec(kind, container, Str(d, "k"));
            if (!n) {
                return false;
            }
            container.remove_child(n);
        }
    }
    if (auto it = c.find("chg"); it != c.end()) {
        if (!it->is_array()) {
            return false;
        }
        for (const auto& ch : *it) {
            if (!ch.is_object() || !IsStr(ch, "k") || !IsStr(ch, "x")) {
                return false;
            }
            pugi::xml_node n = FindRec(kind, container, Str(ch, "k"));
            pugi::xml_document frag;
            if (!n || !Fragment(Str(ch, "x"), frag)) {
                return false;
            }
            pugi::xml_node placed = container.insert_copy_after(frag.document_element(), n);
            container.remove_child(n);
            if (!placed) {
                return false;
            }
        }
    }
    if (auto it = c.find("add"); it != c.end()) {
        if (!it->is_array()) {
            return false;
        }
        for (const auto& a : *it) {
            pugi::xml_document frag;
            if (!a.is_object() || !IsStr(a, "x") || !Fragment(Str(a, "x"), frag)) {
                return false;
            }
            if (kind == FileKind::Networks && std::string_view(frag.document_element().name()) != "Controller") {
                return false;
            }
            if (!container.append_copy(frag.document_element())) {
                return false;
            }
        }
    }
    if (!UniqueKeys(RecordsOf(kind, container))) {
        return false;
    }
    if (auto it = c.find("a"); it != c.end()) {
        if (!SetAttrs(kind, container, *it)) {
            return false;
        }
    }
    if (auto it = c.find("order"); it != c.end()) {
        if (!it->is_array()) {
            return false;
        }
        std::vector<Rec> now = RecordsOf(kind, container);
        if (it->size() != now.size()) {
            return false;
        }
        std::map<std::string, pugi::xml_node> byKey;
        for (const auto& r : now) {
            byKey[r.key] = r.node;
        }
        std::vector<pugi::xml_node> order;
        std::set<std::string> used;
        for (const auto& k : *it) {
            if (!k.is_string()) {
                return false;
            }
            auto f = byKey.find(k.get<std::string>());
            if (f == byKey.end() || !used.insert(f->first).second) {
                return false;
            }
            order.push_back(f->second);
        }
        // Records move to the end in the wanted order; other children stay.
        for (pugi::xml_node n : order) {
            container.append_move(n);
        }
    }
    return true;
}

std::optional<std::string> ApplyFile(FileKind kind, const std::string& text, const json& d) {
    if (d.is_null()) {
        return text;
    }
    if (!d.is_object()) {
        return std::nullopt;
    }
    if (auto f = d.find("file"); f != d.end()) {
        if (!f->is_string() || d.size() != 1) {
            return std::nullopt;
        }
        return f->get<std::string>();
    }
    pugi::xml_document doc;
    if (!Parse(text, doc)) {
        return std::nullopt;
    }
    pugi::xml_node root = doc.document_element();
    for (const auto& [k, v] : d.items()) {
        if (k != "root" && k != "s" && k != "w") {
            return std::nullopt;
        }
    }
    if (auto r = d.find("root"); r != d.end()) {
        if (!SetAttrs(kind, root, *r)) {
            return std::nullopt;
        }
    }
    if (auto s = d.find("s"); s != d.end()) {
        if (!s->is_array()) {
            return std::nullopt;
        }
        for (const auto& c : *s) {
            if (!c.is_object()) {
                return std::nullopt;
            }
            if (kind == FileKind::Networks) {
                if (c.contains("s") || !ApplyContainer(kind, root, c)) {
                    return std::nullopt;
                }
                continue;
            }
            if (!IsStr(c, "s") || !IsRecordSection(kind, Str(c, "s"))) {
                return std::nullopt;
            }
            const std::string& tag = Str(c, "s");
            pugi::xml_node sec = Sections(kind, root)[tag];
            if (auto drop = c.find("drop"); drop != c.end()) {
                if (!drop->is_boolean() || !sec) {
                    return std::nullopt;
                }
                root.remove_child(sec);
                continue;
            }
            if (c.contains("new")) {
                pugi::xml_document frag;
                if (!IsStr(c, "new") || !Fragment(Str(c, "new"), frag) || tag != frag.document_element().name()) {
                    return std::nullopt;
                }
                pugi::xml_node placed = sec ? root.insert_copy_after(frag.document_element(), sec) : root.append_copy(frag.document_element());
                if (sec) {
                    root.remove_child(sec);
                }
                if (!placed) {
                    return std::nullopt;
                }
                continue;
            }
            if (!sec || !ApplyContainer(kind, sec, c)) {
                return std::nullopt;
            }
        }
    }
    if (auto w = d.find("w"); w != d.end()) {
        if (!w->is_array()) {
            return std::nullopt;
        }
        for (const auto& item : *w) {
            if (!item.is_object() || !IsStr(item, "k")) {
                return std::nullopt;
            }
            const std::string& key = Str(item, "k");
            pugi::xml_node existing;
            for (const auto& [k, n] : WholeItems(kind, root)) {
                if (k == key) {
                    existing = n;
                }
            }
            if (item.contains("del")) {
                if (!existing) {
                    return std::nullopt;
                }
                root.remove_child(existing);
                continue;
            }
            pugi::xml_document frag;
            if (!IsStr(item, "x") || !Fragment(Str(item, "x"), frag)) {
                return std::nullopt;
            }
            std::string tag = frag.document_element().name();
            if (key.compare(0, tag.size() + 1, tag + "#") != 0 || IsRecordSection(kind, tag) || (kind == FileKind::Networks && tag == "Controller")) {
                return std::nullopt;
            }
            if (existing) {
                pugi::xml_node placed = root.insert_copy_after(frag.document_element(), existing);
                root.remove_child(existing);
                if (!placed) {
                    return std::nullopt;
                }
            } else if (!root.append_copy(frag.document_element())) {
                return std::nullopt;
            }
        }
    }
    std::ostringstream os;
    doc.save(os, "  ", pugi::format_default);
    return os.str();
}

// --- Hash -------------------------------------------------------------------

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

// Numbers written differently ("6.0" / "6.000000") hash the same.
std::string CanonValue(std::string_view attr, const char* v) {
    std::string_view s(v);
    if (!LooksNumeric(s)) {
        return std::string(s);
    }
    char* end = nullptr;
    if (s.find_first_of(".eE") == std::string_view::npos) {
        long long i = std::strtoll(v, &end, 10);
        if (end != nullptr && *end == 0) {
            return std::to_string(i);
        }
        return std::string(s);
    }
    double d = std::strtod(v, &end);
    if (end == nullptr || *end != 0) {
        return std::string(s);
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), LayoutRecords::IsPositionAttribute(attr) ? "%.4f" : "%.6g", d);
    std::string out = buf;
    if (out.find_first_not_of("-0.") == std::string::npos) {
        out = "0";
    }
    return out;
}

void Canon(FileKind kind, pugi::xml_node n, std::string& out) {
    if (n.type() == pugi::node_pcdata || n.type() == pugi::node_cdata) {
        const char* v = n.value();
        bool blank = true;
        for (const char* p = v; *p; ++p) {
            if (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') {
                blank = false;
                break;
            }
        }
        if (!blank) {
            out += '"';
            out += v;
            out += '"';
        }
        return;
    }
    if (n.type() != pugi::node_element) {
        return;
    }
    out += '<';
    out += n.name();
    std::vector<std::pair<std::string, std::string>> attrs;
    for (pugi::xml_attribute at = n.first_attribute(); at; at = at.next_attribute()) {
        if (!IgnoredAttribute(kind, n, at.name())) {
            attrs.push_back({ at.name(), CanonValue(at.name(), at.value()) });
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
    for (pugi::xml_node c = n.first_child(); c; c = c.next_sibling()) {
        Canon(kind, c, out);
    }
    out += "</>";
}

uint64_t Fnv(std::string_view s, uint64_t h = 1469598103934665603ull) {
    for (unsigned char c : s) {
        h ^= c;
        h *= 1099511628211ull;
    }
    return h;
}

std::string CanonString(FileKind kind, pugi::xml_node n) {
    std::string s;
    Canon(kind, n, s);
    return s;
}

uint64_t HashContainer(FileKind kind, const std::string& section, pugi::xml_node container) {
    std::vector<std::string> recs;
    for (const auto& r : RecordsOf(kind, container)) {
        recs.push_back(CanonString(kind, r.node));
    }
    if (!OrderCounts(kind, section)) {
        std::sort(recs.begin(), recs.end());
    }
    std::string head = "[" + section;
    if (!section.empty()) {
        json attrs = AttrMap(kind, container);
        for (const auto& [k, v] : attrs.items()) {
            head += " " + k + "=" + CanonValue(k, v.get_ref<const std::string&>().c_str());
        }
    }
    uint64_t h = Fnv(head);
    for (const auto& r : recs) {
        h = Fnv(r, Fnv("|", h));
    }
    return h;
}

uint64_t HashFile(FileKind kind, const std::string& text) {
    pugi::xml_document doc;
    if (!Parse(text, doc)) {
        return text.empty() ? 0 : Fnv(text, Fnv("raw"));
    }
    pugi::xml_node root = doc.document_element();
    std::vector<std::string> parts;
    std::string head = std::string("root ") + root.name();
    json rootAttrs = AttrMap(kind, root);
    for (const auto& [k, v] : rootAttrs.items()) {
        head += " " + k + "=" + CanonValue(k, v.get_ref<const std::string&>().c_str());
    }
    parts.push_back(head);
    if (kind == FileKind::Networks) {
        parts.push_back(std::to_string(HashContainer(kind, "", root)));
    } else {
        for (const auto& [tag, n] : Sections(kind, root)) {
            parts.push_back(tag + ":" + std::to_string(HashContainer(kind, tag, n)));
        }
    }
    for (const auto& [k, n] : WholeItems(kind, root)) {
        parts.push_back(k + ":" + CanonString(kind, n));
    }
    std::sort(parts.begin() + 1, parts.end());
    uint64_t h = 0;
    for (const auto& p : parts) {
        h = Fnv(p, Fnv("|", h == 0 ? 1469598103934665603ull : h));
    }
    return h;
}

// Every record's (section, name, id), in a fixed order.
std::vector<std::string> RecordIds(FileKind kind, pugi::xml_node root) {
    std::vector<std::string> out;
    auto add = [&](const std::string& section, pugi::xml_node container) {
        for (const auto& r : RecordsOf(kind, container)) {
            out.push_back(section + "/" + r.tag + "/" + r.name + "/" + r.id);
        }
    };
    if (kind == FileKind::Networks) {
        add("", root);
    } else {
        for (const auto& [tag, n] : Sections(kind, root)) {
            add(tag, n);
        }
    }
    std::sort(out.begin(), out.end());
    return out;
}

// --- Describe ---------------------------------------------------------------

std::string Label(const std::string& tag) {
    static const std::map<std::string, std::string> labels = {
        { "model", "Model" },           { "modelGroup", "Group" },          { "view_object", "Object" },
        { "modelSet", "Model set" },    { "view", "View" },                 { "perspective", "Perspective" },
        { "layoutGroup", "Preview" },   { "Controller", "Controller" },
    };
    auto it = labels.find(tag);
    return it == labels.end() ? tag : it->second;
}

std::string TagOfKey(const std::string& key) {
    // "n:<tag>:<name>"
    if (key.rfind("n:", 0) == 0) {
        size_t c = key.find(':', 2);
        if (c != std::string::npos) {
            return key.substr(2, c - 2);
        }
    }
    return {};
}

void DescribeFile(FileKind kind, const json& d, std::vector<std::string>& out) {
    if (!d.is_object()) {
        return;
    }
    auto push = [&](const std::string& s) {
        if (std::find(out.begin(), out.end(), s) == out.end()) {
            out.push_back(s);
        }
    };
    if (d.contains("file")) {
        push(kind == FileKind::Networks ? "Controllers" : "Layout");
        return;
    }
    if (d.contains("root")) {
        push(kind == FileKind::Networks ? "Controller settings" : "Layout settings");
    }
    if (auto s = d.find("s"); s != d.end() && s->is_array()) {
        for (const auto& c : *s) {
            if (!c.is_object()) {
                continue;
            }
            std::string sectionTag = IsStr(c, "s") ? Str(c, "s") : std::string();
            if (c.contains("drop") || c.contains("new") || c.contains("a") || c.contains("order")) {
                push(sectionTag.empty() ? std::string("Controller order") : sectionTag);
            }
            for (const char* part : { "add", "chg" }) {
                auto it = c.find(part);
                if (it == c.end() || !it->is_array()) {
                    continue;
                }
                for (const auto& r : *it) {
                    pugi::xml_document frag;
                    if (r.is_object() && IsStr(r, "x") && Fragment(Str(r, "x"), frag)) {
                        pugi::xml_node n = frag.document_element();
                        push(Label(n.name()) + " '" + LayoutRecords::NameOf(n) + "'");
                    }
                }
            }
            if (auto it = c.find("del"); it != c.end() && it->is_array()) {
                for (const auto& r : *it) {
                    if (r.is_object() && IsStr(r, "n")) {
                        std::string tag = IsStr(r, "t") ? Str(r, "t") : (IsStr(r, "k") ? TagOfKey(Str(r, "k")) : std::string());
                        if (tag.empty()) {
                            tag = kind == FileKind::Networks ? "Controller" : sectionTag;
                        }
                        push(Label(tag) + " '" + Str(r, "n") + "'");
                    }
                }
            }
        }
    }
    if (auto w = d.find("w"); w != d.end() && w->is_array()) {
        for (const auto& item : *w) {
            if (item.is_object() && IsStr(item, "k")) {
                const std::string& k = Str(item, "k");
                push(k.substr(0, k.find('#')));
            }
        }
    }
}

} // namespace

namespace LayoutDelta {

json Diff(const LayoutFiles& from, const LayoutFiles& to) {
    try {
        json rgb = DiffFile(FileKind::RgbEffects, from.rgbEffects, to.rgbEffects);
        json net = DiffFile(FileKind::Networks, from.networks, to.networks);
        if (rgb.is_null() && net.is_null()) {
            return nullptr;
        }
        json out = json::object();
        if (!rgb.is_null()) {
            out["rgb"] = std::move(rgb);
        }
        if (!net.is_null()) {
            out["net"] = std::move(net);
        }
        return out;
    } catch (...) {
        // Diff of two whole files always works; this only guards allocation failures.
        return json{ { "rgb", json{ { "file", to.rgbEffects } } }, { "net", json{ { "file", to.networks } } } };
    }
}

std::optional<LayoutFiles> Apply(const LayoutFiles& from, const json& delta) {
    try {
        if (delta.is_null()) {
            return from;
        }
        if (!delta.is_object() || delta.empty()) {
            return std::nullopt;
        }
        for (const auto& [k, v] : delta.items()) {
            if (k != "rgb" && k != "net") {
                return std::nullopt;
            }
        }
        LayoutFiles out = from;
        if (auto r = delta.find("rgb"); r != delta.end()) {
            auto t = ApplyFile(FileKind::RgbEffects, from.rgbEffects, *r);
            if (!t) {
                return std::nullopt;
            }
            out.rgbEffects = std::move(*t);
        }
        if (auto n = delta.find("net"); n != delta.end()) {
            auto t = ApplyFile(FileKind::Networks, from.networks, *n);
            if (!t) {
                return std::nullopt;
            }
            out.networks = std::move(*t);
        }
        return out;
    } catch (...) {
        return std::nullopt;
    }
}

std::vector<std::string> Describe(const json& delta) {
    std::vector<std::string> out;
    try {
        if (!delta.is_object()) {
            return out;
        }
        if (auto r = delta.find("rgb"); r != delta.end()) {
            DescribeFile(FileKind::RgbEffects, *r, out);
        }
        if (auto n = delta.find("net"); n != delta.end()) {
            DescribeFile(FileKind::Networks, *n, out);
        }
    } catch (...) {
    }
    return out;
}

uint64_t Hash(const LayoutFiles& files) {
    uint64_t a = HashFile(FileKind::RgbEffects, files.rgbEffects);
    uint64_t b = HashFile(FileKind::Networks, files.networks);
    uint64_t h = Fnv(std::to_string(a), Fnv("layout"));
    h = Fnv(std::to_string(b), Fnv("|", h));
    return h == 0 ? 1 : h;
}

bool Equivalent(const LayoutFiles& a, const LayoutFiles& b) {
    if (!Diff(a, b).is_null()) {
        return false;
    }
    try {
        for (FileKind kind : { FileKind::RgbEffects, FileKind::Networks }) {
            const std::string& ta = kind == FileKind::Networks ? a.networks : a.rgbEffects;
            const std::string& tb = kind == FileKind::Networks ? b.networks : b.rgbEffects;
            pugi::xml_document da;
            pugi::xml_document db;
            if (!Parse(ta, da) || !Parse(tb, db)) {
                continue; // equal text (Diff found nothing)
            }
            if (RecordIds(kind, da.document_element()) != RecordIds(kind, db.document_element())) {
                return false;
            }
        }
    } catch (...) {
        return false;
    }
    return true;
}

} // namespace LayoutDelta

namespace LayoutMerge {

namespace {

std::string RecordName(const ShowFileMerge::Conflict& c) {
    std::string n = !c.mineName.empty() ? c.mineName : (!c.baseName.empty() ? c.baseName : c.theirsName);
    return n.empty() ? c.section : n;
}

// Resolves every conflict of `m` by `policy`; false when the proposal is
// refused.
bool Resolve(ShowFileMerge::Merge& m, Policy policy, Outcome& out) {
    using ShowFileMerge::ConflictKind;
    using ShowFileMerge::Resolution;
    if (policy == Policy::Theirs) {
        m.ResolveAll(Resolution::Theirs);
        return true;
    }
    const auto conflicts = m.Conflicts();
    for (size_t i = 0; i < conflicts.size(); ++i) {
        const auto& c = conflicts[i];
        switch (c.kind) {
        case ConflictKind::BothAdded:
            out.reason = Reason::NameTaken;
            out.text = "'" + RecordName(c) + "' was added by someone else at the same time, so your change wasn't made";
            return false;
        case ConflictKind::DeletedByTheirs:
            m.Resolve(i, Resolution::Theirs);
            out.dropped.push_back(RecordName(c));
            break;
        default:
            m.Resolve(i, Resolution::Mine);
            break;
        }
    }
    return true;
}

} // namespace

Outcome Merge(const LayoutFiles& base, const LayoutFiles& mine, const LayoutFiles& theirs, Policy policy) {
    Outcome out;
    try {
        // Where a version has no file at all there is nothing to merge
        // record by record: the side the policy prefers stands.
        auto whole = [&](const std::string& b, const std::string& m, const std::string& t) {
            if (m == b) {
                return t;
            }
            if (t == b) {
                return m;
            }
            return policy == Policy::Theirs ? t : m;
        };
        auto mergeable = [](const std::string& b, const std::string& m, const std::string& t) {
            return !b.empty() && !m.empty() && !t.empty() && !(b == m && b == t);
        };

        ShowFileMerge::Options rgbOptions;
        if (mergeable(base.networks, mine.networks, theirs.networks)) {
            ShowFileMerge::Merge net = ShowFileMerge::Merge::Run(FileKind::Networks, base.networks, mine.networks, theirs.networks);
            if (!net.Ok()) {
                out.reason = Reason::Invalid;
                out.text = "the controllers could not be merged: " + net.Error();
                return out;
            }
            if (!Resolve(net, policy, out)) {
                return out;
            }
            out.result.networks = net.ResultText();
            rgbOptions.controllerNameMaps = net.ControllerNameMaps();
        } else {
            out.result.networks = whole(base.networks, mine.networks, theirs.networks);
        }

        if (mergeable(base.rgbEffects, mine.rgbEffects, theirs.rgbEffects)) {
            ShowFileMerge::Merge rgb = ShowFileMerge::Merge::Run(FileKind::RgbEffects, base.rgbEffects, mine.rgbEffects, theirs.rgbEffects, rgbOptions);
            if (!rgb.Ok()) {
                out.reason = Reason::Invalid;
                out.text = "the layout could not be merged: " + rgb.Error();
                return out;
            }
            if (!Resolve(rgb, policy, out)) {
                return out;
            }
            out.result.rgbEffects = rgb.ResultText();
        } else {
            out.result.rgbEffects = whole(base.rgbEffects, mine.rgbEffects, theirs.rgbEffects);
        }
        out.ok = true;
    } catch (...) {
        out.ok = false;
        out.reason = Reason::Invalid;
        out.text = "the layout could not be merged";
    }
    return out;
}

} // namespace LayoutMerge

} // namespace Collab
