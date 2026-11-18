#include <catch2/catch_test_macros.hpp>

#include <string>

#include "discovery/Discovery.h"

namespace {

DiscoveredData* Add(Discovery& d, const std::string& ip, const std::string& host = "", const std::string& uuid = "") {
    auto* dd = new DiscoveredData();
    dd->ip = ip;
    dd->hostname = host;
    dd->uuid = uuid;
    d.GetResults().push_back(dd);
    return dd;
}

} // namespace

TEST_CASE("DiscoveredData starts with documented defaults", "[misc][discovery]") {
    DiscoveredData d;
    CHECK(d.controller == nullptr);
    CHECK(d.mode == "bridge");
    CHECK(d.typeId == 0);
    CHECK(d.majorVersion == 0);
    CHECK(d.minorVersion == 0);
    CHECK(d.patchVersion == 0);
    CHECK_FALSE(d.canZipUpload);
    CHECK(d.ip.empty());
    CHECK(d.extraData.is_null());
}

TEST_CASE("DiscoveredData setters work without an attached controller", "[misc][discovery]") {
    DiscoveredData d;
    d.SetModel("F16V4");
    d.SetVendor("Falcon");
    d.SetVariant("Standard");
    d.SetDescription("Roof");
    d.SetProxy("10.0.0.1");
    CHECK(d.model == "F16V4");
    CHECK(d.vendor == "Falcon");
    CHECK(d.variant == "Standard");
    CHECK(d.description == "Roof");
    CHECK(d.proxy == "10.0.0.1");
}

TEST_CASE("DiscoveredData extra data holds arbitrary json", "[misc][discovery]") {
    DiscoveredData d;
    d.extraData["httpConnected"] = true;
    d.extraData["list"] = nlohmann::json::array({ 1, 2, 3 });
    CHECK(d.extraData["httpConnected"].get<bool>());
    CHECK(d.extraData["list"].size() == 3);
}

TEST_CASE("Discovery FindByIp matches by address then by hostname", "[misc][discovery]") {
    Discovery d(nullptr);
    auto* a = Add(d, "10.0.0.2", "alpha");
    auto* b = Add(d, "10.0.0.3", "beta");
    CHECK(d.FindByIp("10.0.0.2") == a);
    CHECK(d.FindByIp("10.0.0.3") == b);
    CHECK(d.FindByIp("alpha") == a);
    CHECK(d.FindByIp("beta") == b);
    CHECK(d.FindByIp("10.0.0.9") == nullptr);
    CHECK(d.FindByIp("") == nullptr);
}

TEST_CASE("Discovery FindByIp can match an entry that has no address yet by hostname", "[misc][discovery]") {
    Discovery d(nullptr);
    auto* a = Add(d, "", "gamma");
    CHECK(d.FindByIp("10.0.0.7", "gamma") == a);
    CHECK(d.FindByIp("10.0.0.7", "other") == nullptr);
}

TEST_CASE("Discovery FindByIp with create adds an entry once", "[misc][discovery]") {
    Discovery d(nullptr);
    auto* made = d.FindByIp("10.1.1.1", "newhost", true);
    REQUIRE(made != nullptr);
    CHECK(made->ip == "10.1.1.1");
    CHECK(made->hostname == "newhost");
    CHECK(d.GetResults().size() == 1);
    CHECK(d.FindByIp("10.1.1.1", "newhost", true) == made);
    CHECK(d.GetResults().size() == 1);
    CHECK(d.FindByIp("10.1.1.2") == nullptr);
    CHECK(d.GetResults().size() == 1);
}

TEST_CASE("Discovery DescribeHost combines address and hostname", "[misc][discovery]") {
    Discovery d(nullptr);
    Add(d, "10.0.0.2", "alpha");
    Add(d, "10.0.0.3", "10.0.0.3");
    Add(d, "10.0.0.4", "");
    Add(d, "", "nameonly");
    CHECK(d.DescribeHost("10.0.0.2") == "10.0.0.2 (alpha)");
    CHECK(d.DescribeHost("10.0.0.3") == "10.0.0.3");
    CHECK(d.DescribeHost("10.0.0.4") == "10.0.0.4");
    CHECK(d.DescribeHost("nameonly") == "nameonly");
    CHECK(d.DescribeHost("192.168.5.5") == "192.168.5.5");
}

TEST_CASE("Discovery FindByUUID matches on uuid and a compatible address", "[misc][discovery]") {
    Discovery d(nullptr);
    auto* a = Add(d, "10.0.0.2", "alpha", "uuid-a");
    Add(d, "10.0.0.3", "beta", "uuid-b");
    CHECK(d.FindByUUID("uuid-a", "10.0.0.2") == a);
    CHECK(d.FindByUUID("uuid-a", "") == a);
    CHECK(d.FindByUUID("uuid-a", "127.0.0.1") == a);
    CHECK(d.FindByUUID("uuid-a", "10.0.0.9") == nullptr);
    CHECK(d.FindByUUID("uuid-zzz", "") == nullptr);
}

TEST_CASE("Discovery FindByUUID never matches an empty uuid", "[misc][discovery]") {
    Discovery d(nullptr);
    Add(d, "10.0.0.2", "alpha", "");
    CHECK(d.FindByUUID("", "") == nullptr);
    CHECK(d.FindByUUID("", "10.0.0.2") == nullptr);
}

TEST_CASE("Discovery with no work to do closes cleanly", "[misc][discovery]") {
    Discovery d(nullptr);
    d.Close(false);
    d.Close(true);
    CHECK(d.GetResults().empty());
    CHECK(d.GetOutputManager() == nullptr);
}

TEST_CASE("Discovery DetectControllerType ignores pages it cannot identify", "[misc][discovery]") {
    Discovery d(nullptr);
    CHECK(d.DetectControllerType("10.0.0.2", "", "") == nullptr);
    CHECK(d.DetectControllerType("10.0.0.2", "", "<html><body>Hello router</body></html>") == nullptr);
    CHECK(d.DetectControllerType("10.0.0.2", "", std::string(100000, 'x')) == nullptr);
    CHECK(d.GetResults().empty());
}

class RecordingDelegate : public DiscoveryDelegate {
public:
    int yields = 0;
    void YieldToUI() override { ++yields; }
};

TEST_CASE("DiscoveryDelegate defaults decline credentials", "[misc][discovery]") {
    DiscoveryDelegate dd;
    std::string u, p;
    bool save = false;
    CHECK_FALSE(dd.PromptForPassword("host", u, p, save));
    CHECK_FALSE(dd.GetStoredPassword("svc", u, p));
    CHECK_FALSE(dd.StorePassword("svc", "u", "p"));
    dd.YieldToUI();
}

TEST_CASE("Discovery accepts a custom delegate", "[misc][discovery]") {
    RecordingDelegate del;
    {
        Discovery d(nullptr, &del);
        d.Close(false);
    }
    CHECK(del.yields >= 0);
}
