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
    // GET /api/board/info: 404 is firmware from before the endpoint
    int infoStatus = 404;
    std::string info;
    int busyReplies = 0; // how many 503s to answer /api/board/info with first
    std::mutex m;
    std::string lastPut;
    FakeHttpServer srv{ [this](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/api/board/info") {
            std::lock_guard<std::mutex> l(m);
            if (busyReplies > 0) {
                busyReplies--;
                resp.status = 503;
            } else {
                resp.status = infoStatus;
                resp.body = info;
            }
        } else if (r.path != "/settings") {
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
    void Capabilities(const std::string& caps) {
        infoStatus = 200;
        info = R"({"versions":{"firmware":"test","recovery":"","bootloader":"","partition_table":""},"capabilities":[)" + caps + "]}";
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

TEST_CASE("ILightThat3 SetOutputs sends grouping, zig zag and null pixels", "[vendors3][ilightthat3]") {
    VendorsTestInit();
    FakeBaldrick fb;
    fb.Capabilities(R"("grouping","null","zigzag")");
    ILightThat c(fb.srv.Address(), "");
    VendorShow s("ILightThat", "Baldrick8", "", 1);
    s.Pixels("Arch", 1, 20, 1, "groupCount=\"3\" ZigZag=\"10\" nullNodes=\"2\" endNullNodes=\"4\"");
    s.Pixels("Star", 1, 10, 61);
    s.Pixels("Tree", 2, 10, 91, "colorOrder=\"RGBW\" nullNodes=\"1\" endNullNodes=\"2\"");
    REQUIRE_UPLOAD(s, c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto j = fb.Put();
    INFO(j.dump());
    // The board counts channels on the wire: it divides num_channels by the grouping, and
    // its nulls are channels at the width of the colour order, not pixels.
    auto& arch = j["ports"][0]["models"][0];
    CHECK(arch["num_channels"] == 180);
    CHECK(arch["grouping"] == 3);
    CHECK(arch["zigzag"] == 10);
    CHECK(arch["start_null_channels"] == 6);
    CHECK(arch["end_null_channels"] == 12);
    auto& star = j["ports"][0]["models"][1];
    CHECK(star["num_channels"] == 30);
    CHECK(star["grouping"] == 1);
    CHECK(star["zigzag"] == 0);
    CHECK(star["start_null_channels"] == 0);
    CHECK(star["end_null_channels"] == 0);
    auto& tree = j["ports"][1]["models"][0];
    CHECK(tree["start_null_channels"] == 4);
    CHECK(tree["end_null_channels"] == 8);
}

TEST_CASE("ILightThat3 SetOutputs refuses grouping the board cannot hold", "[vendors3][ilightthat3]") {
    VendorsTestInit();
    FakeBaldrick fb;
    ILightThat c(fb.srv.Address(), "");
    VendorShow s("ILightThat", "Baldrick8", "", 1);
    s.Pixels("Arch", 1, 10, 1, "groupCount=\"256\"");
    CHECK_FALSE(c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(fb.lastPut.empty());
    CHECK(s.ui.Said("grouped pixels"));
}

TEST_CASE("ILightThat3 SetOutputs leaves out settings the firmware does not list", "[vendors3][ilightthat3]") {
    VendorsTestInit();
    FakeBaldrick fb;
    SECTION("no /api/board/info") {
    }
    SECTION("no capabilities") {
        fb.Capabilities("");
    }
    SECTION("unknown capabilities only") {
        fb.Capabilities(R"("teleport",7)");
    }
    SECTION("board info without a capabilities list") {
        fb.infoStatus = 200;
        fb.info = R"({"versions":{"firmware":"test"}})";
    }
    SECTION("capabilities that is not a list") {
        fb.infoStatus = 200;
        fb.info = R"({"capabilities":null})";
    }
    ILightThat c(fb.srv.Address(), "");
    VendorShow s("ILightThat", "Baldrick8", "", 1);
    s.Pixels("Arch", 1, 20, 1, "groupCount=\"1\" ZigZag=\"1\" nullNodes=\"0\"");
    REQUIRE_UPLOAD(s, c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto j = fb.Put();
    INFO(j.dump());
    auto& arch = j["ports"][0]["models"][0];
    CHECK(arch["num_channels"] == 60);
    CHECK_FALSE(arch.contains("grouping"));
    CHECK_FALSE(arch.contains("zigzag"));
    CHECK_FALSE(arch.contains("start_null_channels"));
    CHECK_FALSE(arch.contains("end_null_channels"));
}

TEST_CASE("ILightThat3 SetOutputs refuses settings the firmware cannot apply", "[vendors3][ilightthat3]") {
    VendorsTestInit();
    FakeBaldrick fb;
    VendorShow s("ILightThat", "Baldrick8", "", 1);
    SECTION("grouping on firmware without /api/board/info") {
        s.Pixels("Arch", 1, 20, 1, "groupCount=\"3\"");
        ILightThat c(fb.srv.Address(), "");
        CHECK_FALSE(c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("Arch uses grouping"));
    }
    SECTION("start nulls without the null capability") {
        fb.Capabilities(R"("grouping","zigzag")");
        s.Pixels("Arch", 1, 20, 1, "nullNodes=\"2\"");
        ILightThat c(fb.srv.Address(), "");
        CHECK_FALSE(c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("Arch uses null pixels"));
    }
    SECTION("end nulls without the null capability") {
        fb.Capabilities(R"("grouping","zigzag")");
        s.Pixels("Arch", 1, 20, 1, "endNullNodes=\"2\"");
        ILightThat c(fb.srv.Address(), "");
        CHECK_FALSE(c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("Arch uses null pixels"));
    }
    SECTION("zig zag without the zigzag capability") {
        fb.Capabilities(R"("grouping","null")");
        s.Pixels("Arch", 1, 20, 1, "ZigZag=\"5\"");
        ILightThat c(fb.srv.Address(), "");
        CHECK_FALSE(c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("Arch uses zig zag"));
    }
    CHECK(fb.lastPut.empty());
}

TEST_CASE("ILightThat3 SetOutputs refuses a port grouping and nulls make too long", "[vendors3][ilightthat3]") {
    VendorsTestInit();
    FakeBaldrick fb;
    fb.Capabilities(R"("grouping","null","zigzag")");
    ILightThat c(fb.srv.Address(), "");
    VendorShow s("ILightThat", "Baldrick8", "", 7);
    SECTION("grouping") {
        // 400 pixels pass xLights' 2250 channel check; grouped by 2 they are 2400 on the wire
        s.Pixels("Arch", 1, 400, 1, "groupCount=\"2\"");
        CHECK_FALSE(c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("Port 1 needs 2400 channels"));
        CHECK(fb.lastPut.empty());
    }
    SECTION("nulls") {
        s.Pixels("Arch", 1, 740, 1, "nullNodes=\"6\" endNullNodes=\"5\"");
        CHECK_FALSE(c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("Port 1 needs 2253 channels"));
        CHECK(fb.lastPut.empty());
    }
    SECTION("exactly at the limit") {
        s.Pixels("Arch", 1, 375, 1, "groupCount=\"2\"");
        REQUIRE_UPLOAD(s, c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    }
    SECTION("only the missing capability is reported when that is the cause") {
        fb.Capabilities(R"("null","zigzag")");
        s.Pixels("Arch", 1, 400, 1, "groupCount=\"2\"");
        s.Pixels("Star", 2, 740, 1201, "nullNodes=\"6\" endNullNodes=\"5\"");
        CHECK_FALSE(c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("Arch uses grouping"));
        CHECK_FALSE(s.ui.Said("Port 1 needs"));
        // a port with nothing unsupported on it still gets its length checked
        CHECK(s.ui.Said("Port 2 needs 2253 channels"));
        CHECK(fb.lastPut.empty());
    }
}

TEST_CASE("ILightThat3 SetOutputs needs a readable /api/board/info", "[vendors3][ilightthat3]") {
    VendorsTestInit();
    FakeBaldrick fb;
    VendorShow s("ILightThat", "Baldrick8", "", 1);
    s.Pixels("Arch", 1, 20, 1);
    SECTION("server error") {
        fb.infoStatus = 500;
        ILightThat c(fb.srv.Address(), "");
        CHECK_FALSE(c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("HTTP 500"));
        CHECK(fb.lastPut.empty());
    }
    SECTION("reply that is not board info") {
        fb.infoStatus = 200;
        fb.info = "<html>hello</html>";
        ILightThat c(fb.srv.Address(), "");
        CHECK_FALSE(c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("not understood"));
        CHECK(fb.lastPut.empty());
    }
    SECTION("busy for longer than the retries") {
        fb.busyReplies = 3;
        ILightThat c(fb.srv.Address(), "");
        CHECK_FALSE(c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("HTTP 503"));
        CHECK(fb.lastPut.empty());
    }
    SECTION("a layout error is reported without asking the board") {
        fb.infoStatus = 500;
        s.Pixels("Star", 1, 10, 61, "groupCount=\"256\"");
        ILightThat c(fb.srv.Address(), "");
        CHECK_FALSE(c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("grouped pixels"));
        CHECK_FALSE(s.ui.Said("capabilities"));
        CHECK(fb.lastPut.empty());
    }
    SECTION("busy, then answers") {
        fb.Capabilities(R"("grouping")");
        fb.busyReplies = 2;
        ILightThat c(fb.srv.Address(), "");
        REQUIRE_UPLOAD(s, c.SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(fb.Put()["ports"][0]["models"][0]["grouping"] == 1);
    }
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
