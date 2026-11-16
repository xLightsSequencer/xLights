#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>

#include "controllers/ControllerCaps.h"
#include "controllers/Falcon.h"
#include "utils/FileUtils.h"

namespace {

std::filesystem::path RepoResourcesDir() {
    // this file lives in tests/unit/core/
    return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path().parent_path() / "resources";
}

struct RealCapsFixture {
    std::string prevResources;

    RealCapsFixture() {
        prevResources = FileUtils::GetResourcesDir();
        ControllerCaps::UnloadControllers();
        FileUtils::SetResourcesDir(RepoResourcesDir().string());
        ControllerCaps::LoadControllers();
    }
    ~RealCapsFixture() {
        ControllerCaps::UnloadControllers();
        FileUtils::SetResourcesDir(prevResources);
    }
};

} // namespace

TEST_CASE("Falcon serial start channel check only applies in DDP mode", "[controllers][falcon]") {
    CHECK(Falcon::CheckSerialStartChannel(42000, false, 1, 42000).empty());
    CHECK(Falcon::CheckSerialStartChannel(42000, false, 1, 500000).empty());
    CHECK_FALSE(Falcon::CheckSerialStartChannel(42000, true, 1, 42000).empty());
}

TEST_CASE("Falcon serial start channel check boundary", "[controllers][falcon]") {
    CHECK(Falcon::CheckSerialStartChannel(42000, true, 2, 41999).empty());
    const std::string err = Falcon::CheckSerialStartChannel(42000, true, 2, 42000);
    REQUIRE_FALSE(err.empty());
    CHECK(err.find("Serial port 2") != std::string::npos);
    CHECK(err.find("start channel 42000") != std::string::npos);
    CHECK(err.find("below 42000") != std::string::npos);
}

TEST_CASE("Falcon serial start channel check is skipped without a limit", "[controllers][falcon]") {
    CHECK(Falcon::CheckSerialStartChannel(0, true, 1, 500000).empty());
}

TEST_CASE("Only the F16V3 carries the Falcon DDP serial channel limit", "[controllers][falcon]") {
    RealCapsFixture f;

    for (const char* variant : { "No Expansion Boards", "One Expansion Board", "Two Expansion Boards" }) {
        auto* c = ControllerCaps::GetControllerConfig("Falcon", "F16V3", variant);
        REQUIRE(c != nullptr);
        CHECK(c->GetMaxDDPChannels() == 42000);
    }

    auto* f4v3 = ControllerCaps::GetControllerConfig("Falcon", "F4V3", "No Expansion Board");
    REQUIRE(f4v3 != nullptr);
    CHECK(f4v3->GetMaxDDPChannels() == 0);

    auto* f48 = ControllerCaps::GetControllerConfig("Falcon", "F48", "");
    REQUIRE(f48 != nullptr);
    CHECK(f48->GetMaxDDPChannels() == 0);

    auto* f16v4 = ControllerCaps::GetControllerConfig("Falcon", "F16V4", "16 Local Ports");
    REQUIRE(f16v4 != nullptr);
    CHECK(f16v4->GetMaxDDPChannels() == 0);
}
