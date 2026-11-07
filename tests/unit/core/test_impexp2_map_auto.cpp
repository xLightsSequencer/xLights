#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "import_export/AutoMapper.h"
#include "import_export/BasicImportMappingNode.h"
#include "import_export/ImportMappingNode.h"
#include "test_impexp2_map_common.h"

#include <string>
#include <unordered_set>
#include <vector>

using namespace impexp2;

namespace {

const std::list<std::string> kNoAliases;

constexpr const char* kLayout =
    R"(<model name="Tree" DisplayAs="Matrix" StartChannel="1" NumStrings="1" NodesPerString="12" StrandsPerString="4" StringType="RGB Nodes">)"
    R"(<subModel name="Left" layout="horizontal" type="ranges" bufferstyle="Default" line0="1-3"/>)"
    R"(<subModel name="Right" layout="horizontal" type="ranges" bufferstyle="Default" line0="4-6"/></model>)"
    R"(<model name="Star" DisplayAs="Single Line" StartChannel="100" NumStrings="1" NodesPerString="4" StringType="RGB Nodes"/>)";

using Roots = std::vector<ImportMappingNode*>;
using Avail = std::vector<AvailableSource>;
const std::unordered_set<const ImportMappingNode*> kNoSel;

void RunNorm(const Roots& r, const Avail& a, RenderContext& rc, const std::string& mg = "B", bool selOnly = false,
             const std::unordered_set<const ImportMappingNode*>& sel = kNoSel) {
    AutoMapper::Run(r, a, rc, AutoMapper::MatchNorm, AutoMapper::MatchNorm, AutoMapper::MatchNorm, "", "", mg, selOnly, sel);
}
void RunAggr(const Roots& r, const Avail& a, RenderContext& rc, const std::string& mg = "B") {
    AutoMapper::Run(r, a, rc, AutoMapper::MatchAggressive, AutoMapper::MatchAggressive, AutoMapper::MatchAggressive, "", "", mg, false, kNoSel);
}

} // namespace

TEST_CASE("MatchNorm compares trimmed lowered target against the raw candidate", "[impexp2][mapper]") {
    auto [target, candidate, expected] = GENERATE(table<std::string, std::string, bool>({
        { "Star", "star", true },
        { "STAR", "star", true },
        { "  Star  ", "star", true },
        { "Star", "Star", false },
        { "Star", " star", false },
        { "Star", "star ", false },
        { "Star 2", "star 1", false },
        { "Star", "stars", false },
        { "Star", "sta", false },
        { "", "", true },
        { "   ", "", true },
        { "", "x", false },
        { "x", "", false },
        { "été", "été", true },
        { "日本語", "日本語", true },
        { "Ünï", "ünï", false },
        { "a/b", "a/b", true },
        { "Mega Tree", "mega tree", true },
        { "Mega  Tree", "mega tree", false },
    }));
    CAPTURE(target, candidate);
    CHECK(AutoMapper::MatchNorm(target, candidate, "", "", kNoAliases) == expected);
}

TEST_CASE("MatchNorm ignores aliases and extra arguments", "[impexp2][mapper]") {
    CHECK_FALSE(AutoMapper::MatchNorm("Star", "sun", "star", "sun", { "sun", "oldname:sun" }));
    CHECK(AutoMapper::MatchNorm("Star", "star", "zzz", "yyy", {}));
}

TEST_CASE("MatchAggressive strips punctuation from both sides", "[impexp2][mapper]") {
    auto [target, candidate, expected] = GENERATE(table<std::string, std::string, bool>({
        { "Mega Tree", "megatree", true },
        { "Mega-Tree", "mega_tree", true },
        { "Mega_Tree (1)", "megatree1", true },
        { "Mega Tree", "mega.tree", true },
        { "A:B;C|D", "abcd", true },
        { "A{B}[C]", "abc", true },
        { "A+B=C*D^E#F,G", "abcdefg", true },
        { "A\\B", "ab", true },
        { "Tree 1", "tree 2", false },
        { "Tree", "trees", false },
        { "Tree", "tre", false },
        { "TREE", "tree", true },
        { "  Tree  ", "tree", true },
        { "Tree", "tree.", true },
        { "été", "été", true },
        { "日本語 1", "日本語-1", true },
        { "Tree/1", "tree1", false },
        { "Tree!", "tree", false },
        { "Tree@", "tree", false },
        { "a'b", "ab", false },
        { "", "", true },
        { "", "x", false },
    }));
    CAPTURE(target, candidate);
    CHECK(AutoMapper::MatchAggressive(target, candidate, "", "", kNoAliases) == expected);
}

