#include <algorithm>
#include <functional>
#include <list>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "XmlSerializer/XmlSerializer.h"
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
        return mm.createAndAddModel(doc.first_child(), 100, 100);
    }

    Model* Line(const std::string& name, int nodes, const std::string& startChannel = "1", const std::string& extra = "") {
        return Add("<model name=\"" + name + "\" DisplayAs=\"Single Line\" NumStrings=\"1\" NodesPerString=\"" + std::to_string(nodes) +
                   "\" StartChannel=\"" + startChannel + "\" " + extra + "/>");
    }

    ModelGroup* Group(const std::string& name, const std::string& members, const std::string& extra = "") {
        Model* m = Add("<modelGroup name=\"" + name + "\" models=\"" + members + "\" " + extra + "/>");
        auto* g = dynamic_cast<ModelGroup*>(m);
        REQUIRE(g != nullptr);
        return g;
    }
};

} // namespace

TEST_CASE("Group lists its member names in order", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    f.Line("B", 5);
    f.Line("C", 5);
    ModelGroup* g = f.Group("G", "C,A,B");
    CHECK(g->ModelNames() == std::vector<std::string>{ "C", "A", "B" });
    CHECK(g->GetModelCount() == 3);
    CHECK(g->GetDisplayAs() == DisplayAsType::ModelGroup);
    CHECK(g->GetName() == "G");
}

TEST_CASE("Group resolves members to model pointers", "[models][group]") {
    ShowFixture f;
    Model* a = f.Line("A", 5);
    Model* b = f.Line("B", 5);
    ModelGroup* g = f.Group("G", "A,B");
    REQUIRE(g->Models().size() == 2);
    CHECK(g->Models()[0] == a);
    CHECK(g->Models()[1] == b);
    CHECK(g->GetModel("A") == a);
    CHECK(g->GetModel("Missing") == nullptr);
    CHECK(g->GetFirstModel() == a);
}

TEST_CASE("Group keeps names of members that do not exist but does not resolve them", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    ModelGroup* g = f.Group("G", "A,Ghost");
    CHECK(g->ModelNames().size() == 2);
    CHECK(g->Models().size() == 1);
    CHECK(g->GetModelCount() == 1);
}

TEST_CASE("Group member names are trimmed and blanks skipped", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    f.Line("B", 5);
    ModelGroup* g = f.Group("G", " A , ,B,,");
    CHECK(g->ModelNames() == std::vector<std::string>{ "A", "B" });
}

TEST_CASE("Empty group is valid", "[models][group]") {
    ShowFixture f;
    ModelGroup* g = f.Group("G", "");
    CHECK(g->ModelNames().empty());
    CHECK(g->GetModelCount() == 0);
    CHECK(g->GetFirstModel() == nullptr);
    CHECK(g->GetFirstChannel() == 0);
    CHECK(g->GetLastChannel() == 0);
    CHECK(g->GetFlatModels().empty());
    CHECK_FALSE(g->OnlyContainsModel("A"));
}

TEST_CASE("Group channel range spans its members", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5, "100");
    f.Line("B", 5, "500");
    f.Line("Outside", 5, "1000");
    ModelGroup* g = f.Group("G", "A,B");
    f.mm.RecalcStartChannels();
    CHECK(g->GetFirstChannel() == 99);
    CHECK(g->GetLastChannel() == 499 + 15 - 1);
}

TEST_CASE("Group buffer node count adds up the member nodes", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5, "1");
    f.Line("B", 7, "100");
    ModelGroup* g = f.Group("G", "A,B");
    f.mm.RecalcStartChannels();
    std::vector<NodeBaseClassPtr> nodes;
    int w = 0, h = 0;
    g->InitRenderBufferNodes("Default", "2D", "None", nodes, w, h, 0);
    CHECK(nodes.size() == 12);
}

TEST_CASE("Group buffer styles include the group specific styles", "[models][group]") {
    ShowFixture f;
    ModelGroup* g = f.Group("G", "");
    const auto& styles = g->GetBufferStyles();
    for (const char* s : { "Default", "Horizontal Stack", "Vertical Stack", "Overlay - Centered", "Per Model Default", "Single Line" }) {
        INFO(s);
        CHECK(std::find(styles.begin(), styles.end(), s) != styles.end());
    }
}

