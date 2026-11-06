#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "render/SequenceFaces.h"
#include "render/SequenceMedia.h"
#include "render/SongStructureManager.h"
#include "render/ViewpointMgr.h"
#include "XmlSerializer/XmlSerializingVisitor.h"

#include <pugixml.hpp>

#include <string>

using Catch::Matchers::WithinAbs;

namespace {
pugi::xml_document ParseXml(const std::string& xml) {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(xml.c_str()));
    return doc;
}
}

TEST_CASE("SongStructure starts empty", "[sequence][songstructure]") {
    SongStructureManager m;
    CHECK(m.GetRegionCount() == 0);
    CHECK_FALSE(m.HasRegions());
    CHECK(m.GetViewCount() == 0);
    CHECK(m.GetActiveViewIndex() == -1);
    CHECK(m.GetActiveViewName().empty());
    CHECK(m.GetRegionAtTime(0) == nullptr);
    CHECK_FALSE(m.AnyViewHasRegions());
}

TEST_CASE("SongStructure first boundary splits the sequence in two", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddBoundary(10000, 60000);
    REQUIRE(m.GetRegionCount() == 2);
    CHECK(m.GetViewCount() == 1);
    CHECK(m.GetActiveViewName() == "Default");
    CHECK(m.GetRegion(0).startTimeMS == 0);
    CHECK(m.GetRegion(0).endTimeMS == 10000);
    CHECK(m.GetRegion(1).startTimeMS == 10000);
    CHECK(m.GetRegion(1).endTimeMS == 60000);
    CHECK(m.GetRegion(0).name == "Region 1");
    CHECK(m.GetRegion(1).name == "Region 2");
    CHECK(m.GetRegion(0).colorARGB == SongStructureManager::GetPaletteColor(0));
    CHECK(m.GetRegion(1).colorARGB == SongStructureManager::GetPaletteColor(1));
}

TEST_CASE("SongStructure rejects degenerate first boundaries", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddBoundary(0, 60000);
    m.AddBoundary(-5, 60000);
    m.AddBoundary(60000, 60000);
    m.AddBoundary(70000, 60000);
    CHECK(m.GetRegionCount() == 0);
}

TEST_CASE("SongStructure later boundaries split the containing region", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddBoundary(10000, 60000);
    m.AddBoundary(30000, 60000);
    REQUIRE(m.GetRegionCount() == 3);
    CHECK(m.GetRegion(1).startTimeMS == 10000);
    CHECK(m.GetRegion(1).endTimeMS == 30000);
    CHECK(m.GetRegion(2).startTimeMS == 30000);
    CHECK(m.GetRegion(2).endTimeMS == 60000);
    m.AddBoundary(5000, 60000);
    REQUIRE(m.GetRegionCount() == 4);
    CHECK(m.GetRegion(0).endTimeMS == 5000);
    CHECK(m.GetRegion(1).startTimeMS == 5000);
}

TEST_CASE("SongStructure ignores a boundary on an existing edge or outside", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddBoundary(10000, 60000);
    m.AddBoundary(10000, 60000);
    m.AddBoundary(0, 60000);
    m.AddBoundary(60000, 60000);
    m.AddBoundary(99999, 60000);
    CHECK(m.GetRegionCount() == 2);
}

TEST_CASE("SongStructure region ids are unique and regions are contiguous", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddBoundary(10000, 60000);
    m.AddBoundary(20000, 60000);
    m.AddBoundary(40000, 60000);
    m.AddBoundary(15000, 60000);
    REQUIRE(m.GetRegionCount() == 5);
    for (size_t i = 0; i < m.GetRegionCount(); ++i) {
        for (size_t j = i + 1; j < m.GetRegionCount(); ++j) {
            CHECK(m.GetRegion(i).id != m.GetRegion(j).id);
        }
        if (i > 0) {
            CHECK(m.GetRegion(i).startTimeMS == m.GetRegion(i - 1).endTimeMS);
        }
    }
    CHECK(m.GetRegion(0).startTimeMS == 0);
    CHECK(m.GetRegion(4).endTimeMS == 60000);
}