TEST_CASE("MatchAggressive matches aliases", "[impexp2][mapper]") {
    auto [aliases, candidate, expected] = GENERATE(table<std::list<std::string>, std::string, bool>({
        { { "oldname:bigstar" }, "bigstar", true },
        { { "oldname:big star" }, "big star", true },
        { { "oldname:bigstar" }, "big star", false },
        { { "oldname:big-star" }, "bigstar", false },
        { { "bigstar" }, "bigstar", true },
        { { "big star" }, "bigstar", true },
        { { "mega tree (vendor)" }, "megatreevendor", true },
        { { "mega tree (vendor)" }, "mega tree (vendor)", true },
        { { "other", "mega tree (vendor)" }, "megatreevendor", true },
        { { "other" }, "megatreevendor", false },
        { {}, "megatreevendor", false },
        { { "oldname:" }, "", true },
        { { "" }, "x", false },
    }));
    CAPTURE(aliases, candidate);
    CHECK(AutoMapper::MatchAggressive("Target", candidate, "", "", aliases) == expected);
}

TEST_CASE("MatchAggressive does not treat punctuation-only names as equal", "[impexp2][mapper][!mayfail]") {
    // Both names strip to "", so unrelated punctuation-only names compare equal.
    CHECK_FALSE(AutoMapper::MatchAggressive("---", "...", "", "", kNoAliases));
    CHECK_FALSE(AutoMapper::MatchAggressive("()", "[]", "", "", kNoAliases));
}

TEST_CASE("MatchRegex requires both candidate=replacement and a full pattern match", "[impexp2][mapper]") {
    auto [target, candidate, pattern, repl, expected] = GENERATE(table<std::string, std::string, std::string, std::string, bool>({
        { "Star1", "sun", "Star\\d", "sun", true },
        { "star1", "sun", "STAR\\d", "sun", true },
        { "Star12", "sun", "Star\\d", "sun", false },
        { "Star12", "sun", "Star\\d+", "sun", true },
        { "xStar1", "sun", "Star\\d", "sun", false },
        { "Star1", "moon", "Star\\d", "sun", false },
        { "Star1", " SUN ", "Star\\d", "sun", true },
        { "Star1", "sun", "Star\\d", " Sun", true },
        { "Star1", "sun", "(Star", "sun", false },
        { "Star1", "sun", "[", "sun", false },
        { "Star1", "sun", "*", "sun", false },
        { "Star1", "sun", "", "sun", false },
        { "", "sun", "", "sun", true },
        { "", "", "", "", true },
        { "Star1", "sun", ".*", "sun", true },
        { "Star1", "sun", "^Star1$", "sun", true },
        { "日本", "sun", "日本", "sun", true },
        { "A\nB", "sun", "A.B", "sun", false },
    }));
    CAPTURE(target, candidate, pattern, repl);
    CHECK(AutoMapper::MatchRegex(target, candidate, pattern, repl, kNoAliases) == expected);
}

TEST_CASE("MatchRegex recovers after an invalid pattern and caches valid ones", "[impexp2][mapper]") {
    CHECK(AutoMapper::MatchRegex("a1", "x", "a\\d", "x", kNoAliases));
    CHECK_FALSE(AutoMapper::MatchRegex("a1", "x", "(", "x", kNoAliases));
    CHECK_FALSE(AutoMapper::MatchRegex("a1", "x", "(", "x", kNoAliases));
    CHECK(AutoMapper::MatchRegex("a1", "x", "a\\d", "x", kNoAliases));
    CHECK(AutoMapper::MatchRegex("b1", "x", "b\\d", "x", kNoAliases));
    CHECK_FALSE(AutoMapper::MatchRegex("a1", "x", "b\\d", "x", kNoAliases));
}

TEST_CASE("MatchRegex copes with a long target", "[impexp2][mapper]") {
    std::string longName(5000, 'a');
    CHECK(AutoMapper::MatchRegex(longName, "x", "a+", "x", kNoAliases));
    CHECK_FALSE(AutoMapper::MatchRegex(longName, "x", "a{1,10}", "x", kNoAliases));
}

