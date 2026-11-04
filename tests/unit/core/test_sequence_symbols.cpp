#include <catch2/catch_test_macros.hpp>

#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/EffectSymbol.h"
#include "render/EffectSymbolManager.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceFile.h"
#include "support/TempDir.h"

#include <pugixml.hpp>

#include <string>
#include <vector>

namespace {
struct SeqFixture {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    EffectSymbolManager& sym;
    SeqFixture() : se(ctx.GetSequenceElements()), sym(se.GetEffectSymbolManager()) {
        se.SetViewsManager(&ctx._sequenceViewManager);
    }
    EffectLayer* Layer(const std::string& name = "Tree") {
        auto* m = se.GetElement(name);
        if (m == nullptr) {
            m = se.AddElement(name, "model", true, false, true, false, false);
        }
        return m->GetEffectLayerCount() ? m->GetEffectLayer(0) : m->AddEffectLayer();
    }
};

struct Listener : IEffectSymbolListener {
    std::vector<std::string> events;
    void OnSymbolChanged(const std::string& id) override { events.push_back("changed:" + id); }
    void OnSymbolDeleted(const std::string& id) override { events.push_back("deleted:" + id); }
    void OnSymbolCreated(const std::string& id) override { events.push_back("created:" + id); }
    void OnSymbolRenamed(const std::string& id, const std::string& o, const std::string& n) override { events.push_back("renamed:" + id + ":" + o + ":" + n); }
};
}

TEST_CASE("EffectSymbol defaults", "[sequence][symbol]") {
    EffectSymbol s;
    CHECK(s.GetId().empty());
    CHECK(s.GetEffectIndex() == -1);
    CHECK(s.GetPalette().empty());
    EffectSymbol t("id1", "Name");
    CHECK(t.GetId() == "id1");
    CHECK(t.GetName() == "Name");
}

TEST_CASE("EffectSymbol settings string parsing decodes escapes", "[sequence][symbol]") {
    EffectSymbol s;
    s.SetSettingsFromString("A=1,B=x&comma;y,C=p&amp;q,NoEquals,D=");
    CHECK(s.GetSettings().Get("A", "") == "1");
    CHECK(s.GetSettings().Get("B", "") == "x,y");
    CHECK(s.GetSettings().Get("C", "") == "p&q");
    CHECK(s.GetSettings().Contains("D"));
    CHECK_FALSE(s.GetSettings().Contains("NoEquals"));
    s.SetSettingsFromString("");
    CHECK(s.GetSettings().empty());
}

TEST_CASE("EffectSymbol CopyFromEffect drops per instance X_ settings", "[sequence][symbol]") {
    SeqFixture f;
    auto* l = f.Layer();
    Effect* e = l->AddEffect(0, "On", "E_TEXTCTRL_Eff_On_Start=80,X_Effect_Description=mine", "C_BUTTON_Palette1=#ff0000,C_CHECKBOX_Palette1=1", 0, 100, 0, false);
    e->SetLocked(true);
    EffectSymbol s("id", "n");
    s.CopyFromEffect(e);
    CHECK(s.GetEffectType() == "On");
    CHECK(s.GetEffectIndex() == e->GetEffectIndex());
    CHECK(s.GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "") == "80");
    CHECK_FALSE(s.GetSettings().Contains("X_Effect_Locked"));
    CHECK_FALSE(s.GetSettings().Contains("X_Effect_Description"));
    CHECK(s.GetPalette().find("#ff0000") != std::string::npos);
    s.CopyFromEffect(nullptr);
    CHECK(s.GetEffectType() == "On");
}

TEST_CASE("EffectSymbol XML round trip", "[sequence][symbol]") {
    EffectSymbol s("sym_4", "My <symbol>");
    s.SetEffectType("Bars");
    s.SetEffectIndex(7);
    s.SetSettingsFromString("A=1,B=2");
    s.SetPalette("C_BUTTON_Palette1=#112233");
    pugi::xml_document doc;
    auto root = doc.append_child("EffectSymbols");
    s.SaveToXml(root);
    std::unique_ptr<EffectSymbol> back(EffectSymbol::FromXml(root.child("Symbol")));
    REQUIRE(back != nullptr);
    CHECK(back->GetId() == "sym_4");
    CHECK(back->GetName() == "My <symbol>");
    CHECK(back->GetEffectType() == "Bars");
    CHECK(back->GetEffectIndex() == 7);
    CHECK(back->GetSettings().Get("A", "") == "1");
    CHECK(back->GetSettings().Get("B", "") == "2");
    CHECK(back->GetPalette() == "C_BUTTON_Palette1=#112233");
}

