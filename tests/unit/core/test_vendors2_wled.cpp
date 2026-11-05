// The handlers are reached through ExposeVendors.h and a loopback server, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "support/ExposeVendors.h"
#include "controllers/WLED.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"
#include "support/VendorCapsFixture.h"

// WLEDOutput is only defined inside WLED.cpp. The data members are repeated here, in the same order, so
// the tests can read and set them; the inline helpers are deliberately left out.
struct WLEDOutput {
    const int output;
    int startCount{ 0 };
    int pixels{ 0 };
    int colorOrder{ 0 };
    int protocol{ 1 };
    bool reverse{ false };
    bool ref{ false };
    int nullPixels{ 0 };
    uint8_t pin{ 255 };
    bool upload{ false };
    explicit WLEDOutput(int output_) : output(output_) {}
};

namespace {
WLED MakeDisconnectedWLED() {
    VendorsTestInit();
    return WLED("127.0.0.1:1", "");
}

const char* kWledVendor = R"(<Vendor Name="WLED">
    <Controller Name="WLED">
        <Variant Name="Generic ESP32" ID="wled-esp32">
            <MaxPixelPort>3</MaxPixelPort>
            <Port1>16</Port1>
            <Port2>17</Port2>
            <Port3>300</Port3>
        </Variant>
    </Controller>
</Vendor>)";

template <class P>
WLEDOutput* M(const P& p) {
    return &*p;
}

nlohmann::json Cfg(const std::vector<nlohmann::json>& ins) {
    nlohmann::json j;
    j["hw"]["led"]["ins"] = ins;
    return j;
}
} // namespace

