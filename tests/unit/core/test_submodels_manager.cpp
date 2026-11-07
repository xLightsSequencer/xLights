#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "models/SubModel.h"
#include "outputs/OutputManager.h"

namespace {

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

    Model* Tree(const std::string& name, const std::string& start = "1") {
        return Add("<model name=\"" + name + "\" DisplayAs=\"Tree 360\" NumStrings=\"1\" NodesPerString=\"10\" StrandsPerString=\"1\" StartChannel=\"" + start + "\">"
                   "<subModel name=\"h\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-5\"/>"
                   "<subModel name=\"v\" layout=\"vertical\" type=\"ranges\" bufferstyle=\"Default\" line0=\"6-10\"/>"
                   "</model>");
    }

    Model* Line(const std::string& name, int nodes, const std::string& start = "1") {
        return Add("<model name=\"" + name + "\" DisplayAs=\"Single Line\" NumStrings=\"1\" NodesPerString=\"" + std::to_string(nodes) + "\" StartChannel=\"" + start + "\"/>");
    }

    ModelGroup* Group(const std::string& name, const std::string& members) {
        Model* m = Add("<modelGroup name=\"" + name + "\" models=\"" + members + "\"/>");
        auto* g = dynamic_cast<ModelGroup*>(m);
        REQUIRE(g != nullptr);
        return g;
    }
};

} // namespace

TEST_CASE("Deleting a parent model removes its submodel entries from groups", "[models2][manager]") {
    ShowFixture f;
    f.Tree("T");
    f.Line("A", 5, "100");
    ModelGroup* g = f.Group("G", "A,T/h,T/v");
    REQUIRE(g->ModelNames().size() == 3);
    CHECK(f.mm.Delete("T"));
    CHECK(g->ModelNames() == std::vector<std::string>{ "A" });
    CHECK(g->Models().size() == 1);
    CHECK(f.mm.GetModel("T/h") == nullptr);
}

TEST_CASE("Deleting a model keeps similarly named models and entries", "[models2][manager]") {
    ShowFixture f;
    f.Tree("T");
    f.Tree("T2", "100");
    ModelGroup* g = f.Group("G", "T/h,T2/h,T2");
    CHECK(f.mm.Delete("T"));
    CHECK(g->ModelNames() == std::vector<std::string>{ "T2/h", "T2" });
    CHECK(f.mm.GetModel("T2/h") != nullptr);
}

TEST_CASE("A submodel path is not deletable as a model", "[models2][manager]") {
    ShowFixture f;
    f.Tree("T");
    CHECK_FALSE(f.mm.Delete("T/h"));
    CHECK(f.mm.GetModel("T")->GetSubModels().size() == 2);
    CHECK(f.mm.size() == 1);
}

TEST_CASE("Renaming a parent changes the submodel full names and lookups", "[models2][manager]") {
    ShowFixture f;
    Model* t = f.Tree("T");
    f.mm.Rename("T", "Tree");
    CHECK(f.mm.GetModel("T") == nullptr);
    CHECK(f.mm.GetModel("T/h") == nullptr);
    REQUIRE(f.mm.GetModel("Tree") == t);
    Model* h = f.mm.GetModel("Tree/h");
    REQUIRE(h != nullptr);
    CHECK(h->GetFullName() == "Tree/h");
    CHECK(t->GetName() == "Tree");
}

TEST_CASE("Renaming a parent updates all of its submodel entries in a group", "[models2][manager]") {
    ShowFixture f;
    f.Tree("T");
    ModelGroup* g = f.Group("G", "T/h,T/v");
    CHECK(f.mm.Rename("T", "Spruce"));
    CHECK(g->ModelNames() == std::vector<std::string>{ "Spruce/h", "Spruce/v" });
    REQUIRE(g->Models().size() == 2);
    CHECK(g->Models()[0] == f.mm.GetModel("Spruce/h"));
}

TEST_CASE("Renaming a model updates nested groups", "[models2][manager]") {
    ShowFixture f;
    f.Line("A", 5);
    ModelGroup* inner = f.Group("Inner", "A");
    ModelGroup* outer = f.Group("Outer", "Inner");
    CHECK(f.mm.Rename("A", "Alpha"));
    CHECK(inner->ModelNames() == std::vector<std::string>{ "Alpha" });
    CHECK(outer->ModelNames() == std::vector<std::string>{ "Inner" });
    CHECK(outer->GetFlatModels().size() == 1);
}

