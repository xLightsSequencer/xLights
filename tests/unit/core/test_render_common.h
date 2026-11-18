#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include "render/FSEQFile.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceData.h"
#include "support/TempDir.h"
#include "utils/FileUtils.h"

namespace renderhelp {

struct EffectSpec {
    std::string model;
    int layer = 0;
    std::string name = "On";
    int startMs = 0;
    int endMs = 1000;
    std::string settings;
    std::string palette = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1";
};

struct ShowSpec {
    std::string models = R"(<model name="M" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="4" StringType="RGB Nodes"/>)";
    std::string groups;
    std::vector<EffectSpec> effects;
    int durationMs = 1000;
    int frameMs = 50;
    std::vector<std::string> modelElements = { "M" };
    std::string extraElementEffects;
    std::vector<std::string> disabledElements;
};

inline std::string XmlEscape(const std::string& s) {
    std::string r;
    for (char c : s) {
        switch (c) {
        case '&': r += "&amp;"; break;
        case '<': r += "&lt;"; break;
        case '>': r += "&gt;"; break;
        case '"': r += "&quot;"; break;
        default: r += c;
        }
    }
    return r;
}

inline std::string Seconds(int ms) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d.%03d", ms / 1000, ms % 1000);
    return buf;
}

inline std::string BuildXsq(const ShowSpec& s) {
    std::vector<std::string> pals;
    std::vector<std::string> sets;
    auto indexOf = [](std::vector<std::string>& v, const std::string& x) {
        for (size_t i = 0; i < v.size(); ++i) {
            if (v[i] == x) return (int)i;
        }
        v.push_back(x);
        return (int)v.size() - 1;
    };
    std::string out = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\" FixedPointTiming=\"1\" ModelBlending=\"true\">"
                      "<head><version>2024.01</version><mediaFile></mediaFile><sequenceTiming>" + std::to_string(s.frameMs) + " ms</sequenceTiming><sequenceType>Animation</sequenceType><sequenceDuration>" + Seconds(s.durationMs) + "</sequenceDuration></head>";
    out += "<DisplayElements>";
    for (const auto& m : s.modelElements) {
        bool dis = std::find(s.disabledElements.begin(), s.disabledElements.end(), m) != s.disabledElements.end();
        out += "<Element collapsed=\"0\" type=\"model\" name=\"" + m + "\" visible=\"1\"" + (dis ? " RenderDisabled=\"1\"" : "") + "/>";
    }
    out += "</DisplayElements>";
    std::string body = "<ElementEffects>";
    for (const auto& m : s.modelElements) {
        body += "<Element type=\"model\" name=\"" + m + "\">";
        int maxLayer = 0;
        for (const auto& e : s.effects) {
            if (e.model == m && e.layer > maxLayer) maxLayer = e.layer;
        }
        for (int l = 0; l <= maxLayer; ++l) {
            body += "<EffectLayer>";
            for (const auto& e : s.effects) {
                if (e.model != m || e.layer != l) continue;
                body += "<Effect ref=\"" + std::to_string(indexOf(sets, e.settings)) + "\" name=\"" + e.name + "\" startTime=\"" + std::to_string(e.startMs) + "\" endTime=\"" + std::to_string(e.endMs) + "\" palette=\"" + std::to_string(indexOf(pals, e.palette)) + "\"/>";
            }
            body += "</EffectLayer>";
        }
        body += "</Element>";
    }
    body += s.extraElementEffects + "</ElementEffects>";
    out += "<EffectDB>";
    for (const auto& x : sets) out += "<Effect>" + XmlEscape(x) + "</Effect>";
    out += "</EffectDB><ColorPalettes>";
    for (const auto& x : pals) out += "<ColorPalette>" + XmlEscape(x) + "</ColorPalette>";
    out += "</ColorPalettes>" + body + "</xsequence>";
    return out;
}

struct Rendered {
    bool loaded = false;
    bool opened = false;
    bool rendered = false;
    unsigned int channels = 0;
    unsigned int frames = 0;
    unsigned int frameMs = 0;
    std::vector<std::vector<uint8_t>> data;

    uint8_t at(unsigned int frame, unsigned int channel) const { return data.at(frame).at(channel); }
    struct Rgb {
        int r, g, b;
        bool operator==(const Rgb& o) const { return r == o.r && g == o.g && b == o.b; }
    };
    Rgb rgb(unsigned int frame, unsigned int node) const {
        return { at(frame, node * 3), at(frame, node * 3 + 1), at(frame, node * 3 + 2) };
    }
    bool isNear(unsigned int frame, unsigned int node, int r, int g, int b, int tol = 2) const {
        auto c = rgb(frame, node);
        return std::abs(c.r - r) <= tol && std::abs(c.g - g) <= tol && std::abs(c.b - b) <= tol;
    }
};

// The render engine looks its loggers up by name; the apps register them at startup.
inline void EnsureLoggers() {
    for (const char* n : { "render", "job", "work", "curl", "opengl" }) {
        if (!spdlog::get(n)) {
            spdlog::register_logger(std::make_shared<spdlog::logger>(n, std::make_shared<spdlog::sinks::null_sink_mt>()));
        }
    }
}