TEST_CASE("Adding a member twice does not duplicate it", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    ModelGroup* g = f.Group("G", "A");
    g->AddModel("A");
    g->AddModel("  A  ");
    CHECK(g->ModelNames().size() == 1);
}

TEST_CASE("Adding a member resolves it immediately", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    Model* b = f.Line("B", 5);
    ModelGroup* g = f.Group("G", "A");
    g->AddModel("B");
    CHECK(g->ModelNames() == std::vector<std::string>{ "A", "B" });
    CHECK(g->DirectlyContainsModel(b));
    CHECK(g->DirectlyContainsModel(std::string("B")));
    CHECK_FALSE(g->DirectlyContainsModel(std::string("C")));
}

TEST_CASE("Removing a member removes its submodel entries too", "[models][group]") {
    ShowFixture f;
    f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="1" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="h" layout="horizontal" type="ranges" bufferstyle="Default" line0="1-5"/>
    </model>)");
    f.Line("A", 5);
    ModelGroup* g = f.Group("G", "A,T,T/h");
    g->ModelRemoved("T");
    CHECK(g->ModelNames() == std::vector<std::string>{ "A" });
}

TEST_CASE("RemoveDuplicates keeps the first occurrence", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    f.Line("B", 5);
    ModelGroup* g = f.Group("G", "");
    g->SetModels({ "A", "B", "A", "B", "A" });
    CHECK(g->ModelNames().size() == 5);
    CHECK(g->RemoveDuplicates());
    CHECK(g->ModelNames() == std::vector<std::string>{ "A", "B" });
    CHECK_FALSE(g->RemoveDuplicates());
}

TEST_CASE("Nested groups are searched for contained models", "[models][group]") {
    ShowFixture f;
    Model* a = f.Line("A", 5);
    Model* b = f.Line("B", 5);
    Model* c = f.Line("C", 5);
    ModelGroup* inner = f.Group("Inner", "A,B");
    ModelGroup* outer = f.Group("Outer", "Inner,C");
    CHECK(outer->ContainsModel(a));
    CHECK(outer->ContainsModel(b));
    CHECK(outer->ContainsModel(c));
    CHECK(inner->ContainsModel(a));
    CHECK_FALSE(inner->ContainsModel(c));
    CHECK(outer->ContainsModelGroup(inner));
    CHECK_FALSE(inner->ContainsModelGroup(outer));
    CHECK_FALSE(outer->DirectlyContainsModel(a));
}

TEST_CASE("Flat model list expands nested groups without duplicates", "[models][group]") {
    ShowFixture f;
    Model* a = f.Line("A", 5);
    Model* b = f.Line("B", 5);
    Model* c = f.Line("C", 5);
    f.Group("Inner", "A,B");
    ModelGroup* outer = f.Group("Outer", "C,Inner,A");
    auto flat = outer->GetFlatModels();
    CHECK(flat == std::list<Model*>{ c, a, b });
    auto dup = outer->GetFlatModels(false);
    CHECK(dup == std::list<Model*>{ c, a, b, a });
}

TEST_CASE("Inactive members are excluded from the active model list", "[models][group]") {
    ShowFixture f;
    Model* a = f.Line("A", 5);
    f.Line("Off", 5, "1", "Active=\"0\"");
    ModelGroup* g = f.Group("G", "A,Off");
    CHECK(g->Models().size() == 2);
    REQUIRE(g->ActiveModels().size() == 1);
    CHECK(g->ActiveModels()[0] == a);
    CHECK(g->GetFlatModels().size() == 1);
    CHECK(g->GetFlatModels(true, false).size() == 2);
}

TEST_CASE("Group loading skips duplicate members", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    f.Line("B", 5, "100");
    ModelGroup* g = f.Group("G", "A,B,A,B,A");
    CHECK(g->ModelNames() == std::vector<std::string>{ "A", "B" });
}

TEST_CASE("A group cannot add itself as a member", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    ModelGroup* g = f.Group("G", "A");
    g->AddModel("G");
    CHECK(g->ModelNames() == std::vector<std::string>{ "A" });
    g->SetModels({ "A", "G" });
    CHECK(g->ModelNames() == std::vector<std::string>{ "A" });
    CHECK(g->MemberWouldCreateLoop("G"));
    CHECK_FALSE(g->MemberWouldCreateLoop("A"));
    CHECK_FALSE(g->MemberWouldCreateLoop("Unknown"));
}

