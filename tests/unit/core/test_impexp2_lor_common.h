#pragma once

// Synthetic LOR document builders for the [impexp2][lor] tests. The shapes follow
// what LOREdit.cpp / LORMusic.cpp / LORPixelEditor.cpp parse; none of it is real data.

#include "import_export/LOREdit.h"

#include <pugixml.hpp>

#include <string>
#include <vector>

namespace lortest {

inline pugi::xml_document Parse(const std::string& xml) {
    pugi::xml_document d;
    d.load_buffer(xml.data(), xml.size(), pugi::parse_default, pugi::encoding_auto);
    return d;
}

inline std::string Esc(const std::string& s) {
    std::string r;
    for (char c : s) {
        if (c == '&') r += "&amp;";
        else if (c == '<') r += "&lt;";
        else if (c == '>') r += "&gt;";
        else if (c == '"') r += "&quot;";
        else r += c;
    }
    return r;
}

// "lightorama_<type>:<palette>:<params>"
inline std::string Layer(const std::string& type, const std::string& params, const std::string& palette = "FFFF0000,1;FF00FF00,1") {
    return "lightorama_" + type + ":" + palette + ":" + params;
}
inline const std::string kNone = "lightorama_none::";

// "Mix_<blend>|a|b|mode|c|<left>|<right>"
inline std::string Settings(const std::string& left, const std::string& right = kNone, const std::string& mix = "Mix_Average",
                            const std::string& a = "0", const std::string& b = "0", const std::string& mode = "full", const std::string& c = "10") {
    return mix + "|" + a + "|" + b + "|" + mode + "|" + c + "|" + left + "|" + right;
}

inline std::string TrackEffect(int s, int e, const std::string& settings, const std::string& extra = " intensity=\"100\"") {
    return "<effect startCentisecond=\"" + std::to_string(s) + "\" endCentisecond=\"" + std::to_string(e) + "\"" + extra + " settings=\"" + Esc(settings) + "\"/>";
}

inline std::string Track(const std::string& effects, const std::string& attrs = " name=\"T\" type=\"none\"") {
    return "<track" + attrs + ">" + effects + "</track>";
}

inline std::string SeqProp(const std::string& name, const std::string& children, const std::string& attrs = "") {
    return "<SeqProp name=\"" + Esc(name) + "\"" + attrs + ">" + children + "</SeqProp>";
}

inline std::string Seq(const std::string& body) {
    return "<sequence saveFileVersion=\"16\">" + body + "</sequence>";
}

inline std::string Props(const std::string& seqProps) {
    return "<SequenceProps>" + seqProps + "</SequenceProps>";
}

inline std::string Chan(int row, int col, int color, const std::string& effects) {
    return "<channel row=\"" + std::to_string(row) + "\" col=\"" + std::to_string(col) + "\" color=\"" + std::to_string(color) + "\">" + effects + "</channel>";
}

inline std::string ChanEffect(int s, int e, const std::string& settings, const std::string& intens) {
    return "<effect startCentisecond=\"" + std::to_string(s) + "\" endCentisecond=\"" + std::to_string(e) + "\" " + intens + " settings=\"" + settings + "\"/>";
}

// A single-track prop "P" holding one effect, the layers given
inline std::string OneEffectDoc(const std::string& settings, int s = 0, int e = 100, const std::string& extra = " intensity=\"100\"",
                                const std::string& trackAttrs = " name=\"T\" type=\"none\"") {
    return Seq(Props(SeqProp("P", Track(TrackEffect(s, e, settings, extra), trackAttrs))));
}

struct Converted {
    bool ok = false;
    std::string name;
    std::string settings;
    std::string palette;
    LOREditEffect eff;
};

// Run the first layer of prop "P" through the same getters the importer uses.
inline Converted Convert(const std::string& settings, int s = 0, int e = 100, const std::string& extra = " intensity=\"100\"") {
    auto doc = Parse(OneEffectDoc(settings, s, e, extra));
    LOREdit l(doc, 20);
    auto v = l.GetTrackEffects("P", 0, 0);
    Converted c;
    if (v.empty()) return c;
    c.ok = true;
    c.eff = v[0];
    c.name = c.eff.GetxLightsEffect();
    c.palette = c.eff.GetPalette();
    c.settings = c.eff.GetSettings(c.palette);
    return c;
}

} // namespace lortest
