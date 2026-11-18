#include <catch2/catch_test_macros.hpp>

#include "import_export/BasicImportMappingNode.h"
#include "import_export/MusicCatalog.h"
#include "import_export/VendorCatalog.h"
#include "import_export/AutoMapper.h"

#include <pugixml.hpp>

#include <string>

namespace {
pugi::xml_document P(const std::string& s) {
    pugi::xml_document d;
    d.load_string(s.c_str());
    return d;
}
}

TEST_CASE("BasicImportMappingNode default constructed is empty", "[impexp2][mapper]") {
    BasicImportMappingNode n;
    CHECK(n.GetModelName().empty());
    CHECK_FALSE(n.IsGroup());
    CHECK(n.GetAliases().empty());
    CHECK(n.GetChildCount() == 0);
    CHECK_FALSE(n.HasMapping());
    CHECK(n._nodeCount == 0);
    CHECK(n._strandCount == 0);
    CHECK(n._effectCount == 0);
    CHECK_FALSE(n._isSubmodel);
}

TEST_CASE("BasicImportMappingNode constructor stores every argument", "[impexp2][mapper]") {
    BasicImportMappingNode n("M", "S", "N", true, { "x" }, "ModelGroup", true);
    CHECK(n._model == "M");
    CHECK(n._strand == "S");
    CHECK(n._node == "N");
    CHECK(n._group);
    CHECK(n._modelType == "ModelGroup");
    CHECK(n._isSubmodel);
    CHECK(n.GetAliases().size() == 1);
}

TEST_CASE("BasicImportMappingNode children keep insertion order", "[impexp2][mapper]") {
    BasicImportMappingNode root;
    for (int i = 0; i < 5; ++i) {
        root.AddChild(std::make_unique<BasicImportMappingNode>("M", "S" + std::to_string(i), "", false, std::list<std::string>{}, "", true));
    }
    REQUIRE(root.GetChildCount() == 5);
    for (unsigned i = 0; i < 5; ++i) CHECK(root.GetNthChild(i)->GetCoreStrand() == "S" + std::to_string(i));
}

TEST_CASE("BasicImportMappingNode HasMapping looks through deep descendants", "[impexp2][mapper]") {
    BasicImportMappingNode root;
    BasicImportMappingNode* cur = &root;
    for (int i = 0; i < 100; ++i) cur = cur->AddChild(std::make_unique<BasicImportMappingNode>());
    CHECK_FALSE(root.HasMapping());
    cur->Map("deep", "Node");
    CHECK(root.HasMapping());
    root.ClearMapping();
    CHECK_FALSE(root.HasMapping());
}

TEST_CASE("BasicImportMappingNode stacked mappings are independent of the primary", "[impexp2][mapper]") {
    BasicImportMappingNode n;
    n.AddStackedMapping("a", "Model");
    n.AddStackedMapping("b", "Strand");
    CHECK(n._stackedMappings.size() == 2);
    CHECK(n._stackedMappingModelTypes.size() == 2);
    CHECK(n.GetMapping().empty());
    CHECK_FALSE(n.HasMapping());
}

TEST_CASE("AutoMapper matchers are usable as MatcherFn", "[impexp2][mapper]") {
    AutoMapper::MatcherFn f = AutoMapper::MatchNorm;
    CHECK(f("A", "a", "", "", {}));
    f = AutoMapper::MatchAggressive;
    CHECK(f("A-B", "ab", "", "", {}));
    f = AutoMapper::MatchRegex;
    CHECK(f("A1", "x", "A\\d", "x", {}));
}

TEST_CASE("AvailableSource defaults", "[impexp2][mapper]") {
    AvailableSource s;
    CHECK(s.displayName.empty());
    CHECK_FALSE(s.selected);
    CHECK(s.effectCount == 0);
    CHECK(s.durationMs == 0);
}

TEST_CASE("Vendor catalog Model ignores wiring element case", "[impexp2][catalog]") {
    auto d = P("<model><WIRING><name>x</name></WIRING><Wiring/></model>");
    vendor_catalog::Model m(d.document_element(), nullptr);
    CHECK(m._wiring.size() == 2);
}

TEST_CASE("Vendor catalog Model with only whitespace text elements", "[impexp2][catalog]") {
    auto d = P("<model><name>   </name><type>\n</type></model>");
    vendor_catalog::Model m(d.document_element(), nullptr);
    CHECK(m._name.find_first_not_of(" ") == std::string::npos);
}