TEST_CASE("A group naming itself in XML never resolves itself as a member", "[models][group]") {
    ShowFixture f;
    Model* a = f.Line("A", 5);
    ModelGroup* g = f.Group("G", "A,G");
    REQUIRE(g->Models().size() == 1);
    CHECK(g->Models()[0] == a);
}

// ModelGroup::GetFlatModels has no cycle guard, so a group that names itself
// (accepted when loaded through createAndAddModel) recurses forever.
TEST_CASE("A self naming group crashes GetFlatModels", "[known-crash][.]") {
    ShowFixture f;
    Model* a = f.Line("A", 5);
    ModelGroup* g = f.Group("G", "A,G");
    CHECK(g->GetFlatModels() == std::list<Model*>{ a });
}

TEST_CASE("Groups that would contain each other are refused", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    ModelGroup* one = f.Group("One", "A");
    ModelGroup* two = f.Group("Two", "One");
    CHECK(two->ContainsModelGroup(one));
    CHECK(one->MemberWouldCreateLoop("Two"));
    one->AddModel("Two");
    CHECK(one->ModelNames() == std::vector<std::string>{ "A" });
    one->SetModels({ "A", "Two" });
    CHECK(one->ModelNames() == std::vector<std::string>{ "A" });
}

// Two groups that name each other can both be created through
// createAndAddModel when the first is created before the second exists (the
// loop check only sees groups already in the manager). RecalcStartChannels then
// recurses until the stack overflows. LoadGroups avoids this path, so it is
// hidden; run it by name to reproduce.
TEST_CASE("Mutually referencing groups crash a recalculation", "[known-crash][.]") {
    ShowFixture f;
    f.Line("A", 5);
    f.Group("One", "A,Two");
    f.Group("Two", "One");
    f.mm.RecalcStartChannels();
    SUCCEED();
}

TEST_CASE("Submodels can be group members", "[models][group][submodel]") {
    ShowFixture f;
    Model* t = f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="1" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="h" layout="horizontal" type="ranges" bufferstyle="Default" line0="1-5"/>
    </model>)");
    ModelGroup* g = f.Group("G", "T/h");
    REQUIRE(g->Models().size() == 1);
    CHECK(g->Models()[0]->GetDisplayAs() == DisplayAsType::SubModel);
    CHECK(g->GetModel("T/h") == g->Models()[0]);
    CHECK_FALSE(g->ContainsModel(t));
    CHECK(g->ContainsModelOrSubmodel(t));
    CHECK(g->ContainsModel(t->GetSubModel("h")));
    CHECK(g->GetFirstModel() == t);
    CHECK(g->OnlyContainsModel("T"));
}

TEST_CASE("Group buffer with a submodel counts only the submodel nodes", "[models][group][submodel]") {
    ShowFixture f;
    f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="1" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="h" layout="horizontal" type="ranges" bufferstyle="Default" line0="1-5"/>
    </model>)");
    ModelGroup* g = f.Group("G", "T/h");
    std::vector<NodeBaseClassPtr> nodes;
    int w = 0, h = 0;
    g->InitRenderBufferNodes("Default", "2D", "None", nodes, w, h, 0);
    CHECK(nodes.size() == 5);
}

// ModelGroup::ContainsModelOrSubmodel and OnlyContainsModel compare names with
// StartsWith (ModelGroup.cpp:204, 283, 310), so "Tree10" is mistaken for a
// submodel of "Tree1".
TEST_CASE("Group membership of a model is not confused by a longer model name", "[models][group][!mayfail]") {
    ShowFixture f;
    Model* tree1 = f.Line("Tree1", 5);
    f.Line("Tree10", 5, "100");
    ModelGroup* g = f.Group("G", "Tree10");
    CHECK_FALSE(g->ContainsModelOrSubmodel(tree1));
    CHECK_FALSE(g->OnlyContainsModel("Tree1"));
    CHECK(f.mm.GetGroupsContainingModelOrSubmodel(tree1).empty());
}

