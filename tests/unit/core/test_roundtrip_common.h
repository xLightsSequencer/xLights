#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "XmlSerializer/StringSerializingVisitor.h"
#include "XmlSerializer/XmlSerializer.h"
#include "XmlSerializer/XmlSerializingVisitor.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "outputs/OutputManager.h"

namespace rt {

inline std::string Canon(const pugi::xml_node& n) {
    std::ostringstream os;
    n.print(os, "", pugi::format_raw);
    return os.str();
}

struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };

    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        mm.RecalcStartChannels();
        return m;
    }
};

inline std::string Ser(const Model* m, bool exporting = false) {
    StringSerializingVisitor v(exporting, false);
    m->Accept(v);
    std::string s = v.GetResult();
    auto p = s.find("?>");
    return p == std::string::npos ? s : s.substr(p + 2);
}

inline std::string Esc(const std::string& s) {
    std::string o;
    for (char c : s) {
        switch (c) {
        case '&': o += "&amp;"; break;
        case '<': o += "&lt;"; break;
        case '>': o += "&gt;"; break;
        case '"': o += "&quot;"; break;
        case '\n': o += "&#10;"; break;
        case '\r': o += "&#13;"; break;
        case '\t': o += "&#9;"; break;
        default: o += c;
        }
    }
    return o;
}

using Attrs = std::vector<std::pair<std::string, std::string>>;

inline std::string ModelXml(const std::string& name, const std::string& type, const Attrs& attrs, const std::string& children = "") {
    std::string x = "<model name=\"" + Esc(name) + "\" DisplayAs=\"" + Esc(type) + "\" StartChannel=\"1\"";
    for (auto& a : attrs) {
        x += " " + a.first + "=\"" + Esc(a.second) + "\"";
    }
    return x + ">" + children + "</model>";
}

inline std::map<std::string, std::string> AttrMap(const pugi::xml_node& n) {
    std::map<std::string, std::string> m;
    for (auto a = n.first_attribute(); a; a = a.next_attribute()) {
        m[a.name()] = a.value();
    }
    return m;
}

// Equal as strings, as numbers (written precision may differ) or ignoring case (colours).
inline bool Same(const std::string& a, const std::string& b) {
    if (a == b) return true;
    if (a.find(',') != std::string::npos && b.find(',') != std::string::npos) {
        std::stringstream sa(a), sb(b);
        std::string ta, tb;
        while (true) {
            bool ra = (bool)std::getline(sa, ta, ',');
            bool rb = (bool)std::getline(sb, tb, ',');
            if (ra != rb) return false;
            if (!ra) return true;
            if (!Same(ta, tb)) return false;
        }
    }
    char* ea = nullptr;
    char* eb = nullptr;
    double da = std::strtod(a.c_str(), &ea);
    double db = std::strtod(b.c_str(), &eb);
    if (!a.empty() && !b.empty() && *ea == 0 && *eb == 0) {
        return std::fabs(da - db) <= 1e-4 * std::max(1.0, std::fabs(da));
    }
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i])) return false;
    }
    return true;
}

// Names of attributes/children that differ between two serialized models (for diagnostics).
inline std::string DiffAttrs(const std::string& a, const std::string& b) {
    pugi::xml_document da, db;
    da.load_string(a.c_str());
    db.load_string(b.c_str());
    auto ma = AttrMap(da.first_child());
    auto mb = AttrMap(db.first_child());
    std::string out;
    for (auto& kv : ma) {
        auto it = mb.find(kv.first);
        if (it == mb.end()) out += " -" + kv.first;
        else if (it->second != kv.second) out += " ~" + kv.first + "(" + kv.second + "->" + it->second + ")";
    }
    for (auto& kv : mb) {
        if (!ma.count(kv.first)) out += " +" + kv.first + "(" + kv.second + ")";
    }
    if (Canon(da.first_child()).size() != 0 && out.empty()) out = " (child elements differ)";
    return out;
}

// Recursive description of the differences between two XML fragments (for diagnostics).
inline void DiffNodes(const pugi::xml_node& a, const pugi::xml_node& b, const std::string& path, std::string& out) {
    auto ma = AttrMap(a);
    auto mb = AttrMap(b);
    for (auto& kv : ma) {
        auto it = mb.find(kv.first);
        if (it == mb.end()) out += " " + path + "-" + kv.first;
        else if (it->second != kv.second) out += " " + path + "@" + kv.first + "(" + kv.second + "->" + it->second + ")";
    }
    for (auto& kv : mb) {
        if (!ma.count(kv.first)) out += " " + path + "+" + kv.first + "(" + kv.second + ")";
    }
    auto ca = a.first_child();
    auto cb = b.first_child();
    int i = 0;
    for (; ca && cb; ca = ca.next_sibling(), cb = cb.next_sibling(), ++i) {
        std::string nm = std::string(ca.name()) + "[" + std::to_string(i) + "]";
        if (std::string(ca.name()) != cb.name()) out += " " + path + "/" + nm + " is " + cb.name();
        else DiffNodes(ca, cb, path + "/" + nm, out);
    }
    if (ca) out += " " + path + " has extra child " + ca.name();
    if (cb) out += " " + path + " is missing child " + cb.name();
}

inline std::string DiffXml(const std::string& a, const std::string& b) {
    pugi::xml_document da, db;
    da.load_string(a.c_str());
    db.load_string(b.c_str());
    std::string out;
    DiffNodes(da.first_child(), db.first_child(), da.first_child().name(), out);
    return out;
}

} // namespace rt