TEST_CASE("SongStructure region lookup by time uses half open intervals", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddBoundary(10000, 60000);
    CHECK(m.GetRegionIndexAtTime(0) == 0);
    CHECK(m.GetRegionIndexAtTime(9999) == 0);
    CHECK(m.GetRegionIndexAtTime(10000) == 1);
    CHECK(m.GetRegionIndexAtTime(59999) == 1);
    CHECK(m.GetRegionIndexAtTime(60000) == -1);
    CHECK(m.GetRegionIndexAtTime(-1) == -1);
    REQUIRE(m.GetRegionAtTime(10000) != nullptr);
    CHECK(m.GetRegionAtTime(10000)->name == "Region 2");
}

TEST_CASE("SongStructure DeleteBoundary merges regions and clears the last one", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddBoundary(10000, 60000);
    m.AddBoundary(30000, 60000);
    m.DeleteBoundary(12345);
    CHECK(m.GetRegionCount() == 3);
    m.DeleteBoundary(10000);
    REQUIRE(m.GetRegionCount() == 2);
    CHECK(m.GetRegion(0).endTimeMS == 30000);
    m.DeleteBoundary(30000);
    CHECK(m.GetRegionCount() == 0);
}

TEST_CASE("SongStructure MoveBoundary clamps between neighbours", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddBoundary(10000, 60000);
    m.AddBoundary(30000, 60000);
    m.MoveBoundary(10000, 15000);
    CHECK(m.GetRegion(0).endTimeMS == 15000);
    CHECK(m.GetRegion(1).startTimeMS == 15000);
    m.MoveBoundary(15000, 100000);
    CHECK(m.GetRegion(1).startTimeMS == 29999);
    m.MoveBoundary(29999, -100);
    CHECK(m.GetRegion(1).startTimeMS == 1);
    m.MoveBoundary(555, 777);
    CHECK(m.GetRegionCount() == 3);
}

TEST_CASE("SongStructure FindNearestBoundary honours tolerance", "[sequence][songstructure]") {
    SongStructureManager m;
    CHECK(m.FindNearestBoundary(100, 50) == -1);
    m.AddBoundary(10000, 60000);
    m.AddBoundary(30000, 60000);
    CHECK(m.FindNearestBoundary(10100, 200) == 10000);
    CHECK(m.FindNearestBoundary(29000, 2000) == 30000);
    CHECK(m.FindNearestBoundary(20000, 100) == -1);
    CHECK(m.FindNearestBoundary(10000, 0) == 10000);
}

TEST_CASE("SongStructure region editing", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddBoundary(10000, 60000);
    m.SetRegionName(0, "Intro");
    m.SetRegionColor(1, 0xFF112233);
    CHECK(m.GetRegion(0).name == "Intro");
    CHECK(m.GetRegion(1).colorARGB == 0xFF112233);
    m.SetRegionName(99, "x");
    m.SetRegionColor(99, 1);
    CHECK(m.GetRegionCount() == 2);
}

TEST_CASE("SongStructure SetRegions needs an active view and updates the next id", "[sequence][songstructure]") {
    SongStructureManager m;
    m.SetRegions({ SongStructureRegion(1, 0, 10, "a", 0) });
    CHECK(m.GetRegionCount() == 0);
    m.AddView("V");
    m.SetActiveView(0);
    m.SetRegions({ SongStructureRegion(7, 0, 10, "a", 1), SongStructureRegion(9, 10, 20, "b", 2) });
    CHECK(m.GetRegionCount() == 2);
    m.AddBoundary(5, 20);
    REQUIRE(m.GetRegionCount() == 3);
    CHECK(m.GetRegion(1).id == 10);
}