TEST_CASE("MatchRegex handles a very long target without overflowing the stack", "[impexp2][mapper][.][known-crash]") {
    // std::regex_match recurses per character; hundreds of thousands of chars can crash.
    std::string longName(500000, 'a');
    CHECK(AutoMapper::MatchRegex(longName, "x", "(a|b)*", "x", kNoAliases));
}

TEST_CASE("BasicImportMappingNode builds composite names", "[impexp2][mapper]") {
    auto [m, s, n, expected] = GENERATE(table<std::string, std::string, std::string, std::string>({
        { "Tree", "", "", "Tree" },
        { "Tree", "Left", "", "Tree/Left" },
        { "Tree", "Left", "Node 3", "Tree/Left/Node 3" },
        { "Tree", "", "Node 3", "Tree/Node 3" },
        { "", "", "", "" },
        { "", "S", "", "/S" },
        { "A/B", "C", "", "A/B/C" },
    }));
    BasicImportMappingNode node(m, s, n, false, {}, "", false);
    CHECK(node.GetModelName() == expected);
    CHECK(node.GetCoreModel() == m);
    CHECK(node.GetCoreStrand() == s);
    CHECK(node.GetCoreNode() == n);
}

TEST_CASE("BasicImportMappingNode mapping lifecycle", "[impexp2][mapper]") {
    BasicImportMappingNode root("Tree", "", "", true, { "a1", "a2" }, "ModelGroup", false);
    CHECK(root.IsGroup());
    CHECK(root.GetAliases() == std::list<std::string>{ "a1", "a2" });
    CHECK(root.GetMapping().empty());
    CHECK_FALSE(root.HasMapping());
    CHECK(root.GetChildCount() == 0);
    CHECK(root.GetNthChild(0) == nullptr);
    CHECK(root.GetNthChild(1000) == nullptr);

    auto* kid = root.AddChild(Node("Tree", "Left"));
    auto* grandkid = kid->AddChild(Node("Tree", "Left", "Node 1"));
    REQUIRE(root.GetChildCount() == 1);
    CHECK(root.GetNthChild(0) == kid);
    CHECK(root.GetNthChild(1) == nullptr);
    CHECK(kid->GetNthChild(0) == grandkid);
    CHECK_FALSE(root.HasMapping());

    grandkid->Map("Src", "Node");
    CHECK(grandkid->GetMapping() == "Src");
    CHECK(grandkid->_mappingModelType == "Node");
    CHECK(grandkid->_mappingExists);
    CHECK(root.HasMapping());
    CHECK(kid->HasMapping());

    root.AddStackedMapping("Extra", "Strand");
    CHECK(root._stackedMappings == std::vector<std::string>{ "Extra" });
    CHECK(root._stackedMappingModelTypes == std::vector<std::string>{ "Strand" });
    CHECK(root.GetMapping().empty());

    root.ClearMapping();
    CHECK_FALSE(root.HasMapping());
    CHECK(grandkid->GetMapping().empty());
    CHECK(grandkid->_mappingModelType.empty());
    CHECK(root._stackedMappings.empty());
    CHECK(root._stackedMappingModelTypes.empty());
}

TEST_CASE("BasicImportMappingNode remapping overwrites and empty maps count as unmapped", "[impexp2][mapper]") {
    BasicImportMappingNode n("A", "", "", false, {}, "", false);
    n.Map("one", "Model");
    n.Map("two", "ModelGroup");
    CHECK(n.GetMapping() == "two");
    CHECK(n._mappingModelType == "ModelGroup");
    n.Map("", "");
    CHECK_FALSE(n.HasMapping());
}

TEST_CASE("AutoMapper::Run maps by exact name with MatchNorm", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    REQUIRE(f.loaded);
    auto tree = Node("Tree");
    auto star = Node("Star");
    auto other = Node("Zebra");
    Avail av{ Src("tree", "Model"), Src("STAR", "Model") };
    RunNorm({ tree.get(), star.get(), other.get() }, av, *f.ctx);
    CHECK(tree->GetMapping() == "tree");
    CHECK(tree->_mappingModelType == "Model");
    CHECK(star->GetMapping() == "STAR");
    CHECK(other->GetMapping().empty());
}