TEST_CASE("Renaming a group updates groups that contain it", "[models2][manager]") {
    ShowFixture f;
    f.Line("A", 5);
    f.Group("Inner", "A");
    ModelGroup* outer = f.Group("Outer", "Inner");
    f.mm.Rename("Inner", "Core");
    CHECK(outer->ModelNames() == std::vector<std::string>{ "Core" });
    CHECK(f.mm.GetModel("Core") != nullptr);
    CHECK(f.mm.GetModel("Inner") == nullptr);
}

TEST_CASE("Rename trims whitespace around names", "[models2][manager]") {
    ShowFixture f;
    f.Line("A", 5);
    f.mm.Rename("  A  ", "  B  ");
    CHECK(f.mm.GetModel("B") != nullptr);
    CHECK(f.mm.GetModel("A") == nullptr);
    CHECK(f.mm.GetModel("B")->GetName() == "B");
}

TEST_CASE("Renaming a missing model does nothing", "[models2][manager]") {
    ShowFixture f;
    f.Line("A", 5);
    CHECK_FALSE(f.mm.Rename("Nope", "B"));
    CHECK(f.mm.size() == 1);
    CHECK(f.mm.GetModel("B") == nullptr);
}

TEST_CASE("Model names are case sensitive", "[models2][manager]") {
    ShowFixture f;
    Model* a = f.Line("Alpha", 5, "1");
    Model* b = f.Line("alpha", 5, "100");
    CHECK(a != b);
    CHECK(f.mm.size() == 2);
    CHECK(f.mm.GetModel("Alpha") == a);
    CHECK(f.mm.GetModel("alpha") == b);
    CHECK(f.mm.GetModel("ALPHA") == nullptr);
}

TEST_CASE("Replacing a model by adding the same name drops the old submodels", "[models2][manager]") {
    ShowFixture f;
    f.Tree("T");
    REQUIRE(f.mm.GetModel("T/h") != nullptr);
    f.Add("<model name=\"T\" DisplayAs=\"Tree 360\" NumStrings=\"1\" NodesPerString=\"4\" StrandsPerString=\"1\" StartChannel=\"1\"/>");
    CHECK(f.mm.size() == 1);
    CHECK(f.mm.GetModel("T")->GetNodeCount() == 4);
    CHECK(f.mm.GetModel("T/h") == nullptr);
}

TEST_CASE("Replacing a model with a renamed model removes the old key", "[models2][manager]") {
    ShowFixture f;
    f.Line("A", 5);
    pugi::xml_document doc;
    REQUIRE(doc.load_string("<model name=\"B\" DisplayAs=\"Single Line\" NumStrings=\"1\" NodesPerString=\"7\" StartChannel=\"1\"/>"));
    Model* nm = f.mm.CreateModel(doc.first_child(), 100, 100);
    REQUIRE(nm != nullptr);
    f.mm.ReplaceModel("A", nm);
    CHECK(f.mm.GetModel("A") == nullptr);
    CHECK(f.mm.GetModel("B") == nm);
    CHECK(f.mm.size() == 1);
}

TEST_CASE("Replacing with an empty name or a null model is ignored", "[models2][manager]") {
    ShowFixture f;
    Model* a = f.Line("A", 5);
    f.mm.ReplaceModel("A", nullptr);
    CHECK(f.mm.GetModel("A") == a);
    pugi::xml_document doc;
    REQUIRE(doc.load_string("<model name=\"B\" DisplayAs=\"Single Line\" NumStrings=\"1\" NodesPerString=\"7\" StartChannel=\"1\"/>"));
    Model* nm = f.mm.CreateModel(doc.first_child(), 100, 100);
    f.mm.ReplaceModel("", nm);
    CHECK(f.mm.GetModel("B") == nullptr);
    delete nm;
}

TEST_CASE("Overlap detection compares channel ranges of non group models", "[models2][manager]") {
    ShowFixture f;
    Model* a = f.Line("A", 10, "1");
    Model* b = f.Line("B", 10, "20");
    Model* c = f.Line("C", 10, "100");
    CHECK(f.mm.IsModelOverlapping(a));
    CHECK(f.mm.IsModelOverlapping(b));
    CHECK_FALSE(f.mm.IsModelOverlapping(c));
}

TEST_CASE("Models that merely touch do not overlap", "[models2][manager]") {
    ShowFixture f;
    Model* a = f.Line("A", 10, "1");
    Model* b = f.Line("B", 10, "31");
    CHECK_FALSE(f.mm.IsModelOverlapping(a));
    CHECK_FALSE(f.mm.IsModelOverlapping(b));
}