TEST_CASE("SongStructure palette wraps", "[sequence][songstructure]") {
    CHECK(SongStructureManager::GetPaletteColor(0) == SongStructureManager::GetPaletteColor(SongStructureManager::PALETTE_SIZE));
    CHECK(SongStructureManager::GetPaletteColor(1) != SongStructureManager::GetPaletteColor(2));
}

TEST_CASE("SongStructure view management", "[sequence][songstructure]") {
    SongStructureManager m;
    int a = m.AddView("A");
    int b = m.AddView("B");
    CHECK(a == 0);
    CHECK(b == 1);
    CHECK(m.GetViewCount() == 2);
    m.SetActiveView(1);
    CHECK(m.GetActiveViewName() == "B");
    m.RenameView(1, "Bee");
    CHECK(m.GetViewName(1) == "Bee");
    CHECK(m.GetViewName(5).empty());
    m.RenameView(9, "x");
    m.SetActiveView(7);
    CHECK(m.GetActiveViewIndex() == 0);
}

TEST_CASE("SongStructure views hold separate regions", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddView("A");
    m.AddView("B");
    m.SetActiveView(0);
    m.AddBoundary(10, 100);
    CHECK(m.GetRegionCount() == 2);
    m.SetActiveView(1);
    CHECK(m.GetRegionCount() == 0);
    CHECK(m.AnyViewHasRegions());
}

TEST_CASE("SongStructure DuplicateView copies regions", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddBoundary(10, 100);
    int dup = m.DuplicateView(0, "Copy");
    CHECK(dup == 1);
    m.SetActiveView(dup);
    CHECK(m.GetRegionCount() == 2);
    m.SetRegionName(0, "changed");
    m.SetActiveView(0);
    CHECK(m.GetRegion(0).name == "Region 1");
    int bad = m.DuplicateView(99, "Empty");
    m.SetActiveView(bad);
    CHECK(m.GetRegionCount() == 0);
}

TEST_CASE("SongStructure DeleteView keeps the active view valid", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddView("A");
    m.AddView("B");
    m.AddView("C");
    m.SetActiveView(2);
    m.DeleteView(0);
    CHECK(m.GetActiveViewIndex() == 1);
    CHECK(m.GetActiveViewName() == "C");
    m.DeleteView(1);
    CHECK(m.GetActiveViewIndex() == 0);
    m.DeleteView(0);
    CHECK(m.GetActiveViewIndex() == -1);
    CHECK(m.GetViewCount() == 0);
    m.DeleteView(0);
    m.DeleteView(-3);
}

TEST_CASE("SongStructure Clear removes everything", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddBoundary(10, 100);
    m.Clear();
    CHECK(m.GetViewCount() == 0);
    CHECK(m.GetRegionCount() == 0);
    CHECK_FALSE(m.AnyViewHasRegions());
}

TEST_CASE("SongStructure XML round trip", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddBoundary(10000, 60000);
    m.AddBoundary(30000, 60000);
    m.SetRegionName(1, "Verse & <Chorus>");
    m.SetRegionColor(2, 0x80ABCDEF);
    m.AddView("Second");
    m.SetActiveView(1);
    m.AddBoundary(500, 1000);

    pugi::xml_document doc;
    auto root = doc.append_child("root");
    m.SaveToXml(root);

    SongStructureManager back;
    back.LoadFromXml(root.child("SongStructure"));
    REQUIRE(back.GetViewCount() == 2);
    CHECK(back.GetActiveViewIndex() == 1);
    CHECK(back.GetViewName(0) == "Default");
    CHECK(back.GetViewName(1) == "Second");
    REQUIRE(back.GetRegionCount() == 2);
    back.SetActiveView(0);
    REQUIRE(back.GetRegionCount() == 3);
    CHECK(back.GetRegion(1).name == "Verse & <Chorus>");
    CHECK(back.GetRegion(2).colorARGB == 0x80ABCDEF);
    CHECK(back.GetRegion(1).startTimeMS == 10000);
    CHECK(back.GetRegion(1).endTimeMS == 30000);
    CHECK(back.GetRegion(0).id == m.GetRegions()[0].id);
}

