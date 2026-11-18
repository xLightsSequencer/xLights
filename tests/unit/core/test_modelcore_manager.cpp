#include <algorithm>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "outputs/OutputManager.h"

namespace {

struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };

    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 1000, 1000);
        REQUIRE(m != nullptr);
        mm.RecalcStartChannels();
        return m;
    }
    Model* Line(const std::string& name, const std::string& extra = "") {
        return Add("<model name=\"" + name + "\" DisplayAs=\"Single Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"5\" " + extra + "/>");
    }
    // a pixel port model; chain is ">Other" or empty
    Model* OnPort(const std::string& name, const std::string& controller, int port, const std::string& chain, const std::string& protocol = "ws2811", int sr = 0) {
        Model* m = Line(name);
        m->SetControllerName(controller, true);
        m->SetControllerProtocol(protocol);
        m->SetControllerPort(port);
        m->SetSmartRemote(sr);
        m->SetModelChain(chain);
        return m;
    }
    ModelGroup* Group(const std::string& name) {
        auto* g = dynamic_cast<ModelGroup*>(mm.GetModel(name));
        REQUIRE(g != nullptr);
        return g;
    }
    void ImportGroups(const std::string& groupXml, const std::string& model, bool merge, bool ask = false) {
        pugi::xml_document d;
        REQUIRE(d.load_string(groupXml.c_str()));
        mm.AddModelGroups(d.first_child(), model, merge, ask);
    }
};

std::vector<std::string> Members(ModelGroup* g) {
    return std::vector<std::string>(g->ModelNames().begin(), g->ModelNames().end());
}

} // namespace

TEST_CASE("Imported model groups name the imported model", "[modelcore][manager][import]") {
    ShowFixture f;
    f.Line("Arch");
    f.ImportGroups(R"(<modelGroup name="Arches" models="EXPORTEDMODEL,EXPORTEDMODEL/Left" layout="minimalGrid" GridSize="400"/>)", "Arch", false);
    CHECK(Members(f.Group("Arches")) == std::vector<std::string>{ "Arch", "Arch/Left" });

    // a clash without merging makes a new group
    f.Line("Arch2");
    f.ImportGroups(R"(<modelGroup name="Arches" models="EXPORTEDMODEL" layout="minimalGrid" GridSize="400"/>)", "Arch2", false, true);
    CHECK(Members(f.Group("Arches_1")) == std::vector<std::string>{ "Arch2" });
    CHECK(Members(f.Group("Arches")) == std::vector<std::string>{ "Arch", "Arch/Left" });

    // groups with no members are ignored
    size_t before = f.mm.size();
    f.ImportGroups(R"(<modelGroup name="Empty" models="" layout="minimalGrid" GridSize="400"/>)", "Arch", false);
    CHECK(f.mm.size() == before);
}

TEST_CASE("Merging imported groups adds the model where the group matched", "[modelcore][manager][import]") {
    ShowFixture f;
    f.Line("A");
    f.Line("B");
    f.Line("M");
    f.Add(R"(<modelGroup name="G" models="A,B" layout="minimalGrid" GridSize="400"/>)");

    // the export listed one of the group's members alongside itself
    f.ImportGroups(R"(<modelGroup name="G" models="A,EXPORTEDMODEL" layout="minimalGrid" GridSize="400"/>)", "M", true);
    CHECK(Members(f.Group("G")) == std::vector<std::string>{ "A", "B", "M" });
    // merging again does not add it twice
    f.ImportGroups(R"(<modelGroup name="G" models="A,EXPORTEDMODEL" layout="minimalGrid" GridSize="400"/>)", "M", true);
    CHECK(Members(f.Group("G")) == std::vector<std::string>{ "A", "B", "M" });
}

TEST_CASE("Merging imported groups maps submodels by name", "[modelcore][manager][import]") {
    ShowFixture f;
    f.Add(R"(<modelGroup name="Arms" models="A/Arm,A/Arm 1 Left,A/1 Top" layout="minimalGrid" GridSize="400"/>)");

    // same submodel name
    f.ImportGroups(R"(<modelGroup name="Arms" models="EXPORTEDMODEL/Arm" layout="minimalGrid" GridSize="400"/>)", "M", true);
    auto m = Members(f.Group("Arms"));
    CHECK(std::find(m.begin(), m.end(), "M/Arm") != m.end());

    // numbers zero padded in the export still match
    f.ImportGroups(R"(<modelGroup name="Arms" models="EXPORTEDMODEL/Arm 01 Left,EXPORTEDMODEL/01 Top" layout="minimalGrid" GridSize="400"/>)", "N", true);
    m = Members(f.Group("Arms"));
    CHECK(std::find(m.begin(), m.end(), "N/Arm 01 Left") != m.end());
    CHECK(std::find(m.begin(), m.end(), "N/01 Top") != m.end());

    // nothing matched: the exported submodels are added anyway
    f.ImportGroups(R"(<modelGroup name="Arms" models="EXPORTEDMODEL/Leg" layout="minimalGrid" GridSize="400"/>)", "P", true);
    m = Members(f.Group("Arms"));
    CHECK(std::find(m.begin(), m.end(), "P/Leg") != m.end());
}