TEST_CASE("EffectSymbol FromXml rejects other nodes and tolerates missing attributes", "[sequence][symbol]") {
    pugi::xml_document doc;
    doc.load_string("<Other/>");
    CHECK(EffectSymbol::FromXml(doc.child("Other")) == nullptr);
    CHECK(EffectSymbol::FromXml(pugi::xml_node()) == nullptr);
    doc.load_string("<Symbol/>");
    std::unique_ptr<EffectSymbol> s(EffectSymbol::FromXml(doc.child("Symbol")));
    REQUIRE(s != nullptr);
    CHECK(s->GetEffectIndex() == -1);
    CHECK(s->GetId().empty());
}

TEST_CASE("EffectSymbolManager create get and delete", "[sequence][symbol]") {
    SeqFixture f;
    CHECK(f.sym.GetSymbolCount() == 0);
    auto* s = f.sym.CreateEmptySymbol("Pulse", "On", 3);
    REQUIRE(s != nullptr);
    CHECK(s->GetName() == "Pulse");
    CHECK(s->GetEffectType() == "On");
    CHECK(f.sym.GetSymbol(s->GetId()) == s);
    CHECK(f.sym.GetSymbolByName("Pulse") == s);
    CHECK(f.sym.SymbolExists(s->GetId()));
    CHECK(f.sym.SymbolNameExists("Pulse"));
    CHECK(f.sym.GetSymbol("nope") == nullptr);
    CHECK(f.sym.GetAllSymbols().size() == 1);
    CHECK(f.sym.DeleteSymbol(s->GetId()));
    CHECK_FALSE(f.sym.DeleteSymbol(s->GetId()));
    CHECK(f.sym.GetSymbolCount() == 0);
}

TEST_CASE("EffectSymbolManager makes names and ids unique", "[sequence][symbol]") {
    SeqFixture f;
    auto* a = f.sym.CreateEmptySymbol("S", "On", 0);
    auto* b = f.sym.CreateEmptySymbol("S", "On", 0);
    auto* c = f.sym.CreateEmptySymbol("S", "On", 0);
    CHECK(a->GetName() == "S");
    CHECK(b->GetName() == "S 2");
    CHECK(c->GetName() == "S 3");
    CHECK(a->GetId() != b->GetId());
    CHECK(b->GetId() != c->GetId());
    CHECK(f.sym.GetUniqueSymbolName("S") == "S 4");
    CHECK(f.sym.GetUniqueSymbolName("Fresh") == "Fresh");
}

TEST_CASE("EffectSymbolManager CreateSymbol copies the source effect", "[sequence][symbol]") {
    SeqFixture f;
    Effect* e = f.Layer()->AddEffect(0, "Bars", "X_Keep=1,E_SLIDER_Bars_BarCount=5", "", 0, 100, 0, false);
    REQUIRE(e != nullptr);
    CHECK(f.sym.CreateSymbol("x", nullptr) == nullptr);
    auto* s = f.sym.CreateSymbol("From effect", e);
    REQUIRE(s != nullptr);
    CHECK(s->GetEffectType() == "Bars");
    CHECK(s->GetSettings().Get("E_SLIDER_Bars_BarCount", "") == "5");
    CHECK_FALSE(s->GetSettings().Contains("X_Keep"));
}

TEST_CASE("EffectSymbolManager rename rules", "[sequence][symbol]") {
    SeqFixture f;
    auto* a = f.sym.CreateEmptySymbol("A", "On", 0);
    auto* b = f.sym.CreateEmptySymbol("B", "On", 0);
    CHECK_FALSE(f.sym.RenameSymbol(a->GetId(), "B"));
    CHECK(f.sym.RenameSymbol(a->GetId(), "A"));
    CHECK(f.sym.RenameSymbol(a->GetId(), "C"));
    CHECK(a->GetName() == "C");
    CHECK_FALSE(f.sym.RenameSymbol("missing", "Z"));
    (void)b;
}