TEST_CASE("SongStructure loads the legacy flat Region format", "[sequence][songstructure]") {
    auto doc = ParseXml("<SongStructure><Region id=\"2\" startTimeMS=\"500\" endTimeMS=\"1000\" name=\"B\" color=\"40FF0000\"/><Region id=\"1\" startTimeMS=\"0\" endTimeMS=\"500\" name=\"A\" color=\"40808080\"/></SongStructure>");
    SongStructureManager m;
    m.LoadFromXml(doc.child("SongStructure"));
    REQUIRE(m.GetViewCount() == 1);
    REQUIRE(m.GetRegionCount() == 2);
    CHECK(m.GetRegion(0).name == "A");
    CHECK(m.GetRegion(1).name == "B");
    CHECK(m.GetRegion(1).colorARGB == 0x40FF0000);
    m.AddBoundary(250, 1000);
    CHECK(m.GetRegion(1).id == 3);
}

TEST_CASE("SongStructure load tolerates junk values and a missing node", "[sequence][songstructure]") {
    SongStructureManager m;
    m.AddBoundary(10, 100);
    m.LoadFromXml(pugi::xml_node());
    CHECK(m.GetViewCount() == 0);

    auto doc = ParseXml("<SongStructure activeView=\"99\"><View name=\"X\"><Region id=\"zz\" startTimeMS=\"abc\" endTimeMS=\"\" name=\"\" color=\"nothex\"/><Other/></View></SongStructure>");
    m.LoadFromXml(doc.child("SongStructure"));
    REQUIRE(m.GetViewCount() == 1);
    CHECK(m.GetActiveViewIndex() == 0);
    REQUIRE(m.GetRegionCount() == 1);
    CHECK(m.GetRegion(0).startTimeMS == 0);
    CHECK(m.GetRegion(0).id == 0);
}

TEST_CASE("SongStructure load clamps a negative active view", "[sequence][songstructure]") {
    auto doc = ParseXml("<SongStructure activeView=\"-4\"><View name=\"X\"/><View name=\"Y\"/></SongStructure>");
    SongStructureManager m;
    m.LoadFromXml(doc.child("SongStructure"));
    CHECK(m.GetActiveViewIndex() == 0);
}

TEST_CASE("SequenceFaces set get remove", "[sequence][faces]") {
    SequenceFaces f;
    CHECK(f.empty());
    CHECK(f.GetFace("x") == nullptr);
    f.SetFace("", { { "a", "b" } });
    CHECK(f.empty());
    f.SetFace("Santa", { { "Mouth-AI-EyesOpen", "ai.png" } });
    REQUIRE(f.GetFace("Santa") != nullptr);
    CHECK(f.GetFace("Santa")->at("Type") == "Matrix");
    CHECK(f.GetFace("Santa")->at("Mouth-AI-EyesOpen") == "ai.png");
    CHECK(f.RemoveFace("Santa"));
    CHECK_FALSE(f.RemoveFace("Santa"));
    CHECK(f.empty());
}

TEST_CASE("SequenceFaces rename rules", "[sequence][faces]") {
    SequenceFaces f;
    f.SetFace("A", { { "Mouth-AI-EyesOpen", "a.png" } });
    f.SetFace("B", {});
    CHECK_FALSE(f.RenameFace("A", "B"));
    CHECK_FALSE(f.RenameFace("A", "A"));
    CHECK_FALSE(f.RenameFace("A", ""));
    CHECK_FALSE(f.RenameFace("Z", "Y"));
    CHECK(f.RenameFace("A", "C"));
    CHECK(f.GetFace("A") == nullptr);
    REQUIRE(f.GetFace("C") != nullptr);
    CHECK(f.GetFace("C")->at("Mouth-AI-EyesOpen") == "a.png");
}