TEST_CASE("AutoMapper::Run records the source model type for plain models", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto tree = Node("Tree");
    Avail av{ Src("Tree", "ModelGroup") };
    RunNorm({ tree.get() }, av, *f.ctx);
    CHECK(tree->GetMapping() == "Tree");
    CHECK(tree->_mappingModelType == "ModelGroup");
}

TEST_CASE("AutoMapper::Run first matching source wins and mapped nodes are not overwritten", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto tree = Node("Tree");
    Avail av{ Src("Tree"), Src("tree"), Src(" TREE ") };
    RunNorm({ tree.get() }, av, *f.ctx);
    CHECK(tree->GetMapping() == "Tree");
    // a second run must not replace an existing mapping
    Avail av2{ Src("TREE") };
    RunNorm({ tree.get() }, av2, *f.ctx);
    CHECK(tree->GetMapping() == "Tree");
}

TEST_CASE("AutoMapper::Run two destinations with the same name both map to the same source", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto a = Node("Tree");
    auto b = Node("Tree");
    Avail av{ Src("Tree") };
    RunNorm({ a.get(), b.get() }, av, *f.ctx);
    CHECK(a->GetMapping() == "Tree");
    CHECK(b->GetMapping() == "Tree");
}

TEST_CASE("AutoMapper::Run handles empty inputs", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    SECTION("no roots") {
        Avail av{ Src("Tree") };
        RunNorm({}, av, *f.ctx);
    }
    SECTION("no sources") {
        auto tree = Node("Tree");
        RunNorm({ tree.get() }, {}, *f.ctx);
        CHECK(tree->GetMapping().empty());
    }
    SECTION("null root is skipped") {
        auto tree = Node("Tree");
        Avail av{ Src("Tree") };
        RunNorm({ nullptr, tree.get(), nullptr }, av, *f.ctx);
        CHECK(tree->GetMapping() == "Tree");
    }
    SECTION("nothing at all") {
        RunNorm({}, {}, *f.ctx);
        AutoMapper::RunSubModelFallback({}, {}, *f.ctx, false, kNoSel);
    }
}

TEST_CASE("AutoMapper::Run model/group filter", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto [mg, mapModel, mapGroup] = GENERATE(table<std::string, bool, bool>({
        { "B", true, true },
        { "M", true, false },
        { "G", false, true },
        { "", false, false },
        { "X", false, false },
        { "b", false, false },
        { "BM", false, false },
    }));
    auto model = Node("Tree", "", "", false);
    auto group = Node("Star", "", "", true);
    Avail av{ Src("Tree"), Src("Star") };
    RunNorm({ model.get(), group.get() }, av, *f.ctx, mg);
    CAPTURE(mg);
    CHECK(model->GetMapping().empty() == !mapModel);
    CHECK(group->GetMapping().empty() == !mapGroup);
}

TEST_CASE("AutoMapper::Run with MatchAggressive finds punctuation and alias variants", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto a = Node("Mega Tree (1)");
    auto b = Node("Old Name", "", "", false, { "oldname:spire" });
    auto c = Node("Plain", "", "", false, { "sparkly star" });
    auto d = Node("Unmatched", "", "", false, { "nothing" });
    Avail av{ Src("MegaTree1"), Src("Spire"), Src("Sparkly-Star") };
    RunAggr({ a.get(), b.get(), c.get(), d.get() }, av, *f.ctx);
    CHECK(a->GetMapping() == "MegaTree1");
    CHECK(b->GetMapping() == "Spire");
    CHECK(c->GetMapping() == "Sparkly-Star");
    CHECK(d->GetMapping().empty());
}

TEST_CASE("AutoMapper::Run norm pass leaves near misses for the aggressive pass", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto a = Node("Mega-Tree");
    Avail av{ Src("Mega Tree") };
    RunNorm({ a.get() }, av, *f.ctx);
    CHECK(a->GetMapping().empty());
    RunAggr({ a.get() }, av, *f.ctx);
    CHECK(a->GetMapping() == "Mega Tree");
}

