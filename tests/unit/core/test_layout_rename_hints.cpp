#include <algorithm>
#include <ctime>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/Model.h"
#include "models/ModelManager.h"
#include "outputs/OutputManager.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/ShowMergeApply.h"
#include "support/TempDir.h"
#include "utils/FileUtils.h"
#include "utils/LayoutRenameHints.h"
#include "utils/LayoutRenameLog.h"
#include "utils/ShowFileMerge.h"
#include "XmlSerializer/StringSerializingVisitor.h"

// Rename hints: the <renames> section of the layout file, its merge, chain
// following, pruning, and the automatic mapping of renamed models when a
// sequence saved before the rename is opened.

namespace {

using Entry = LayoutRenameHints::Entry;
using Outcome = LayoutRenameHints::Outcome;

const std::time_t kNow = 1800000000;

Entry E(const std::string& from, const std::string& to, std::time_t t, const std::string& id = "", const std::string& type = LayoutRenameHints::TYPE_MODEL) {
    return { type, from, to, t, id };
}

LayoutRenameHints Hints(const std::vector<Entry>& entries) {
    LayoutRenameHints h;
    for (const auto& e : entries) {
        h.Add(e);
    }
    return h;
}

std::function<bool(const std::string&)> ExistsIn(std::vector<std::string> names) {
    return [names](const std::string& n) { return std::find(names.begin(), names.end(), n) != names.end(); };
}

struct FixFileReset {
    ~FixFileReset() {
        FileUtils::SetFixFileShowDir("");
        FileUtils::SetFixFileDirectories({});
        FileUtils::ClearNonExistentFiles();
    }
};

std::string Line(const std::string& name, int start) {
    return "<model name=\"" + name + "\" DisplayAs=\"Single Line\" StartChannel=\"" + std::to_string(start) +
           "\" NumStrings=\"1\" NodesPerString=\"10\" StringType=\"RGB Nodes\"/>";
}

std::string Rgb(const std::vector<std::string>& models, const std::string& renames) {
    std::string s = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb>\n<models>\n";
    int start = 1;
    for (const auto& m : models) {
        s += Line(m, start) + "\n";
        start += 30;
    }
    s += "</models>\n<modelGroups/>\n" + renames + "</xrgb>\n";
    return s;
}

// A sequence with one On effect on each named row.
std::string Xsq(const std::vector<std::string>& rows) {
    std::string display, effects;
    for (const auto& r : rows) {
        display += "<Element collapsed=\"0\" type=\"model\" name=\"" + r + "\" visible=\"1\"/>";
        effects += "<Element type=\"model\" name=\"" + r + "\"><EffectLayer><Effect ref=\"0\" name=\"On\" startTime=\"0\" endTime=\"500\" palette=\"0\"/></EffectLayer></Element>";
    }
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\" FixedPointTiming=\"1\" ModelBlending=\"true\">"
           "<head><version>2026.20</version><mediaFile></mediaFile><sequenceTiming>50 ms</sequenceTiming><sequenceType>Animation</sequenceType><sequenceDuration>0.500</sequenceDuration></head>"
           "<DisplayElements>" + display + "</DisplayElements>"
           "<EffectDB><Effect>E_TEXTCTRL_Eff_On_Start=100</Effect></EffectDB><ColorPalettes><ColorPalette>C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1</ColorPalette></ColorPalettes>"
           "<ElementEffects>" + effects + "</ElementEffects></xsequence>";
}

std::string RenamesXml(const std::vector<Entry>& entries) {
    LayoutRenameHints h = Hints(entries);
    pugi::xml_document d;
    pugi::xml_node root = d.append_child("xrgb");
    h.WriteTo(root);
    std::ostringstream os;
    root.child(LayoutRenameHints::SECTION).print(os, "", pugi::format_raw);
    return os.str();
}

struct OpenedShow {
    TempDir dir;
    FixFileReset reset;
    HeadlessRenderContext ctx;

    OpenedShow(const std::vector<std::string>& models, const std::string& renames, const std::vector<std::string>& rows) {
        dir.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks/>\n");
        dir.Write("xlights_rgbeffects.xml", Rgb(models, renames));
        REQUIRE(ctx.LoadShowFolder(dir.path.string()));
        REQUIRE(ctx.OpenSequence(dir.Write("seq.xsq", Xsq(rows))));
    }