TEST_CASE("WLED2 EncodeColorOrder maps names to WLED codes", "[vendors2][wled2]") {
    WLED w = MakeDisconnectedWLED();
    auto row = GENERATE(table<std::string, int>({ { "grb", 0 }, { "GRB", 0 }, { "grbw", 0 }, { "rgb", 1 }, { "RGBW", 1 }, { "brg", 2 }, { "brgw", 2 },
                                                  { "rbg", 3 }, { "rbgw", 3 }, { "bgr", 4 }, { "bgrw", 4 }, { "gbr", 5 }, { "gbrw", 5 },
                                                  { "wrgb", 34 }, { "WRGB", 34 }, { "wgrb", 36 }, { "wbrg", 37 }, { "wrbg", 32 }, { "wbgr", 35 }, { "wgbr", 33 } }));
    INFO(std::get<0>(row));
    CHECK(w.EncodeColorOrder(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("WLED2 EncodeColorOrder of an unknown order asserts", "[vendors2][wled2][.][known-crash]") {
    // WLED.cpp EncodeColorOrder: assert(false) for an unrecognised order (e.g. an empty colour order from a model); release builds return 1
    WLED w = MakeDisconnectedWLED();
    CHECK(w.EncodeColorOrder("") == 1);
}

TEST_CASE("WLED2 EncodeStringPortProtocol maps chips to WLED types", "[vendors2][wled2]") {
    WLED w = MakeDisconnectedWLED();
    auto row = GENERATE(table<std::string, int>({ { "ws2811", 22 }, { "WS2811", 22 }, { "tm1829", 25 }, { "ucs8903", 26 }, { "ucs8904", 29 }, { "sk6812rgbw", 30 },
                                                  { "apa109", 30 }, { "tm1814", 31 }, { "ws2801", 50 }, { "apa102", 51 }, { "lpd8806", 52 }, { "p9813", 53 },
                                                  { "lpd6803", 54 } }));
    INFO(std::get<0>(row));
    CHECK(w.EncodeStringPortProtocol(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("WLED2 EncodeStringPortProtocol of an unknown chip should return minus one", "[vendors2][wled2][.][known-crash]") {
    // WLED.cpp EncodeStringPortProtocol: assert(false) then return 22, but UpdatePortData tests for -1 to mean "unknown",
    // so an unsupported chip aborts a debug build and silently becomes WS2811 in release
    WLED w = MakeDisconnectedWLED();
    CHECK(w.EncodeStringPortProtocol("tls3001") == -1);
}

TEST_CASE("WLED2 EncodeDirection only recognises reverse", "[vendors2][wled2]") {
    WLED w = MakeDisconnectedWLED();
    CHECK(w.EncodeDirection("reverse"));
    CHECK(w.EncodeDirection("Reverse"));
    CHECK(w.EncodeDirection("REVERSE"));
    CHECK_FALSE(w.EncodeDirection("forward"));
    CHECK_FALSE(w.EncodeDirection(""));
    CHECK_FALSE(w.EncodeDirection("reversed"));
    CHECK_FALSE(w.EncodeDirection(" reverse"));
}

TEST_CASE("WLED2 static urls", "[vendors2][wled2]") {
    CHECK(WLED::GetInfoURL() == "/json/info");
    CHECK(WLED::GetCfgURL() == "/json/cfg");
}

TEST_CASE("WLED2 UsesHTTP and default state", "[vendors2][wled2]") {
    WLED w = MakeDisconnectedWLED();
    CHECK(w.UsesHTTP());
    CHECK_FALSE(w.IsConnected());
    CHECK(w.GetFullName().empty());
    CHECK(w._pixelOutputs.empty());
}

TEST_CASE("WLED2 GetOutputPin reads the per port custom property", "[vendors2][wled2]") {
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    REQUIRE(caps != nullptr);
    WLED w = MakeDisconnectedWLED();
    CHECK(w.GetOutputPin(1, caps) == 16);
    CHECK(w.GetOutputPin(2, caps) == 17);
    CHECK(w.GetOutputPin(3, caps) == 44); // 300 truncated into the uint8_t return
    CHECK(w.GetOutputPin(4, caps) == 2);  // default when the port has no entry
}

TEST_CASE("WLED2 ExtractOutputJSON reads an instance", "[vendors2][wled2]") {
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    auto cfg = Cfg({ { { "len", 50 }, { "start", 10 }, { "pin", { 4 } }, { "type", 22 }, { "order", 1 }, { "rev", true }, { "skip", 2 } } });
    auto out = w.ExtractOutputJSON(cfg, 1, caps, false);
    REQUIRE(out != nullptr);
    CHECK(M(out)->output == 1);
    CHECK(M(out)->pixels == 50);
    CHECK(M(out)->startCount == 10);
    CHECK(M(out)->pin == 4);
    CHECK(M(out)->protocol == 22);
    CHECK(M(out)->colorOrder == 1);
    CHECK(M(out)->reverse);
    CHECK(M(out)->nullPixels == 2);
    CHECK_FALSE(M(out)->upload);
}

TEST_CASE("WLED2 ExtractOutputJSON in full control ignores the controller config", "[vendors2][wled2]") {
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    auto cfg = Cfg({ { { "len", 50 } } });
    auto out = w.ExtractOutputJSON(cfg, 1, caps, true);
    CHECK(M(out)->pixels == 0);
    CHECK(M(out)->pin == 16);
}

TEST_CASE("WLED2 ExtractOutputJSON defaults when the port is beyond the instances", "[vendors2][wled2]") {
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    auto cfg = Cfg({ { { "len", 50 } } });
    auto out = w.ExtractOutputJSON(cfg, 2, caps, false);
    CHECK(M(out)->pixels == 0);
    CHECK(M(out)->protocol == 1);
    CHECK(M(out)->colorOrder == 0);
    CHECK(M(out)->pin == 17);
    auto none = w.ExtractOutputJSON(nlohmann::json::object(), 1, caps, false);
    CHECK(M(none)->pin == 16);
}

TEST_CASE("WLED2 ExtractOutputJSON ignores mistyped members", "[vendors2][wled2]") {
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    auto cfg = Cfg({ { { "len", "50" }, { "start", 1.5 }, { "pin", 4 }, { "type", nullptr }, { "order", "rgb" }, { "rev", 1 }, { "skip", true } } });
    auto out = w.ExtractOutputJSON(cfg, 1, caps, false);
    CHECK(M(out)->pixels == 0);
    CHECK(M(out)->startCount == 0);
    CHECK(M(out)->protocol == 1);
    CHECK(M(out)->colorOrder == 0);
    CHECK_FALSE(M(out)->reverse);
    CHECK(M(out)->nullPixels == 0);
    CHECK(M(out)->pin == 16);
}

TEST_CASE("WLED2 ExtractOutputJSON tolerates a null instance", "[vendors2][wled2]") {
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    auto cfg = Cfg({ nlohmann::json() });
    auto out = w.ExtractOutputJSON(cfg, 1, caps, false);
    CHECK(M(out)->pin == 16);
}

TEST_CASE("WLED2 ExtractOutputJSON with an empty pin array should not throw", "[vendors2][wled2][!mayfail]") {
    // WLED.cpp ExtractOutputJSON: json.at("pin").at(0) throws out_of_range for "pin":[] (and get<int> throws for a non-number pin)
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    auto cfg = Cfg({ { { "len", 5 }, { "pin", nlohmann::json::array() } } });
    CHECK_NOTHROW(w.ExtractOutputJSON(cfg, 1, caps, false));
}

TEST_CASE("WLED2 ExtractOutputJSON with a string pin should not throw", "[vendors2][wled2][!mayfail]") {
    // WLED.cpp ExtractOutputJSON: pin array element is read with get<int>() without a type check
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    auto cfg = Cfg({ { { "pin", { "D4" } } } });
    CHECK_NOTHROW(w.ExtractOutputJSON(cfg, 1, caps, false));
}

TEST_CASE("WLED2 ExtractOutputJSON with a pin above 255 truncates", "[vendors2][wled2][!mayfail]") {
    // WLED.cpp ExtractOutputJSON: the pin is stored in a uint8_t so 256 becomes 0 instead of being rejected
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    auto cfg = Cfg({ { { "pin", { 256 } } } });
    auto out = w.ExtractOutputJSON(cfg, 1, caps, false);
    CHECK(M(out)->pin != 0);
}

TEST_CASE("WLED2 ExtractOutputJSON with ins as an object should not throw", "[vendors2][wled2][!mayfail]") {
    // WLED.cpp ExtractOutputJSON: ins.size() > port-1 is checked but .at(int) on an object throws type_error
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    nlohmann::json cfg;
    cfg["hw"]["led"]["ins"] = { { "a", 1 }, { "b", 2 } };
    CHECK_NOTHROW(w.ExtractOutputJSON(cfg, 1, caps, false));
}

TEST_CASE("WLED2 ExtractOutputJSON copes with hw of the wrong type", "[vendors2][wled2]") {
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    for (nlohmann::json hw : { nlohmann::json("x"), nlohmann::json(5), nlohmann::json::array(), nlohmann::json(nullptr) }) {
        nlohmann::json cfg;
        cfg["hw"] = hw;
        INFO(hw.dump());
        CHECK_NOTHROW(w.ExtractOutputJSON(cfg, 1, caps, false));
    }
    nlohmann::json cfg;
    cfg["hw"]["led"] = 5;
    CHECK_NOTHROW(w.ExtractOutputJSON(cfg, 1, caps, false));
}

TEST_CASE("WLED2 ExtractOutputJSON with ins as a scalar should not throw", "[vendors2][wled2][!mayfail]") {
    // WLED.cpp ExtractOutputJSON: a scalar has size() 1, so .at(0) is attempted on a number and throws type_error
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    nlohmann::json cfg;
    cfg["hw"]["led"]["ins"] = 5;
    CHECK_NOTHROW(w.ExtractOutputJSON(cfg, 1, caps, false));
}

TEST_CASE("WLED2 ExtractOutputJSON for port zero should not throw", "[vendors2][wled2][!mayfail]") {
    // WLED.cpp ExtractOutputJSON: (int)size() > port-1 is true for port 0 and .at(-1) throws out_of_range
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    CHECK_NOTHROW(w.ExtractOutputJSON(Cfg({ { { "len", 50 } } }), 0, caps, false));
}

TEST_CASE("WLED2 ParseOutputJSON builds one entry per port and replaces old ones", "[vendors2][wled2]") {
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    auto cfg = Cfg({ { { "len", 5 } }, { { "len", 6 } } });
    CHECK(w.ParseOutputJSON(cfg, 3, caps, false));
    REQUIRE(w._pixelOutputs.size() == 3);
    CHECK(M(w._pixelOutputs[0])->pixels == 5);
    CHECK(M(w._pixelOutputs[1])->pixels == 6);
    CHECK(M(w._pixelOutputs[2])->pixels == 0);
    CHECK(M(w.FindPortData(2))->pixels == 6);
    CHECK(w.ParseOutputJSON(cfg, 1, caps, false));
    CHECK(w._pixelOutputs.size() == 1);
    CHECK(w.ParseOutputJSON(cfg, 0, caps, false));
    CHECK(w._pixelOutputs.empty());
    CHECK(w.ParseOutputJSON(cfg, -4, caps, false));
    CHECK(w._pixelOutputs.empty());
}

TEST_CASE("WLED2 FindPortData is one based", "[vendors2][wled2]") {
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    w.ParseOutputJSON(nlohmann::json::object(), 2, caps, true);
    CHECK(w.FindPortData(1) != nullptr);
    CHECK(w.FindPortData(2) != nullptr);
    CHECK(M(w.FindPortData(1))->output == 1);
}

TEST_CASE("WLED2 FindPortData of a missing port asserts", "[vendors2][wled2][.][known-crash]") {
    // WLED.cpp FindPortData: assert(false) before returning nullptr, and SetOutputs relies on the nullptr
    WLED w = MakeDisconnectedWLED();
    CHECK(w.FindPortData(7) == nullptr);
}

TEST_CASE("WLED2 UpdatePixelOutputs writes totals and only uploaded ports", "[vendors2][wled2]") {
    VendorCapsFixture fx(kWledVendor);
    auto* caps = ControllerCaps::GetControllerConfig("WLED", "WLED", "Generic ESP32");
    WLED w = MakeDisconnectedWLED();
    w.ParseOutputJSON(Cfg({ { { "len", 5 }, { "start", 0 }, { "type", 22 } } }), 3, caps, false);
    M(w._pixelOutputs[0])->upload = true;
    M(w._pixelOutputs[2])->upload = true;
    M(w._pixelOutputs[2])->pixels = 9;
    nlohmann::json val;
    bool worked = true;
    w.UpdatePixelOutputs(worked, 14, val);
    CHECK(val["hw"]["led"]["total"] == 14);
    REQUIRE(val["hw"]["led"]["ins"].is_array());
    REQUIRE(val["hw"]["led"]["ins"].size() == 2);
    CHECK(val["hw"]["led"]["ins"][0]["len"] == 5);
    CHECK(val["hw"]["led"]["ins"][0]["type"] == 22);
    CHECK(val["hw"]["led"]["ins"][1]["len"] == 9);
    CHECK(val["hw"]["led"]["ins"][0]["pin"][0] == 16);
    CHECK(val["hw"]["led"]["ins"][0]["rev"] == false);
    CHECK(val["hw"]["led"]["ins"][0]["skip"] == 0);
    CHECK(val["hw"]["led"]["ins"][0]["ref"] == false);
}

TEST_CASE("WLED2 UpdatePixelOutputs with nothing to upload should write an empty array", "[vendors2][wled2][!mayfail]") {
    // WLED.cpp UpdatePixelOutputs: newLEDS stays a null json when no port uploads, so "ins" becomes null rather than []
    WLED w = MakeDisconnectedWLED();
    nlohmann::json val;
    bool worked = true;
    w.UpdatePixelOutputs(worked, 0, val);
    CHECK(val["hw"]["led"]["ins"].is_array());
}

TEST_CASE("WLED2 constructor reads the info document", "[vendors2][wled2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = R"({"ver":"0.14.0","vid":2310130,"arch":"esp32","name":"WLED"})";
        return r;
    });
    WLED w(srv.Address(), "");
    CHECK(w.IsConnected());
    CHECK(w.GetVersion() == "0.14.0");
    CHECK(w.GetModel() == "esp32");
    CHECK(w._vid == 2310130);
    CHECK(w.GetFullName() == "esp32 0.14.0");
}

TEST_CASE("WLED2 constructor rejects bad info documents", "[vendors2][wled2][http]") {
    VendorsTestInit();
    auto body = GENERATE(std::string(""), std::string("<html>offline</html>"), std::string("{}"), std::string("[]"), std::string("null"), std::string("5"),
                         std::string(R"({"ver":"1","vid":1,"arch":"x"})"), std::string(R"({"ver":"1","vid":1,"name":"x"})"),
                         std::string(R"({"vid":1,"arch":"x","name":"x"})"), std::string(R"({"ver":"1","arch":"x","name":"x"})"), std::string("{\"ver\":"));
    FakeHttpServer srv([&](const FakeRequest&) {
        FakeResponse r;
        r.body = body;
        return r;
    });
    WLED w(srv.Address(), "");
    INFO("body '" << body << "'");
    CHECK_FALSE(w.IsConnected());
}

TEST_CASE("WLED2 constructor with mistyped info members should not throw", "[vendors2][wled2][http][!mayfail]") {
    // WLED.cpp constructor: ver/vid/arch are read with get<>() after only contains() checks, so a numeric ver or a string vid throws out of the constructor
    VendorsTestInit();
    auto body = GENERATE(std::string(R"({"ver":14,"vid":1,"arch":"x","name":"x"})"), std::string(R"({"ver":"1","vid":"1","arch":"x","name":"x"})"),
                         std::string(R"({"ver":"1","vid":1,"arch":null,"name":"x"})"));
    FakeHttpServer srv([&](const FakeRequest&) {
        FakeResponse r;
        r.body = body;
        return r;
    });
    INFO(body);
    CHECK_NOTHROW(WLED(srv.Address(), ""));
}

TEST_CASE("WLED2 constructor on an unreachable controller is disconnected", "[vendors2][wled2]") {
    WLED w = MakeDisconnectedWLED();
    CHECK_FALSE(w.IsConnected());
    CHECK(w.GetVersion().empty());
}

TEST_CASE("WLED2 constructor uses the proxy prefix in requests", "[vendors2][wled2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = R"({"ver":"0.14.0","vid":1,"arch":"esp8266","name":"n"})";
        return r;
    });
    WLED w("10.9.9.9", srv.Address());
    CHECK(w.IsConnected());
    auto reqs = srv.Requests();
    REQUIRE(reqs.size() == 1);
    CHECK(reqs[0].path == "/proxy/10.9.9.9/json/info");
}

#endif
