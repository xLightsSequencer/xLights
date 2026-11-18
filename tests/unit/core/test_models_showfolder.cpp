#include <functional>
#include <list>
#include <optional>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "support/TempDir.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "render/HeadlessRenderContext.h"
#include "utils/FileUtils.h"

namespace {

const char* kNetworks = R"(<?xml version="1.0" encoding="UTF-8"?>
<xrgb version="2"/>
)";

std::string ShowXml(const std::string& models, const std::string& groups = "") {
    return std::string("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb>\n<models>\n") + models + "\n</models>\n<modelGroups>\n" + groups + "\n</modelGroups>\n</xrgb>\n";
}

struct FixFileReset {
    ~FixFileReset() {
        FileUtils::SetFixFileShowDir("");
        FileUtils::SetFixFileDirectories({});
        FileUtils::ClearNonExistentFiles();
    }
};

} // namespace

TEST_CASE("Show folder loads models and groups", "[models][showfolder]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", kNetworks);
    dir.Write("xlights_rgbeffects.xml",
              ShowXml(R"(<model name="A" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="10"/>
<model name="B" DisplayAs="Matrix" StartChannel="&gt;A:1" NumStrings="1" NodesPerString="12" StrandsPerString="3"/>)",
                      R"(<modelGroup name="G" models="A,B"/>)"));
    FixFileReset reset;
    HeadlessRenderContext ctx;
    REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    CHECK(ctx.AllModels.size() == 3);
    Model* a = ctx.GetModel("A");
    Model* b = ctx.GetModel("B");
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    CHECK(a->GetNodeCount() == 10);
    CHECK(b->GetNodeCount() == 12);
    CHECK(b->GetFirstChannel() == a->GetLastChannel() + 1);
    auto* g = dynamic_cast<ModelGroup*>(ctx.GetModel("G"));
    REQUIRE(g != nullptr);
    CHECK(g->ModelNames() == std::vector<std::string>{ "A", "B" });
    CHECK(g->GetModelCount() == 2);
}

TEST_CASE("Show folder without a layout file fails to load", "[models][showfolder]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", kNetworks);
    FixFileReset reset;
    HeadlessRenderContext ctx;
    CHECK_FALSE(ctx.LoadShowFolder(dir.path.string()));
}

TEST_CASE("Show folder with a malformed layout file fails to load", "[models][showfolder]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", kNetworks);
    dir.Write("xlights_rgbeffects.xml", "<xrgb><models><model name=\"A\"");
    FixFileReset reset;
    HeadlessRenderContext ctx;
    CHECK_FALSE(ctx.LoadShowFolder(dir.path.string()));
}

TEST_CASE("Show folder with the wrong root or no models element fails to load", "[models][showfolder]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", kNetworks);
    dir.Write("xlights_rgbeffects.xml", "<other><models/></other>");
    {
        FixFileReset reset;
        HeadlessRenderContext ctx;
        CHECK_FALSE(ctx.LoadShowFolder(dir.path.string()));
    }
    dir.Write("xlights_rgbeffects.xml", "<xrgb><settings/></xrgb>");
    {
        FixFileReset reset;
        HeadlessRenderContext ctx;
        CHECK_FALSE(ctx.LoadShowFolder(dir.path.string()));
    }
}

TEST_CASE("Show folder with no models loads an empty show", "[models][showfolder]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", kNetworks);
    dir.Write("xlights_rgbeffects.xml", ShowXml(""));
    FixFileReset reset;
    HeadlessRenderContext ctx;
    REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    CHECK(ctx.AllModels.size() == 0);
}

TEST_CASE("Show folder skips models that cannot be created", "[models][showfolder]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", kNetworks);
    dir.Write("xlights_rgbeffects.xml",
              ShowXml(R"(<model name="Good" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="10"/>
<model name="Weird" DisplayAs="NotARealType" StartChannel="1"/>
<model DisplayAs="Single Line" StartChannel="1"/>
<model name="NoType" StartChannel="1"/>)"));
    FixFileReset reset;
    HeadlessRenderContext ctx;
    REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    CHECK(ctx.GetModel("Good") != nullptr);
    CHECK(ctx.GetModel("Weird") == nullptr);
    CHECK(ctx.GetModel("NoType") == nullptr);
    CHECK(ctx.AllModels.size() == 1);
}