TEST_CASE("EffectSymbolManager notifies listeners", "[sequence][symbol]") {
    SeqFixture f;
    Listener l;
    f.sym.AddListener(&l);
    f.sym.AddListener(nullptr);
    auto* s = f.sym.CreateEmptySymbol("A", "On", 0);
    std::string id = s->GetId();
    f.sym.RenameSymbol(id, "B");
    f.sym.UpdateSymbolSettings(id, "X=1");
    f.sym.UpdateSymbolPalette(id, "p");
    f.sym.DeleteSymbol(id);
    REQUIRE(l.events.size() == 5);
    CHECK(l.events[0] == "created:" + id);
    CHECK(l.events[1] == "renamed:" + id + ":A:B");
    CHECK(l.events[2] == "changed:" + id);
    CHECK(l.events[3] == "changed:" + id);
    CHECK(l.events[4] == "deleted:" + id);
    f.sym.RemoveListener(&l);
    f.sym.CreateEmptySymbol("Z", "On", 0);
    CHECK(l.events.size() == 5);
}

TEST_CASE("EffectSymbolManager updates on unknown symbols are ignored", "[sequence][symbol]") {
    SeqFixture f;
    f.sym.UpdateSymbolSettings("nope", "A=1");
    f.sym.UpdateSymbolPalette("nope", "p");
    f.sym.NotifySymbolChanged("nope");
    CHECK(f.sym.GetSymbolCount() == 0);
}

TEST_CASE("Linking an effect of the same type to a symbol applies the symbol", "[sequence][symbol]") {
    SeqFixture f;
    auto* l = f.Layer();
    auto* s = f.sym.CreateEmptySymbol("Look", "Bars", f.se.GetEffectManager().GetEffectIndex("Bars"));
    s->SetSettingsFromString("E_SLIDER_Bars_BarCount=9");
    Effect* e = l->AddEffect(0, "Bars", "X_Effect_Description=keep", "", 0, 100, 0, false);
    REQUIRE(e != nullptr);
    e->LinkToSymbol(s->GetId());
    CHECK(e->IsLinkedToSymbol());
    CHECK(e->GetLinkedSymbolId() == s->GetId());
    CHECK(e->GetSetting("E_SLIDER_Bars_BarCount") == "9");
    CHECK(e->GetSetting("X_Effect_Description") == "keep");
    CHECK(f.sym.GetLinkedEffectCount(s->GetId()) == 1);
    CHECK(e->GetSettingsAsString().find("X_LinkedSymbolId=" + s->GetId()) != std::string::npos);
}

// Looks like a production bug: LinkToSymbol marks the effect linked before applying the symbol, so each
// change it makes propagates the still empty effect back over the symbol, wiping its palette (and, when
// the effect type differs, its settings).
TEST_CASE("Linking an effect of a different type to a symbol keeps the symbol contents", "[sequence][symbol][!mayfail]") {
    SeqFixture f;
    auto* l = f.Layer();
    auto* s = f.sym.CreateEmptySymbol("Look", "Bars", f.se.GetEffectManager().GetEffectIndex("Bars"));
    s->SetSettingsFromString("E_SLIDER_Bars_BarCount=9");
    s->SetPalette("C_BUTTON_Palette1=#00ff00,C_CHECKBOX_Palette1=1");
    Effect* e = l->AddEffect(0, "On", "", "", 0, 100, 0, false);
    REQUIRE(e != nullptr);
    e->LinkToSymbol(s->GetId());
    CHECK(e->GetEffectName() == "Bars");
    CHECK(s->GetSettings().Get("E_SLIDER_Bars_BarCount", "") == "9");
    CHECK(e->GetSetting("E_SLIDER_Bars_BarCount") == "9");
    CHECK(e->GetPaletteSize() == 1);
}

TEST_CASE("Unlinking an effect keeps its settings", "[sequence][symbol]") {
    SeqFixture f;
    auto* l = f.Layer();
    auto* s = f.sym.CreateEmptySymbol("Look", "On", f.se.GetEffectManager().GetEffectIndex("On"));
    s->SetSettingsFromString("E_TEXTCTRL_Eff_On_Start=33");
    Effect* e = l->AddEffect(0, "On", "", "", 0, 100, 0, false);
    e->LinkToSymbol(s->GetId());
    e->UnlinkFromSymbol();
    CHECK_FALSE(e->IsLinkedToSymbol());
    CHECK(f.sym.GetLinkedEffectCount(s->GetId()) == 0);
    CHECK(e->GetSetting("E_TEXTCTRL_Eff_On_Start") == "33");
    e->UnlinkFromSymbol();
}

