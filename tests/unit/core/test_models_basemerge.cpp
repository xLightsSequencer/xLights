#include <string>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "outputs/OutputManager.h"
#include "support/TempDir.h"

// Merging a base show folder's models and groups into the current show
// (ModelManager::MergeBaseXml / MergeFromBase). Only models and groups that are
// new, or that came from the base before (FromBase="1"), take the base version;
// the controller wiring stays local.
namespace {

// The attribute names the current serializer writes, as a base folder saved by
// this version would have them.
const std::string kLineAttrs = R"(DisplayAs="Single Line" NumStrings="1" StringType="RGB Nodes")";

// A single line model; nodes and the start channel are attributes of their own
// so a test can change them without repeating an attribute.
std::string ModelXml(const std::string& name, const std::string& extra = "", const std::string& children = "",
                  int nodes = 10, const std::string& startChannel = "1") {
    return "<model name=\"" + name + "\" " + kLineAttrs + " NodesPerString=\"" + std::to_string(nodes) + "\" StartChannel=\"" + startChannel +
           "\" " + extra + ">" + children + "</model>";
}

std::string GroupXml(const std::string& name, const std::string& models, const std::string& extra = "") {
    return "<modelGroup name=\"" + name + "\" models=\"" + models + "\" layout=\"minimalGrid\" GridSize=\"400\" " + extra + "/>";
}

struct Base {
    TempDir dir;
    void Write(const std::string& models, const std::string& groups = "") {
        dir.Write("xlights_rgbeffects.xml", "<xrgb><models>" + models + "</models><modelGroups>" + groups + "</modelGroups></xrgb>");
    }
    std::string Path() const { return dir.path.string(); }
};

struct Local {
    pugi::xml_document doc;
    pugi::xml_node models;
    pugi::xml_node groups;
    Local(const std::string& m, const std::string& g = "") {
        REQUIRE(doc.load_string(("<xrgb><models>" + m + "</models><modelGroups>" + g + "</modelGroups></xrgb>").c_str()));
        models = doc.document_element().child("models");
        groups = doc.document_element().child("modelGroups");
    }
    pugi::xml_node M(const std::string& name) const { return models.find_child_by_attribute("model", "name", name.c_str()); }
    pugi::xml_node G(const std::string& name) const { return groups.find_child_by_attribute("modelGroup", "name", name.c_str()); }
    int Count() const {
        int n = 0;
        for (auto c : models.children("model")) {
            (void)c;
            ++n;
        }
        return n;
    }
};

bool Merge(const Base& b, Local& l) {
    bool changed = false;
    REQUIRE(ModelManager::MergeBaseXml(b.Path(), l.models, l.groups, &changed));
    return changed;
}

} // namespace

TEST_CASE("Base merge fails when the base folder has no layout file", "[models][basemerge]") {
    Base b;
    Local l("");
    bool changed = false;
    CHECK_FALSE(ModelManager::MergeBaseXml(b.Path(), l.models, l.groups, &changed));
    CHECK_FALSE(changed);
}

TEST_CASE("Base merge adds base models the show does not have and marks them", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("Arch"));
    Local l(ModelXml("Mine"));
    CHECK(Merge(b, l));
    REQUIRE(l.M("Arch"));
    CHECK(std::string(l.M("Arch").attribute("FromBase").value()) == "1");
    CHECK(l.M("Mine").attribute("FromBase").empty());
}

TEST_CASE("Base merge leaves a local model of the same name alone", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("Arch", "Description=\"base\""));
    Local l(ModelXml("Arch", "Description=\"local\""));
    CHECK_FALSE(Merge(b, l));
    CHECK(std::string(l.M("Arch").attribute("Description").value()) == "local");
    CHECK(l.Count() == 1);
}

TEST_CASE("Base merge updates a model that came from the base when the base changes", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("Arch", "", "", 20));
    Local l(ModelXml("Arch", "FromBase=\"1\""));
    CHECK(Merge(b, l));
    CHECK(l.Count() == 1);
    CHECK(std::string(l.M("Arch").attribute("NodesPerString").value()) == "20");
    CHECK(std::string(l.M("Arch").attribute("FromBase").value()) == "1");
}