TEST_CASE("Groups containing a model are found through nested groups", "[models][group]") {
    ShowFixture f;
    Model* a = f.Line("A", 5);
    f.Line("B", 5);
    f.Group("Inner", "A");
    f.Group("Outer", "Inner");
    f.Group("Other", "B");
    auto groups = f.mm.GetGroupsContainingModel(a);
    std::sort(groups.begin(), groups.end());
    CHECK(groups == std::vector<std::string>{ "Inner", "Outer" });
    auto objs = f.mm.GetModelGroups(a);
    CHECK(objs.size() == 2);
    CHECK(f.mm.GetGroupsContainingModelOrSubmodel(a).size() == 2);
}

TEST_CASE("GetGroupsContainingModel tolerates a null model", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    f.Group("G", "A");
    CHECK(f.mm.GetGroupsContainingModel(nullptr).empty());
}

TEST_CASE("Renaming a model renames it inside groups", "[models][group]") {
    ShowFixture f;
    f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="1" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="h" layout="horizontal" type="ranges" bufferstyle="Default" line0="1-5"/>
    </model>)");
    f.Line("A", 5);
    ModelGroup* g = f.Group("G", "A,T,T/h");
    CHECK(f.mm.Rename("T", "Tree"));
    CHECK(g->ModelNames() == std::vector<std::string>{ "A", "Tree", "Tree/h" });
    CHECK(g->Models().size() == 3);
}

TEST_CASE("Renaming a model to an existing name merges duplicate group entries", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    f.Line("B", 5, "100");
    ModelGroup* g = f.Group("G", "A,B");
    f.mm.Rename("A", "B2");
    CHECK(g->ModelNames() == std::vector<std::string>{ "B2", "B" });
    CHECK(g->ModelRenamed("B2", "B"));
    CHECK(g->ModelNames() == std::vector<std::string>{ "B" });
}

TEST_CASE("Renaming a submodel in a group updates the entry", "[models][group][submodel]") {
    ShowFixture f;
    f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="1" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="h" layout="horizontal" type="ranges" bufferstyle="Default" line0="1-5"/>
    </model>)");
    ModelGroup* g = f.Group("G", "T/h");
    CHECK(f.mm.RenameSubModel("T/h", "T/top"));
    CHECK(g->ModelNames() == std::vector<std::string>{ "T/top" });
    CHECK_FALSE(f.mm.RenameSubModel("T/nothing", "T/x"));
}

TEST_CASE("Deleting a model removes it from every group", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    f.Line("B", 5, "100");
    ModelGroup* g = f.Group("G", "A,B");
    ModelGroup* outer = f.Group("Outer", "G,A");
    CHECK(f.mm.Delete("A"));
    CHECK(f.mm["A"] == nullptr);
    CHECK(g->ModelNames() == std::vector<std::string>{ "B" });
    CHECK(outer->ModelNames() == std::vector<std::string>{ "G" });
    CHECK(g->Models().size() == 1);
}

TEST_CASE("Deleting a model that does not exist returns false", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    CHECK_FALSE(f.mm.Delete("Missing"));
    CHECK_FALSE(f.mm.Delete(""));
    CHECK(f.mm.size() == 1);
}

TEST_CASE("Deleting a model re-points models chained to it", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5, "10");
    f.Line("B", 5, ">A:1");
    Model* c = f.Line("C", 5, ">B:1");
    CHECK(f.mm.Delete("B"));
    f.mm.RecalcStartChannels();
    CHECK(c->GetModelStartChannel() == ">A:1");
    CHECK(c->GetFirstChannel() == 9 + 15);
}

TEST_CASE("Deleting a model that a group depends on leaves the group usable", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    ModelGroup* g = f.Group("G", "A");
    f.mm.Delete("A");
    CHECK(g->GetModelCount() == 0);
    CHECK(g->GetFirstChannel() == 0);
    CHECK(g->GetLastChannel() == 0);
}

