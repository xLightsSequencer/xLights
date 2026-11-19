#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "models/ViewObject.h"
#include "models/ViewObjectManager.h"
#include "outputs/Controller.h"
#include "outputs/OutputManager.h"
#include "render/HeadlessRenderContext.h"
#include "render/ShowMergeApply.h"
#include "support/TempDir.h"
#include "utils/FileUtils.h"
#include "utils/LayoutLock.h"

// The layout lock a collaboration session sets (utils/LayoutLock.h,
// xLightsShowContext::SetCollaborationLock): the flag itself and the core
// mutators and writes that refuse while it is on.

namespace {

void SetEnv(const char* name, const char* value) {
#ifdef _WIN32
    _putenv_s(name, value ? value : "");
#else
    if (value) {
        setenv(name, value, 1);
    } else {
        unsetenv(name);
    }
#endif
}

std::string ReadFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

struct FixFileReset {
    ~FixFileReset() {
        FileUtils::SetFixFileShowDir("");
        FileUtils::SetFixFileDirectories({});
        FileUtils::ClearNonExistentFiles();
    }
};

const char* kNetworks = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks computer=\"one\">"
                        "<Controller Type=\"Null\" Id=\"64001\" Name=\"First\"><network NetworkType=\"NULL\" MaxChannels=\"512\"/></Controller>"
                        "<Controller Type=\"Null\" Id=\"64002\" Name=\"Second\"><network NetworkType=\"NULL\" MaxChannels=\"512\"/></Controller>"
                        "</Networks>";

const char* kLayout = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb><models>"
                      "<model name=\"Arch\" DisplayAs=\"Single Line\" StringType=\"RGB Nodes\" NumStrings=\"1\" NodesPerString=\"10\" StartChannel=\"1\">"
                      "<subModel name=\"Top\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-2\"/></model>"
                      "<model name=\"Line\" DisplayAs=\"Single Line\" StringType=\"RGB Nodes\" NumStrings=\"1\" NodesPerString=\"10\" StartChannel=\"31\"/>"
                      "</models><modelGroups><modelGroup name=\"G\" models=\"Arch/Top,Line\" layout=\"minimalGrid\" GridSize=\"400\"/></modelGroups>"
                      "<view_objects><view_object name=\"Ground\" DisplayAs=\"Gridlines\"/></view_objects></xrgb>";

struct LockedShow {
    TempDir dir;
    FixFileReset reset;
    HeadlessRenderContext ctx;
    std::string nets;
    LockedShow() {
        nets = dir.Write("xlights_networks.xml", kNetworks);
        dir.Write("xlights_rgbeffects.xml", kLayout);
        REQUIRE(ctx.LoadShowFolder(dir.path.string()));
        REQUIRE(ctx.AllModels.GetModel("Arch") != nullptr);
        REQUIRE(ctx.AllObjects.GetViewObject("Ground") != nullptr);
    }
};

// Records the host hook.
class HookContext : public HeadlessRenderContext {
public:
    int changes = 0;
    bool lockedInHook = false;
    void OnLayoutLockChanged() override {
        ++changes;
        lockedInHook = IsLayoutLocked();
    }
};

} // namespace

TEST_CASE("Layout lock: the flag, its reason and the null lock", "[layoutlock]") {
    LayoutLock lock;
    CHECK_FALSE(lock.IsLocked());
    CHECK(lock.GetReason().empty());
    CHECK_FALSE(LayoutLock::Refuses(&lock, "test"));
    CHECK_FALSE(LayoutLock::Refuses(nullptr, "test"));

    lock.Set(true);
    CHECK(lock.IsLocked());
    CHECK(lock.GetReason() == LayoutLock::DEFAULT_REASON);
    CHECK(LayoutLock::Refuses(&lock, "test"));
    CHECK_FALSE(LayoutLock::Refuses(nullptr, "test"));

    lock.Set(true, "Hosting a session");
    CHECK(lock.GetReason() == "Hosting a session");

    lock.Set(false, "ignored");
    CHECK_FALSE(lock.IsLocked());
    CHECK(lock.GetReason().empty());
    CHECK_FALSE(LayoutLock::Refuses(&lock, "test"));
}