TEST_CASE("Base merge reports no change when a base model is unchanged", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("Arch"));
    Local l(ModelXml("Arch", "FromBase=\"1\""));
    CHECK_FALSE(Merge(b, l));
}

TEST_CASE("Base merge ignores attributes that are not real changes", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("Arch", "SourceVersion=\"2026.01\" WorldPosX=\"10.00001\" NodeNames=\"\""));
    // The version that saved it, float noise past 4 decimals, and an empty
    // attribute older versions wrote that newer ones leave out.
    Local l(ModelXml("Arch", "FromBase=\"1\" SourceVersion=\"2026.10\" WorldPosX=\"10.00002\""));
    CHECK_FALSE(Merge(b, l));
}

TEST_CASE("Base merge sees a float change past the fourth decimal place", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("Arch", "WorldPosX=\"10.001\""));
    Local l(ModelXml("Arch", "FromBase=\"1\" WorldPosX=\"10.0\""));
    CHECK(Merge(b, l));
    CHECK(std::string(l.M("Arch").attribute("WorldPosX").value()) == "10.001");
}

TEST_CASE("Base merge keeps the local controller wiring of an updated model", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("Arch", "Description=\"new\" Controller=\"BaseCtl\"", "<ControllerConnection Protocol=\"ws2811\"/>"));
    Local l(ModelXml("Arch", "FromBase=\"1\" Controller=\"Mine\" ModelChain=\">Other\"",
                  "<ControllerConnection Protocol=\"ws2811\" Port=\"4\"/>", 10, "!Mine:31"));
    CHECK(Merge(b, l));
    pugi::xml_node m = l.M("Arch");
    CHECK(std::string(m.attribute("Description").value()) == "new");
    CHECK(std::string(m.attribute("Controller").value()) == "Mine");
    CHECK(std::string(m.attribute("StartChannel").value()) == "!Mine:31");
    CHECK(std::string(m.attribute("ModelChain").value()) == ">Other");
    CHECK(std::string(m.child("ControllerConnection").attribute("Port").value()) == "4");
}

TEST_CASE("Base merge treats a base with no controller as no change to the local one", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("Arch", "Controller=\"No Controller\""));
    Local l(ModelXml("Arch", "FromBase=\"1\" Controller=\"Mine\""));
    CHECK_FALSE(Merge(b, l));
}

TEST_CASE("Base merge ignores the start channel of a model wired to a local controller", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("Arch", "", "", 10, "500"));
    Local l(ModelXml("Arch", "FromBase=\"1\" Controller=\"Mine\""));
    CHECK_FALSE(Merge(b, l));
}

TEST_CASE("Base merge picks up a submodel added to a base model", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("Arch", "", "<subModel name=\"Half\" layout=\"horizontal\" type=\"ranges\" line0=\"1-5\"/>"));
    Local l(ModelXml("Arch", "FromBase=\"1\""));
    CHECK(Merge(b, l));
    CHECK(l.M("Arch").child("subModel"));
}

TEST_CASE("Base merge picks up a child the base removed", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("Arch"));
    Local l(ModelXml("Arch", "FromBase=\"1\"", "<dimmingCurve><all gamma=\"2.2\"/></dimmingCurve>"));
    CHECK(Merge(b, l));
    CHECK_FALSE(l.M("Arch").child("dimmingCurve"));
}

TEST_CASE("Base merge does not count a local controller connection as a difference", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("Arch"));
    Local l(ModelXml("Arch", "FromBase=\"1\"", "<ControllerConnection Protocol=\"ws2811\" Port=\"1\"/>"));
    CHECK_FALSE(Merge(b, l));
}

TEST_CASE("Base merge adds base groups the show does not have", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("A") + ModelXml("B"), GroupXml("All", "A,B"));
    Local l("");
    CHECK(Merge(b, l));
    REQUIRE(l.G("All"));
    CHECK(std::string(l.G("All").attribute("FromBase").value()) == "1");
    CHECK(std::string(l.G("All").attribute("models").value()) == "A,B");
}

TEST_CASE("Base merge keeps models the show added to a base group", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("A") + ModelXml("B"), GroupXml("All", "A,B"));
    Local l(ModelXml("A", "FromBase=\"1\"") + ModelXml("B", "FromBase=\"1\"") + ModelXml("Mine"), GroupXml("All", "A,B,Mine", "FromBase=\"1\""));
    CHECK_FALSE(Merge(b, l));
    CHECK(std::string(l.G("All").attribute("models").value()) == "A,B,Mine");
}