TEST_CASE("Replacing a model keeps group membership by name", "[models][group]") {
    ShowFixture f;
    Model* a = f.Line("A", 5);
    ModelGroup* g = f.Group("G", "A");
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<model name="A" DisplayAs="Single Line" NumStrings="1" NodesPerString="9" StartChannel="1"/>)"));
    Model* replacement = f.mm.CreateModel(doc.first_child(), 100, 100);
    f.mm.ReplaceModel("A", replacement);
    REQUIRE(f.mm["A"] == replacement);
    CHECK(f.mm["A"] != a);
    REQUIRE(g->Models().size() == 1);
    CHECK(g->Models()[0] == replacement);
    CHECK(g->Models()[0]->GetNodeCount() == 9);
}

TEST_CASE("Group replacement modes reconcile memberships", "[models][group]") {
    ShowFixture f;
    f.Line("Source", 5);
    f.Line("Target", 5, "100");
    ModelGroup* srcGroup = f.Group("SrcGroup", "Source");
    ModelGroup* tgtGroup = f.Group("TgtGroup", "Target");

    f.mm.ReconcileReplacedModelGroups("Source", { "Target" }, ReplaceGroupMode::NoChange);
    CHECK(srcGroup->ModelNames() == std::vector<std::string>{ "Source" });
    CHECK(tgtGroup->ModelNames() == std::vector<std::string>{ "Target" });

    f.mm.ReconcileReplacedModelGroups("Source", { "Target" }, ReplaceGroupMode::MergeSourceIntoTarget);
    CHECK(srcGroup->ModelNames() == std::vector<std::string>{ "Source", "Target" });
    CHECK(tgtGroup->ModelNames() == std::vector<std::string>{ "Target" });

    f.mm.ReconcileReplacedModelGroups("Source", { "Target" }, ReplaceGroupMode::MergeSourceIntoTarget);
    CHECK(srcGroup->ModelNames() == std::vector<std::string>{ "Source", "Target" });
}

TEST_CASE("Group replacement mode ReplaceWithSource drops the old memberships", "[models][group]") {
    ShowFixture f;
    f.Line("Source", 5);
    f.Line("Target", 5, "100");
    ModelGroup* srcGroup = f.Group("SrcGroup", "Source");
    ModelGroup* tgtGroup = f.Group("TgtGroup", "Target");
    f.mm.ReconcileReplacedModelGroups("Source", { "Target" }, ReplaceGroupMode::ReplaceWithSource);
    CHECK(srcGroup->ModelNames() == std::vector<std::string>{ "Source", "Target" });
    CHECK(tgtGroup->ModelNames().empty());
}

TEST_CASE("Group replacement ignores empty requests", "[models][group]") {
    ShowFixture f;
    f.Line("Source", 5);
    ModelGroup* g = f.Group("G", "Source");
    f.mm.ReconcileReplacedModelGroups("", { "Target" }, ReplaceGroupMode::MergeSourceIntoTarget);
    f.mm.ReconcileReplacedModelGroups("Source", {}, ReplaceGroupMode::MergeSourceIntoTarget);
    CHECK(g->ModelNames() == std::vector<std::string>{ "Source" });
}

TEST_CASE("Group centre and offset settings come from XML", "[models][group]") {
    ShowFixture f;
    ModelGroup* g = f.Group("G", "", "centreDefined=\"1\" centrex=\"0.25\" centrey=\"0.75\" centreMinx=\"1\" centreMiny=\"2\" centreMaxx=\"3\" centreMaxy=\"4\" XCentreOffset=\"5\" YCentreOffset=\"6\" GridSize=\"100\" layout=\"vertical\" DefaultCamera=\"3D\"");
    CHECK(g->GetCentreDefined());
    CHECK(g->GetCentreX() == 0.25f);
    CHECK(g->GetCentreY() == 0.75f);
    CHECK(g->GetCentreMinx() == 1);
    CHECK(g->GetCentreMiny() == 2);
    CHECK(g->GetCentreMaxx() == 3);
    CHECK(g->GetCentreMaxy() == 4);
    CHECK(g->GetXCentreOffset() == 5);
    CHECK(g->GetYCentreOffset() == 6);
    CHECK(g->GetGridSize() == 100);
    CHECK(g->GetLayout() == "vertical");
    CHECK(g->GetDefaultCamera() == "3D");
}

TEST_CASE("Group centre is undefined unless centreDefined is set", "[models][group]") {
    ShowFixture f;
    ModelGroup* g = f.Group("G", "", "centrex=\"0.25\" centrey=\"0.75\"");
    CHECK_FALSE(g->GetCentreDefined());
}

