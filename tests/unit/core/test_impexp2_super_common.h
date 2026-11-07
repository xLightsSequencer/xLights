#pragma once
#include "import_export/SuperStarImporter.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceMedia.h"

#include <pugixml.hpp>

#include <string>
#include <vector>

namespace ss2 {

struct Eff {
    int layer;
    std::string name;
    int start;
    int end;
    std::string settings;
    std::string palette;
    std::string Get(const std::string& k) const {
        // settings/palette strings are comma separated key=value pairs
        for (const std::string* src : { &settings, &palette }) {
            size_t pos = 0;
            while (pos <= src->size()) {
                size_t c = src->find(',', pos);
                std::string tok = src->substr(pos, c == std::string::npos ? std::string::npos : c - pos);
                if (tok.compare(0, k.size() + 1, k + "=") == 0) return tok.substr(k.size() + 1);
                if (c == std::string::npos) break;
                pos = c + 1;
            }
        }
        return "<none>";
    }
    bool Has(const std::string& k) const { return Get(k) != "<none>"; }
};

struct Fixture {
    HeadlessRenderContext ctx;
    Element* el = nullptr;
    SuperStar::Options opt;
    std::string err;
    bool ok = false;
    std::vector<std::string> prompts;

    Fixture() {
        auto& se = ctx.GetSequenceElements();
        se.SetViewsManager(&ctx._sequenceViewManager);
        el = se.AddElement("Model", "model", true, false, true, false, false);
        opt.layerBlend = "Normal";
        opt.imageGroupPrefix = "P";
    }
    SequenceElements& Seq() { return ctx.GetSequenceElements(); }
    SequenceMedia& Media() { return Seq().GetSequenceMedia(); }

    // Runs the full desktop path: raw buffer -> PreprocessXmlBuffer -> pugixml -> Importer
    bool RunRaw(const std::string& text, bool preprocess = true) {
        std::vector<char> buf(text.begin(), text.end());
        if (preprocess) SuperStar::PreprocessXmlBuffer(buf);
        pugi::xml_document d;
        d.load_buffer(buf.data(), buf.size());
        return RunDoc(d);
    }
    bool RunDoc(pugi::xml_document& d) {
        SuperStar::Importer imp(el, &Media(), opt);
        imp.SetPrefixPromptCallback([this](std::string& p) {
            prompts.push_back(p);
            if (promptCancel) return false;
            p = promptAnswer;
            return true;
        });
        err.clear();
        ok = imp.Run(d, &err);
        return ok;
    }
    bool promptCancel = false;
    std::string promptAnswer;

    std::vector<Eff> Effects() const {
        std::vector<Eff> out;
        for (size_t l = 0; l < el->GetEffectLayerCount(); ++l) {
            auto* L = el->GetEffectLayer(l);
            for (int i = 0; i < L->GetEffectCount(); ++i) {
                Effect* e = L->GetEffect(i);
                out.push_back({ (int)l, e->GetEffectName(), e->GetStartTimeMS(), e->GetEndTimeMS(), e->GetSettingsAsString(), e->GetPaletteAsString() });
            }
        }
        return out;
    }
    size_t Count() const { return Effects().size(); }
    // find the first effect whose name matches (and optionally start time)
    Eff Find(const std::string& name, int start = -1) const {
        for (auto& e : Effects())
            if (e.name == name && (start < 0 || e.start == start)) return e;
        return { -1, "", 0, 0, "", "" };
    }
};

inline std::string Layout(const char* orient = "vertical", const char* mode = "true", const std::string& extra = "") {
    return std::string("<layouts><layout visualizationMode=\"") + mode + "\" ribbonOrientation=\"" + orient + "\" " + extra + "/></layouts>";
}
inline std::string Doc(const std::string& layout, const std::string& body) {
    return "<SuperStarMetaFile saveFileVersion=\"7\">" + layout + body + "</SuperStarMetaFile>";
}
inline std::string Scene(int idx, int layer, int s, int e, const std::string& colors, const std::string& elems, const char* type = "normal") {
    return "<scene savedIndex=\"" + std::to_string(idx) + "\" layer=\"" + std::to_string(layer) + "\" startCentisecond=\"" + std::to_string(s) +
           "\" endCentisecond=\"" + std::to_string(e) + "\" type=\"" + type + "\" " + colors + ">" + elems + "</scene>";
}
inline std::string Elem(int r, int p) {
    return "<element ribbonIndex=\"" + std::to_string(r) + "\" pixelIndex=\"" + std::to_string(p) + "\"/>";
}
inline std::string Grid(int cols, int rows) {
    std::string s;
    for (int c = 0; c < cols; ++c)
        for (int r = 0; r < rows; ++r) s += Elem(c, r);
    return s;
}
inline const char* RED = "red1=\"100\" green1=\"0\" blue1=\"0\" red2=\"100\" green2=\"0\" blue2=\"0\"";
inline const char* BLACK_TO_GREEN = "red1=\"0\" green1=\"0\" blue1=\"0\" red2=\"0\" green2=\"100\" blue2=\"0\"";
inline const char* RED_TO_BLACK = "red1=\"100\" green1=\"0\" blue1=\"0\" red2=\"0\" green2=\"0\" blue2=\"0\"";
inline const char* RED_TO_BLUE = "red1=\"100\" green1=\"0\" blue1=\"0\" red2=\"0\" green2=\"0\" blue2=\"100\"";

} // namespace ss2
