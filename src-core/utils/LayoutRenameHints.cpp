/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LayoutRenameHints.h"

#include <algorithm>
#include <cstdlib>
#include <set>
#include <tuple>

#include "LayoutObjectId.h"
#include "../XmlSerializer/BaseSerializingVisitor.h"

namespace {

using Entry = LayoutRenameHints::Entry;

void SortByTime(std::vector<Entry>& entries) {
    std::stable_sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) { return a.time < b.time; });
}

// Adds e, or folds it into the same rename already in `into`.
void Fold(std::vector<Entry>& into, const Entry& e) {
    for (auto& x : into) {
        if (x.SameRename(e)) {
            x.time = std::max(x.time, e.time);
            if (x.id.empty()) {
                x.id = e.id;
            }
            return;
        }
    }
    into.push_back(e);
}

} // namespace

std::vector<Entry> LayoutRenameHints::Read(pugi::xml_node renamesNode) {
    std::vector<Entry> out;
    for (pugi::xml_node n : renamesNode.children("rename")) {
        Entry e;
        e.type = n.attribute("type").as_string(TYPE_MODEL);
        e.from = n.attribute("from").as_string();
        e.to = n.attribute("to").as_string();
        e.time = (std::time_t)std::strtoll(n.attribute("time").as_string(), nullptr, 10);
        e.id = n.attribute(LayoutObjectId::ATTRIBUTE).as_string();
        if (e.from.empty() || e.to.empty() || e.from == e.to) {
            continue;
        }
        Fold(out, e);
    }
    SortByTime(out);
    return out;
}

void LayoutRenameHints::Load(pugi::xml_node renamesNode) {
    _entries = Read(renamesNode);
}

void LayoutRenameHints::Add(const Entry& e) {
    if (e.from.empty() || e.to.empty() || e.from == e.to || e.type.empty()) {
        return;
    }
    Fold(_entries, e);
    SortByTime(_entries);
}

void LayoutRenameHints::Absorb(const std::string& type, const std::vector<LayoutRenameLog::TimedEntry>& renames) {
    for (const auto& r : renames) {
        Add({ type, r.oldName, r.newName, r.time, r.id });
    }
}

size_t LayoutRenameHints::Prune(std::time_t now, int maxAgeDays, size_t maxEntries) {
    const size_t before = _entries.size();
    const std::time_t cutoff = now - (std::time_t)maxAgeDays * 24 * 60 * 60;
    _entries.erase(std::remove_if(_entries.begin(), _entries.end(), [cutoff](const Entry& e) { return e.time < cutoff; }), _entries.end());
    if (_entries.size() > maxEntries) {
        _entries.erase(_entries.begin(), _entries.begin() + (_entries.size() - maxEntries));
    }
    return before - _entries.size();
}

void LayoutRenameHints::Serialize(BaseSerializingVisitor& visitor) const {
    if (_entries.empty()) {
        return;
    }
    visitor.WriteOpenTag(SECTION);
    for (const auto& e : _entries) {
        BaseSerializingVisitor::AttrCollector attrs;
        attrs.Add("type", e.type);
        attrs.Add("from", e.from);
        attrs.Add("to", e.to);
        attrs.Add("time", std::to_string((long long)e.time));
        if (!e.id.empty()) {
            attrs.Add(LayoutObjectId::ATTRIBUTE, e.id);
        }
        visitor.WriteOpenTag("rename", attrs, true);
    }
    visitor.WriteCloseTag();
}

void LayoutRenameHints::WriteTo(pugi::xml_node root) const {
    while (pugi::xml_node old = root.child(SECTION)) {
        root.remove_child(old);
    }
    if (_entries.empty()) {
        return;
    }
    pugi::xml_node section = root.append_child(SECTION);
    for (const auto& e : _entries) {
        pugi::xml_node n = section.append_child("rename");
        n.append_attribute("type") = e.type.c_str();
        n.append_attribute("from") = e.from.c_str();
        n.append_attribute("to") = e.to.c_str();
        n.append_attribute("time") = std::to_string((long long)e.time).c_str();
        if (!e.id.empty()) {
            n.append_attribute(LayoutObjectId::ATTRIBUTE) = e.id.c_str();
        }
    }
}

std::vector<Entry> LayoutRenameHints::Merge(const std::vector<Entry>& base, const std::vector<Entry>& mine, const std::vector<Entry>& theirs) {
    auto keys = [](const std::vector<Entry>& v) {
        std::set<std::tuple<std::string, std::string, std::string>> s;
        for (const auto& e : v) {
            s.insert({ e.type, e.from, e.to });
        }
        return s;
    };
    const auto kb = keys(base), km = keys(mine), kt = keys(theirs);
    std::vector<Entry> out;
    for (const auto* side : { &mine, &theirs }) {
        for (const auto& e : *side) {
            const std::tuple<std::string, std::string, std::string> k{ e.type, e.from, e.to };
            if (kb.count(k) && (!km.count(k) || !kt.count(k))) {
                continue; // one side pruned it
            }
            Fold(out, e);
        }
    }
    SortByTime(out);
    return out;
}

LayoutRenameHints::Resolution LayoutRenameHints::Resolve(const std::string& type, const std::string& name,
                                                          const std::function<bool(const std::string&)>& exists,
                                                          const std::function<std::string(const std::string&)>& nameForId) const {
    Resolution r;
    r.chain.push_back(name);
    std::string current = name;
    std::time_t after = 0;
    std::set<std::string> visited{ name };
    for (size_t step = 0; step <= _entries.size(); ++step) {
        const Entry* next = nullptr;
        for (const auto& e : _entries) {
            if (e.type != type || e.from != current || e.time < after) {
                continue;
            }
            if (next != nullptr && next->to != e.to) {
                r.outcome = Outcome::Ambiguous;
                return r;
            }
            if (next == nullptr) {
                next = &e;
            }
        }
        if (next == nullptr) {
            if (step == 0) {
                r.outcome = Outcome::NotRenamed;
                return r;
            }
            break;
        }
        if (!next->id.empty() && nameForId) {
            std::string byId = nameForId(next->id);
            if (!byId.empty()) {
                if (byId != r.chain.back()) {
                    r.chain.push_back(byId);
                }
                r.target = byId;
                r.outcome = byId == name ? Outcome::Broken : Outcome::Mapped;
                return r;
            }
        }
        current = next->to;
        after = next->time;
        if (!visited.insert(current).second) {
            r.chain.push_back(current);
            r.outcome = Outcome::Broken;
            return r;
        }
        r.chain.push_back(current);
    }
    if (exists && exists(current)) {
        r.outcome = Outcome::Mapped;
        r.target = current;
    } else {
        r.outcome = Outcome::Broken;
    }
    return r;
}