TEST_CASE("Last model on a port is the one ending highest", "[modelcore][manager]") {
    ShowFixture f;
    Model* a = f.OnPort("A", "Ctl", 1, "");
    Model* b = f.OnPort("B", "Ctl", 1, ">A");
    f.OnPort("C", "Ctl", 2, "");
    Model* d = f.OnPort("D", "Ctl", 1, "", "ws2811", 2);
    a->SetStartChannel("1");
    b->SetStartChannel("16");
    d->SetStartChannel("100");
    f.mm.RecalcStartChannels();
    CHECK(f.mm.GetLastModelOnPort("Ctl", 1, "", "ws2811") == "D");
    CHECK(f.mm.GetLastModelOnPort("Ctl", 1, "D", "ws2811") == "B");
    CHECK(f.mm.GetLastModelOnPort("Ctl", 3, "", "ws2811").empty());
    CHECK(f.mm.GetLastModelOnPort("Ctl", 1, "", "apa102").empty());
    CHECK(f.mm.GetLastModelOnPort("Ctl", 1, "", "ws2811", 0) == "B");
    CHECK(f.mm.GetLastModelOnPort("Ctl", 1, "", "ws2811", 2) == "D");
}

TEST_CASE("Controller model chains are validated", "[modelcore][manager][chain]") {
    ShowFixture f;
    std::string tip;

    // models without a controller or port are not checked
    Model* loose = f.Line("Loose");
    CHECK(f.mm.IsValidControllerModelChain(loose, tip));
    Model* noPort = f.OnPort("NoPort", "Ctl", 0, ">X");
    CHECK(f.mm.IsValidControllerModelChain(noPort, tip));

    // the only model on a port must be at the beginning
    Model* only = f.OnPort("Only", "Ctl", 9, ">Elsewhere");
    CHECK_FALSE(f.mm.IsValidControllerModelChain(only, tip));
    CHECK(tip == "Only model on an output must not chain to anything.");
    only->SetModelChain("");
    CHECK(f.mm.IsValidControllerModelChain(only, tip));

    // A <- B <- C is fine
    Model* a = f.OnPort("A", "Ctl", 1, "");
    Model* b = f.OnPort("B", "Ctl", 1, ">A");
    Model* c = f.OnPort("C", "Ctl", 1, ">B");
    CHECK(f.mm.IsValidControllerModelChain(a, tip));
    CHECK(f.mm.IsValidControllerModelChain(c, tip));

    // two models in the same place
    Model* d = f.OnPort("D", "Ctl", 1, ">B");
    CHECK_FALSE(f.mm.IsValidControllerModelChain(d, tip));
    CHECK(tip.starts_with("Model shares chain with "));

    // chained to a model on another port
    d->SetModelChain(">Only");
    CHECK_FALSE(f.mm.IsValidControllerModelChain(d, tip));
    CHECK(tip == "Chained to Only but that model is not on this port");

    // a chain back to the starting model is not on the port for that model
    a->SetModelChain(">D");
    d->SetModelChain(">C");
    CHECK_FALSE(f.mm.IsValidControllerModelChain(b, tip));
    CHECK(tip == "Chained to B but that model is not on this port");
    // a loop further down never reaches the beginning
    c->SetModelChain(">D");
    CHECK_FALSE(f.mm.IsValidControllerModelChain(b, tip));
    CHECK(tip.find("loop") != std::string::npos);

    // serial ports may always start at the beginning
    Model* dmx = f.OnPort("Dmx", "Ctl", 1, "", "DMX");
    CHECK(f.mm.IsValidControllerModelChain(dmx, tip));
}

TEST_CASE("Base and local model mixes are detected", "[modelcore][manager]") {
    ShowFixture f;
    Model* a = f.Line("A");
    f.Line("B");
    CHECK_FALSE(f.mm.ModelsMixBaseAndLocal({ "A", "B", "Missing" }));
    a->SetFromBase(true);
    CHECK(f.mm.ModelsMixBaseAndLocal({ "A", "B" }));
    CHECK_FALSE(f.mm.ModelsMixBaseAndLocal({ "A" }));
}

TEST_CASE("Model lookups tolerate surrounding whitespace", "[modelcore][manager]") {
    ShowFixture f;
    Model* a = f.Line("Arch");
    CHECK(f.mm.GetModel("  Arch ") == a);
    CHECK(f.mm["Arch"] == a);
    CHECK(f.mm.GetModel("Nope") == nullptr);
}

TEST_CASE("Manager replaces an IP in every model's start channel", "[modelcore][manager]") {
    ShowFixture f;
    Model* a = f.Add(R"(<model name="A" DisplayAs="Single Line" StartChannel="#198.51.100.5:1:1" NumStrings="1" NodesPerString="5"/>)");
    Model* b = f.Add(R"(<model name="B" DisplayAs="Single Line" StartChannel="#198.51.100.6:1:1" NumStrings="1" NodesPerString="5"/>)");
    f.mm.ReplaceIPInStartChannels("198.51.100.5", "192.0.2.9");
    CHECK(a->GetModelStartChannel() == "#192.0.2.9:1:1");
    CHECK(b->GetModelStartChannel() == "#198.51.100.6:1:1");
}

TEST_CASE("Manager clears every model's preview caches", "[modelcore][manager]") {
    ShowFixture f;
    f.Line("A");
    f.Line("B");
    f.mm.clearUIObjects();
    CHECK(f.mm.size() == 2);
    f.mm.clear();
    CHECK(f.mm.size() == 0);
}
