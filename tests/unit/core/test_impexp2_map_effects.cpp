#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "import_export/EffectMapper.h"
#include "models/ModelGroup.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/SequenceElements.h"
#include "render/SequencePackage.h"
#include "test_impexp2_map_common.h"
#include "test_render_common.h"

#include <map>
#include <string>
#include <vector>

using namespace renderhelp;

namespace {

ShowSpec SrcSpec(std::vector<EffectSpec> effects, std::vector<std::string> elements = { "A", "B" }) {
    ShowSpec s;
    s.models = SingleLine("A", 1, 4) + SingleLine("B", 100, 4);
    s.modelElements = std::move(elements);
    s.effects = std::move(effects);
    s.durationMs = 20000;
    return s;
}

ShowSpec DstSpec(std::vector<EffectSpec> effects = {}, std::vector<std::string> elements = { "X", "Y", "G" }) {
    ShowSpec s;
    s.models = SingleLine("X", 1, 4) + SingleLine("Y", 100, 4);
    s.groups = R"(<modelGroup name="G" models="X,Y" layout="minimalGrid" GridSize="400"/>)";
    s.modelElements = std::move(elements);
    s.effects = std::move(effects);
    s.durationMs = 20000;
    return s;
}

struct Maps {
    SequencePackage pkg;
    std::vector<EffectLayer*> mapped;
    std::map<std::string, std::string> mapping;
    std::map<std::string, std::string> types;
    explicit Maps(Show& dst) : pkg(dst.dir.File("x.xsq"), dst.dir.path.string(), "x.xsq", &dst.ctx->AllModels) {}
};

struct Pair {
    Show src;
    Show dst;
    Maps m;
    Pair(const ShowSpec& s, const ShowSpec& d) : src(s), dst(d), m(dst) {}
    Element* S(const std::string& n) { return src.ctx->GetSequenceElements().GetElement(n); }
    Element* D(const std::string& n) { return dst.ctx->GetSequenceElements().GetElement(n); }
    EffectLayer* SL(const std::string& n, int l = 0) { return S(n)->GetEffectLayer(l); }
    EffectLayer* DL(const std::string& n, int l = 0) { return D(n)->GetEffectLayer(l); }
    void CopyLayer(EffectLayer* t, EffectLayer* s, bool erase = false, bool lock = false, bool convert = false) {
        MapXLightsEffects(t, s, m.mapped, erase, m.pkg, lock, m.mapping, convert, m.types);
    }
};

std::vector<std::tuple<std::string, int, int>> Contents(EffectLayer* l) {
    std::vector<std::tuple<std::string, int, int>> out;
    for (int i = 0; i < l->GetEffectCount(); ++i) {
        out.emplace_back(l->GetEffect(i)->GetEffectName(), l->GetEffect(i)->GetStartTimeMS(), l->GetEffect(i)->GetEndTimeMS());
    }
    return out;
}

using Tup = std::vector<std::tuple<std::string, int, int>>;

} // namespace

TEST_CASE("MapXLightsEffects copies names and times in order", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")), Eff("A", 0, "Bars", 1000, 3000, Pal("#00FF00")), Eff("A", 0, "Fire", 5000, 9000, Pal("#0000FF")) }), DstSpec());
    REQUIRE(p.src.result.opened);
    REQUIRE(p.dst.result.opened);
    p.CopyLayer(p.DL("X"), p.SL("A"));
    CHECK(Contents(p.DL("X")) == Tup{ { "On", 0, 1000 }, { "Bars", 1000, 3000 }, { "Fire", 5000, 9000 } });
}

TEST_CASE("MapXLightsEffects preserves palette and settings text", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "Bars", 0, 1000, Pal("#123456", "#ABCDEF"), "E_SLIDER_Bars_BarCount=7,E_CHOICE_Bars_Direction=down") }), DstSpec());
    p.CopyLayer(p.DL("X"), p.SL("A"));
    REQUIRE(p.DL("X")->GetEffectCount() == 1);
    Effect* e = p.DL("X")->GetEffect(0);
    CHECK(e->GetPaletteAsString() == p.SL("A")->GetEffect(0)->GetPaletteAsString());
    CHECK(e->GetSettings().Get("E_SLIDER_Bars_BarCount", "") == "7");
    CHECK(e->GetSettings().Get("E_CHOICE_Bars_Direction", "") == "down");
    CHECK(e->GetPaletteAsString().find("#123456") != std::string::npos);
}

