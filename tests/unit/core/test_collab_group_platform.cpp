// The platform's group-session source as the test binary sees it. The test
// binary is not signed with the group-session entitlement (and there is no
// SharePlay off Apple platforms), so the source must report itself
// unavailable and fail requests at once rather than reach the system.

#include <catch2/catch_test_macros.hpp>

#include "collab/CollabGroup.h"

#include <memory>
#include <string>

using namespace Collab;

TEST_CASE("Platform group sessions are unavailable without the entitlement", "[collab][collabgroup]") {
    IGroupSessionSource& source = PlatformGroupSessions();
    CHECK_FALSE(source.Available());
    CHECK_FALSE(source.InGroup());
    CHECK(&source == &PlatformGroupSessions());
    CHECK(source.OfferedSession().id.empty());

    SessionAdvert advert;
    advert.id = "test-session";
    advert.sequenceName = "Test.xsq";
    advert.hostName = "Tester";

    bool called = false;
    bool gotChannel = true;
    std::string error;
    source.Start(advert, [&](std::unique_ptr<IGroupChannel> channel, const std::string& e) {
        called = true;
        gotChannel = channel != nullptr;
        error = e;
    });
    CHECK(called);
    CHECK_FALSE(gotChannel);
    CHECK_FALSE(error.empty());

    called = false;
    gotChannel = true;
    error.clear();
    source.JoinOffered("test-session", [&](std::unique_ptr<IGroupChannel> channel, const std::string& e) {
        called = true;
        gotChannel = channel != nullptr;
        error = e;
    });
    CHECK(called);
    CHECK_FALSE(gotChannel);
    CHECK_FALSE(error.empty());
}