TEST_CASE("AutoMapper::Run handles ambiguous near-duplicate names by exactness", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto a = Node("Arch 1");
    auto b = Node("Arch 10");
    auto c = Node("Arch 2");
    Avail av{ Src("Arch 10"), Src("Arch 1"), Src("Arch 2") };
    RunNorm({ a.get(), b.get(), c.get() }, av, *f.ctx);
    CHECK(a->GetMapping() == "Arch 1");
    CHECK(b->GetMapping() == "Arch 10");
    CHECK(c->GetMapping() == "Arch 2");
}

TEST_CASE("AutoMapper::Run copes with unicode, empty and very long names", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    std::string longName(100000, 'x');
    auto uni = Node("Árbol été 日本");
    auto emp = Node("");
    auto lng = Node(longName);
    Avail av{ Src("Árbol été 日本"), Src(""), Src(longName), Src(longName + "y") };
    RunNorm({ uni.get(), emp.get(), lng.get() }, av, *f.ctx);
    CHECK(uni->GetMapping() == "Árbol été 日本");
    CHECK(emp->GetMapping() == "");
    CHECK(lng->GetMapping() == longName);
    // the empty destination ends up with an empty-string mapping, i.e. looks unmapped
    CHECK_FALSE(emp->HasMapping());
}

TEST_CASE("AutoMapper::Run slashed sources map strands by name through the parent model", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto tree = Node("Tree");
    auto* left = tree->AddChild(Node("Tree", "Left"));
    auto* right = tree->AddChild(Node("Tree", "Right"));
    Avail av{ Src("Tree/Left"), Src("Tree/Right") };
    RunNorm({ tree.get() }, av, *f.ctx);
    CHECK(left->GetMapping() == "Tree/Left");
    CHECK(right->GetMapping() == "Tree/Right");
    CHECK(left->_mappingModelType == "Strand");
    CHECK(tree->GetMapping().empty());
}

TEST_CASE("AutoMapper::Run slashed source needs the parent model to match", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto tree = Node("Tree");
    auto* left = tree->AddChild(Node("Tree", "Left"));
    Avail av{ Src("Other/Left") };
    RunNorm({ tree.get() }, av, *f.ctx);
    CHECK(left->GetMapping().empty());
}

TEST_CASE("AutoMapper::Run slashed edge forms never throw or map", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto name = GENERATE(as<std::string>{}, "/", "//", "///", "Tree/", "/Left", "Tree//Left", "Tree/Left/Node 1/Extra", "Tree/Left/Node 1/", "a/b/c/d/e/f");
    auto tree = Node("Tree");
    auto* left = tree->AddChild(Node("Tree", "Left"));
    left->AddChild(Node("Tree", "Left", "Node 1"));
    Avail av{ Src(name) };
    CAPTURE(name);
    RunNorm({ tree.get() }, av, *f.ctx);
    CHECK(tree->GetMapping().empty());
    CHECK(left->GetNthChild(0)->GetMapping().empty());
}

TEST_CASE("AutoMapper::Run three part sources map nodes of a matched strand", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto tree = Node("Tree");
    auto* left = tree->AddChild(Node("Tree", "Left"));
    auto* n1 = left->AddChild(Node("Tree", "Left", "Node 1"));
    auto* n2 = left->AddChild(Node("Tree", "Left", "Node 2"));
    Avail av{ Src("Tree/Left/Node 2"), Src("Tree/Left/Node 1") };
    RunNorm({ tree.get() }, av, *f.ctx);
    CHECK(n1->GetMapping() == "Tree/Left/Node 1");
    CHECK(n2->GetMapping() == "Tree/Left/Node 2");
    CHECK(n1->_mappingModelType == "Node");
    CHECK(left->GetMapping().empty());
}

TEST_CASE("AutoMapper::Run a strand with its own mapping still gets node mappings", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto tree = Node("Tree");
    auto* left = tree->AddChild(Node("Tree", "Left"));
    auto* n1 = left->AddChild(Node("Tree", "Left", "Node 1"));
    Avail av{ Src("Tree/Left"), Src("Tree/Left/Node 1") };
    RunNorm({ tree.get() }, av, *f.ctx);
    CHECK(left->GetMapping() == "Tree/Left");
    CHECK(n1->GetMapping() == "Tree/Left/Node 1");
}