TEST_CASE("Layout lock: the show context sets it, reports it through RenderContext and tells the host", "[layoutlock]") {
    HookContext ctx;
    RenderContext& rc = ctx;
    CHECK_FALSE(ctx.IsLayoutLocked());
    CHECK_FALSE(rc.IsLayoutLocked());
    CHECK_FALSE(ctx.RefusesLayoutWrite("test"));

    ctx.SetCollaborationLock(true);
    CHECK(ctx.IsLayoutLocked());
    CHECK(rc.IsLayoutLocked());
    CHECK(ctx.GetLayoutLockReason() == "Locked during collaboration session");
    CHECK(ctx.RefusesLayoutWrite("test"));
    CHECK(ctx.changes == 1);
    CHECK(ctx.lockedInHook);
    // the managers see the context's lock
    CHECK(ctx.AllModels.IsLayoutLocked());
    CHECK(ctx.AllObjects.IsLayoutLocked());
    CHECK(ctx._outputManager.IsLayoutLocked());

    // no change, no hook
    ctx.SetCollaborationLock(true);
    CHECK(ctx.changes == 1);
    // a new reason is a change
    ctx.SetCollaborationLock(true, "Joined a session");
    CHECK(ctx.GetLayoutLockReason() == "Joined a session");
    CHECK(ctx.changes == 2);

    ctx.SetCollaborationLock(false);
    CHECK_FALSE(ctx.IsLayoutLocked());
    CHECK_FALSE(ctx.AllModels.IsLayoutLocked());
    CHECK_FALSE(ctx._outputManager.IsLayoutLocked());
    CHECK(ctx.GetLayoutLockReason().empty());
    CHECK(ctx.changes == 3);
    CHECK_FALSE(ctx.lockedInHook);
    ctx.SetCollaborationLock(false);
    CHECK(ctx.changes == 3);
}

TEST_CASE("Layout lock: XL_COLLAB_LAYOUT_LOCK turns it on at construction", "[layoutlock]") {
    struct EnvReset {
        ~EnvReset() { SetEnv("XL_COLLAB_LAYOUT_LOCK", nullptr); }
    } envReset;
    SetEnv("XL_COLLAB_LAYOUT_LOCK", nullptr);
    {
        HeadlessRenderContext ctx;
        CHECK_FALSE(ctx.IsLayoutLocked());
    }
    SetEnv("XL_COLLAB_LAYOUT_LOCK", "0");
    {
        HeadlessRenderContext ctx;
        CHECK_FALSE(ctx.IsLayoutLocked());
    }
    SetEnv("XL_COLLAB_LAYOUT_LOCK", "1");
    {
        HeadlessRenderContext ctx;
        CHECK(ctx.IsLayoutLocked());
        CHECK(ctx.GetLayoutLockReason() == LayoutLock::DEFAULT_REASON);
    }
}

TEST_CASE("Layout lock: models are not deleted or renamed while locked", "[layoutlock]") {
    LockedShow s;
    ModelManager& mm = s.ctx.AllModels;
    auto HasMember = [&](const std::string& name) {
        auto* g = dynamic_cast<ModelGroup*>(mm.GetModel("G"));
        REQUIRE(g != nullptr);
        const auto& names = g->ModelNames();
        return std::find(names.begin(), names.end(), name) != names.end();
    };
    // Each section ends with the same call unlocked, which does change the layout.
    SECTION("delete") {
        s.ctx.SetCollaborationLock(true);
        CHECK_FALSE(mm.Delete("Line", false));
        CHECK(mm.GetModel("Line") != nullptr);
        s.ctx.SetCollaborationLock(false);
        CHECK(mm.Delete("Line", false));
        CHECK(mm.GetModel("Line") == nullptr);
    }
    SECTION("rename") {
        s.ctx.SetCollaborationLock(true);
        CHECK_FALSE(mm.Rename("Arch", "Arch 2"));
        CHECK(mm.GetModel("Arch 2") == nullptr);
        CHECK(mm.GetModel("Arch") != nullptr);
        CHECK(mm.GetRenameLog().Entries().empty());
        s.ctx.SetCollaborationLock(false);
        mm.Rename("Arch", "Arch 2");
        CHECK(mm.GetModel("Arch 2") != nullptr);
        CHECK(mm.GetModel("Arch") == nullptr);
    }
    SECTION("rename submodel") {
        // the group keeps naming the submodel by its old name
        s.ctx.SetCollaborationLock(true);
        CHECK_FALSE(mm.RenameSubModel("Arch/Top", "Arch/Upper"));
        CHECK(HasMember("Arch/Top"));
        CHECK_FALSE(HasMember("Arch/Upper"));
        s.ctx.SetCollaborationLock(false);
        CHECK(mm.RenameSubModel("Arch/Top", "Arch/Upper"));
        CHECK(HasMember("Arch/Upper"));
    }
}