TEST_CASE("MapXLightsEffects records the source layer as mapped even when empty", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }, { "A", "B" }), DstSpec());
    p.CopyLayer(p.DL("X"), p.SL("B"));
    REQUIRE(p.m.mapped.size() == 1);
    CHECK(p.m.mapped[0] == p.SL("B"));
    CHECK(p.DL("X")->GetEffectCount() == 0);
    p.CopyLayer(p.DL("X"), p.SL("A"));
    CHECK(p.m.mapped.size() == 2);
}

TEST_CASE("MapXLightsEffects skips source effects overlapping existing target effects", "[impexp2][mapper]") {
    auto [s, e, copied] = GENERATE(table<int, int, bool>({
        { 0, 500, true },       // before existing 1000-2000
        { 0, 1000, true },      // touches start
        { 2000, 3000, true },   // touches end
        { 500, 1500, false },
        { 1500, 1800, false },
        { 1900, 2500, false },
        { 0, 5000, false },
        { 1000, 2000, false },
        { 3000, 4000, true },
    }));
    Pair p(SrcSpec({ Eff("A", 0, "Bars", s, e, Pal("#FF0000")) }), DstSpec({ Eff("X", 0, "On", 1000, 2000, Pal("#00FF00")) }));
    p.CopyLayer(p.DL("X"), p.SL("A"));
    CAPTURE(s, e);
    CHECK(p.DL("X")->GetEffectCount() == (copied ? 2 : 1));
}

TEST_CASE("MapXLightsEffects eraseExisting clears the target first", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "Bars", 1000, 2000, Pal("#FF0000")) }), DstSpec({ Eff("X", 0, "On", 1000, 2000, Pal("#00FF00")), Eff("X", 0, "On", 5000, 6000, Pal("#00FF00")) }));
    REQUIRE(p.DL("X")->GetEffectCount() == 2);
    p.CopyLayer(p.DL("X"), p.SL("A"), true);
    CHECK(Contents(p.DL("X")) == Tup{ { "Bars", 1000, 2000 } });
}

TEST_CASE("MapXLightsEffects without erase keeps target effects", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "Bars", 1000, 2000, Pal("#FF0000")) }), DstSpec({ Eff("X", 0, "On", 5000, 6000, Pal("#00FF00")) }));
    p.CopyLayer(p.DL("X"), p.SL("A"), false);
    CHECK(Contents(p.DL("X")) == Tup{ { "Bars", 1000, 2000 }, { "On", 5000, 6000 } });
}