TEST_CASE("Groups never count as overlapping models", "[models2][manager]") {
    ShowFixture f;
    Model* a = f.Line("A", 10, "1");
    f.Group("G", "A");
    CHECK_FALSE(f.mm.IsModelOverlapping(a));
}

TEST_CASE("Models on channels lists the models that touch a range", "[models2][manager]") {
    ShowFixture f;
    f.Line("A", 10, "1");
    f.Line("B", 10, "100");
    f.Line("C", 10, "200");
    f.Group("G", "A,B");
    std::string res = f.mm.GetModelsOnChannels(95, 105, 0);
    CHECK(res == "B");
    res = f.mm.GetModelsOnChannels(1, 1000, 0);
    CHECK(res == "A, B, C");
    CHECK(f.mm.GetModelsOnChannels(500, 600, 0).empty());
}

TEST_CASE("Models on channels wraps lines", "[models2][manager]") {
    ShowFixture f;
    f.Line("A", 5, "1");
    f.Line("B", 5, "100");
    f.Line("C", 5, "200");
    std::string res = f.mm.GetModelsOnChannels(1, 1000, 2);
    CHECK(res == "A, B\nC");
}

TEST_CASE("Models on channels includes both end channels", "[models2][manager]") {
    ShowFixture f;
    f.Line("A", 10, "11");
    CHECK(f.mm.GetModelsOnChannels(40, 100, 0) == "A");
    CHECK(f.mm.GetModelsOnChannels(41, 100, 0).empty());
    CHECK(f.mm.GetModelsOnChannels(1, 11, 0) == "A");
    CHECK(f.mm.GetModelsOnChannels(1, 10, 0).empty());
}

TEST_CASE("Last channel of the manager follows the highest model", "[models2][manager]") {
    ShowFixture f;
    CHECK(f.mm.GetLastChannel() == 0);
    f.Line("A", 10, "1");
    CHECK(f.mm.GetLastChannel() == 29);
    f.Line("B", 10, "100");
    CHECK(f.mm.GetLastChannel() == 99 + 30 - 1);
    f.mm.Delete("B");
    CHECK(f.mm.GetLastChannel() == 29);
}

TEST_CASE("Generated default model names stay unique", "[models2][manager]") {
    ShowFixture f;
    std::set<std::string> names;
    for (int i = 0; i < 6; ++i) {
        Model* m = f.mm.CreateDefaultModel("Single Line", "1");
        REQUIRE(m != nullptr);
        CHECK(names.insert(m->GetName()).second);
        f.mm.AddModel(m);
    }
    CHECK(f.mm.size() == 6);
}

TEST_CASE("Generated model names avoid existing submodel style names", "[models2][manager]") {
    ShowFixture f;
    f.Line("Tree", 5);
    f.Line("Tree-2", 5, "100");
    std::string n = f.mm.GenerateModelName("Tree");
    CHECK(n != "Tree");
    CHECK(n != "Tree-2");
    CHECK(f.mm.GetModel(n) == nullptr);
}

TEST_CASE("Groups containing a submodel are found from its parent", "[models2][manager]") {
    ShowFixture f;
    Model* t = f.Tree("T");
    f.Group("G", "T/h");
    auto groups = f.mm.GetGroupsContainingModel(t);
    CHECK(groups == std::vector<std::string>{ "G" });
    auto orSub = f.mm.GetGroupsContainingModelOrSubmodel(t);
    CHECK(orSub == std::vector<std::string>{ "G" });
}

TEST_CASE("Group lookup through a parent only reports groups with direct membership", "[models2][manager]") {
    ShowFixture f;
    Model* t = f.Tree("T");
    f.Group("G", "T");
    f.Group("H", "T/v");
    auto direct = f.mm.GetModelGroups(t);
    CHECK(direct.size() == 1);
    CHECK(direct[0]->GetName() == "G");
}

TEST_CASE("Start channel replacement updates IP addresses in models", "[models2][manager]") {
    ShowFixture f;
    Model* a = f.Add("<model name=\"A\" DisplayAs=\"Single Line\" NumStrings=\"1\" NodesPerString=\"5\" StartChannel=\"#192.168.0.5:1:1\"/>");
    Model* b = f.Add("<model name=\"B\" DisplayAs=\"Single Line\" NumStrings=\"1\" NodesPerString=\"5\" StartChannel=\"#192.168.0.6:1:1\"/>");
    f.mm.ReplaceIPInStartChannels("192.168.0.5", "10.0.0.9");
    CHECK(a->GetModelStartChannel() == "#10.0.0.9:1:1");
    CHECK(b->GetModelStartChannel() == "#192.168.0.6:1:1");
}