TEST_CASE("Layout lock: view objects are not deleted or renamed while locked", "[layoutlock]") {
    LockedShow s;
    ViewObjectManager& vm = s.ctx.AllObjects;
    // (Rename reports whether other objects changed, not whether it renamed)
    SECTION("rename") {
        s.ctx.SetCollaborationLock(true);
        vm.Rename("Ground", "Floor");
        CHECK(vm.GetViewObject("Ground") != nullptr);
        CHECK(vm.GetViewObject("Floor") == nullptr);
        s.ctx.SetCollaborationLock(false);
        vm.Rename("Ground", "Floor");
        CHECK(vm.GetViewObject("Floor") != nullptr);
    }
    SECTION("delete") {
        s.ctx.SetCollaborationLock(true);
        vm.Delete("Ground");
        CHECK(vm.GetViewObject("Ground") != nullptr);
        s.ctx.SetCollaborationLock(false);
        vm.Delete("Ground");
        CHECK(vm.GetViewObject("Ground") == nullptr);
    }
}

TEST_CASE("Layout lock: controllers are not deleted, moved or saved while locked", "[layoutlock]") {
    LockedShow s;
    OutputManager& om = s.ctx._outputManager;
    REQUIRE(om.GetController("Second") != nullptr);
    REQUIRE(om.GetControllers().front()->GetName() == "First");
    SECTION("delete") {
        s.ctx.SetCollaborationLock(true);
        om.DeleteController("Second");
        CHECK(om.GetController("Second") != nullptr);
        s.ctx.SetCollaborationLock(false);
        om.DeleteController("Second");
        CHECK(om.GetController("Second") == nullptr);
    }
    SECTION("move") {
        s.ctx.SetCollaborationLock(true);
        om.MoveController(om.GetController("Second"), 0);
        CHECK(om.GetControllers().front()->GetName() == "First");
        s.ctx.SetCollaborationLock(false);
        om.MoveController(om.GetController("Second"), 0);
        CHECK(om.GetControllers().front()->GetName() == "Second");
    }
    SECTION("save") {
        const std::string onDisk = ReadFile(s.nets);
        om.GetController("First")->SetDescription("changed while locked");
        s.ctx.SetCollaborationLock(true);
        CHECK_FALSE(om.Save());
        CHECK(ReadFile(s.nets) == onDisk);
        // a copy elsewhere (backup, autosave) is not the controllers file
        const std::string copy = (s.dir.path / "copy.xml").string();
        CHECK(om.SaveCopyTo(copy));
        CHECK(ReadFile(copy).find("changed while locked") != std::string::npos);
        s.ctx.SetCollaborationLock(false);
        CHECK(om.Save());
        CHECK(ReadFile(s.nets).find("changed while locked") != std::string::npos);
    }
}

TEST_CASE("Layout lock: merged layout and controller changes are not brought into memory while locked", "[layoutlock]") {
    LockedShow s;
    REQUIRE(s.ctx.AbortRender());
    SECTION("layout") {
        pugi::xml_document mine;
        pugi::xml_document result;
        REQUIRE(mine.load_string(kLayout));
        REQUIRE(result.load_string(kLayout));
        result.child("xrgb").child("models").remove_child(result.child("xrgb").child("models").last_child());
        s.ctx.SetCollaborationLock(true);
        CHECK_FALSE(ShowMergeApply::ApplyLayout(s.ctx, mine, result).Any());
        CHECK(s.ctx.AllModels.GetModel("Line") != nullptr);
        s.ctx.SetCollaborationLock(false);
        CHECK(ShowMergeApply::ApplyLayout(s.ctx, mine, result).Any());
        CHECK(s.ctx.AllModels.GetModel("Line") == nullptr);
    }
    SECTION("controllers") {
        s.dir.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks computer=\"one\">"
                                            "<Controller Type=\"Null\" Id=\"64001\" Name=\"First\"><network NetworkType=\"NULL\" MaxChannels=\"512\"/></Controller>"
                                            "</Networks>");
        s.ctx.SetCollaborationLock(true);
        CHECK_FALSE(ShowMergeApply::ReloadControllers(s.ctx));
        CHECK(s.ctx._outputManager.GetController("Second") != nullptr);
        s.ctx.SetCollaborationLock(false);
        CHECK(ShowMergeApply::ReloadControllers(s.ctx));
        CHECK(s.ctx._outputManager.GetController("Second") == nullptr);
    }
}