TEST_CASE("MapXLightsEffects lock option", "[impexp2][mapper]") {
    auto lock = GENERATE(true, false);
    Pair p(SrcSpec({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000"), "E_SLIDER_Bars_BarCount=2") }), DstSpec());
    p.CopyLayer(p.DL("X"), p.SL("A"), false, lock);
    REQUIRE(p.DL("X")->GetEffectCount() == 1);
    CHECK(p.DL("X")->GetEffect(0)->IsLocked() == lock);
    CHECK(p.DL("X")->GetEffect(0)->GetSettings().Get("E_SLIDER_Bars_BarCount", "") == "2");
}

TEST_CASE("MapXLightsEffects does not carry a source lock across when lock is off", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000"), "X_Effect_Locked=True") }), DstSpec());
    p.CopyLayer(p.DL("X"), p.SL("A"), false, false);
    REQUIRE(p.DL("X")->GetEffectCount() == 1);
    CHECK_FALSE(p.DL("X")->GetEffect(0)->IsLocked());
}

TEST_CASE("MapXLightsEffects remaps Duplicate effect references", "[impexp2][mapper]") {
    SECTION("mapped model is rewritten") {
        Pair p(SrcSpec({ Eff("A", 0, "Duplicate", 0, 1000, Pal("#FF0000"), "E_CHOICE_Duplicate_Model=B") }), DstSpec());
        p.m.mapping["B"] = "Y";
        p.CopyLayer(p.DL("X"), p.SL("A"));
        REQUIRE(p.DL("X")->GetEffectCount() == 1);
        CHECK(p.DL("X")->GetEffect(0)->GetSettings().Get("E_CHOICE_Duplicate_Model", "") == "Y");
    }
    SECTION("unmapped model is left alone") {
        Pair p(SrcSpec({ Eff("A", 0, "Duplicate", 0, 1000, Pal("#FF0000"), "E_CHOICE_Duplicate_Model=B") }), DstSpec());
        p.m.mapping["Q"] = "Y";
        p.CopyLayer(p.DL("X"), p.SL("A"));
        REQUIRE(p.DL("X")->GetEffectCount() == 1);
        CHECK(p.DL("X")->GetEffect(0)->GetSettings().Get("E_CHOICE_Duplicate_Model", "") == "B");
    }
    SECTION("empty mapping table") {
        Pair p(SrcSpec({ Eff("A", 0, "Duplicate", 0, 1000, Pal("#FF0000"), "E_CHOICE_Duplicate_Model=B") }), DstSpec());
        p.CopyLayer(p.DL("X"), p.SL("A"));
        REQUIRE(p.DL("X")->GetEffectCount() == 1);
        CHECK(p.DL("X")->GetEffect(0)->GetSettings().Get("E_CHOICE_Duplicate_Model", "") == "B");
    }
    SECTION("it is the only setting (no leading comma)") {
        Pair p(SrcSpec({ Eff("A", 0, "Duplicate", 0, 1000, Pal("#FF0000"), "E_CHOICE_Duplicate_Model=B") }), DstSpec());
        p.m.mapping["B"] = "Y";
        p.CopyLayer(p.DL("X"), p.SL("A"));
        CHECK(p.DL("X")->GetEffect(0)->GetSettingsAsString().find("E_CHOICE_Duplicate_Model=Y") != std::string::npos);
    }
    SECTION("non duplicate effects with the setting are not rewritten") {
        Pair p(SrcSpec({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000"), "E_CHOICE_Duplicate_Model=B") }), DstSpec());
        p.m.mapping["B"] = "Y";
        p.CopyLayer(p.DL("X"), p.SL("A"));
        CHECK(p.DL("X")->GetEffect(0)->GetSettings().Get("E_CHOICE_Duplicate_Model", "") == "B");
    }
}

TEST_CASE("MapXLightsEffects onto a group converts the render buffer style", "[impexp2][mapper]") {
    auto [in, out] = GENERATE(table<std::string, std::string>({
        { "Per Preview", "Per Model Per Preview" },
        { "Default", "Per Model Default" },
        { "", "Per Model Default" },
        { "Per Model Default", "Per Model Default" },
    }));
    std::string settings = in.empty() ? "E_SLIDER_Bars_BarCount=2" : "B_CHOICE_BufferStyle=" + in + ",E_SLIDER_Bars_BarCount=2";
    Pair p(SrcSpec({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000"), settings) }), DstSpec());
    p.CopyLayer(p.DL("G"), p.SL("A"), false, false, true);
    REQUIRE(p.DL("G")->GetEffectCount() == 1);
    CAPTURE(in);
    CHECK(p.DL("G")->GetEffect(0)->GetSettings().Get("B_CHOICE_BufferStyle", "<none>") == out);
}

TEST_CASE("MapXLightsEffects onto a group leaves the buffer style when conversion is off", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000"), "B_CHOICE_BufferStyle=Default") }), DstSpec());
    p.CopyLayer(p.DL("G"), p.SL("A"), false, false, false);
    REQUIRE(p.DL("G")->GetEffectCount() == 1);
    CHECK(p.DL("G")->GetEffect(0)->GetSettings().Get("B_CHOICE_BufferStyle", "<none>") == "Default");
}

TEST_CASE("MapXLightsEffects onto a plain model never changes the buffer style", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000"), "B_CHOICE_BufferStyle=Per Preview") }), DstSpec());
    p.CopyLayer(p.DL("X"), p.SL("A"), false, false, true);
    CHECK(p.DL("X")->GetEffect(0)->GetSettings().Get("B_CHOICE_BufferStyle", "<none>") == "Per Preview");
    CHECK_FALSE(p.DL("X")->GetEffect(0)->GetSettings().Contains("B_CHOICE_PerPreviewCamera"));
}

TEST_CASE("MapXLightsEffects per-preview camera on a group", "[impexp2][mapper]") {
    SECTION("camera setting present: replaced with the group's default") {
        Pair p(SrcSpec({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000"), "B_CHOICE_BufferStyle=Per Preview,B_CHOICE_PerPreviewCamera=SomeCam") }), DstSpec());
        auto* g = dynamic_cast<ModelGroup*>(p.dst.ctx->GetModel("G"));
        REQUIRE(g != nullptr);
        g->SetDefaultCamera("GroupCam");
        p.CopyLayer(p.DL("G"), p.SL("A"), false, false, true);
        CHECK(p.DL("G")->GetEffect(0)->GetSettings().Get("B_CHOICE_PerPreviewCamera", "") == "GroupCam");
    }
    SECTION("camera setting absent: added") {
        Pair p(SrcSpec({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000"), "B_CHOICE_BufferStyle=Per Preview") }), DstSpec());
        auto* g = dynamic_cast<ModelGroup*>(p.dst.ctx->GetModel("G"));
        g->SetDefaultCamera("GroupCam");
        p.CopyLayer(p.DL("G"), p.SL("A"), false, false, true);
        CHECK(p.DL("G")->GetEffect(0)->GetSettings().Get("B_CHOICE_PerPreviewCamera", "") == "GroupCam");
    }
    SECTION("2D / Default camera follow the group default") {
        auto cam = GENERATE(as<std::string>{}, "2D", "Default");
        Pair p(SrcSpec({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000"), "B_CHOICE_BufferStyle=Per Preview,B_CHOICE_PerPreviewCamera=" + cam) }), DstSpec());
        p.CopyLayer(p.DL("G"), p.SL("A"), false, false, true);
        CHECK(p.DL("G")->GetEffect(0)->GetSettings().Get("B_CHOICE_PerPreviewCamera", "") == "2D");
    }
    SECTION("non per-preview styles get no camera") {
        Pair p(SrcSpec({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000"), "B_CHOICE_BufferStyle=Horizontal Per Strand") }), DstSpec());
        p.CopyLayer(p.DL("G"), p.SL("A"), false, false, true);
        CHECK_FALSE(p.DL("G")->GetEffect(0)->GetSettings().Contains("B_CHOICE_PerPreviewCamera"));
    }
}

TEST_CASE("MapXLightsEffects onto an element missing from the layout still copies", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }), DstSpec({}, { "X", "Ghost" }));
    p.CopyLayer(p.DL("Ghost"), p.SL("A"));
    CHECK(Contents(p.DL("Ghost")) == Tup{ { "On", 0, 1000 } });
}

TEST_CASE("MapXLightsEffects handles zero length and adjacent source effects", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")), Eff("A", 0, "On", 1000, 2000, Pal("#FF0000")), Eff("A", 0, "On", 2000, 3000, Pal("#FF0000")) }), DstSpec());
    p.CopyLayer(p.DL("X"), p.SL("A"));
    CHECK(Contents(p.DL("X")) == Tup{ { "On", 0, 1000 }, { "On", 1000, 2000 }, { "On", 2000, 3000 } });
}

TEST_CASE("MapXLightsEffects copies a large number of effects", "[impexp2][mapper]") {
    std::vector<EffectSpec> v;
    for (int i = 0; i < 400; ++i) {
        v.push_back(Eff("A", 0, "On", i * 50, i * 50 + 50, Pal("#FF0000")));
    }
    ShowSpec s = SrcSpec(v);
    s.durationMs = 20000;
    Pair p(s, DstSpec());
    p.CopyLayer(p.DL("X"), p.SL("A"));
    CHECK(p.DL("X")->GetEffectCount() == 400);
    CHECK(std::get<2>(Contents(p.DL("X")).back()) == 20000);
}

TEST_CASE("MapXLightsEffects mapping an effect layer onto itself changes nothing", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }), DstSpec());
    p.CopyLayer(p.SL("A"), p.SL("A"));
    CHECK(Contents(p.SL("A")) == Tup{ { "On", 0, 1000 } });
}

TEST_CASE("MapXLightsEffects onto a layer with erase when source equals empty layer wipes the target", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }), DstSpec({ Eff("X", 0, "On", 0, 1000, Pal("#FF0000")) }));
    p.CopyLayer(p.DL("X"), p.SL("B"), true);
    CHECK(p.DL("X")->GetEffectCount() == 0);
}

TEST_CASE("MapXLightsEffects with video effect registers the imported video", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "Video", 0, 1000, Pal("#FF0000"), "E_FILEPICKERCTRL_Video_Filename=clip.mp4") }), DstSpec());
    p.CopyLayer(p.DL("X"), p.SL("A"));
    REQUIRE(p.DL("X")->GetEffectCount() == 1);
    CHECK(p.DL("X")->GetEffect(0)->GetSettings().Get("E_FILEPICKERCTRL_Video_Filename", "") == "clip.mp4");
}

TEST_CASE("MapXLightsEffects media effects without embedded media copy unchanged", "[impexp2][mapper]") {
    auto [name, key] = GENERATE(table<std::string, std::string>({
        { "Pictures", "E_TEXTCTRL_Pictures_Filename" },
        { "Shape", "E_FILEPICKERCTRL_SVG" },
        { "Text", "E_FILEPICKERCTRL_Text_File" },
        { "Ripple", "E_FILEPICKERCTRL_Ripple_SVG" },
        { "Shader", "E_0FILEPICKERCTRL_IFS" },
    }));
    Pair p(SrcSpec({ Eff("A", 0, name, 0, 1000, Pal("#FF0000"), key + "=missing/file.dat") }), DstSpec());
    p.CopyLayer(p.DL("X"), p.SL("A"));
    REQUIRE(p.DL("X")->GetEffectCount() == 1);
    CAPTURE(name);
    CHECK(p.DL("X")->GetEffect(0)->GetSettings().Get(key, "") == "missing/file.dat");
}

TEST_CASE("MapXLightsEffects Faces effect without face definitions is harmless", "[impexp2][mapper]") {
    auto face = GENERATE(as<std::string>{}, "", "Default", "Nope");
    Pair p(SrcSpec({ Eff("A", 0, "Faces", 0, 1000, Pal("#FF0000"), face.empty() ? "" : "E_CHOICE_Faces_FaceDefinition=" + face) }), DstSpec());
    p.CopyLayer(p.DL("X"), p.SL("A"));
    CHECK(p.DL("X")->GetEffectCount() == 1);
}

TEST_CASE("MapXLightsStrandEffects uses the layer map first", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")), Eff("B", 0, "Bars", 0, 2000, Pal("#FF0000")) }), DstSpec());
    std::map<std::string, EffectLayer*> lm{ { "A", p.SL("B") } };
    MapXLightsStrandEffects(p.DL("X"), "A", lm, p.src.ctx->GetSequenceElements(), p.m.mapped, false, p.m.pkg, false, p.m.mapping, p.m.types);
    CHECK(Contents(p.DL("X")) == Tup{ { "Bars", 0, 2000 } });
}

TEST_CASE("MapXLightsStrandEffects falls back to the first layer of the named element", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")), Eff("A", 1, "Bars", 0, 1000, Pal("#FF0000")) }), DstSpec());
    std::map<std::string, EffectLayer*> lm;
    MapXLightsStrandEffects(p.DL("X"), "A", lm, p.src.ctx->GetSequenceElements(), p.m.mapped, false, p.m.pkg, false, p.m.mapping, p.m.types);
    CHECK(Contents(p.DL("X")) == Tup{ { "On", 0, 1000 } });
    CHECK(p.m.mapped.size() == 1);
}

TEST_CASE("MapXLightsStrandEffects with a layer map entry that is null falls back to the element", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }), DstSpec());
    std::map<std::string, EffectLayer*> lm{ { "A", nullptr } };
    MapXLightsStrandEffects(p.DL("X"), "A", lm, p.src.ctx->GetSequenceElements(), p.m.mapped, false, p.m.pkg, false, p.m.mapping, p.m.types);
    CHECK(p.DL("X")->GetEffectCount() == 1);
}

TEST_CASE("MapXLightsStrandEffects for a missing source maps nothing", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }), DstSpec());
    std::map<std::string, EffectLayer*> lm;
    for (const char* name : { "Nope", "", "A/B", "a" }) {
        MapXLightsStrandEffects(p.DL("X"), name, lm, p.src.ctx->GetSequenceElements(), p.m.mapped, false, p.m.pkg, false, p.m.mapping, p.m.types);
    }
    CHECK(p.DL("X")->GetEffectCount() == 0);
    CHECK(p.m.mapped.empty());
}

TEST_CASE("MapXLightsEffects (Element) maps all layers and grows the target", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")), Eff("A", 1, "Bars", 0, 1000, Pal("#FF0000")), Eff("A", 2, "Fire", 0, 1000, Pal("#FF0000")) }), DstSpec());
    REQUIRE(p.D("X")->GetEffectLayerCount() == 1);
    std::map<std::string, Element*> em;
    std::map<std::string, EffectLayer*> lm;
    MapXLightsEffects(p.D("X"), "A", p.src.ctx->GetSequenceElements(), em, lm, p.m.mapped, false, p.m.pkg, false, p.m.mapping, false, p.m.types);
    REQUIRE(p.D("X")->GetEffectLayerCount() == 3);
    CHECK(Contents(p.DL("X", 0)) == Tup{ { "On", 0, 1000 } });
    CHECK(Contents(p.DL("X", 1)) == Tup{ { "Bars", 0, 1000 } });
    CHECK(Contents(p.DL("X", 2)) == Tup{ { "Fire", 0, 1000 } });
    CHECK(p.m.mapped.size() == 3);
}

TEST_CASE("MapXLightsEffects (Element) keeps extra target layers", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }), DstSpec({ Eff("X", 0, "On", 5000, 6000, Pal("#FF0000")), Eff("X", 1, "Bars", 5000, 6000, Pal("#FF0000")), Eff("X", 2, "Fire", 5000, 6000, Pal("#FF0000")) }));
    REQUIRE(p.D("X")->GetEffectLayerCount() == 3);
    std::map<std::string, Element*> em;
    std::map<std::string, EffectLayer*> lm;
    MapXLightsEffects(p.D("X"), "A", p.src.ctx->GetSequenceElements(), em, lm, p.m.mapped, false, p.m.pkg, false, p.m.mapping, false, p.m.types);
    CHECK(p.D("X")->GetEffectLayerCount() == 3);
    CHECK(p.DL("X", 0)->GetEffectCount() == 2);
    CHECK(p.DL("X", 1)->GetEffectCount() == 1);
}

