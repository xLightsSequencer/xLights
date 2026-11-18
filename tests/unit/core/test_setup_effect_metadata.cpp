#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include <filesystem>
#include <string>

#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "utils/FileUtils.h"

namespace {

std::filesystem::path RepoEffectMetadataDir() {
    // this file lives in tests/unit/core/
    return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path().parent_path() / "resources" / "effectmetadata";
}

// The app points EffectManager at resources/effectmetadata before any sequence is read, and effects
// cache limits/defaults from it in OnMetadataLoaded (migrations depend on them). Do the same for the
// test run: resolve the metadata directory once, before any EffectManager is built. The directory is
// cached on first successful lookup, so the resources dir is restored afterwards for tests that
// swap it out for their own fixtures.
class EffectMetadataSetup : public Catch::EventListenerBase {
public:
    using Catch::EventListenerBase::EventListenerBase;

    void testRunStarting(Catch::TestRunInfo const&) override {
        std::filesystem::path dir = RepoEffectMetadataDir();
        std::error_code ec;
        if (!std::filesystem::is_directory(dir, ec)) {
            return;
        }
        const std::string prev = FileUtils::GetResourcesDir();
        FileUtils::SetResourcesDir(dir.parent_path().string());
        FileUtils::GetEffectMetadataDirectory();
        FileUtils::SetResourcesDir(prev);
    }
};

} // namespace

CATCH_REGISTER_LISTENER(EffectMetadataSetup)

TEST_CASE("Unit tests load effect metadata from the source tree", "[effects][metadata]") {
    REQUIRE(std::filesystem::is_directory(RepoEffectMetadataDir()));
    CHECK(std::filesystem::path(FileUtils::GetEffectMetadataDirectory()) == RepoEffectMetadataDir());
    EffectManager em;
    RenderableEffect* twinkle = em.GetEffect("Twinkle");
    REQUIRE(twinkle != nullptr);
    CHECK(twinkle->HasMetadata());
}