TEST_CASE("Base merge adds models the base added to a group, after the base's own", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("A") + ModelXml("B") + ModelXml("C"), GroupXml("All", "A,B,C"));
    Local l(ModelXml("Mine"), GroupXml("All", "A,Mine", "FromBase=\"1\" GridCentreX=\"12\" GridCentreY=\"-3\""));
    CHECK(Merge(b, l));
    pugi::xml_node g = l.G("All");
    CHECK(std::string(g.attribute("models").value()) == "A,B,C,Mine");
    CHECK(std::string(g.attribute("BaseModels").value()) == "A,B,C");
    // The group's centre offsets are a local choice
    CHECK(std::string(g.attribute("GridCentreX").value()) == "12");
    CHECK(std::string(g.attribute("GridCentreY").value()) == "-3");
}

TEST_CASE("Base merge leaves a local group of the same name alone", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("A"), GroupXml("All", "A"));
    Local l(ModelXml("Mine"), GroupXml("All", "Mine"));
    Merge(b, l);
    CHECK(std::string(l.G("All").attribute("models").value()) == "Mine");
}

TEST_CASE("Base merge makes base file references absolute", "[models][basemerge]") {
    Base b;
    b.Write(R"(<model name="Pic" DisplayAs="Image" Image="images/house.png" StartChannel="1"/>)");
    Local l("");
    Merge(b, l);
    // Whatever the model type, a relative path in the base resolves against
    // the base folder, not the current show folder.
    pugi::xml_node m = l.M("Pic");
    REQUIRE(m);
    std::string img = m.attribute("Image").value();
    CHECK(img.find(b.Path()) == 0);
}

// ---- through the model manager ----

namespace {
struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };

    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m != nullptr);
        mm.RecalcStartChannels();
        return m;
    }
};
} // namespace

TEST_CASE("MergeFromBase adds base models and groups to the show", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("A") + ModelXml("B"), GroupXml("Both", "A,B"));
    ShowFixture f;
    f.Add(ModelXml("Mine"));
    bool accept = false, reject = false, changed = false;
    REQUIRE(f.mm.MergeFromBase(b.Path(), false, accept, reject, &changed));
    CHECK(changed);
    REQUIRE(f.mm.GetModel("A") != nullptr);
    CHECK(f.mm.GetModel("A")->IsFromBase());
    CHECK_FALSE(f.mm.GetModel("Mine")->IsFromBase());
    auto* g = dynamic_cast<ModelGroup*>(f.mm.GetModel("Both"));
    REQUIRE(g != nullptr);
    CHECK(g->ModelNames().size() == 2);
}

TEST_CASE("MergeFromBase replaces a changed base model in place", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("A", "Description=\"v2\""));
    ShowFixture f;
    f.Add(ModelXml("A", "FromBase=\"1\" Description=\"v1\""));
    bool accept = false, reject = false, changed = false;
    REQUIRE(f.mm.MergeFromBase(b.Path(), false, accept, reject, &changed));
    CHECK(changed);
    CHECK(f.mm.GetModel("A")->GetDescription() == "v2");
    CHECK(f.mm.GetModel("A")->IsFromBase());
}

TEST_CASE("MergeFromBase twice in a row changes nothing the second time", "[models][basemerge]") {
    Base b;
    b.Write(ModelXml("A") + ModelXml("B", "WorldPosX=\"12.5\""), GroupXml("Both", "A,B"));
    ShowFixture f;
    bool accept = false, reject = false, changed = false;
    REQUIRE(f.mm.MergeFromBase(b.Path(), false, accept, reject, &changed));
    CHECK(changed);
    changed = false;
    REQUIRE(f.mm.MergeFromBase(b.Path(), false, accept, reject, &changed));
    CHECK_FALSE(changed);
}

TEST_CASE("MergeFromBase with a base that has no models section is a successful no-op", "[models][basemerge]") {
    Base b;
    b.dir.Write("xlights_rgbeffects.xml", "<xrgb/>");
    ShowFixture f;
    bool accept = false, reject = false, changed = true;
    CHECK(f.mm.MergeFromBase(b.Path(), false, accept, reject, &changed));
}