TEST_CASE("SequenceFaces IsImageKey recognises mouth and eye image keys only", "[sequence][faces]") {
    CHECK(SequenceFaces::IsImageKey("Mouth-AI-EyesOpen"));
    CHECK(SequenceFaces::IsImageKey("Eyes-Open"));
    CHECK(SequenceFaces::IsImageKey("Mouth"));
    CHECK_FALSE(SequenceFaces::IsImageKey("Mouth-AI-Color"));
    CHECK_FALSE(SequenceFaces::IsImageKey("Eyes-Open-Color"));
    CHECK_FALSE(SequenceFaces::IsImageKey("Type"));
    CHECK_FALSE(SequenceFaces::IsImageKey(""));
    CHECK_FALSE(SequenceFaces::IsImageKey("Nodes"));
}

TEST_CASE("SequenceFaces image path helpers", "[sequence][faces]") {
    SequenceFaces f;
    f.SetFace("A", { { "Mouth-AI-EyesOpen", "x.png" }, { "Mouth-E-EyesOpen", "y.png" }, { "Mouth-AI-Color", "x.png" }, { "Other", "x.png" } });
    f.SetFace("B", { { "Mouth-AI-EyesOpen", "x.png" }, { "Mouth-O-EyesOpen", "" } });
    auto paths = f.GetImagePaths();
    CHECK(paths.size() == 2);
    CHECK(f.CountImageReferences("x.png") == 2);
    CHECK(f.CountImageReferences("y.png") == 1);
    CHECK(f.CountImageReferences("none.png") == 0);
    CHECK(f.RewriteImagePath("x.png", "z.png") == 2);
    CHECK(f.CountImageReferences("x.png") == 0);
    CHECK(f.CountImageReferences("z.png") == 2);
    CHECK(f.GetFace("A")->at("Other") == "x.png");
    CHECK(f.GetFace("A")->at("Mouth-AI-Color") == "x.png");
}

TEST_CASE("SequenceFaces XML round trip", "[sequence][faces]") {
    SequenceFaces f;
    f.SetFace("Santa", { { "Mouth-AI-EyesOpen", "missing/ai.png" }, { "Mouth-AI-Color", "#ff0000" } });
    f.SetFace("Elf", { { "Eyes-Open", "missing/eyes.png" } });
    pugi::xml_document doc;
    auto root = doc.append_child("xsequence");
    f.SaveToXml(root);
    auto node = root.child("FaceDefinitions");
    REQUIRE(node);

    SequenceFaces back;
    SequenceMedia media;
    CHECK(back.LoadFromXml(node, media));
    REQUIRE(back.GetFaces().size() == 2);
    CHECK(back.GetFace("Santa")->at("Mouth-AI-EyesOpen") == "missing/ai.png");
    CHECK(back.GetFace("Santa")->at("Mouth-AI-Color") == "#ff0000");
    CHECK(back.GetFace("Elf")->at("Type") == "Matrix");
}

TEST_CASE("SequenceFaces saves nothing when empty", "[sequence][faces]") {
    SequenceFaces f;
    pugi::xml_document doc;
    auto root = doc.append_child("xsequence");
    f.SaveToXml(root);
    CHECK_FALSE(root.child("FaceDefinitions"));
}

TEST_CASE("SequenceFaces load skips nameless and non matrix faces", "[sequence][faces]") {
    auto doc = ParseXml("<FaceDefinitions><faceInfo Type=\"Matrix\"/><faceInfo Name=\"N\" Type=\"NodeRange\" Eyes-Open=\"x\"/><faceInfo Name=\"M\" Type=\"Matrix\"/><faceInfo Name=\"Implicit\"/></FaceDefinitions>");
    SequenceFaces f;
    SequenceMedia media;
    f.LoadFromXml(doc.child("FaceDefinitions"), media);
    CHECK(f.GetFace("N") == nullptr);
    CHECK(f.GetFace("M") != nullptr);
    CHECK(f.GetFace("Implicit") != nullptr);
    CHECK(f.GetFaces().size() == 2);
}