TEST_CASE("AutoMapper::Run a strand mapping is not overwritten by a later duplicate source", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto tree = Node("Tree");
    auto* left = tree->AddChild(Node("Tree", "Left"));
    Avail av{ Src("Tree/Left"), Src("tree/LEFT") };
    RunNorm({ tree.get() }, av, *f.ctx);
    CHECK(left->GetMapping() == "Tree/Left");
}

TEST_CASE("AutoMapper::Run null children do not crash", "[impexp2][mapper]") {
    // BasicImportMappingNode::GetNthChild returns nullptr out of range; a custom node can return null in range
    struct NullKid : ImportMappingNode {
        std::string m = "Tree", e;
        std::string mapping;
        const std::string& GetCoreModel() const override { return m; }
        const std::string& GetCoreStrand() const override { return e; }
        const std::string& GetCoreNode() const override { return e; }
        const std::string& GetMapping() const override { return mapping; }
        std::list<std::string> GetAliases() const override { return {}; }
        bool IsGroup() const override { return false; }
        std::string GetModelName() const override { return m; }
        void Map(const std::string& to, const std::string&) override { mapping = to; }
        unsigned int GetChildCount() const override { return 3; }
        ImportMappingNode* GetNthChild(unsigned int) override { return nullptr; }
    } nk;
    LayoutFixture f(kLayout);
    Avail av{ Src("Tree/Left"), Src("Tree") };
    RunNorm({ &nk }, av, *f.ctx);
    CHECK(nk.mapping == "Tree");
    AutoMapper::RunSubModelFallback({ &nk }, av, *f.ctx, false, kNoSel);
}

TEST_CASE("AutoMapper::Run matches strands through submodel slashed aliases", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    REQUIRE(f.M("Tree") != nullptr);
    f.M("Tree")->AddAlias("Spire");
    Model* left = f.M("Tree")->GetSubModel("Left");
    REQUIRE(left != nullptr);
    left->AddAlias("Spire/Lhs");
    auto tree = Node("Tree", "", "", false, { "spire" });
    auto* l = tree->AddChild(Node("Tree", "Left"));
    auto* r = tree->AddChild(Node("Tree", "Right"));
    Avail av{ Src("Spire/Lhs") };
    RunAggr({ tree.get() }, av, *f.ctx);
    CHECK(l->GetMapping() == "Spire/Lhs");
    CHECK(r->GetMapping().empty());
}

TEST_CASE("AutoMapper::Run slashed alias is ignored when the model lacks the alias prefix", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    Model* left = f.M("Tree")->GetSubModel("Left");
    REQUIRE(left != nullptr);
    left->AddAlias("Spire/Lhs");
    auto tree = Node("Tree");
    auto* l = tree->AddChild(Node("Tree", "Left"));
    Avail av{ Src("Spire/Lhs") };
    RunAggr({ tree.get() }, av, *f.ctx);
    CHECK(l->GetMapping().empty());
}

TEST_CASE("AutoMapper::Run selection scoping", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto tree = Node("Tree");
    auto star = Node("Star");
    Avail av{ Src("Tree", "Model", false), Src("Star", "Model", true) };

    SECTION("only selected sources are used") {
        RunNorm({ tree.get(), star.get() }, av, *f.ctx, "B", true, {});
        CHECK(tree->GetMapping().empty());
        CHECK(star->GetMapping() == "Star");
    }
    SECTION("only selected targets are mapped") {
        Avail all{ Src("Tree"), Src("Star") };
        RunNorm({ tree.get(), star.get() }, all, *f.ctx, "B", true, { tree.get() });
        CHECK(tree->GetMapping() == "Tree");
        CHECK(star->GetMapping().empty());
    }
    SECTION("both restrictions") {
        RunNorm({ tree.get(), star.get() }, av, *f.ctx, "B", true, { tree.get() });
        CHECK(tree->GetMapping().empty());
        CHECK(star->GetMapping().empty());
    }
    SECTION("selectOnly with nothing selected maps everything") {
        Avail none{ Src("Tree"), Src("Star") };
        RunNorm({ tree.get(), star.get() }, none, *f.ctx, "B", true, {});
        CHECK(tree->GetMapping() == "Tree");
        CHECK(star->GetMapping() == "Star");
    }
    SECTION("selection ignored when selectOnly is false") {
        RunNorm({ tree.get(), star.get() }, av, *f.ctx, "B", false, { tree.get() });
        CHECK(tree->GetMapping() == "Tree");
        CHECK(star->GetMapping() == "Star");
    }
}