TEST_CASE("MapXLightsEffects (Element) copies layer names", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")), Eff("A", 1, "Bars", 0, 1000, Pal("#FF0000")) }), DstSpec());
    p.SL("A", 0)->SetLayerName("Base");
    p.SL("A", 1)->SetLayerName("Top, \"quoted\"");
    std::map<std::string, Element*> em;
    std::map<std::string, EffectLayer*> lm;
    MapXLightsEffects(p.D("X"), "A", p.src.ctx->GetSequenceElements(), em, lm, p.m.mapped, false, p.m.pkg, false, p.m.mapping, false, p.m.types);
    CHECK(p.DL("X", 0)->GetLayerName() == "Base");
    CHECK(p.DL("X", 1)->GetLayerName() == "Top, \"quoted\"");
}

TEST_CASE("MapXLightsEffects (Element) layer map entry wins and only layer 0 is written", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")), Eff("A", 1, "Bars", 0, 1000, Pal("#FF0000")), Eff("B", 0, "Fire", 0, 1000, Pal("#FF0000")) }), DstSpec());
    std::map<std::string, Element*> em;
    std::map<std::string, EffectLayer*> lm{ { "A", p.SL("B") } };
    MapXLightsEffects(p.D("X"), "A", p.src.ctx->GetSequenceElements(), em, lm, p.m.mapped, false, p.m.pkg, false, p.m.mapping, false, p.m.types);
    CHECK(p.D("X")->GetEffectLayerCount() == 1);
    CHECK(Contents(p.DL("X")) == Tup{ { "Fire", 0, 1000 } });
}

