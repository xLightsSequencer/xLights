// The handlers are reached through ExposeVendors.h and loopback sockets, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <mutex>

#include "support/ExposeVendors.h"
#include "controllers/AlphaPix.h"
#include "controllers/ESPixelStick.h"
#include "controllers/Experience.h"
#include "controllers/FPP.h"
#include "controllers/Falcon.h"
#include "controllers/HinksPix.h"
#include "controllers/ILightThat.h"
#include "controllers/J1Sys.h"
#include "controllers/PowerDMX.h"
#include "controllers/SanDevices.h"
#include "controllers/WLED.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"
#include "support/VendorShow.h"

// ---------------------------------------------------------------- ILightThat

namespace {
struct FakeBaldrick {
    std::string settings = R"({"ports":[{"port_type":"ws2811","models":[{"name":"Arch","test_colour":5}]},{"port_type":"relay"},{"port_type":"ws2811"}]})";
    std::string putReply = "OK";
    std::mutex m;
    std::string lastPut;
    FakeHttpServer srv{ [this](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path != "/settings") {
            resp.status = 404;
        } else if (r.method == "POST") {
            std::lock_guard<std::mutex> l(m);
            lastPut = r.body;
            resp.body = putReply;
        } else {
            resp.body = settings;
        }
        return resp;
    } };
    nlohmann::json Put() {
        std::lock_guard<std::mutex> l(m);
        return nlohmann::json::parse(lastPut);
    }
};
} // namespace

