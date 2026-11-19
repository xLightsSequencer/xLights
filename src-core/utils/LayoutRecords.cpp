/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LayoutRecords.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <set>
#include <sstream>
#include <unordered_map>

#include "LayoutObjectId.h"
#include "../XmlSerializer/XmlNodeKeys.h"

namespace LayoutRecords {

std::string NameOf(pugi::xml_node node) {
    const char* name = node.attribute("name").value();
    if (*name == 0) {
        name = node.attribute("Name").value();
    }
    return name;
}

const char* NameAttributeOf(pugi::xml_node node) {
    return node.attribute("name") ? "name" : "Name";
}

pugi::xml_node FindByName(pugi::xml_node parent, std::string_view tag, std::string_view name) {
    for (pugi::xml_node n = parent.first_child(); n; n = n.next_sibling()) {
        if (std::string_view(n.name()) == tag && std::string_view(n.attribute("name").as_string()) == name) {
            return n;
        }
    }
    return {};
}

bool SameName(pugi::xml_node a, pugi::xml_node b) {
    if (a.attribute("name")) {
        return std::string_view(b.attribute("name").as_string()) == a.attribute("name").as_string();
    }
    if (a.attribute("Name")) {
        return std::string_view(b.attribute("Name").as_string()) == a.attribute("Name").as_string();
    }
    return true;
}

void SetAttribute(pugi::xml_node node, const char* name, const std::string& value) {
    pugi::xml_attribute a = node.attribute(name);
    if (!a) {
        a = node.append_attribute(name);
    }
    a.set_value(value.c_str());
}

bool IsBookkeepingAttribute(std::string_view name) {
    return LayoutObjectId::IsIdAttribute(name) || name == "SourceVersion";
}

bool IsPositionAttribute(std::string_view name) {
    static const std::set<std::string_view> positions = {
        XmlNodeKeys::WorldPosXAttribute, XmlNodeKeys::WorldPosYAttribute, XmlNodeKeys::WorldPosZAttribute,
        XmlNodeKeys::RotateXAttribute,   XmlNodeKeys::RotateYAttribute,   XmlNodeKeys::RotateZAttribute,
        XmlNodeKeys::ScaleXAttribute,    XmlNodeKeys::ScaleYAttribute,    XmlNodeKeys::ScaleZAttribute,
        XmlNodeKeys::LengthAttribute,    XmlNodeKeys::X2Attribute,        XmlNodeKeys::Y2Attribute,
        XmlNodeKeys::Z2Attribute,
    };
    return positions.count(name) != 0;
}

bool PositionsEqual(std::string_view a, std::string_view b) {
    double da = std::strtod(std::string(a).c_str(), nullptr);
    double db = std::strtod(std::string(b).c_str(), nullptr);
    return std::round(da * 10000.0) == std::round(db * 10000.0);
}

// --- member lists -----------------------------------------------------------

std::vector<std::string> SplitMembers(const std::string& list) {
    std::vector<std::string> out;
    size_t start = 0;
    while (start <= list.size()) {
        size_t comma = list.find(',', start);
        if (comma == std::string::npos) {
            comma = list.size();
        }
        if (comma > start) {
            out.push_back(list.substr(start, comma - start));
        }
        start = comma + 1;
    }
    return out;
}

std::string JoinMembers(const std::vector<std::string>& members) {
    std::string out;
    for (const auto& s : members) {
        if (!out.empty()) {
            out += ',';
        }
        out += s;
    }
    return out;
}

std::string UnionMembers(const std::string& first, const std::string& second) {
    std::vector<std::string> out;
    for (const std::string* list : { &first, &second }) {
        std::istringstream in(*list);
        std::string cell;
        while (std::getline(in, cell, ',')) {
            if (std::find(out.begin(), out.end(), cell) == out.end()) {
                out.push_back(cell);
            }
        }
    }
    std::string res;
    for (const auto& s : out) {
        if (!res.empty()) res += ',';
        res += s;
    }
    return res;
}

std::vector<int> MergeOrder(const std::vector<int>& primary, const std::vector<int>& secondary, const std::vector<int>& base,
                            const std::vector<char>& keep) {
    std::vector<int> out;
    std::vector<char> placed(keep.size(), 0);
    std::vector<char> known(keep.size(), 0);
    for (int x : primary) known[x] = 1;
    for (int x : base) known[x] = 1;
    for (int x : primary) {
        if (keep[x] && !placed[x]) {
            out.push_back(x);
            placed[x] = 1;
        }
    }
    for (size_t i = 0; i < secondary.size(); ++i) {
        int x = secondary[i];
        if (!keep[x] || placed[x]) {
            continue;
        }
        bool appended = true;
        for (size_t k = i + 1; k < secondary.size() && appended; ++k) {
            appended = !known[secondary[k]];
        }
        size_t pos = out.size();
        if (!appended) {
            bool found = false;
            for (size_t k = i; k-- > 0;) {
                int pred = secondary[k];
                if (placed[pred]) {
                    pos = (size_t)(std::find(out.begin(), out.end(), pred) - out.begin()) + 1;
                    found = true;
                    break;
                }
            }
            for (size_t k = i + 1; k < secondary.size() && !found; ++k) {
                int succ = secondary[k];
                if (placed[succ]) {
                    pos = (size_t)(std::find(out.begin(), out.end(), succ) - out.begin());
                    found = true;
                }
            }
        }
        out.insert(out.begin() + pos, x);
        placed[x] = 1;
    }
    for (int x : base) {
        if (keep[x] && !placed[x]) {
            out.push_back(x);
            placed[x] = 1;
        }
    }
    for (size_t x = 0; x < keep.size(); ++x) {
        if (keep[x] && !placed[x]) {
            out.push_back((int)x);
        }
    }
    return out;
}

namespace {
std::vector<int> CommonOrder(const std::vector<int>& order, const std::vector<char>& common) {
    std::vector<int> out;
    for (int x : order) {
        if (common[x]) out.push_back(x);
    }
    return out;
}
} // namespace

std::string MergeMembers(const std::string* base, const std::string* mine, const std::string* theirs) {
    std::vector<std::string> names;
    std::unordered_map<std::string, int> index;
    auto keys = [&](const std::string* v) {
        std::vector<int> order;
        std::unordered_map<std::string, int> seen;
        for (const auto& s : SplitMembers(v ? *v : std::string())) {
            int n = ++seen[s];
            std::string key = n == 1 ? s : s + "\x1f" + std::to_string(n);
            auto f = index.find(key);
            if (f == index.end()) {
                f = index.emplace(key, (int)names.size()).first;
                names.push_back(s);
            }
            order.push_back(f->second);
        }
        return order;
    };
    std::vector<int> ob = keys(base), om = keys(mine), ot = keys(theirs);
    std::vector<char> inB(names.size(), 0), inM(names.size(), 0), inT(names.size(), 0), keep(names.size(), 0), common(names.size(), 0);
    for (int x : ob) inB[x] = 1;
    for (int x : om) inM[x] = 1;
    for (int x : ot) inT[x] = 1;
    for (size_t x = 0; x < names.size(); ++x) {
        keep[x] = inB[x] ? (inM[x] && inT[x]) : (inM[x] || inT[x]);
        common[x] = inB[x] && inM[x] && inT[x];
    }
    bool mineMoved = CommonOrder(om, common) != CommonOrder(ob, common);
    bool theirsMoved = CommonOrder(ot, common) != CommonOrder(ob, common);
    std::vector<int> order = (mineMoved && !theirsMoved) ? MergeOrder(om, ot, ob, keep) : MergeOrder(ot, om, ob, keep);
    std::vector<std::string> result;
    for (int x : order) {
        result.push_back(names[x]);
    }
    return JoinMembers(result);
}

// --- base show folder updates -----------------------------------------------

namespace {

// Empty attributes older versions wrote that newer ones leave out.
bool IsLegacyEmptyAttribute(std::string_view name) {
    static const std::set<std::string_view> names = { "StrandNames", "NodeNames", "PixelSpacing", "PixelCount", "PixelType" };
    return names.count(name) != 0;
}

bool IsNoController(std::string_view v) {
    return v.empty() || v == "No Controller";
}

bool ModelRecordChanged(pugi::xml_node local, pugi::xml_node base) {
    for (pugi::xml_attribute a : base.attributes()) {
        std::string_view aName = a.name();
        std::string_view aValue = a.value();
        pugi::xml_attribute localAttr = local.attribute(a.name());
        if (localAttr && std::string_view(localAttr.as_string()) == aValue) {
            continue;
        }
        if (IsBookkeepingAttribute(aName)) continue;
        // the show wires its own controllers
        if (aName == "Controller" && IsNoController(aValue) && !IsNoController(localAttr.as_string(""))) continue;
        if (aName == "StartChannel" && local.attribute("Controller")) continue;
        if (IsPositionAttribute(aName) && localAttr && PositionsEqual(aValue, localAttr.as_string())) continue;
        if (IsLegacyEmptyAttribute(aName) && aValue.empty() && !localAttr) continue;
        return true;
    }

    // every base child exists in the show with the same content
    for (pugi::xml_node nn = base.first_child(); nn; nn = nn.next_sibling()) {
        bool found = false;
        for (pugi::xml_node cc = local.first_child(); cc; cc = cc.next_sibling()) {
            if (std::string_view(cc.name()) == std::string_view(nn.name()) && SameName(nn, cc)) {
                found = true;
                if (ModelRecordChanged(cc, nn)) {
                    return true;
                }
                break;
            }
        }
        if (!found) {
            return true;
        }
    }
    // a child the base removed (e.g. a dimming curve reset to the default)
    for (pugi::xml_node cc = local.first_child(); cc; cc = cc.next_sibling()) {
        if (std::string_view(cc.name()) == "ControllerConnection") continue;
        bool found = false;
        for (pugi::xml_node nn = base.first_child(); nn; nn = nn.next_sibling()) {
            if (std::string_view(cc.name()) == std::string_view(nn.name()) && SameName(nn, cc)) {
                found = true;
                break;
            }
        }
        if (!found) {
            return true;
        }
    }
    return false;
}

bool ViewObjectRecordChanged(pugi::xml_node local, pugi::xml_node base) {
    for (pugi::xml_attribute a : base.attributes()) {
        if (LayoutObjectId::IsIdAttribute(a.name())) continue;
        pugi::xml_attribute localAttr = local.attribute(a.name());
        if (!localAttr || std::string_view(localAttr.as_string()) != std::string_view(a.as_string())) {
            return true;
        }
    }
    for (pugi::xml_node nn = base.first_child(); nn; nn = nn.next_sibling()) {
        bool found = false;
        for (pugi::xml_node cc = local.first_child(); cc; cc = cc.next_sibling()) {
            if (std::string_view(cc.name()) == std::string_view(nn.name()) && SameName(nn, cc)) {
                found = true;
                for (pugi::xml_attribute a : nn.attributes()) {
                    pugi::xml_attribute ccAttr = cc.attribute(a.name());
                    if (!ccAttr || std::string_view(ccAttr.as_string()) != std::string_view(a.as_string())) {
                        return true;
                    }
                }
            }
        }
        if (!found) return true;
    }
    return false;
}

} // namespace

bool BaseRecordChanged(pugi::xml_node local, pugi::xml_node base, BaseCompare rules) {
    return rules == BaseCompare::Model ? ModelRecordChanged(local, base) : ViewObjectRecordChanged(local, base);
}

bool IsShowOwnedAttribute(std::string_view recordTag, std::string_view attr) {
    if (recordTag == "model") {
        return attr == "Controller" || attr == "StartChannel" || attr == "ModelChain";
    }
    if (recordTag == "modelGroup") {
        return attr == "GridCentreX" || attr == "GridCentreY" || attr == "selected";
    }
    return false;
}

bool IsShowOwnedChild(std::string_view recordTag, std::string_view childTag) {
    return (recordTag == "model" || recordTag == "modelGroup") && childTag == "ControllerConnection";
}

void KeepLocalControllerWiring(pugi::xml_node local, pugi::xml_node updated) {
    pugi::xml_node localConnection;
    std::string port;
    for (pugi::xml_node p = local.first_child(); p; p = p.next_sibling()) {
        if (std::string_view(p.name()) == "ControllerConnection") {
            localConnection = p;
            port = p.attribute("Port").as_string();
        }
    }
    if (port.empty()) {
        return;
    }
    std::string modelChain = local.attribute("ModelChain").as_string();
    if (!modelChain.empty()) {
        SetAttribute(updated, "ModelChain", modelChain);
    }
    SetAttribute(updated, "StartChannel", local.attribute("StartChannel").as_string());
    SetAttribute(updated, "Controller", local.attribute("Controller").as_string());

    for (pugi::xml_node bp = updated.first_child(); bp; bp = bp.next_sibling()) {
        if (std::string_view(bp.name()) == "ControllerConnection") {
            if (!bp.attribute("Port")) {
                bp.append_attribute("Port") = port.c_str();
            }
            return;
        }
    }
    updated.append_copy(localConnection);
}

void KeepLocalGroupSettings(pugi::xml_node local, pugi::xml_node updated) {
    for (const char* attr : { "GridCentreX", "GridCentreY" }) {
        std::string value = local.attribute(attr).as_string();
        if (!value.empty()) {
            SetAttribute(updated, attr, value);
        }
    }
}

void RemoveControllerConnection(pugi::xml_node node) {
    pugi::xml_node cc = node.child("ControllerConnection");
    if (cc) {
        node.remove_child(cc);
    }
}

} // namespace LayoutRecords