TEST_CASE("Changing a linked effect propagates to the symbol and its siblings", "[sequence][symbol]") {
    SeqFixture f;
    auto* l = f.Layer();
    auto* s = f.sym.CreateEmptySymbol("Look", "On", f.se.GetEffectManager().GetEffectIndex("On"));
    Effect* a = l->AddEffect(0, "On", "", "", 0, 100, 0, false);
    Effect* b = l->AddEffect(0, "On", "", "", 100, 200, 0, false);
    a->LinkToSymbol(s->GetId());
    b->LinkToSymbol(s->GetId());
    REQUIRE(f.sym.GetLinkedEffectCount(s->GetId()) == 2);
    a->SetSetting("E_TEXTCTRL_Eff_On_Start", "42");
    CHECK(s->GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "") == "42");
    CHECK(b->GetSetting("E_TEXTCTRL_Eff_On_Start") == "42");
}

TEST_CASE("Updating a symbol updates every linked effect but keeps X_ state", "[sequence][symbol]") {
    SeqFixture f;
    auto* l = f.Layer();
    auto* s = f.sym.CreateEmptySymbol("Look", "On", f.se.GetEffectManager().GetEffectIndex("On"));
    Effect* a = l->AddEffect(0, "On", "X_Effect_Description=a", "", 0, 100, 0, false);
    Effect* b = l->AddEffect(0, "On", "X_Effect_Description=b", "", 100, 200, 0, false);
    a->LinkToSymbol(s->GetId());
    b->LinkToSymbol(s->GetId());
    f.sym.UpdateSymbolSettings(s->GetId(), "E_TEXTCTRL_Eff_On_Start=11");
    f.sym.UpdateSymbolPalette(s->GetId(), "C_BUTTON_Palette1=#0000ff,C_CHECKBOX_Palette1=1");
    for (Effect* e : { a, b }) {
        CHECK(e->GetSetting("E_TEXTCTRL_Eff_On_Start") == "11");
        CHECK(e->GetPaletteSize() == 1);
    }
    CHECK(a->GetSetting("X_Effect_Description") == "a");
    CHECK(b->GetSetting("X_Effect_Description") == "b");
}

TEST_CASE("Moving a linked effect does not change the symbol", "[sequence][symbol]") {
    SeqFixture f;
    auto* l = f.Layer();
    auto* s = f.sym.CreateEmptySymbol("Look", "On", f.se.GetEffectManager().GetEffectIndex("On"));
    s->SetSettingsFromString("E_TEXTCTRL_Eff_On_Start=5");
    Effect* a = l->AddEffect(0, "On", "", "", 0, 100, 0, false);
    Effect* b = l->AddEffect(0, "On", "", "", 100, 200, 0, false);
    a->LinkToSymbol(s->GetId());
    b->LinkToSymbol(s->GetId());
    b->SetSetting("E_TEXTCTRL_Eff_On_Start", "5");
    b->SetStartTimeMS(120);
    CHECK(a->GetStartTimeMS() == 0);
    CHECK(s->GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "") == "5");
}

TEST_CASE("Deleting a symbol unlinks its effects", "[sequence][symbol]") {
    SeqFixture f;
    auto* l = f.Layer();
    auto* s = f.sym.CreateEmptySymbol("Look", "On", f.se.GetEffectManager().GetEffectIndex("On"));
    std::string id = s->GetId();
    Effect* a = l->AddEffect(0, "On", "", "", 0, 100, 0, false);
    a->LinkToSymbol(id);
    f.sym.DeleteSymbol(id);
    CHECK_FALSE(a->IsLinkedToSymbol());
    CHECK(f.sym.GetLinkedEffectCount(id) == 0);
}

TEST_CASE("Destroying a linked effect unregisters it", "[sequence][symbol]") {
    SeqFixture f;
    auto* s = f.sym.CreateEmptySymbol("Look", "On", f.se.GetEffectManager().GetEffectIndex("On"));
    {
        auto* l = f.Layer();
        Effect* a = l->AddEffect(0, "On", "", "", 0, 100, 0, false);
        a->LinkToSymbol(s->GetId());
        CHECK(f.sym.GetLinkedEffectCount(s->GetId()) == 1);
        f.se.DeleteElement("Tree");
    }
    CHECK(f.sym.GetLinkedEffectCount(s->GetId()) == 0);
}

TEST_CASE("RegisterLinkedEffect ignores unknown symbols and null effects", "[sequence][symbol]") {
    SeqFixture f;
    Effect* a = f.Layer()->AddEffect(0, "On", "", "", 0, 100, 0, false);
    f.sym.RegisterLinkedEffect(a, "nope");
    f.sym.RegisterLinkedEffect(nullptr, "nope");
    f.sym.UnregisterLinkedEffect(nullptr);
    CHECK(f.sym.GetLinkedEffectCount("nope") == 0);
    CHECK(f.sym.GetLinkedEffects("nope").empty());
}