TEST_CASE("MapXLightsEffects (Element) uses the element map before the sequence", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")), Eff("B", 0, "Fire", 0, 1000, Pal("#FF0000")) }), DstSpec());
    std::map<std::string, Element*> em{ { "A", p.S("B") } };
    std::map<std::string, EffectLayer*> lm;
    MapXLightsEffects(p.D("X"), "A", p.src.ctx->GetSequenceElements(), em, lm, p.m.mapped, false, p.m.pkg, false, p.m.mapping, false, p.m.types);
    CHECK(Contents(p.DL("X")) == Tup{ { "Fire", 0, 1000 } });
}

TEST_CASE("MapXLightsEffects (Element) with unknown source does nothing", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }), DstSpec({ Eff("X", 0, "On", 5000, 6000, Pal("#FF0000")) }));
    std::map<std::string, Element*> em;
    std::map<std::string, EffectLayer*> lm;
    for (const char* name : { "Nope", "", "A/B", "a" }) {
        MapXLightsEffects(p.D("X"), name, p.src.ctx->GetSequenceElements(), em, lm, p.m.mapped, true, p.m.pkg, false, p.m.mapping, false, p.m.types);
    }
    CHECK(Contents(p.DL("X")) == Tup{ { "On", 5000, 6000 } });
    CHECK(p.m.mapped.empty());
}