TEST_CASE("SequenceFaces load replaces previous contents", "[sequence][faces]") {
    SequenceFaces f;
    f.SetFace("Old", {});
    auto doc = ParseXml("<FaceDefinitions><faceInfo Name=\"New\"/></FaceDefinitions>");
    SequenceMedia media;
    f.LoadFromXml(doc.child("FaceDefinitions"), media);
    CHECK(f.GetFace("Old") == nullptr);
    CHECK(f.GetFace("New") != nullptr);
}

TEST_CASE("PreviewCamera defaults", "[sequence][viewpoint]") {
    PreviewCamera c(true);
    CHECK(c.GetIs3D());
    CHECK(c.GetName() == "Name Unspecified");
    CHECK_THAT(c.GetAngleX(), WithinAbs(20.0, 1e-5));
    CHECK_THAT(c.GetAngleY(), WithinAbs(5.0, 1e-5));
    CHECK_THAT(c.GetDistance(), WithinAbs(-2000.0, 1e-3));
    CHECK_THAT(c.GetZoom(), WithinAbs(1.0, 1e-5));
}

TEST_CASE("PreviewCamera angles wrap into 0..360", "[sequence][viewpoint]") {
    PreviewCamera c(true);
    c.SetAngleX(-10.0f);
    CHECK_THAT(c.GetAngleX(), WithinAbs(350.0, 1e-4));
    c.SetAngleY(365.0f);
    CHECK_THAT(c.GetAngleY(), WithinAbs(5.0, 1e-4));
    c.SetAngleZ(360.0f);
    CHECK_THAT(c.GetAngleZ(), WithinAbs(0.0, 1e-4));
    c.SetAngleZ(90.0f);
    CHECK_THAT(c.GetAngleZ(), WithinAbs(90.0, 1e-4));
}

TEST_CASE("PreviewCamera Reset restores defaults", "[sequence][viewpoint]") {
    PreviewCamera c(false);
    c.SetPosX(5);
    c.SetZoom(3);
    c.SetPanX(2);
    c.SetAngleX(100);
    c.Reset();
    CHECK_THAT(c.GetPosX(), WithinAbs(0.0, 1e-6));
    CHECK_THAT(c.GetZoom(), WithinAbs(1.0, 1e-6));
    CHECK_THAT(c.GetPanX(), WithinAbs(0.0, 1e-6));
    CHECK_THAT(c.GetAngleX(), WithinAbs(20.0, 1e-6));
}

TEST_CASE("PreviewCamera copy keeps values and name", "[sequence][viewpoint]") {
    PreviewCamera c(true);
    c.SetName("cam");
    c.SetPosX(4);
    c.SetZoom(2.5f);
    PreviewCamera d(c);
    CHECK(d.GetName() == "cam");
    CHECK_THAT(d.GetPosX(), WithinAbs(4.0, 1e-6));
    CHECK_THAT(d.GetZoom(), WithinAbs(2.5, 1e-6));
    PreviewCamera e(false);
    e = c;
    CHECK_THAT(e.GetZoom(), WithinAbs(2.5, 1e-6));
    CHECK(e.GetIs3D());
}

TEST_CASE("PreviewCamera view matrix changes when parameters change", "[sequence][viewpoint]") {
    PreviewCamera c(true);
    glm::mat4 m1 = c.GetViewMatrix();
    c.SetDistance(-500.0f);
    glm::mat4 m2 = c.GetViewMatrix();
    CHECK(m1 != m2);
    c.SetDistance(-2000.0f);
    CHECK(c.GetViewMatrix() == m1);
}