TEST_CASE("Group aliases load from XML", "[models][group]") {
    ShowFixture f;
    Model* m = f.Add(R"(<modelGroup name="G" models=""><Aliases><alias name="Old G"/></Aliases></modelGroup>)");
    REQUIRE(m != nullptr);
    CHECK(m->IsAlias("Old G"));
}

TEST_CASE("Group with thousands of members loads", "[models][group]") {
    ShowFixture f;
    f.Line("A", 5);
    std::vector<std::string> members;
    for (int i = 0; i < 3000; ++i) {
        members.push_back(i % 2 == 0 ? "A" : "Missing");
    }
    ModelGroup* g = f.Group("G", "");
    g->SetModels(members);
    CHECK(g->ModelNames().size() == 3000);
    CHECK(g->Models().size() == 1500);
    CHECK(g->RemoveDuplicates());
    CHECK(g->ModelNames() == std::vector<std::string>{ "A", "Missing" });
}

TEST_CASE("Model manager starts empty and tracks additions", "[models][manager]") {
    ShowFixture f;
    CHECK(f.mm.size() == 0);
    CHECK(f.mm.begin() == f.mm.end());
    f.Line("A", 5);
    f.Line("B", 5);
    CHECK(f.mm.size() == 2);
    CHECK(f.mm["A"] != nullptr);
    CHECK(f.mm.GetModel("B") != nullptr);
    CHECK(f.mm["C"] == nullptr);
    CHECK(f.mm[""] == nullptr);
}

TEST_CASE("Model manager iterates models in name order", "[models][manager]") {
    ShowFixture f;
    f.Line("Zed", 5);
    f.Line("Alpha", 5, "100");
    f.Line("Mid", 5, "200");
    std::vector<std::string> names;
    for (const auto& it : f.mm) {
        names.push_back(it.first);
    }
    CHECK(names == std::vector<std::string>{ "Alpha", "Mid", "Zed" });
}

TEST_CASE("Model lookup tolerates surrounding whitespace", "[models][manager]") {
    ShowFixture f;
    Model* a = f.Line("A", 5);
    CHECK(f.mm["  A "] == a);
}

TEST_CASE("Model lookup finds submodels by path", "[models][manager]") {
    ShowFixture f;
    f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="1" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="h" layout="horizontal" type="ranges" bufferstyle="Default" line0="1-5"/>
    </model>)");
    CHECK(f.mm["T/h"] != nullptr);
    CHECK(f.mm[" T / h "] != nullptr);
    CHECK(f.mm["T/"] == nullptr);
    CHECK(f.mm["/h"] == nullptr);
    CHECK(f.mm["/"] == nullptr);
}

TEST_CASE("GenerateModelName keeps unique names and numbers clashes", "[models][manager]") {
    ShowFixture f;
    f.Line("Tree", 5);
    f.Line("Arch 1", 5, "100");
    f.Line("Pole_1", 5, "200");
    f.Line("Bar-1", 5, "300");
    f.Line("Star1", 5, "400");
    CHECK(f.mm.GenerateModelName("Fresh") == "Fresh");
    CHECK(f.mm.GenerateModelName("Tree") == "Tree-2");
    CHECK(f.mm.GenerateModelName("Arch 1") == "Arch 2");
    CHECK(f.mm.GenerateModelName("Pole_1") == "Pole_2");
    CHECK(f.mm.GenerateModelName("Bar-1") == "Bar-2");
    CHECK(f.mm.GenerateModelName("Star1") == "Star2");
    CHECK(f.mm.GetLastGeneratedModelName() == "Star2");
}

TEST_CASE("GenerateModelName skips names that are taken", "[models][manager]") {
    ShowFixture f;
    f.Line("Tree", 5);
    f.Line("Tree-2", 5, "100");
    f.Line("Tree-3", 5, "200");
    CHECK(f.mm.GenerateModelName("Tree") == "Tree-4");
}

