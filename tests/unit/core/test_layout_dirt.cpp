#include <string>

#include <catch2/catch_test_macros.hpp>

#include "utils/LayoutDirt.h"

// What a load-modify-write host keeps of its unsaved layout edits
// (utils/LayoutDirt.h): the records a save patches in, the base they are
// patched into, and the generation a live session watches.

TEST_CASE("LayoutDirt starts clean", "[layoutdirt]") {
    LayoutDirt d;
    CHECK_FALSE(d.HasLayoutChanges());
    CHECK_FALSE(d.HasChanges());
    CHECK_FALSE(d.Base().has_value());
    CHECK(d.Generation() == 0);
}

TEST_CASE("LayoutDirt generation moves on every mark", "[layoutdirt]") {
    LayoutDirt d;
    d.MarkModel("Arch 1");
    const auto g1 = d.Generation();
    // A drag marks the same model on every move; each one is a change.
    d.MarkModel("Arch 1");
    CHECK(d.Generation() > g1);
    const auto g2 = d.Generation();
    d.MarkControllers();
    CHECK(d.Generation() > g2);
    const auto g3 = d.Generation();
    d.MarkViews();
    d.MarkViewpoints();
    d.MarkBackground("Default");
    d.MarkViewObject("House");
    CHECK(d.Generation() >= g3 + 4);
    // Empty names are not edits.
    const auto g4 = d.Generation();
    d.MarkModel("");
    d.MarkViewObject("");
    CHECK(d.Generation() == g4);
}

TEST_CASE("LayoutDirt applied session layout stays unsaved until cleared", "[layoutdirt]") {
    // A host that applied someone else's layout edits and made none of its
    // own: memory differs from the file, so the save must write it.
    LayoutDirt d;
    const auto before = d.Generation();
    d.AdoptBase("<xrgb><models/></xrgb>");
    CHECK(d.HasLayoutChanges());
    CHECK(d.HasChanges());
    REQUIRE(d.Base().has_value());
    CHECK(*d.Base() == "<xrgb><models/></xrgb>");
    CHECK(d.Generation() > before);

    // Saved: the file is memory again.
    d.ClearLayout();
    CHECK_FALSE(d.HasLayoutChanges());
    CHECK_FALSE(d.Base().has_value());
}

TEST_CASE("LayoutDirt adopting a layout drops marks relative to the old base", "[layoutdirt]") {
    LayoutDirt d;
    d.MarkModel("Arch 1");
    d.MarkModelRenamed("Arch 2", "Arch 3");
    d.MarkGroupCreated("All Arches");
    d.MarkViewObjectDeleted("Mesh");
    d.MarkLayoutGroupDeleted("Yard");
    d.MarkViews();
    d.MarkControllers();
    d.AdoptBase("layout");
    // The new base already holds every one of those edits.
    CHECK(d.Models().empty());
    CHECK(d.RenamedModels().empty());
    CHECK(d.CreatedGroups().empty());
    CHECK(d.DeletedViewObjects().empty());
    CHECK(d.DeletedLayoutGroups().empty());
    CHECK_FALSE(d.Views());
    // The controllers file is saved whole, not patched into a base.
    CHECK(d.Controllers());

    // Later edits are relative to the adopted base.
    d.MarkModelRenamed("Arch 3", "Arch 4");
    REQUIRE(d.RenamedModels().count("Arch 4") == 1);
    CHECK(d.RenamedModels().at("Arch 4") == "Arch 3");
}

TEST_CASE("LayoutDirt discarding local edits keeps an adopted layout", "[layoutdirt]") {
    LayoutDirt d;
    d.AdoptBase("layout");
    d.MarkModel("Arch 1");
    // What iPadRenderContext::ClearDirtyLayoutModels does after undoing
    // every local edit.
    d.AdoptBase(*d.Base());
    CHECK(d.Models().empty());
    CHECK(d.HasLayoutChanges());
}

TEST_CASE("LayoutDirt model delete removes the record under its name in the base", "[layoutdirt]") {
    LayoutDirt d;
    d.MarkModelRenamed("Arch 1", "Arch 2");
    d.MarkModel("Arch 2");
    d.MarkModelDeleted("Arch 2");
    CHECK(d.Models().count("Arch 2") == 0);
    CHECK(d.RenamedModels().empty());
    CHECK(d.DeletedModels().count("Arch 1") == 1);
    CHECK(d.DeletedModels().count("Arch 2") == 0);
    CHECK(d.HasLayoutChanges());

    LayoutDirt plain;
    plain.MarkModelDeleted("Tree");
    CHECK(plain.DeletedModels().count("Tree") == 1);
    CHECK(plain.HasLayoutChanges());
}

TEST_CASE("LayoutDirt renames collapse to the base name", "[layoutdirt]") {
    LayoutDirt d;
    d.MarkModelRenamed("A", "B");
    d.MarkModelRenamed("B", "C");
    REQUIRE(d.RenamedModels().size() == 1);
    CHECK(d.RenamedModels().at("C") == "A");
    // Renamed back: nothing to rename in the file.
    d.MarkModelRenamed("C", "A");
    CHECK(d.RenamedModels().empty());

    d.MarkGroupRenamed("G1", "G2");
    d.MarkGroupRenamed("G2", "G3");
    REQUIRE(d.RenamedGroups().size() == 1);
    CHECK(d.RenamedGroups().at("G3") == "G1");

    d.MarkViewObjectRenamed("V1", "V2");
    REQUIRE(d.RenamedViewObjects().size() == 1);
    CHECK(d.RenamedViewObjects().at("V2") == "V1");
}

TEST_CASE("LayoutDirt created then deleted groups and objects leave nothing", "[layoutdirt]") {
    LayoutDirt d;
    d.MarkGroupCreated("New Group");
    d.MarkModel("New Group");
    d.MarkGroupRenamed("New Group", "Renamed Group");
    CHECK(d.CreatedGroups().count("Renamed Group") == 1);
    d.MarkGroupDeleted("Renamed Group");
    CHECK(d.CreatedGroups().empty());
    CHECK(d.DeletedGroups().empty());

    d.MarkViewObjectCreated("Ruler");
    d.MarkViewObject("Ruler");
    d.MarkViewObjectDeleted("Ruler");
    CHECK(d.CreatedViewObjects().empty());
    CHECK(d.DeletedViewObjects().empty());
    CHECK(d.ViewObjects().empty());

    // A group renamed then deleted goes from the file under its old name.
    d.MarkGroupRenamed("Old", "Mid");
    d.MarkGroupDeleted("Mid");
    CHECK(d.DeletedGroups().count("Old") == 1);
    CHECK(d.DeletedGroups().count("Mid") == 0);
}

TEST_CASE("LayoutDirt whole-section marks count as layout changes", "[layoutdirt]") {
    LayoutDirt views;
    views.MarkViews();
    CHECK(views.HasLayoutChanges());
    LayoutDirt viewpoints;
    viewpoints.MarkViewpoints();
    CHECK(viewpoints.HasLayoutChanges());
    LayoutDirt controllers;
    controllers.MarkControllers();
    CHECK_FALSE(controllers.HasLayoutChanges());
    CHECK(controllers.HasChanges());
    controllers.ClearAll();
    CHECK_FALSE(controllers.HasChanges());
}