TEST_CASE("ViewpointMgr AddCamera and lookup", "[sequence][viewpoint]") {
    ViewpointMgr v;
    CHECK(v.GetNum2DCameras() == 0);
    CHECK(v.GetNum3DCameras() == 0);
    PreviewCamera cam(true);
    cam.SetZoom(2.0f);
    v.AddCamera("front", &cam, true);
    v.AddCamera("top", nullptr, false);
    v.AddCamera("", nullptr, true);
    CHECK(v.GetNum3DCameras() == 2);
    CHECK(v.GetNum2DCameras() == 1);
    REQUIRE(v.GetNamedCamera3D("front") != nullptr);
    CHECK_THAT(v.GetNamedCamera3D("front")->GetZoom(), WithinAbs(2.0, 1e-6));
    CHECK(v.GetNamedCamera3D("nope") == nullptr);
    CHECK(v.GetCamera3D(1)->GetName() == "...");
    CHECK(v.GetCamera2D(0)->GetName() == "top");
    CHECK_FALSE(v.GetCamera2D(0)->GetIs3D());
}

TEST_CASE("ViewpointMgr name uniqueness is per dimensionality", "[sequence][viewpoint]") {
    ViewpointMgr v;
    v.AddCamera("a", nullptr, true);
    CHECK_FALSE(v.IsNameUnique("a", true));
    CHECK(v.IsNameUnique("a", false));
    CHECK(v.IsNameUnique("b", true));
}

TEST_CASE("ViewpointMgr delete ignores out of range indices", "[sequence][viewpoint]") {
    ViewpointMgr v;
    v.AddCamera("a", nullptr, true);
    v.AddCamera("b", nullptr, true);
    v.AddCamera("c", nullptr, false);
    v.DeleteCamera3D(5);
    v.DeleteCamera3D(-1);
    v.DeleteCamera2D(1);
    CHECK(v.GetNum3DCameras() == 2);
    CHECK(v.GetNum2DCameras() == 1);
    v.DeleteCamera3D(0);
    CHECK(v.GetNum3DCameras() == 1);
    CHECK(v.GetCamera3D(0)->GetName() == "b");
    v.DeleteCamera2D(0);
    CHECK(v.GetNum2DCameras() == 0);
}

TEST_CASE("ViewpointMgr default cameras are copies", "[sequence][viewpoint]") {
    ViewpointMgr v;
    PreviewCamera c(false);
    c.SetZoom(4.0f);
    v.SetDefaultCamera2D(&c);
    c.SetZoom(9.0f);
    REQUIRE(v.GetDefaultCamera2D() != nullptr);
    CHECK_THAT(v.GetDefaultCamera2D()->GetZoom(), WithinAbs(4.0, 1e-6));
    CHECK(v.GetDefaultCamera3D() == nullptr);
    v.Clear();
    CHECK(v.GetDefaultCamera2D() == nullptr);
}

TEST_CASE("ViewpointMgr XML save and load round trip", "[sequence][viewpoint]") {
    ViewpointMgr v;
    PreviewCamera c3(true);
    c3.SetPosX(1.5f);
    c3.SetAngleX(33.0f);
    c3.SetDistance(-1234.0f);
    c3.SetZoom(1.25f);
    v.AddCamera("Front & <Left>", &c3, true);
    PreviewCamera c2(false);
    c2.SetPanX(7.0f);
    v.AddCamera("Top", &c2, false);
    v.SetDefaultCamera3D(&c3);

    pugi::xml_document doc;
    {
        XmlSerializingVisitor vis(&doc);
        v.Save(vis);
    }
    auto vp = doc.child("Viewpoints");
    REQUIRE(vp);

    ViewpointMgr back;
    back.Load(vp);
    REQUIRE(back.GetNum3DCameras() == 1);
    REQUIRE(back.GetNum2DCameras() == 1);
    auto* f = back.GetCamera3D(0);
    CHECK(f->GetName() == "Front & <Left>");
    CHECK_THAT(f->GetPosX(), WithinAbs(1.5, 1e-4));
    CHECK_THAT(f->GetAngleX(), WithinAbs(33.0, 1e-3));
    CHECK_THAT(f->GetDistance(), WithinAbs(-1234.0, 1e-2));
    CHECK_THAT(f->GetZoom(), WithinAbs(1.25, 1e-4));
    CHECK(f->GetIs3D());
    CHECK_THAT(back.GetCamera2D(0)->GetPanX(), WithinAbs(7.0, 1e-4));
    REQUIRE(back.GetDefaultCamera3D() != nullptr);
    CHECK(back.GetDefaultCamera2D() == nullptr);
}

