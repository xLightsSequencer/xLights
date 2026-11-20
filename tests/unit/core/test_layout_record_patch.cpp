#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "XmlSerializer/LayoutRecordPatch.h"

// The iPad saves the layout by patching the records of xlights_rgbeffects.xml
// as loaded from disk; a deleted model has to be removed from it by the name
// it has there.

namespace {

const char* kLayout = R"(<xrgb>
  <models>
    <model name="A" DisplayAs="Tree"/>
    <model name="B" DisplayAs="Arches"/>
    <model name="C" DisplayAs="Star"/>
  </models>
  <modelGroups>
    <modelGroup name="G" models="A,B,C"/>
    <modelGroup name="A" models="B"/>
  </modelGroups>
</xrgb>)";

std::vector<std::string> Names(pugi::xml_node parent, const char* element) {
    std::vector<std::string> out;
    for (pugi::xml_node n = parent.child(element); n; n = n.next_sibling(element)) {
        out.emplace_back(n.attribute("name").as_string());
    }
    return out;
}

struct Pending {
    std::set<std::string> deleted;
    std::map<std::string, std::string> renamed; // current name -> on-disk name
    std::set<std::string> dirty;
    void Delete(const std::string& n) { LayoutRecordPatch::MarkDeleted(n, deleted, renamed, dirty); }
};

} // namespace

TEST_CASE("A deleted model is removed from <models> and nothing else", "[layout][xml]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(kLayout));
    Pending p;
    p.dirty.insert("A");
    p.Delete("A");
    CHECK(p.deleted == std::set<std::string>{ "A" });
    CHECK(p.dirty.empty());

    auto root = doc.child("xrgb");
    CHECK(LayoutRecordPatch::RemoveNamed(root.child("models"), "model", p.deleted) == 1);
    CHECK(Names(root.child("models"), "model") == std::vector<std::string>{ "B", "C" });
    // A group that lists the model, and a group of the same name, are left alone.
    CHECK(Names(root.child("modelGroups"), "modelGroup") == std::vector<std::string>{ "G", "A" });
    CHECK(std::string(root.child("modelGroups").child("modelGroup").attribute("models").as_string()) == "A,B,C");
}

TEST_CASE("A model deleted after a rename is removed by its on-disk name", "[layout][xml]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(kLayout));
    Pending p;
    p.renamed["B2"] = "B"; // B renamed to B2 in memory
    p.dirty.insert("B2");
    p.Delete("B2");
    CHECK(p.deleted == std::set<std::string>{ "B" });
    CHECK(p.renamed.empty());
    CHECK(p.dirty.empty());

    auto models = doc.child("xrgb").child("models");
    CHECK(LayoutRecordPatch::RemoveNamed(models, "model", p.deleted) == 1);
    CHECK(Names(models, "model") == std::vector<std::string>{ "A", "C" });
}

TEST_CASE("Deleting a model that took a deleted model's name removes both on-disk records", "[layout][xml]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(kLayout));
    Pending p;
    p.Delete("A");        // on-disk A deleted
    p.renamed["A"] = "C"; // C then renamed to A
    p.Delete("A");        // and deleted as well
    CHECK(p.deleted == std::set<std::string>{ "A", "C" });

    auto models = doc.child("xrgb").child("models");
    CHECK(LayoutRecordPatch::RemoveNamed(models, "model", p.deleted) == 2);
    CHECK(Names(models, "model") == std::vector<std::string>{ "B" });
}

TEST_CASE("Removing names the file doesn't have changes nothing", "[layout][xml]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(kLayout));
    auto models = doc.child("xrgb").child("models");
    CHECK(LayoutRecordPatch::RemoveNamed(models, "model", { "Imported" }) == 0);
    CHECK(LayoutRecordPatch::RemoveNamed(models, "model", {}) == 0);
    CHECK(LayoutRecordPatch::RemoveNamed(pugi::xml_node(), "model", { "A" }) == 0);
    CHECK(Names(models, "model") == std::vector<std::string>{ "A", "B", "C" });
}