    Element* Row(const std::string& name) { return ctx._sequenceElements.GetElement(name); }
    int Effects(const std::string& name) {
        Element* e = Row(name);
        return e == nullptr ? -1 : e->GetEffectCount();
    }
};

} // namespace

TEST_CASE("Rename hints round-trip through the layout file", "[renamehints]") {
    LayoutRenameHints h = Hints({ E("Arch 1", "Arch Left", 100, "id-1"), E("Garage", "Garage Door", 200, "", LayoutRenameHints::TYPE_CONTROLLER),
                                  E("Mesh", "House", 150, "", LayoutRenameHints::TYPE_VIEW_OBJECT) });
    REQUIRE(h.Entries().size() == 3);
    CHECK(h.Entries()[1].from == "Mesh"); // sorted by time

    SECTION("serialized the way the desktop writes the file") {
        StringSerializingVisitor v(false, false);
        v.WriteOpenTag("xrgb");
        h.Serialize(v);
        v.WriteCloseTag();
        pugi::xml_document d;
        REQUIRE(d.load_string(v.GetResult().c_str()));
        LayoutRenameHints back;
        back.Load(d.document_element().child(LayoutRenameHints::SECTION));
        CHECK(back.Entries() == h.Entries());
        CHECK(std::string(d.document_element().child("renames").child("rename").attribute("xlId").value()) == "id-1");
    }
    SECTION("patched into a file the way the iPad writes it") {
        pugi::xml_document d;
        pugi::xml_node root = d.append_child("xrgb");
        root.append_child("models");
        root.append_child(LayoutRenameHints::SECTION).append_child("rename"); // a stale one is replaced
        h.WriteTo(root);
        int sections = 0;
        for (pugi::xml_node n : root.children(LayoutRenameHints::SECTION)) {
            (void)n;
            ++sections;
        }
        CHECK(sections == 1);
        CHECK(LayoutRenameHints::Read(root.child(LayoutRenameHints::SECTION)) == h.Entries());
        LayoutRenameHints().WriteTo(root);
        CHECK(!root.child(LayoutRenameHints::SECTION));
    }
    SECTION("an empty set writes nothing") {
        StringSerializingVisitor v(false, false);
        v.WriteOpenTag("xrgb");
        LayoutRenameHints().Serialize(v);
        v.WriteCloseTag();
        CHECK(v.GetResult().find("renames") == std::string::npos);
    }
    SECTION("malformed and duplicate entries are dropped when read") {
        pugi::xml_document d;
        REQUIRE(d.load_string("<renames><rename from=\"A\" to=\"B\" time=\"5\"/><rename from=\"A\" to=\"B\" time=\"9\"/>"
                              "<rename from=\"\" to=\"B\"/><rename from=\"C\" to=\"C\"/></renames>"));
        auto entries = LayoutRenameHints::Read(d.document_element());
        REQUIRE(entries.size() == 1);
        CHECK(entries[0].type == "model");
        CHECK(entries[0].time == 9);
    }
}

TEST_CASE("A layout file without rename hints loads with none", "[renamehints]") {
    TempDir dir;
    FixFileReset reset;
    dir.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks/>\n");
    dir.Write("xlights_rgbeffects.xml", Rgb({ "A" }, RenamesXml({ E("X", "A", kNow) })));
    HeadlessRenderContext ctx;
    REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    CHECK(ctx._renameHints.Entries().size() == 1);
    // The file an older desktop writes back has no section: reloading it clears the hints.
    dir.Write("xlights_rgbeffects.xml", Rgb({ "A" }, ""));
    REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    CHECK(ctx._renameHints.Empty());
    CHECK(ctx.AllModels.GetModel("A") != nullptr);
}