TEST_CASE("ViewpointMgr Load replaces content and tolerates unknown nodes", "[sequence][viewpoint]") {
    ViewpointMgr v;
    v.AddCamera("old", nullptr, true);
    auto doc = ParseXml("<Viewpoints><Junk/><Camera name=\"n\" is_3d=\"1\" zoom=\"2\"/><Camera name=\"flat\" is_3d=\"0\"/><DefaultCamera2D name=\"DEFAULT2D\"/></Viewpoints>");
    v.Load(doc.child("Viewpoints"));
    CHECK(v.GetNum3DCameras() == 1);
    CHECK(v.GetNum2DCameras() == 1);
    CHECK(v.GetNamedCamera3D("old") == nullptr);
    CHECK_THAT(v.GetNamedCamera3D("n")->GetZoom(), WithinAbs(2.0, 1e-6));
    CHECK(v.GetDefaultCamera2D() != nullptr);
    v.Load(pugi::xml_node());
    CHECK(v.GetNum3DCameras() == 1);
}

TEST_CASE("ViewpointMgr ImportCameraFromNode renames duplicates", "[sequence][viewpoint]") {
    ViewpointMgr v;
    auto doc = ParseXml("<Camera name=\"cam\" is_3d=\"1\"/>");
    v.ImportCameraFromNode(doc.child("Camera"));
    v.ImportCameraFromNode(doc.child("Camera"));
    v.ImportCameraFromNode(doc.child("Camera"));
    REQUIRE(v.GetNum3DCameras() == 3);
    CHECK(v.GetCamera3D(0)->GetName() == "cam");
    CHECK(v.GetCamera3D(1)->GetName() == "cam-2");
    CHECK(v.GetCamera3D(2)->GetName() == "cam-3");
}

TEST_CASE("ViewpointMgr import numbering styles", "[sequence][viewpoint]") {
    ViewpointMgr v;
    auto a = ParseXml("<Camera name=\"left_1\" is_3d=\"1\"/>");
    v.ImportCameraFromNode(a.child("Camera"));
    v.ImportCameraFromNode(a.child("Camera"));
    CHECK(v.GetCamera3D(1)->GetName() == "left_2");
    auto b = ParseXml("<Camera name=\"cam 4\" is_3d=\"1\"/>");
    v.ImportCameraFromNode(b.child("Camera"));
    v.ImportCameraFromNode(b.child("Camera"));
    CHECK(v.GetCamera3D(3)->GetName() == "cam 2");
    auto c = ParseXml("<Camera name=\"cam9\" is_3d=\"1\"/>");
    v.ImportCameraFromNode(c.child("Camera"));
    v.ImportCameraFromNode(c.child("Camera"));
    CHECK(v.GetCamera3D(5)->GetName() == "cam2");
}

TEST_CASE("ViewpointMgr legacy camera nodes use the element name", "[sequence][viewpoint]") {
    ViewpointMgr v;
    auto doc = ParseXml("<Viewpoints><Camera is_3d=\"1\" posX=\"3\"/></Viewpoints>");
    v.Load(doc.child("Viewpoints"));
    REQUIRE(v.GetNum3DCameras() == 1);
    CHECK(v.GetCamera3D(0)->GetName() == "Camera");
    CHECK_THAT(v.GetCamera3D(0)->GetPosX(), WithinAbs(3.0, 1e-6));
}