TEST_CASE("AutoMapper::Run selected submodels are mapped by composite name", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto tree = Node("Tree");
    auto* left = tree->AddChild(Node("Tree", "Left"));
    auto* right = tree->AddChild(Node("Tree", "Right"));
    Avail av{ Src("tree/left", "SubModel", true) };
    RunNorm({ tree.get() }, av, *f.ctx, "B", true, { tree.get(), left });
    CHECK(left->GetMapping() == "tree/left");
    CHECK(left->_mappingModelType == "Strand"); // already mapped by the main pass
    CHECK(right->GetMapping().empty());
}

TEST_CASE("AutoMapper::Run selected submodel matches via parent model aliases", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    f.M("Tree")->AddAlias("Spire");
    auto tree = Node("Tree");
    auto* left = tree->AddChild(Node("Tree", "Left"));
    Avail av{ Src("Spire/Left", "SubModel", true) };
    // Source "spire/left" is slashed, so the first loop is also applicable; the selected pass uses the model alias form.
    RunNorm({ tree.get() }, av, *f.ctx, "B", true, { tree.get(), left });
    CHECK(left->GetMapping() == "Spire/Left");
}

TEST_CASE("AutoMapper::Run selected submodel under an unknown layout model does not crash", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto ghost = Node("Ghost");
    auto* sm = ghost->AddChild(Node("Ghost", "Part"));
    Avail av{ Src("ghost/part", "SubModel", true) };
    RunNorm({ ghost.get() }, av, *f.ctx, "B", true, { ghost.get(), sm });
    CHECK(sm->GetMapping() == "ghost/part");
}

TEST_CASE("AutoMapper::Run regex matcher uses the hint pattern and substitution", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto a = Node("Star 1");
    auto b = Node("Star 2");
    auto c = Node("Moon");
    Avail av{ Src("Sun") };
    AutoMapper::Run({ a.get(), b.get(), c.get() }, av, *f.ctx, AutoMapper::MatchRegex, AutoMapper::MatchRegex, AutoMapper::MatchRegex,
                    "Star \\d", "sun", "B", false, kNoSel);
    CHECK(a->GetMapping() == "Sun");
    CHECK(b->GetMapping() == "Sun");
    CHECK(c->GetMapping().empty());
}

TEST_CASE("AutoMapper::Run regex matcher with an invalid pattern maps nothing", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto a = Node("Star 1");
    Avail av{ Src("Sun") };
    AutoMapper::Run({ a.get() }, av, *f.ctx, AutoMapper::MatchRegex, AutoMapper::MatchRegex, AutoMapper::MatchRegex,
                    "Star (", "sun", "B", false, kNoSel);
    CHECK(a->GetMapping().empty());
}

TEST_CASE("AutoMapper::Run uses distinct matchers per level", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto tree = Node("Tree");
    auto* left = tree->AddChild(Node("Tree", "Left"));
    int modelCalls = 0, strandCalls = 0, nodeCalls = 0;
    left->AddChild(Node("Tree", "Left", "N"));
    AutoMapper::MatcherFn mm = [&](const std::string& t, const std::string& c, const std::string&, const std::string&, const std::list<std::string>&) { ++modelCalls; return t == "Tree" && c == "tree"; };
    AutoMapper::MatcherFn sm = [&](const std::string& t, const std::string& c, const std::string&, const std::string&, const std::list<std::string>&) { ++strandCalls; return t == "Left" && c == "left"; };
    AutoMapper::MatcherFn nm = [&](const std::string& t, const std::string& c, const std::string&, const std::string&, const std::list<std::string>&) { ++nodeCalls; return t == "N" && c == "n"; };
    Avail av{ Src("Tree/Left/N") };
    AutoMapper::Run({ tree.get() }, av, *f.ctx, mm, sm, nm, "", "", "B", false, kNoSel);
    CHECK(modelCalls == 1);
    CHECK(strandCalls == 1);
    CHECK(nodeCalls == 1);
    CHECK(left->GetNthChild(0)->GetMapping() == "Tree/Left/N");
}