TEST_CASE("Rename hints follow chains in time order", "[renamehints]") {
    SECTION("not renamed") {
        auto r = Hints({ E("A", "B", 10) }).Resolve("model", "Z", ExistsIn({ "B" }));
        CHECK(r.outcome == Outcome::NotRenamed);
    }
    SECTION("one step") {
        auto r = Hints({ E("A", "B", 10) }).Resolve("model", "A", ExistsIn({ "B" }));
        CHECK(r.outcome == Outcome::Mapped);
        CHECK(r.target == "B");
    }
    SECTION("A to B to C") {
        auto r = Hints({ E("B", "C", 20), E("A", "B", 10) }).Resolve("model", "A", ExistsIn({ "C" }));
        CHECK(r.outcome == Outcome::Mapped);
        CHECK(r.target == "C");
        CHECK(r.chain == std::vector<std::string>{ "A", "B", "C" });
    }
    SECTION("an intermediate name that exists again is passed through") {
        // A became B, then that B became C; a new model took the name B since.
        auto r = Hints({ E("A", "B", 10), E("B", "C", 20) }).Resolve("model", "A", ExistsIn({ "B", "C" }));
        CHECK(r.target == "C");
    }
    SECTION("an older rename of a reused name is not followed") {
        // B became C long before A took the name B.
        auto h = Hints({ E("B", "C", 5), E("A", "B", 10) });
        auto r = h.Resolve("model", "A", ExistsIn({ "C" }));
        CHECK(r.outcome == Outcome::Broken);
        r = h.Resolve("model", "A", ExistsIn({ "B", "C" }));
        CHECK(r.outcome == Outcome::Mapped);
        CHECK(r.target == "B");
    }
    SECTION("a name renamed to two different things is ambiguous") {
        auto r = Hints({ E("A", "B", 10), E("A", "C", 20) }).Resolve("model", "A", ExistsIn({ "B", "C" }));
        CHECK(r.outcome == Outcome::Ambiguous);
    }
    SECTION("a cycle is broken") {
        auto r = Hints({ E("A", "B", 10), E("B", "A", 20) }).Resolve("model", "A", ExistsIn({}));
        CHECK(r.outcome == Outcome::Broken);
    }
    SECTION("a chain ending at a name that does not exist is broken") {
        auto r = Hints({ E("A", "B", 10) }).Resolve("model", "A", ExistsIn({}));
        CHECK(r.outcome == Outcome::Broken);
    }
    SECTION("other types do not mix in") {
        auto r = Hints({ E("A", "B", 10, "", LayoutRenameHints::TYPE_CONTROLLER) }).Resolve("model", "A", ExistsIn({ "B" }));
        CHECK(r.outcome == Outcome::NotRenamed);
    }
    SECTION("a layout id settles where the record went") {
        // Renamed again by a version that kept the id but no hint.
        auto byId = [](const std::string& id) { return id == "id-7" ? std::string("Renamed Again") : std::string(); };
        auto r = Hints({ E("A", "B", 10, "id-7") }).Resolve("model", "A", ExistsIn({ "Renamed Again" }), byId);
        CHECK(r.outcome == Outcome::Mapped);
        CHECK(r.target == "Renamed Again");
        // Without the id lookup the hint leads to B, which is gone.
        CHECK(Hints({ E("A", "B", 10, "id-7") }).Resolve("model", "A", ExistsIn({ "Renamed Again" })).outcome == Outcome::Broken);
    }
}

TEST_CASE("Rename hints are pruned by age and count", "[renamehints]") {
    const std::time_t day = 24 * 60 * 60;
    LayoutRenameHints h = Hints({ E("Old", "X", kNow - 200 * day), E("Recent", "Y", kNow - 10 * day), E("Edge", "Z", kNow - 179 * day) });
    CHECK(h.Prune(kNow) == 1);
    REQUIRE(h.Entries().size() == 2);
    CHECK(h.Entries()[0].from == "Edge");
    CHECK(h.Prune(kNow, LayoutRenameHints::MAX_AGE_DAYS, 1) == 1);
    REQUIRE(h.Entries().size() == 1);
    CHECK(h.Entries()[0].from == "Recent"); // the newest is kept
}

TEST_CASE("Rename hints merge as a set", "[renamehints]") {
    const auto ab = E("A", "B", 10), cd = E("C", "D", 20), ef = E("E", "F", 30), xy = E("X", "Y", 5);
    SECTION("both sides' new entries are kept") {
        auto merged = LayoutRenameHints::Merge({ ab }, { ab, cd }, { ab, ef });
        CHECK(merged == std::vector<Entry>{ ab, cd, ef });
    }
    SECTION("an entry one side pruned stays pruned") {
        auto merged = LayoutRenameHints::Merge({ xy, ab }, { ab }, { xy, ab, cd });
        CHECK(merged == std::vector<Entry>{ ab, cd });
    }
    SECTION("the same rename keeps the newest time") {
        auto merged = LayoutRenameHints::Merge({}, { E("A", "B", 10) }, { E("A", "B", 40, "id") });
        REQUIRE(merged.size() == 1);
        CHECK(merged[0].time == 40);
        CHECK(merged[0].id == "id");
    }
}