TEST_CASE("Start channel replacement skips groups and unrelated addresses", "[models2][manager]") {
    ShowFixture f;
    Model* a = f.Add("<model name=\"A\" DisplayAs=\"Single Line\" NumStrings=\"1\" NodesPerString=\"5\" StartChannel=\"100\"/>");
    f.Group("G", "A");
    f.mm.ReplaceIPInStartChannels("1.2.3.4", "5.6.7.8");
    CHECK(a->GetModelStartChannel() == "100");
}

TEST_CASE("Models chained by name report their dependency order", "[models2][manager]") {
    ShowFixture f;
    f.Line("A", 5, "1");
    Model* b = f.Line("B", 5, ">A:1");
    Model* c = f.Line("C", 5, ">B:1");
    CHECK(b->GetFirstChannel() == 15);
    CHECK(c->GetFirstChannel() == 30);
    f.mm.Rename("A", "Z");
    f.mm.RecalcStartChannels();
    CHECK(b->GetModelStartChannel() == ">Z:1");
    CHECK(b->GetFirstChannel() == 15);
}

TEST_CASE("Deleting the head of a chain keeps the rest in place", "[models2][manager]") {
    ShowFixture f;
    f.Line("A", 5, "50");
    Model* b = f.Line("B", 5, ">A:1");
    CHECK(b->GetFirstChannel() == 49 + 15);
    f.mm.Delete("A");
    f.mm.RecalcStartChannels();
    CHECK(b->GetModelStartChannel() == "50");
    CHECK(b->GetFirstChannel() == 49);
}

TEST_CASE("Iterating the manager visits models in name order", "[models2][manager]") {
    ShowFixture f;
    f.Line("Charlie", 3, "1");
    f.Line("Alpha", 3, "100");
    f.Line("Bravo", 3, "200");
    std::vector<std::string> order;
    for (const auto& it : f.mm) {
        order.push_back(it.first);
    }
    CHECK(order == std::vector<std::string>{ "Alpha", "Bravo", "Charlie" });
}

TEST_CASE("Clearing the manager drops groups and submodels", "[models2][manager]") {
    ShowFixture f;
    f.Tree("T");
    f.Group("G", "T/h");
    f.mm.clear();
    CHECK(f.mm.size() == 0);
    CHECK(f.mm.GetModel("T") == nullptr);
    CHECK(f.mm.GetModel("T/h") == nullptr);
    CHECK(f.mm.GetModel("G") == nullptr);
}

TEST_CASE("The manager can be reused after being cleared", "[models2][manager]") {
    ShowFixture f;
    f.Tree("T");
    f.mm.clear();
    f.Tree("T");
    CHECK(f.mm.size() == 1);
    CHECK(f.mm.GetModel("T/v") != nullptr);
}

TEST_CASE("Submodel pointers become invalid after the parent is deleted", "[models2][manager]") {
    ShowFixture f;
    Model* t = f.Tree("T");
    Model* h = t->GetSubModel("h");
    REQUIRE(h != nullptr);
    CHECK(f.mm.IsModelValid(h));
    f.mm.Delete("T");
    CHECK_FALSE(f.mm.IsModelValid(t));
}

TEST_CASE("Deleting every model empties the manager", "[models2][manager]") {
    ShowFixture f;
    f.Line("A", 5, "1");
    f.Line("B", 5, "100");
    f.Group("G", "A,B");
    CHECK(f.mm.Delete("A"));
    CHECK(f.mm.Delete("B"));
    CHECK(f.mm.Delete("G"));
    CHECK(f.mm.size() == 0);
    CHECK(f.mm.GetLastChannel() == 0);
}

TEST_CASE("Removing a submodel from a model removes it from lookups", "[models2][manager]") {
    ShowFixture f;
    Model* t = f.Tree("T");
    t->RemoveSubModel("h");
    CHECK(t->GetSubModel("h") == nullptr);
    CHECK(f.mm.GetModel("T/h") == nullptr);
    CHECK(f.mm.GetModel("T/v") != nullptr);
    CHECK(t->GetSubModels().size() == 1);
}