TEST_CASE("AutoMapper::RunSubModelFallback maps unmapped submodels by plain name", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto tree = Node("Tree");
    auto* left = tree->AddChild(Node("Tree", "Left"));
    auto* right = tree->AddChild(Node("Tree", "Right"));
    Avail av{ Src("LEFT"), Src("Other") };
    AutoMapper::RunSubModelFallback({ tree.get() }, av, *f.ctx, false, kNoSel);
    CHECK(left->GetMapping() == "LEFT");
    CHECK(left->_mappingModelType == "Unknown");
    CHECK(right->GetMapping().empty());
}

TEST_CASE("AutoMapper::RunSubModelFallback skips slashed sources and already mapped nodes", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto tree = Node("Tree");
    auto* left = tree->AddChild(Node("Tree", "Left"));
    auto* right = tree->AddChild(Node("Tree", "Right"));
    right->Map("Existing", "Strand");
    Avail av{ Src("Tree/Left"), Src("Right") };
    AutoMapper::RunSubModelFallback({ tree.get() }, av, *f.ctx, false, kNoSel);
    CHECK(left->GetMapping().empty());
    CHECK(right->GetMapping() == "Existing");
}

TEST_CASE("AutoMapper::RunSubModelFallback maps by submodel alias", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    f.M("Tree")->GetSubModel("Left")->AddAlias("Port Side");
    auto tree = Node("Tree");
    auto* left = tree->AddChild(Node("Tree", "Left"));
    auto* right = tree->AddChild(Node("Tree", "Right"));
    Avail av{ Src("PortSide") };
    AutoMapper::RunSubModelFallback({ tree.get() }, av, *f.ctx, false, kNoSel);
    CHECK(left->GetMapping() == "PortSide");
    CHECK(right->GetMapping().empty());
}

TEST_CASE("AutoMapper::RunSubModelFallback does not over-map physical strands via model aliases", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    f.M("Tree")->AddAlias("Spire");
    auto tree = Node("Tree", "", "", false, { "spire" });
    auto* s1 = tree->AddChild(Node("Tree", "Strand 1", "", false, { "spire" }));
    auto* s2 = tree->AddChild(Node("Tree", "Strand 2", "", false, { "spire" }));
    Avail av{ Src("Spire") };
    AutoMapper::RunSubModelFallback({ tree.get() }, av, *f.ctx, false, kNoSel);
    CHECK(s1->GetMapping().empty());
    CHECK(s2->GetMapping().empty());
}

TEST_CASE("AutoMapper::RunSubModelFallback honours selection", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto tree = Node("Tree");
    auto star = Node("Star");
    auto* left = tree->AddChild(Node("Tree", "Left"));
    auto* sl = star->AddChild(Node("Star", "Left"));
    Avail av{ Src("Left", "Model", false), Src("Left", "Model", true) };
    SECTION("selected targets") {
        AutoMapper::RunSubModelFallback({ tree.get(), star.get() }, av, *f.ctx, true, { star.get() });
        CHECK(left->GetMapping().empty());
        CHECK(sl->GetMapping() == "Left");
    }
    SECTION("no restrictions") {
        AutoMapper::RunSubModelFallback({ tree.get(), star.get() }, av, *f.ctx, false, {});
        CHECK(left->GetMapping() == "Left");
        CHECK(sl->GetMapping() == "Left");
    }
}

TEST_CASE("AutoMapper::RunSubModelFallback tolerates a null root and unknown layout model", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    auto ghost = Node("Ghost");
    auto* part = ghost->AddChild(Node("Ghost", "Part"));
    Avail av{ Src("Part") };
    AutoMapper::RunSubModelFallback({ nullptr, ghost.get() }, av, *f.ctx, false, kNoSel);
    CHECK(part->GetMapping() == "Part");
}

TEST_CASE("AutoMapper::Run stress with many sources and destinations", "[impexp2][mapper]") {
    LayoutFixture f(kLayout);
    std::vector<std::unique_ptr<BasicImportMappingNode>> nodes;
    Roots roots;
    Avail av;
    for (int i = 0; i < 300; ++i) {
        nodes.push_back(Node("Model " + std::to_string(i)));
        roots.push_back(nodes.back().get());
        av.push_back(Src("model " + std::to_string(299 - i)));
    }
    RunNorm(roots, av, *f.ctx);
    for (int i = 0; i < 300; ++i) {
        CHECK(nodes[i]->GetMapping() == "model " + std::to_string(i));
    }
}