TEST_CASE("MapXLightsEffects (Element) honours erase for every mapped layer", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")), Eff("A", 1, "Bars", 0, 1000, Pal("#FF0000")) }),
           DstSpec({ Eff("X", 0, "Fire", 0, 1000, Pal("#FF0000")), Eff("X", 1, "Fire", 0, 1000, Pal("#FF0000")) }));
    std::map<std::string, Element*> em;
    std::map<std::string, EffectLayer*> lm;
    MapXLightsEffects(p.D("X"), "A", p.src.ctx->GetSequenceElements(), em, lm, p.m.mapped, true, p.m.pkg, false, p.m.mapping, false, p.m.types);
    CHECK(Contents(p.DL("X", 0)) == Tup{ { "On", 0, 1000 } });
    CHECK(Contents(p.DL("X", 1)) == Tup{ { "Bars", 0, 1000 } });
}

TEST_CASE("MapXLightsEffects (Element) group conversion depends on the source model type", "[impexp2][mapper]") {
    auto [type, expected] = GENERATE(table<std::string, std::string>({
        { "Model", "Per Model Default" },
        { "ModelGroup", "Default" },
        { "Unknown", "Per Model Default" },
        { "SubModel", "Per Model Default" },
    }));
    Pair p(SrcSpec({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000"), "B_CHOICE_BufferStyle=Default") }), DstSpec());
    p.m.types["A"] = type;
    std::map<std::string, Element*> em;
    std::map<std::string, EffectLayer*> lm;
    MapXLightsEffects(p.D("G"), "A", p.src.ctx->GetSequenceElements(), em, lm, p.m.mapped, false, p.m.pkg, false, p.m.mapping, true, p.m.types);
    REQUIRE(p.DL("G")->GetEffectCount() == 1);
    CAPTURE(type);
    CHECK(p.DL("G")->GetEffect(0)->GetSettings().Get("B_CHOICE_BufferStyle", "") == expected);
}

TEST_CASE("MapXLightsEffects (Element) missing type entry defaults to converting", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000"), "B_CHOICE_BufferStyle=Per Preview") }), DstSpec());
    std::map<std::string, Element*> em;
    std::map<std::string, EffectLayer*> lm;
    MapXLightsEffects(p.D("G"), "A", p.src.ctx->GetSequenceElements(), em, lm, p.m.mapped, false, p.m.pkg, false, p.m.mapping, true, p.m.types);
    CHECK(p.DL("G")->GetEffect(0)->GetSettings().Get("B_CHOICE_BufferStyle", "") == "Per Model Per Preview");
}

TEST_CASE("MapXLightsEffects (Element) maps the same source onto several targets", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }), DstSpec());
    std::map<std::string, Element*> em;
    std::map<std::string, EffectLayer*> lm;
    for (const char* t : { "X", "Y", "G" }) {
        MapXLightsEffects(p.D(t), "A", p.src.ctx->GetSequenceElements(), em, lm, p.m.mapped, false, p.m.pkg, false, p.m.mapping, true, p.m.types);
    }
    CHECK(p.DL("X")->GetEffectCount() == 1);
    CHECK(p.DL("Y")->GetEffectCount() == 1);
    CHECK(p.DL("G")->GetEffectCount() == 1);
    CHECK(p.m.mapped.size() == 3);
}