TEST_CASE("Show folder resolves start channel chains regardless of file order", "[models][showfolder]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", kNetworks);
    dir.Write("xlights_rgbeffects.xml",
              ShowXml(R"(<model name="C" DisplayAs="Single Line" StartChannel="&gt;B:1" NumStrings="1" NodesPerString="4"/>
<model name="B" DisplayAs="Single Line" StartChannel="&gt;A:1" NumStrings="1" NodesPerString="4"/>
<model name="A" DisplayAs="Single Line" StartChannel="100" NumStrings="1" NodesPerString="4"/>)"));
    FixFileReset reset;
    HeadlessRenderContext ctx;
    REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    CHECK(ctx.GetModel("A")->GetFirstChannel() == 99);
    CHECK(ctx.GetModel("B")->GetFirstChannel() == 111);
    CHECK(ctx.GetModel("C")->GetFirstChannel() == 123);
}

TEST_CASE("Show folder keeps groups that name models loaded later", "[models][showfolder]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", kNetworks);
    dir.Write("xlights_rgbeffects.xml",
              ShowXml(R"(<model name="A" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="4"/>)",
                      R"(<modelGroup name="Outer" models="Inner,A"/>
<modelGroup name="Inner" models="A"/>)"));
    FixFileReset reset;
    HeadlessRenderContext ctx;
    REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    auto* outer = dynamic_cast<ModelGroup*>(ctx.GetModel("Outer"));
    auto* inner = dynamic_cast<ModelGroup*>(ctx.GetModel("Inner"));
    REQUIRE(outer != nullptr);
    REQUIRE(inner != nullptr);
    CHECK(outer->ContainsModelGroup(inner));
    CHECK(outer->GetFlatModels().size() == 1);
}

TEST_CASE("Show folder drops group members that do not exist", "[models][showfolder]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", kNetworks);
    dir.Write("xlights_rgbeffects.xml",
              ShowXml(R"(<model name="A" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="4"/>)",
                      R"(<modelGroup name="G" models="A,Ghost,Phantom/Sub"/>)"));
    FixFileReset reset;
    HeadlessRenderContext ctx;
    REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    auto* g = dynamic_cast<ModelGroup*>(ctx.GetModel("G"));
    REQUIRE(g != nullptr);
    CHECK(g->ModelNames() == std::vector<std::string>{ "A" });
}

TEST_CASE("Show folder breaks group loops", "[models][showfolder]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", kNetworks);
    dir.Write("xlights_rgbeffects.xml",
              ShowXml(R"(<model name="A" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="4"/>)",
                      R"(<modelGroup name="One" models="A,Two"/>
<modelGroup name="Two" models="One"/>
<modelGroup name="Self" models="Self,A"/>)"));
    FixFileReset reset;
    HeadlessRenderContext ctx;
    REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    for (const char* name : { "One", "Two", "Self" }) {
        INFO(name);
        auto* g = dynamic_cast<ModelGroup*>(ctx.GetModel(name));
        if (g == nullptr) {
            continue;
        }
        CHECK_FALSE(g->ContainsModelGroup(g));
        CHECK(g->GetFlatModels().size() <= 1);
    }
    CHECK(ctx.GetModel("A") != nullptr);
}

TEST_CASE("Show folder reads the preview size from settings", "[models][showfolder]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", kNetworks);
    dir.Write("xlights_rgbeffects.xml",
              "<xrgb><settings><previewWidth value=\"640\"/><previewHeight value=\"480\"/></settings><models/></xrgb>");
    FixFileReset reset;
    HeadlessRenderContext ctx;
    REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    int w = 0, h = 0;
    ctx.GetRenderPreviewSize(w, h);
    CHECK(w == 640);
    CHECK(h == 480);
}

TEST_CASE("Show folder ignores non positive preview sizes", "[models][showfolder]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", kNetworks);
    dir.Write("xlights_rgbeffects.xml",
              "<xrgb><settings><previewWidth value=\"-5\"/><previewHeight value=\"abc\"/></settings><models/></xrgb>");
    FixFileReset reset;
    HeadlessRenderContext ctx;
    REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    int w = 0, h = 0;
    ctx.GetRenderPreviewSize(w, h);
    CHECK(w == 1280);
    CHECK(h == 720);
}