TEST_CASE("ILightThat3 SetOutputs posts ports with model settings", "[vendors3][ilightthat3]") {
    VendorsTestInit();
    FakeBaldrick fb;
    ILightThat c(fb.srv.Address(), "");
    REQUIRE(c.IsConnected());
    VendorShow s("ILightThat", "Baldrick8", "", 3);
    s.Pixels("Arch", 1, 50, 1, "Brightness=\"50\" gamma=\"2.2\" colorOrder=\"GRB\"");
    s.Pixels("Tree", 3, 20, 511);
    s.Pixels("Star", 3, 10, 571);
    REQUIRE_UPLOAD(s, c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto j = fb.Put();
    INFO(j.dump());
    CHECK(j["start_address"] == 1);
    CHECK(j["start_universe"] == 1);
    CHECK(j["channels_per_universe"] == 510);
    auto& p0 = j["ports"][0];
    CHECK(p0["port_type"] == "ws2811");
    CHECK(p0["num_pixels"] == 50);
    auto& arch = p0["models"][0];
    CHECK(arch["name"] == "Arch");
    CHECK(arch["brightness"] == 50);
    CHECK_THAT(arch["gamma"].get<double>(), Catch::Matchers::WithinAbs(2.2, 1e-5));
    CHECK(arch["test_colour"] == 5); // kept from the controller
    CHECK(arch["start_channel"] == 0);
    CHECK(arch["num_channels"] == 150);
    CHECK(arch["colour_order"] == "GRB");
    CHECK(j["ports"][1]["port_type"] == "relay");
    CHECK(j["ports"][1]["num_pixels"] == 0);
    auto& p2 = j["ports"][2];
    CHECK(p2["num_pixels"] == 30);
    REQUIRE(p2["models"].size() == 2);
    CHECK(p2["models"][0]["name"] == "Tree");
    CHECK(p2["models"][0]["brightness"] == 100);
    CHECK(p2["models"][0]["gamma"] == 1.0);
    CHECK(p2["models"][0]["colour_order"] == "RGB");
    CHECK(p2["models"][1]["start_channel"] == 60);
    CHECK(p2["models"][1]["num_channels"] == 30);
    // models the controller does not know get the next default test colours in turn
    CHECK(p2["models"][0]["test_colour"] == c._model_test_default_colours[0]);
    CHECK(p2["models"][1]["test_colour"] == c._model_test_default_colours[1]);
}

TEST_CASE("ILightThat3 full control fills brightness and gamma from the controller defaults", "[vendors3][ilightthat3]") {
    VendorsTestInit();
    FakeBaldrick fb;
    fb.settings = "not json";
    ILightThat c(fb.srv.Address(), "");
    VendorShow s("ILightThat", "Baldrick8", "", 1, OUTPUT_DDP);
    s.ctl->SetFullxLightsControl(true);
    s.ctl->SetDefaultBrightnessUnderFullControl(40);
    s.ctl->SetDefaultGammaUnderFullControl(2.5f);
    s.Pixels("Arch", 1, 10, 1);
    REQUIRE_UPLOAD(s, c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto j = fb.Put();
    CHECK(j["channels_per_universe"] == 510);
    CHECK(j["ports"][0]["models"][0]["brightness"] == 40);
    CHECK_THAT(j["ports"][0]["models"][0]["gamma"].get<double>(), Catch::Matchers::WithinAbs(2.5, 1e-5));
    CHECK_FALSE(j["ports"][0].contains("port_type"));
}

TEST_CASE("ILightThat3 SetOutputs fails on errors or a refusal", "[vendors3][ilightthat3]") {
    VendorsTestInit();
    FakeBaldrick fb;
    ILightThat c(fb.srv.Address(), "");
    SECTION("port beyond the board") {
        VendorShow s("ILightThat", "Baldrick8", "", 1);
        s.Pixels("Arch", 9, 10, 1);
        CHECK_FALSE(c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(fb.lastPut.empty());
    }
    SECTION("controller does not answer OK") {
        fb.putReply = "ERROR";
        VendorShow s("ILightThat", "Baldrick8", "", 1);
        s.Pixels("Arch", 1, 10, 1);
        CHECK_FALSE(c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK_FALSE(fb.lastPut.empty());
    }
}

TEST_CASE("ILightThat3 constructor needs a settings document", "[vendors3][ilightthat3]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse{ 404, "" }; });
    ILightThat c(srv.Address(), "");
    CHECK_FALSE(c.IsConnected());
    CHECK(c.UsesHTTP());
}

// ---------------------------------------------------------------- BaseController

TEST_CASE("BaseController3 CreateBaseController picks the handler from the caps driver", "[vendors3][basecontroller3]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse{ 404, "" }; });
    struct Row {
        const char* vendor;
        const char* model;
        const char* variant;
        std::function<bool(BaseController*)> is;
    };
    std::vector<Row> rows = {
        { "Falcon", "F16V3", "No Expansion Boards", [](BaseController* b) { return dynamic_cast<Falcon*>(b) != nullptr; } },
        { "HinksPix", "PRO V1/V2", "", [](BaseController* b) { return dynamic_cast<HinksPix*>(b) != nullptr; } },
        { "ILightThat", "Baldrick8", "", [](BaseController* b) { return dynamic_cast<ILightThat*>(b) != nullptr; } },
        // no config driver: assumed to be FPP based
        { "Acme", "Plain", "", [](BaseController* b) { return dynamic_cast<FPP*>(b) != nullptr; } },
    };
    for (auto& r : rows) {
        INFO(r.vendor << " " << r.model);
        VendorShow s(r.vendor, r.model, r.variant, 1);
        if (s.ctl->GetControllerCaps() == nullptr) {
            FAIL_CHECK("no caps for " << r.vendor << " " << r.model);
            continue;
        }
        std::unique_ptr<BaseController> bc(BaseController::CreateBaseController(s.ctl, srv.Address()));
        REQUIRE(bc != nullptr);
        CHECK(r.is(bc.get()));
    }
}

TEST_CASE("BaseController3 CreateBaseController without caps or for multicast gives nothing", "[vendors3][basecontroller3]") {
    VendorsTestInit();
    {
        VendorShow s("NoSuchVendor", "Nope", "", 1);
        CHECK(BaseController::CreateBaseController(s.ctl, "127.0.0.1:1") == nullptr);
    }
    {
        VendorShow s("Falcon", "F16V3", "No Expansion Boards", 1, OUTPUT_E131, 510, 1, "MULTICAST");
        CHECK(BaseController::CreateBaseController(s.ctl) == nullptr);
    }
}

TEST_CASE("BaseController3 requests go through the FPP proxy when one is set", "[vendors3][basecontroller3]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) { return FakeResponse{ 200, "seen " + r.method + " " + r.path }; });
    ILightThat c("192.0.2.9", srv.Address());
    CHECK(c._baseUrl == "/proxy/192.0.2.9");
    CHECK(c.IsConnected());
    CHECK(c.GetURL("/x") == "seen GET /proxy/192.0.2.9/x");
    CHECK(c.PutURL("/y", "body") == "seen POST /proxy/192.0.2.9/y");
    auto reqs = srv.Requests();
    REQUIRE_FALSE(reqs.empty());
    CHECK(reqs.back().body == "body");
}

TEST_CASE("BaseController3 failed requests return nothing", "[vendors3][basecontroller3]") {
    VendorsTestInit();
    ILightThat c("127.0.0.1:1", "");
    CHECK(c.GetURL("/x").empty());
    CHECK(c.PutURL("/y", "data").empty());
    CHECK(c.GetFullName() == c.GetModel());
}

#endif
