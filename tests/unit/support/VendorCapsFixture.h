#pragma once

#include <filesystem>
#include <string>

#include "controllers/ControllerCaps.h"
#include "support/TempDir.h"
#include "utils/FileUtils.h"

// Loads one hand written vendor file so a handler can be given real ControllerCaps.
struct VendorCapsFixture {
    TempDir dir;
    std::string prevResources;

    explicit VendorCapsFixture(const std::string& xml) {
        prevResources = FileUtils::GetResourcesDir();
        std::filesystem::create_directories(dir.path / "controllers");
        dir.Write("controllers/fixture.xcontroller", xml);
        ControllerCaps::UnloadControllers();
        FileUtils::SetResourcesDir(dir.path.string());
        ControllerCaps::LoadControllers();
    }
    ~VendorCapsFixture() {
        ControllerCaps::UnloadControllers();
        FileUtils::SetResourcesDir(prevResources);
    }
};