TEST_CASE("Model manager clear removes everything", "[models][manager]") {
    ShowFixture f;
    Model* a = f.Line("A", 5);
    f.Group("G", "A");
    CHECK(f.mm.IsModelValid(a));
    f.mm.clear();
    CHECK(f.mm.size() == 0);
    CHECK(f.mm["A"] == nullptr);
    CHECK(f.mm["G"] == nullptr);
}

TEST_CASE("IsModelValid recognises live models and submodels", "[models][manager]") {
    ShowFixture f;
    Model* t = f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="1" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="h" layout="horizontal" type="ranges" bufferstyle="Default" line0="1-5"/>
    </model>)");
    CHECK(f.mm.IsModelValid(t));
    CHECK(f.mm.IsModelValid(t->GetSubModel("h")));
    CHECK(f.mm.IsModelValid(nullptr));
    ShowFixture other;
    Model* stranger = other.Line("X", 5);
    CHECK_FALSE(f.mm.IsModelValid(stranger));
}

TEST_CASE("Model generation changes when models change", "[models][manager]") {
    ShowFixture f;
    auto g0 = f.mm.GetModelGeneration();
    f.Line("A", 5);
    auto g1 = f.mm.GetModelGeneration();
    CHECK(g1 != g0);
    f.mm.Delete("A");
    CHECK(f.mm.GetModelGeneration() != g1);
}

TEST_CASE("Adding a model with an existing name replaces the old one", "[models][manager]") {
    ShowFixture f;
    f.Line("A", 5);
    Model* second = f.Line("A", 8);
    CHECK(f.mm.size() == 1);
    CHECK(f.mm["A"] == second);
    CHECK(f.mm["A"]->GetNodeCount() == 8);
}

TEST_CASE("Adding a null model is ignored", "[models][manager]") {
    ShowFixture f;
    f.mm.AddModel(nullptr);
    CHECK(f.mm.size() == 0);
}

TEST_CASE("Rename to the same name or an unsafe name behaves", "[models][manager]") {
    ShowFixture f;
    f.Line("A", 5);
    f.mm.Rename("A", "A");
    CHECK(f.mm["A"] != nullptr);
    CHECK(f.mm.size() == 1);
    f.mm.Rename("A", "  Padded  ");
    CHECK(f.mm["Padded"] != nullptr);
    CHECK(f.mm["A"] == nullptr);
}

TEST_CASE("Renaming a submodel through the model manager is refused", "[models][manager][submodel]") {
    ShowFixture f;
    f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="1" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="h" layout="horizontal" type="ranges" bufferstyle="Default" line0="1-5"/>
    </model>)");
    CHECK_FALSE(f.mm.Rename("T/h", "x"));
    CHECK(f.mm["T/h"] != nullptr);
}

TEST_CASE("Shadow model relationships are reported", "[models][manager]") {
    ShowFixture f;
    Model* a = f.Line("A", 5);
    f.Line("S", 5, "100", "ShadowModelFor=\"A\"");
    CHECK(f.mm.IsModelShadowing(a));
    CHECK(f.mm.GetModelsShadowing(a) == std::list<std::string>{ "S" });
    Model* s = f.mm["S"];
    CHECK_FALSE(f.mm.IsModelShadowing(s));
    CHECK(s->GetShadowModelFor() == "A");
}

TEST_CASE("Renaming a model updates models shadowing it", "[models][manager]") {
    ShowFixture f;
    f.Line("A", 5);
    Model* s = f.Line("S", 5, "100", "ShadowModelFor=\"A\"");
    f.mm.Rename("A", "B");
    CHECK(s->GetShadowModelFor() == "B");
}

TEST_CASE("Model sets track membership and renames", "[models][manager]") {
    ShowFixture f;
    f.Line("A", 5);
    f.Line("B", 5, "100");
    f.Line("C", 5, "200");
    auto& sets = f.mm.GetSetManager();
    ModelSet* s = sets.CreateSet({ "A", "B" }, "Pair");
    REQUIRE(s != nullptr);
    CHECK(sets.GetSetContaining("A") == s);
    CHECK(sets.GetSetContaining("C") == nullptr);
    f.mm.Rename("A", "A2");
    CHECK(sets.GetSetContaining("A2") == s);
    CHECK(sets.GetSetContaining("A") == nullptr);
    f.mm.Delete("B");
    CHECK(sets.GetSetContaining("A2") == nullptr);
}