TEST_CASE("The layout merge unions the rename hints and never conflicts on them", "[renamehints]") {
    using namespace ShowFileMerge;
    const std::string base = Rgb({ "A", "B" }, RenamesXml({ E("Q", "A", 10) }));
    // Mine renamed a model; theirs changed a different model and added a hint of its own.
    std::string mine = Rgb({ "A2", "B" }, RenamesXml({ E("Q", "A", 10), E("A", "A2", 20) }));
    std::string theirs = base;
    theirs.replace(theirs.find("NodesPerString=\"10\"", theirs.find("name=\"B\"")), 19, "NodesPerString=\"20\"");
    theirs.replace(theirs.find("</renames>"), 10, "<rename type=\"model\" from=\"Old\" to=\"B\" time=\"30\"/></renames>");

    Merge m = Merge::Run(FileKind::RgbEffects, base, mine, theirs);
    REQUIRE(m.Ok());
    CHECK(!m.HasConflicts());
    pugi::xml_node root = m.ResultXml().document_element();
    auto merged = LayoutRenameHints::Read(root.child(LayoutRenameHints::SECTION));
    CHECK(merged == std::vector<Entry>{ E("Q", "A", 10), E("A", "A2", 20), E("Old", "B", 30) });
    CHECK(root.child("models").find_child_by_attribute("model", "name", "A2"));
    CHECK(std::string(root.child("models").find_child_by_attribute("model", "name", "B").attribute("NodesPerString").value()) == "20");

    SECTION("both sides adding different hints is not a conflict") {
        std::string mine2 = Rgb({ "A", "B" }, RenamesXml({ E("Q", "A", 10), E("M", "B", 40) }));
        std::string theirs2 = Rgb({ "A", "B" }, RenamesXml({ E("Q", "A", 10), E("T", "A", 50) }));
        Merge m2 = Merge::Run(FileKind::RgbEffects, base, mine2, theirs2);
        REQUIRE(m2.Ok());
        CHECK(!m2.HasConflicts());
        CHECK(LayoutRenameHints::Read(m2.ResultXml().document_element().child(LayoutRenameHints::SECTION)).size() == 3);
    }
    SECTION("a side written by a version without the section keeps the other's hints") {
        std::string older = Rgb({ "A", "B" }, "");
        older.replace(older.find("NodesPerString=\"10\"", older.find("name=\"B\"")), 19, "NodesPerString=\"30\"");
        Merge m3 = Merge::Run(FileKind::RgbEffects, base, mine, older);
        REQUIRE(m3.Ok());
        CHECK(!m3.HasConflicts());
        // An older desktop drops the section without knowing it: that is not a prune.
        auto hints = LayoutRenameHints::Read(m3.ResultXml().document_element().child(LayoutRenameHints::SECTION));
        CHECK(hints == std::vector<Entry>{ E("Q", "A", 10), E("A", "A2", 20) });
        CHECK(std::string(m3.ResultXml().document_element().child("models").find_child_by_attribute("model", "name", "B").attribute("NodesPerString").value()) == "30");
    }
}

TEST_CASE("The rename log queues renames for the hints until taken", "[renamehints]") {
    LayoutRenameLog log;
    log.Record("A", "B", "id-1");
    log.Clear(); // a save recorded the new base
    CHECK(log.Empty());
    auto queued = log.TakeUnpersisted();
    REQUIRE(queued.size() == 1);
    CHECK(queued[0].oldName == "A");
    CHECK(queued[0].id == "id-1");
    CHECK(queued[0].time > 0);
    CHECK(log.TakeUnpersisted().empty());
    log.Record("C", "D");
    log.Reset(); // a fresh load
    CHECK(log.TakeUnpersisted().empty());

    SECTION("renaming through the show context reaches the hints with the model's id") {
        TempDir dir;
        FixFileReset reset;
        dir.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks/>\n");
        dir.Write("xlights_rgbeffects.xml", Rgb({ "A" }, ""));
        HeadlessRenderContext ctx;
        REQUIRE(ctx.LoadShowFolder(dir.path.string()));
        const std::string id = ctx.AllModels.GetModel("A")->GetObjectId();
        REQUIRE(!id.empty());
        ctx.AllModels.Rename("A", "A Renamed");
        ctx.CollectRenameHints();
        REQUIRE(ctx._renameHints.Entries().size() == 1);
        CHECK(ctx._renameHints.Entries()[0].from == "A");
        CHECK(ctx._renameHints.Entries()[0].to == "A Renamed");
        CHECK(ctx._renameHints.Entries()[0].id == id);
        ctx.CollectRenameHints(); // nothing new: no duplicate
        CHECK(ctx._renameHints.Entries().size() == 1);
    }
}