TEST_CASE("Vendor catalog size strings fed through wiring dimensions", "[impexp2][catalog]") {
    auto d = P("<model><width>1 Feet</width><height>10\"</height><depth>0mm</depth><wiring/></model>");
    vendor_catalog::Model m(d.document_element(), nullptr);
    REQUIRE(m._wiring.size() == 1);
    CHECK(m._wiring.front()->GetWidthMM() == 304);
    CHECK(m._wiring.front()->GetHeightMM() == 254);
    CHECK(m._wiring.front()->GetDepthMM() == 0);
}

TEST_CASE("Vendor catalog wiring with many images", "[impexp2][catalog]") {
    std::string x = "<model>";
    for (int i = 0; i < 200; ++i) x += "<imagefile>u" + std::to_string(i) + "</imagefile>";
    x += "<wiring/></model>";
    auto d = P(x);
    vendor_catalog::Model m(d.document_element(), nullptr);
    CHECK(m._images.size() == 200);
    CHECK(m._wiring.front()->_images.size() == 200);
}

TEST_CASE("Vendor catalog Category constructed alone", "[impexp2][catalog]") {
    auto d = P("<category><id>9</id><name>N</name><categories><category><name>K</name></category><category/></categories></category>");
    vendor_catalog::Category c(d.document_element(), nullptr, nullptr);
    CHECK(c._id == "9");
    CHECK(c._categories.size() == 2);
    CHECK(c._categories.back()->GetPath() == "N/");
}

TEST_CASE("Music catalog Category constructed alone", "[impexp2][catalog]") {
    auto d = P("<category><id>9</id><name>N</name><categories><category><name>K</name></category></categories><zzz/></category>");
    music_catalog::Category c(d.document_element(), nullptr, nullptr);
    CHECK(c._id == "9");
    CHECK(c._categories.front()->GetPath() == "N/K");
}

TEST_CASE("Music catalog item with a very long title", "[impexp2][catalog]") {
    std::string t(100000, 'q');
    auto d = P("<song><title>" + t + "</title><sequence/></song>");
    music_catalog::Item it(d.document_element(), nullptr);
    CHECK(it.GetDownloadFileName() == t + ".zip");
    CHECK(it.GetDescription().size() > t.size());
}

TEST_CASE("Music catalog item with an empty download and a lyric type", "[impexp2][catalog]") {
    auto d = P("<song><lyric/><title>L</title></song>");
    music_catalog::Item it(d.document_element(), nullptr);
    CHECK(it.GetDownloadFileName() == "L.xtiming");
    CHECK(it.GetTypeName() == "Lyrics");
}

TEST_CASE("Music catalog vendor items point back at the vendor", "[impexp2][catalog]") {
    auto d = P("<musicinventory><music><song><title>a</title></song></music></musicinventory>");
    music_catalog::Vendor v(d, -1);
    REQUIRE(v._items.size() == 1);
    CHECK(v._items.front()->_vendor == &v);
}

TEST_CASE("Music catalog vendor with no music section", "[impexp2][catalog]") {
    auto d = P("<musicinventory><vendor><name>Only</name></vendor></musicinventory>");
    music_catalog::Vendor v(d, -1);
    CHECK(v._name == "Only");
    CHECK(v._items.empty());
}

TEST_CASE("Music catalog repeated music sections accumulate and the count limit is per section", "[impexp2][catalog]") {
    auto d = P("<musicinventory><music><song><title>b</title></song><song><title>a</title></song></music><music><song><title>c</title></song><song><title>d</title></song></music></musicinventory>");
    music_catalog::Vendor v(d, 1);
    CHECK(v._items.size() == 2);
}

TEST_CASE("Vendor catalog repeated model sections apply the limit per section", "[impexp2][catalog]") {
    auto d = P("<modelinventory><models><model/><model/></models><models><model/><model/></models></modelinventory>");
    vendor_catalog::Vendor v(d, 1);
    CHECK(v._models.size() == 2);
}

TEST_CASE("Music catalog download target with an empty-looking destination", "[impexp2][catalog]") {
    CHECK(music_catalog::Catalog::DownloadTo("u", "", "f").empty());
    CHECK(music_catalog::Catalog::DownloadTo("", "d", "f").empty());
    CHECK(music_catalog::Catalog::DownloadTo("u", "d", "").empty());
    CHECK(music_catalog::Catalog::DownloadTo("", "", "").empty());
}
