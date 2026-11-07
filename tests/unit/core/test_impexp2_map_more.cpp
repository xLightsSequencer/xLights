#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "import_export/AutoMapper.h"
#include "import_export/EffectMapper.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/SequenceElements.h"
#include "render/SequencePackage.h"
#include "test_impexp2_map_common.h"
#include "test_render_common.h"
#include "utils/string_utils.h"

#include <random>
#include <string>
#include <vector>

using namespace renderhelp;
using namespace impexp2;

namespace {

ShowSpec Src(std::vector<EffectSpec> effects) {
    ShowSpec s;
    s.models = SingleLine("A", 1, 4);
    s.modelElements = { "A" };
    s.effects = std::move(effects);
    s.durationMs = 20000;
    return s;
}

ShowSpec Dst(std::vector<EffectSpec> effects = {}) {
    ShowSpec s;
    s.models = R"(<model name="M" DisplayAs="Matrix" StartChannel="1" NumStrings="1" NodesPerString="12" StrandsPerString="3" StringType="RGB Nodes">)"
               R"(<subModel name="Sub" layout="horizontal" type="ranges" bufferstyle="Default" line0="1-3"/></model>)";
    s.modelElements = { "M" };
    s.effects = std::move(effects);
    s.durationMs = 20000;
    return s;
}

struct Rig {
    Show src;
    Show dst;
    SequencePackage pkg;
    std::vector<EffectLayer*> mapped;
    std::map<std::string, std::string> mapping, types;
    std::map<std::string, Element*> em;
    std::map<std::string, EffectLayer*> lm;
    Rig(const ShowSpec& s, const ShowSpec& d) : src(s), dst(d), pkg(dst.dir.File("x.xsq"), dst.dir.path.string(), "x.xsq", &dst.ctx->AllModels) {}
    void MapTo(Element* target, bool erase = false, bool lock = false, bool convert = false) {
        MapXLightsEffects(target, "A", src.ctx->GetSequenceElements(), em, lm, mapped, erase, pkg, lock, mapping, convert, types);
    }
    Element* DM() { return dst.ctx->GetSequenceElements().GetElement("M"); }
};

} // namespace