struct FixFileReset {
    ~FixFileReset() {
        FileUtils::SetFixFileShowDir("");
        FileUtils::SetFixFileDirectories({});
        FileUtils::ClearNonExistentFiles();
    }
};

// Owns the show directory and the context so a test can poke at both after rendering.
struct Show {
    TempDir dir;
    FixFileReset reset;
    std::unique_ptr<HeadlessRenderContext> ctx;
    Rendered result;
    std::string xsqPath;

    explicit Show(const ShowSpec& spec, const std::string& xsqOverride = "") {
        EnsureLoggers();
        dir.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb version=\"2\"/>\n");
        dir.Write("xlights_rgbeffects.xml",
                  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb>\n<models>\n" + spec.models + "\n</models>\n<modelGroups>\n" + spec.groups + "\n</modelGroups>\n</xrgb>\n");
        xsqPath = dir.Write("seq.xsq", xsqOverride.empty() ? BuildXsq(spec) : xsqOverride);
        ctx = std::make_unique<HeadlessRenderContext>();
        result.loaded = ctx->LoadShowFolder(dir.path.string());
        if (!result.loaded) return;
        result.opened = ctx->OpenSequence(xsqPath);
        if (!result.opened) return;
        result.rendered = ctx->RenderAndWait(60000);
        Capture();
    }

    void Capture() {
        auto& sd = ctx->_seqData;
        result.channels = sd.NumChannels();
        result.frames = sd.NumFrames();
        result.frameMs = sd.FrameTime();
        result.data.assign(result.frames, std::vector<uint8_t>(result.channels));
        for (unsigned int f = 0; f < result.frames; ++f) {
            for (unsigned int c = 0; c < result.channels; ++c) {
                result.data[f][c] = sd[f][c];
            }
        }
    }
};

struct FseqContents {
    bool ok = false;
    uint64_t frames = 0;
    int stepMs = 0;
    uint64_t channelCount = 0;
    uint32_t maxChannel = 0;
    std::vector<std::vector<uint8_t>> data;
    std::vector<FSEQFile::VariableHeader> headers;
};

inline FseqContents ReadFseq(const std::string& path) {
    FseqContents out;
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(path));
    if (!f) return out;
    out.frames = f->getNumFrames();
    out.stepMs = f->getStepTime();
    out.channelCount = f->getChannelCount();
    out.maxChannel = f->getMaxChannel();
    out.headers = f->getVariableHeaders();
    out.ok = true;
    for (uint32_t i = 0; i < out.frames; ++i) {
        std::unique_ptr<FSEQFile::FrameData> fd(f->getFrame(i));
        std::vector<uint8_t> buf(out.maxChannel, 0);
        if (!fd || !fd->readFrame(buf.data(), out.maxChannel)) {
            out.ok = false;
            break;
        }
        out.data.push_back(std::move(buf));
    }
    return out;
}

inline std::string SingleLine(const std::string& name, int startChannel, int nodes, const std::string& type = "RGB Nodes") {
    return "<model name=\"" + name + "\" DisplayAs=\"Single Line\" StartChannel=\"" + std::to_string(startChannel) + "\" NumStrings=\"1\" NodesPerString=\"" + std::to_string(nodes) + "\" StringType=\"" + type + "\"/>";
}

inline std::string Matrix(const std::string& name, int startChannel, int strands, int nodesPerStrand) {
    return "<model name=\"" + name + "\" DisplayAs=\"Matrix\" StartChannel=\"" + std::to_string(startChannel) + "\" NumStrings=\"1\" NodesPerString=\"" + std::to_string(strands * nodesPerStrand) + "\" StrandsPerString=\"" + std::to_string(strands) + "\" StringType=\"RGB Nodes\"/>";
}

inline Rendered Render(const ShowSpec& spec) {
    Show s(spec);
    return s.result;
}

inline EffectSpec Eff(const std::string& model, int layer, const std::string& name, int startMs, int endMs,
                      const std::string& palette, const std::string& settings = "") {
    EffectSpec e;
    e.model = model;
    e.layer = layer;
    e.name = name;
    e.startMs = startMs;
    e.endMs = endMs;
    e.palette = palette;
    e.settings = settings;
    return e;
}

inline std::string Pal(const std::string& c1, const std::string& c2 = "", const std::string& extra = "") {
    std::string p = "C_BUTTON_Palette1=" + c1 + ",C_CHECKBOX_Palette1=1";
    if (!c2.empty()) p += ",C_BUTTON_Palette2=" + c2 + ",C_CHECKBOX_Palette2=1";
    if (!extra.empty()) p += "," + extra;
    return p;
}

} // namespace renderhelp

#include <catch2/catch_tostring.hpp>

// Every TU that compares Rgb values must see the same printer, so it lives here.
template <>
struct Catch::StringMaker<renderhelp::Rendered::Rgb> {
    static std::string convert(const renderhelp::Rendered::Rgb& c) {
        return "{" + std::to_string(c.r) + "," + std::to_string(c.g) + "," + std::to_string(c.b) + "}";
    }
};