TEST_CASE("Opening a sequence maps rows of renamed models", "[renamehints]") {
    const std::string hints = RenamesXml({ E("Arch 1", "Arch Mid", kNow), E("Arch Mid", "Arch Left", kNow + 5) });

    SECTION("the row follows the chain and keeps its effects") {
        OpenedShow s({ "Arch Left", "Tree" }, hints, { "Arch 1", "Tree" });
        CHECK(s.Row("Arch 1") == nullptr);
        CHECK(s.Effects("Arch Left") == 1);
        CHECK(s.Effects("Tree") == 1);
        REQUIRE(s.ctx._autoMappedModels.size() == 1);
        CHECK(s.ctx._autoMappedModels[0] == std::pair<std::string, std::string>{ "Arch 1", "Arch Left" });
        CHECK(s.ctx.ScanForMissingModels().missing.empty());
    }
    SECTION("control: without hints the row stays missing") {
        OpenedShow s({ "Arch Left", "Tree" }, "", { "Arch 1", "Tree" });
        CHECK(s.Effects("Arch 1") == 1);
        CHECK(s.Row("Arch Left") == nullptr);
        CHECK(s.ctx._autoMappedModels.empty());
    }
    SECTION("an ambiguous rename is left for the prompt") {
        OpenedShow s({ "Left", "Right" }, RenamesXml({ E("Arch", "Left", kNow), E("Arch", "Right", kNow + 1) }), { "Arch" });
        CHECK(s.Effects("Arch") == 1);
        CHECK(s.ctx._autoMappedModels.empty());
    }
    SECTION("a broken chain is left for the prompt") {
        OpenedShow s({ "Tree" }, RenamesXml({ E("Arch", "Gone", kNow) }), { "Arch" });
        CHECK(s.Effects("Arch") == 1);
        CHECK(s.ctx._autoMappedModels.empty());
    }
    SECTION("a target that already has a row is left for the prompt") {
        OpenedShow s({ "Arch Left" }, hints, { "Arch 1", "Arch Left" });
        CHECK(s.Effects("Arch 1") == 1);
        CHECK(s.Effects("Arch Left") == 1);
        CHECK(s.ctx._autoMappedModels.empty());
    }
    SECTION("an oldname alias keeps precedence") {
        TempDir dir;
        FixFileReset reset;
        HeadlessRenderContext ctx;
        dir.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks/>\n");
        std::string rgb = Rgb({ "Arch Left", "Other" }, RenamesXml({ E("Arch 1", "Other", kNow) }));
        rgb.replace(rgb.find("/>", rgb.find("name=\"Arch Left\"")), 2, "><Aliases><alias name=\"oldname:arch 1\"/></Aliases></model>");
        dir.Write("xlights_rgbeffects.xml", rgb);
        REQUIRE(ctx.LoadShowFolder(dir.path.string()));
        REQUIRE(ctx.AllModels.GetModel("Arch Left")->IsAlias("Arch 1", true));
        REQUIRE(ctx.OpenSequence(dir.Write("seq.xsq", Xsq({ "Arch 1" }))));
        CHECK(ctx._autoMappedModels.empty());
        CHECK(ctx._sequenceElements.GetElement("Other") == nullptr);
    }
}

TEST_CASE("Bringing memory up to a merged layout takes its rename hints", "[renamehints]") {
    TempDir dir;
    FixFileReset reset;
    dir.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks/>\n");
    dir.Write("xlights_rgbeffects.xml", Rgb({ "A" }, ""));
    HeadlessRenderContext ctx;
    REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    pugi::xml_document mine, result;
    REQUIRE(mine.load_string(Rgb({ "A" }, "").c_str()));
    REQUIRE(result.load_string(Rgb({ "A" }, RenamesXml({ E("Z", "A", kNow) })).c_str()));
    auto changes = ShowMergeApply::ApplyLayout(ctx, mine, result);
    CHECK(ctx._renameHints.Entries().size() == 1);
    CHECK(changes.otherSections.count(LayoutRenameHints::SECTION) == 0);
}