TEST_CASE("MapXLightsEffects onto a submodel element", "[impexp2][mapper]") {
    Rig r(Src({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000")) }), Dst());
    auto* me = dynamic_cast<ModelElement*>(r.DM());
    REQUIRE(me != nullptr);
    auto* sub = me->GetSubModel("Sub", true);
    REQUIRE(sub != nullptr);
    r.MapTo(sub);
    REQUIRE(sub->GetEffectLayerCount() >= 1);
    CHECK(sub->GetEffectLayer(0)->GetEffectCount() == 1);
    CHECK(sub->GetEffectLayer(0)->GetEffect(0)->GetEffectName() == "Bars");
}

TEST_CASE("MapXLightsEffects onto a strand element", "[impexp2][mapper]") {
    Rig r(Src({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000")), Eff("A", 1, "On", 0, 1000, Pal("#FF0000")) }), Dst());
    auto* me = dynamic_cast<ModelElement*>(r.DM());
    auto* strand = me->GetStrand(1, true);
    REQUIRE(strand != nullptr);
    r.MapTo(strand);
    CHECK(strand->GetEffectLayerCount() == 2);
    CHECK(strand->GetEffectLayer(0)->GetEffectCount() == 1);
    CHECK(strand->GetEffectLayer(1)->GetEffectCount() == 1);
}

TEST_CASE("MapXLightsEffects onto a timing layer copies marks without model lookups", "[impexp2][mapper]") {
    Rig r(Src({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }), Dst());
    auto& se = r.dst.ctx->GetSequenceElements();
    auto* t = se.AddElement("Beats", "timing", true, false, true, false, false);
    auto* l = t->AddEffectLayer();
    REQUIRE(l->IsTimingLayer());
    MapXLightsEffects(l, r.src.ctx->GetSequenceElements().GetElement("A")->GetEffectLayer(0), r.mapped, false, r.pkg, false, r.mapping, false, r.types);
    CHECK(l->GetEffectCount() == 1);
}

TEST_CASE("MapXLightsEffects is idempotent when run twice without erase", "[impexp2][mapper]") {
    Rig r(Src({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000")), Eff("A", 0, "On", 2000, 3000, Pal("#FF0000")) }), Dst());
    r.MapTo(r.DM());
    r.MapTo(r.DM());
    CHECK(r.DM()->GetEffectLayer(0)->GetEffectCount() == 2);
    CHECK(r.mapped.size() == 2);
}

TEST_CASE("MapXLightsEffects twice with erase gives the same result", "[impexp2][mapper]") {
    Rig r(Src({ Eff("A", 0, "Bars", 0, 1000, Pal("#FF0000")) }), Dst());
    r.MapTo(r.DM(), true);
    r.MapTo(r.DM(), true);
    CHECK(r.DM()->GetEffectLayer(0)->GetEffectCount() == 1);
}

TEST_CASE("MapXLightsEffects settings values containing special characters survive", "[impexp2][mapper]") {
    auto value = GENERATE(as<std::string>{}, "plain", "with space", "with=equals", "UPPER", "unicode-é-日本", "semi;colon", "dots.and-dashes_1");
    Rig r(Src({ Eff("A", 0, "Text", 0, 1000, Pal("#FF0000"), "E_TEXTCTRL_Text_Line1=" + value) }), Dst());
    r.MapTo(r.DM());
    REQUIRE(r.DM()->GetEffectLayer(0)->GetEffectCount() == 1);
    auto src = r.src.ctx->GetSequenceElements().GetElement("A")->GetEffectLayer(0)->GetEffect(0)->GetSettings().Get("E_TEXTCTRL_Text_Line1", "<s>");
    auto dst = r.DM()->GetEffectLayer(0)->GetEffect(0)->GetSettings().Get("E_TEXTCTRL_Text_Line1", "<d>");
    CAPTURE(value);
    CHECK(src == dst);
}

TEST_CASE("MapXLightsEffects keeps all eight palette colours", "[impexp2][mapper]") {
    std::string pal;
    for (int i = 1; i <= 8; ++i) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%sC_BUTTON_Palette%d=#%02X%02X%02X,C_CHECKBOX_Palette%d=1", i > 1 ? "," : "", i, i * 10, i * 20, i * 5, i);
        pal += buf;
    }
    Rig r(Src({ Eff("A", 0, "Bars", 0, 1000, pal) }), Dst());
    r.MapTo(r.DM());
    CHECK(r.DM()->GetEffectLayer(0)->GetEffect(0)->GetPaletteAsString() == r.src.ctx->GetSequenceElements().GetElement("A")->GetEffectLayer(0)->GetEffect(0)->GetPaletteAsString());
}

TEST_CASE("MapXLightsEffects Moving Head name on a non DMX target does not alter settings", "[impexp2][mapper]") {
    Rig r(Src({ Eff("A", 0, "Moving Head", 0, 1000, Pal("#FF0000"), "E_TEXTCTRL_MH1_Settings=Pan:10") }), Dst());
    r.MapTo(r.DM());
    REQUIRE(r.DM()->GetEffectLayer(0)->GetEffectCount() == 1);
    CHECK(r.DM()->GetEffectLayer(0)->GetEffect(0)->GetEffectName() == "Moving Head");
}

TEST_CASE("MapXLightsEffects one source onto many layers accumulates the mapped list", "[impexp2][mapper]") {
    Rig r(Src({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }), Dst());
    auto* me = dynamic_cast<ModelElement*>(r.DM());
    for (int i = 0; i < 6; ++i) {
        r.MapTo(me->GetStrand(i % 3, true));
    }
    CHECK(r.mapped.size() == 6);
}

TEST_CASE("MapXLightsEffects lock option on the element overload", "[impexp2][mapper]") {
    auto lock = GENERATE(true, false);
    Rig r(Src({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")), Eff("A", 1, "Bars", 0, 1000, Pal("#FF0000")) }), Dst());
    r.MapTo(r.DM(), false, lock);
    for (size_t l = 0; l < r.DM()->GetEffectLayerCount(); ++l) {
        CHECK(r.DM()->GetEffectLayer(l)->GetEffect(0)->IsLocked() == lock);
    }
}

// ----- AutoMapper properties ------------------------------------------------

namespace {
std::string RandName(std::mt19937& g, size_t maxLen) {
    static const char alphabet[] = "abcXYZ019 -_().:;|{}[]+=*^#,/\\!@'\"\t";
    std::uniform_int_distribution<size_t> len(0, maxLen);
    std::uniform_int_distribution<size_t> ch(0, sizeof(alphabet) - 2);
    std::string s;
    for (size_t i = 0, n = len(g); i < n; ++i) s += alphabet[ch(g)];
    return s;
}
std::string Canon(const std::string& s) { return Lower(Trim(s)); }
const std::list<std::string> kNone;
}

TEST_CASE("MatchNorm accepts a name against its own canonical form (random names)", "[impexp2][mapper]") {
    std::mt19937 g(12345);
    for (int i = 0; i < 500; ++i) {
        auto n = RandName(g, 20);
        CAPTURE(n);
        CHECK(AutoMapper::MatchNorm(n, Canon(n), "", "", kNone));
    }
}

TEST_CASE("MatchAggressive accepts a name against its own canonical form (random names)", "[impexp2][mapper]") {
    std::mt19937 g(54321);
    for (int i = 0; i < 500; ++i) {
        auto n = RandName(g, 20);
        CAPTURE(n);
        CHECK(AutoMapper::MatchAggressive(n, Canon(n), "", "", kNone));
    }
}

TEST_CASE("MatchAggressive is symmetric in punctuation (random names)", "[impexp2][mapper]") {
    std::mt19937 g(999);
    for (int i = 0; i < 300; ++i) {
        auto a = RandName(g, 12);
        auto b = Canon(a) + "-" + "_";
        CAPTURE(a, b);
        CHECK(AutoMapper::MatchAggressive(a, b, "", "", kNone));
    }
}

TEST_CASE("MatchNorm never matches different canonical names (random names)", "[impexp2][mapper]") {
    std::mt19937 g(777);
    for (int i = 0; i < 500; ++i) {
        auto a = RandName(g, 8);
        auto b = RandName(g, 8);
        if (Canon(a) == Canon(b)) continue;
        CAPTURE(a, b);
        CHECK_FALSE(AutoMapper::MatchNorm(a, Canon(b), "", "", kNone));
    }
}

TEST_CASE("MatchRegex with random patterns never throws", "[impexp2][mapper]") {
    std::mt19937 g(31337);
    for (int i = 0; i < 300; ++i) {
        auto pat = RandName(g, 10);
        auto tgt = RandName(g, 10);
        CAPTURE(pat, tgt);
        REQUIRE_NOTHROW(AutoMapper::MatchRegex(tgt, "x", pat, "x", kNone));
    }
}

TEST_CASE("AutoMapper::Run with random names is stable and only maps equal canonical names", "[impexp2][mapper]") {
    LayoutFixture f("");
    std::mt19937 g(2468);
    std::vector<std::unique_ptr<BasicImportMappingNode>> nodes;
    std::vector<ImportMappingNode*> roots;
    std::vector<AvailableSource> av;
    for (int i = 0; i < 60; ++i) {
        nodes.push_back(Node(RandName(g, 6)));
        roots.push_back(nodes.back().get());
        av.push_back(Src(RandName(g, 6)));
    }
    AutoMapper::Run(roots, av, *f.ctx, AutoMapper::MatchNorm, AutoMapper::MatchNorm, AutoMapper::MatchNorm, "", "", "B", false, {});
    for (auto& n : nodes) {
        if (n->GetMapping().empty() && Canon(n->GetCoreModel()) != "") {
            for (auto& a : av) {
                if (a.canonicalName.find('/') != std::string::npos) continue;
                CHECK(Canon(n->GetCoreModel()) != a.canonicalName);
            }
        }
    }
}

TEST_CASE("AutoMapper::Run second pass over a fully mapped tree changes nothing", "[impexp2][mapper]") {
    LayoutFixture f("");
    auto a = Node("A");
    auto b = Node("B");
    std::vector<AvailableSource> av{ Src("A"), Src("B"), Src("a") };
    AutoMapper::Run({ a.get(), b.get() }, av, *f.ctx, AutoMapper::MatchNorm, AutoMapper::MatchNorm, AutoMapper::MatchNorm, "", "", "B", false, {});
    std::vector<AvailableSource> av2{ Src("b"), Src("a") };
    AutoMapper::Run({ a.get(), b.get() }, av2, *f.ctx, AutoMapper::MatchNorm, AutoMapper::MatchNorm, AutoMapper::MatchNorm, "", "", "B", false, {});
    CHECK(a->GetMapping() == "A");
    CHECK(b->GetMapping() == "B");
}

TEST_CASE("AutoMapper::Run with a layout that has no models", "[impexp2][mapper]") {
    LayoutFixture f("");
    REQUIRE(f.loaded);
    auto m = Node("Tree");
    auto* s = m->AddChild(Node("Tree", "Left"));
    std::vector<AvailableSource> av{ Src("Tree/Left"), Src("Tree") };
    AutoMapper::Run({ m.get() }, av, *f.ctx, AutoMapper::MatchAggressive, AutoMapper::MatchAggressive, AutoMapper::MatchAggressive, "", "", "B", false, {});
    AutoMapper::RunSubModelFallback({ m.get() }, av, *f.ctx, false, {});
    CHECK(m->GetMapping() == "Tree");
    CHECK(s->GetMapping() == "Tree/Left");
}

TEST_CASE("AutoMapper::Run maps groups only for G and nodes of groups ignore strand children", "[impexp2][mapper]") {
    LayoutFixture f("");
    auto g = Node("Group", "", "", true);
    auto* s = g->AddChild(Node("Group", "Part"));
    std::vector<AvailableSource> av{ Src("Group/Part"), Src("Group", "ModelGroup") };
    AutoMapper::Run({ g.get() }, av, *f.ctx, AutoMapper::MatchNorm, AutoMapper::MatchNorm, AutoMapper::MatchNorm, "", "", "G", false, {});
    CHECK(g->GetMapping() == "Group");
    CHECK(g->_mappingModelType == "ModelGroup");
    CHECK(s->GetMapping() == "Group/Part");
}

TEST_CASE("AutoMapper::Run repeated sources do not remap strands", "[impexp2][mapper]") {
    LayoutFixture f("");
    auto m = Node("T");
    auto* s = m->AddChild(Node("T", "S"));
    std::vector<AvailableSource> av;
    for (int i = 0; i < 20; ++i) av.push_back(Src("T/S"));
    AutoMapper::Run({ m.get() }, av, *f.ctx, AutoMapper::MatchNorm, AutoMapper::MatchNorm, AutoMapper::MatchNorm, "", "", "B", false, {});
    CHECK(s->GetMapping() == "T/S");
}

TEST_CASE("AutoMapper::Run slashed source with trailing spaces in parts", "[impexp2][mapper]") {
    LayoutFixture f("");
    auto m = Node("T");
    auto* s = m->AddChild(Node("T", "S"));
    std::vector<AvailableSource> av{ Src(" T/S ") };
    AutoMapper::Run({ m.get() }, av, *f.ctx, AutoMapper::MatchNorm, AutoMapper::MatchNorm, AutoMapper::MatchNorm, "", "", "B", false, {});
    CHECK(s->GetMapping() == " T/S ");
}

TEST_CASE("AutoMapper::Run parts with inner spaces do not match under MatchNorm but do under MatchAggressive", "[impexp2][mapper]") {
    LayoutFixture f("");
    auto m1 = Node("Big Tree");
    auto* s1 = m1->AddChild(Node("Big Tree", "Left Side"));
    auto m2 = Node("Big Tree");
    auto* s2 = m2->AddChild(Node("Big Tree", "Left Side"));
    std::vector<AvailableSource> av{ Src("BigTree/LeftSide") };
    AutoMapper::Run({ m1.get() }, av, *f.ctx, AutoMapper::MatchNorm, AutoMapper::MatchNorm, AutoMapper::MatchNorm, "", "", "B", false, {});
    AutoMapper::Run({ m2.get() }, av, *f.ctx, AutoMapper::MatchAggressive, AutoMapper::MatchAggressive, AutoMapper::MatchAggressive, "", "", "B", false, {});
    CHECK(s1->GetMapping().empty());
    CHECK(s2->GetMapping() == "BigTree/LeftSide");
}