TEST_CASE("MapXLightsEffects (Element) with many layers", "[impexp2][mapper]") {
    std::vector<EffectSpec> v;
    for (int l = 0; l < 25; ++l) {
        v.push_back(Eff("A", l, "On", l * 100, l * 100 + 100, Pal("#FF0000")));
    }
    Pair p(SrcSpec(v), DstSpec());
    std::map<std::string, Element*> em;
    std::map<std::string, EffectLayer*> lm;
    MapXLightsEffects(p.D("X"), "A", p.src.ctx->GetSequenceElements(), em, lm, p.m.mapped, false, p.m.pkg, false, p.m.mapping, false, p.m.types);
    REQUIRE(p.D("X")->GetEffectLayerCount() == 25);
    for (int l = 0; l < 25; ++l) {
        CHECK(Contents(p.DL("X", l)) == Tup{ { "On", l * 100, l * 100 + 100 } });
    }
}

TEST_CASE("MapXLightsEffects (Element) source with an empty layer between used layers", "[impexp2][mapper]") {
    Pair p(SrcSpec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")), Eff("A", 2, "Bars", 0, 1000, Pal("#FF0000")) }), DstSpec());
    std::map<std::string, Element*> em;
    std::map<std::string, EffectLayer*> lm;
    MapXLightsEffects(p.D("X"), "A", p.src.ctx->GetSequenceElements(), em, lm, p.m.mapped, false, p.m.pkg, false, p.m.mapping, false, p.m.types);
    REQUIRE(p.D("X")->GetEffectLayerCount() == 3);
    CHECK(p.DL("X", 1)->GetEffectCount() == 0);
    CHECK(Contents(p.DL("X", 2)) == Tup{ { "Bars", 0, 1000 } });
}

TEST_CASE("MapXLightsEffects within one sequence between two models", "[impexp2][mapper]") {
    ShowSpec s;
    s.models = SingleLine("A", 1, 4) + SingleLine("B", 100, 4);
    s.modelElements = { "A", "B" };
    s.effects = { Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000")) };
    Show show(s);
    SequencePackage pkg(show.dir.File("x.xsq"), show.dir.path.string(), "x.xsq", &show.ctx->AllModels);
    auto& se = show.ctx->GetSequenceElements();
    std::vector<EffectLayer*> mapped;
    std::map<std::string, std::string> mapping, types;
    MapXLightsEffects(se.GetElement("B")->GetEffectLayer(0), se.GetElement("A")->GetEffectLayer(0), mapped, false, pkg, false, mapping, false, types);
    CHECK(se.GetElement("B")->GetEffectLayer(0)->GetEffectCount() == 1);
    CHECK(se.GetElement("A")->GetEffectLayer(0)->GetEffectCount() == 1);
}