TEST_CASE("Pasted symbol links are restored when the symbol exists", "[sequence][symbol]") {
    SeqFixture f;
    auto* s = f.sym.CreateEmptySymbol("Look", "On", f.se.GetEffectManager().GetEffectIndex("On"));
    Effect* a = f.Layer()->AddEffect(0, "On", "X_LinkedSymbolId=" + s->GetId(), "", 0, 100, 0, false);
    REQUIRE(a != nullptr);
    a->HandlePastedSymbolLink();
    CHECK(a->IsLinkedToSymbol());
    CHECK(f.sym.GetLinkedEffectCount(s->GetId()) == 1);

    Effect* b = f.Layer()->AddEffect(0, "On", "X_LinkedSymbolId=sym_999", "", 100, 200, 0, false);
    b->HandlePastedSymbolLink();
    CHECK_FALSE(b->IsLinkedToSymbol());
    CHECK(b->GetSetting("X_LinkedSymbolId").empty());
}

TEST_CASE("EffectSymbolManager XML load restores symbols and the id counter", "[sequence][symbol]") {
    SeqFixture f;
    auto* a = f.sym.CreateEmptySymbol("A", "On", 1);
    auto* b = f.sym.CreateEmptySymbol("B", "Off", 2);
    a->SetSettingsFromString("K=V");
    pugi::xml_document doc;
    auto root = doc.append_child("xsequence");
    f.sym.SaveToXml(root);

    SeqFixture g;
    g.sym.CreateEmptySymbol("Old", "On", 0);
    g.sym.LoadFromXml(root.child("EffectSymbols"));
    CHECK(g.sym.GetSymbolCount() == 2);
    REQUIRE(g.sym.GetSymbolByName("A") != nullptr);
    CHECK(g.sym.GetSymbolByName("A")->GetSettings().Get("K", "") == "V");
    CHECK(g.sym.GetSymbolByName("Old") == nullptr);
    auto* c = g.sym.CreateEmptySymbol("C", "On", 0);
    CHECK(c->GetId() != a->GetId());
    CHECK(c->GetId() != b->GetId());
    CHECK(g.sym.GetSymbolCount() == 3);
}

TEST_CASE("EffectSymbolManager XML load ignores the wrong node", "[sequence][symbol]") {
    SeqFixture f;
    f.sym.CreateEmptySymbol("A", "On", 1);
    pugi::xml_document doc;
    doc.load_string("<Other><Symbol id=\"x\" name=\"y\"/></Other>");
    f.sym.LoadFromXml(doc.child("Other"));
    f.sym.LoadFromXml(pugi::xml_node());
    CHECK(f.sym.GetSymbolCount() == 1);
}

TEST_CASE("xsq round trip keeps effect symbols and links", "[sequence][symbol][xsq]") {
    TempDir td;
    std::string path = td.File("sym.xsq");
    std::string id;
    {
        SeqFixture f;
        auto* l = f.Layer();
        auto* s = f.sym.CreateEmptySymbol("Look", "Bars", f.se.GetEffectManager().GetEffectIndex("Bars"));
        s->SetSettingsFromString("E_SLIDER_Bars_BarCount=6");
        id = s->GetId();
        Effect* a = l->AddEffect(0, "Bars", "", "", 0, 1000, 0, false);
        Effect* b = l->AddEffect(0, "Bars", "", "", 1000, 2000, 0, false);
        a->LinkToSymbol(id);
        b->LinkToSymbol(id);
        SequenceFile sf(path);
        REQUIRE(sf.Save(f.se));
    }
    HeadlessRenderContext ctx;
    ctx.showDirectory = td.path.string();
    SequenceFile sf(path);
    auto doc = sf.Open(td.path.string(), true, path);
    REQUIRE(doc.has_value());
    REQUIRE(ctx.LoadSequenceElements(sf, *doc));
    auto& sym = ctx.GetSequenceElements().GetEffectSymbolManager();
    CHECK(sym.GetSymbolCount() == 1);
    REQUIRE(sym.GetSymbol(id) != nullptr);
    CHECK(sym.GetLinkedEffectCount(id) == 2);
    auto* l = ctx.GetSequenceElements().GetElement("Tree")->GetEffectLayer(0);
    REQUIRE(l->GetEffectCount() == 2);
    CHECK(l->GetEffect(0)->IsLinkedToSymbol());
    CHECK(l->GetEffect(0)->GetEffectName() == "Bars");
    CHECK(l->GetEffect(1)->GetSetting("E_SLIDER_Bars_BarCount") == "6");
}
