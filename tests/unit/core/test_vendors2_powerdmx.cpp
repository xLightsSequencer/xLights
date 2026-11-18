// The handlers are reached through ExposeVendors.h and a loopback server, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "support/ExposeVendors.h"
#include "controllers/ILightThat.h"
#include "controllers/PowerDMX.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"

#include "outputs/ControllerEthernet.h"
#include "outputs/OutputManager.h"

namespace {
PowerDMX MakeDisconnectedPowerDMX() {
    VendorsTestInit();
    return PowerDMX("127.0.0.1:1", "");
}

std::unique_ptr<Controller> LoadController(OutputManager& om, const std::string& xml) {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(xml.c_str()));
    return std::unique_ptr<Controller>(Controller::Create(&om, doc.first_child(), ""));
}

const char* kStatus = R"({"system":{"vendor":"PowerDMX","product":"PowerDMX","variant":"PowerDMX-49","typeId":208,"version":"1.0.0","mac":"7C:2C:67:60:01:90"},
"capabilities":{"max_ports":49,"max_pixels_per_port":4096,"supported_inputs":["ddp","e131"]}})";
} // namespace

TEST_CASE("PowerDMX2 EncodeBrightness rounds to supported steps", "[vendors2][powerdmx2]") {
    PowerDMX p = MakeDisconnectedPowerDMX();
    auto row = GENERATE(table<int, int>({ { 0, 0 }, { 2, 0 }, { 3, 5 }, { 8, 10 }, { 13, 15 }, { 27, 25 }, { 28, 30 }, { 29, 30 }, { 30, 30 }, { 34, 30 }, { 35, 40 },
                                          { 99, 100 }, { 100, 100 }, { 255, 260 }, { -3, 0 }, { -10, -5 } }));
    INFO("brightness " << std::get<0>(row));
    CHECK(p.EncodeBrightness(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("PowerDMX2 EncodeGamma", "[vendors2][powerdmx2]") {
    PowerDMX p = MakeDisconnectedPowerDMX();
    auto row = GENERATE(table<double, int>({ { 1.0, 10 }, { 1.4999, 10 }, { 1.5, 20 }, { 2.1499, 20 }, { 2.15, 23 }, { 2.3999, 23 }, { 2.4, 25 }, { 2.6499, 25 },
                                             { 2.65, 28 }, { 2.8999, 28 }, { 2.9, 30 }, { 99.0, 30 }, { -1.0, 10 }, { 0.0, 10 } }));
    CHECK(p.EncodeGamma(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("PowerDMX2 EncodeColorOrder lower cases", "[vendors2][powerdmx2]") {
    PowerDMX p = MakeDisconnectedPowerDMX();
    CHECK(p.EncodeColorOrder("RGB") == "rgb");
    CHECK(p.EncodeColorOrder("gRb") == "grb");
    CHECK(p.EncodeColorOrder("") == "");
}

TEST_CASE("PowerDMX2 static urls", "[vendors2][powerdmx2]") {
    CHECK(PowerDMX::GetStatusURL() == "/api/powerdmx/status");
    CHECK(PowerDMX::GetConfigURL() == "/api/powerdmx/config");
    CHECK(PowerDMX::GetUploadURL() == "/api/powerdmx/upload");
}

TEST_CASE("PowerDMX2 full name joins the model and version", "[vendors2][powerdmx2]") {
    PowerDMX p = MakeDisconnectedPowerDMX();
    CHECK(p.GetFullName() == " v");
    p._model = "PowerDMX-49";
    p._version = "1.2.3";
    CHECK(p.GetFullName() == "PowerDMX-49 v1.2.3");
    CHECK(p.UsesHTTP());
    CHECK(p.GetNumberOfPixelOutputs() == 0);
}

TEST_CASE("PowerDMX2 constructor parses the status", "[vendors2][powerdmx2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = kStatus;
        return r;
    });
    PowerDMX p(srv.Address(), "");
    CHECK(p.IsConnected());
    CHECK(p._model == "PowerDMX-49");
    CHECK(p._version == "1.0.0");
    CHECK(p.GetNumberOfPixelOutputs() == 49);
    REQUIRE(srv.Requests().size() == 1);
    CHECK(srv.Requests()[0].path == "/api/powerdmx/status");
}

TEST_CASE("PowerDMX2 constructor on an unusable status stays disconnected", "[vendors2][powerdmx2][http]") {
    VendorsTestInit();
    auto body = GENERATE(std::string(""), std::string("not json"), std::string("{"), std::string("{}"), std::string("[]"));
    FakeHttpServer srv([&](const FakeRequest&) {
        FakeResponse r;
        r.body = body;
        return r;
    });
    PowerDMX p(srv.Address(), "");
    INFO("body '" << body << "'");
    CHECK_FALSE(p.IsConnected());
}

TEST_CASE("PowerDMX2 constructor with a status missing members should not throw", "[vendors2][powerdmx2][http]") {
    // PowerDMX.cpp constructor: status["system"]["variant"].get<std::string>() etc. throw on missing or mistyped members
    VendorsTestInit();
    auto body = GENERATE(std::string(R"({"system":{}})"), std::string(R"({"a":1})"), std::string(R"({"system":{"variant":"x","version":"1"}})"),
                         std::string(R"({"system":{"variant":1,"version":"1"},"capabilities":{"max_ports":2}})"),
                         std::string(R"({"system":{"variant":"x","version":"1"},"capabilities":{"max_ports":"2"}})"));
    FakeHttpServer srv([&](const FakeRequest&) {
        FakeResponse r;
        r.body = body;
        return r;
    });
    INFO(body);
    CHECK_NOTHROW(PowerDMX(srv.Address(), ""));
}

TEST_CASE("PowerDMX2 constructor accepts a fractional port count", "[vendors2][powerdmx2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = R"({"system":{"variant":"x","version":"1"},"capabilities":{"max_ports":2.9}})";
        return r;
    });
    PowerDMX p(srv.Address(), "");
    CHECK(p.GetNumberOfPixelOutputs() == 2);
}

TEST_CASE("PowerDMX2 GetJSONData and PostJSONToURL", "[vendors2][powerdmx2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        resp.body = r.method == "POST" ? "ack" : "{\"k\":[1]}";
        return resp;
    });
    PowerDMX p = MakeDisconnectedPowerDMX();
    p._ip = srv.Address();
    nlohmann::json v;
    CHECK(p.GetJSONData("/x", v));
    CHECK(v["k"][0] == 1);
    CHECK(p.PostJSONToURL("/y", nlohmann::json{ { "z", true } }) == "ack");
    CHECK(srv.Requests().back().body == "{\"z\":true}");
}

TEST_CASE("PowerDMX2 SetInputUniverses selects the input type", "[vendors2][powerdmx2]") {
    OutputManager om;
    PowerDMX p = MakeDisconnectedPowerDMX();
    struct R {
        const char* proto;
        const char* net;
        const char* expect;
    };
    const R rows[] = { { "E131", "E131", "e1.31" }, { "ArtNet", "ArtNet", "artnet" }, { "DDP", "DDP", "ddp" } };
    for (auto const& r : rows) {
        auto c = LoadController(om, std::string("<Controller Id=\"1\" Name=\"P\" Type=\"Ethernet\" Protocol=\"") + r.proto +
                                        "\" IP=\"10.1.2.3\" ActiveState=\"Active\"><network NetworkType=\"" + r.net +
                                        "\" ComPort=\"10.1.2.3\" BaudRate=\"1\" MaxChannels=\"510\"/></Controller>");
        REQUIRE(c);
        nlohmann::json data;
        int32_t sc = p.SetInputUniverses(data, c.get());
        INFO(r.proto);
        CHECK(sc == (int32_t)c->GetFirstOutput()->GetStartChannel());
        CHECK(data["inputs"] == r.expect);
    }
}

TEST_CASE("PowerDMX2 SetInputUniverses refuses non ethernet controllers", "[vendors2][powerdmx2]") {
    OutputManager om;
    auto c = LoadController(om, R"(<Controller Name="n" Type="Null" ActiveState="Inactive"><network NetworkType="NULL" Id="3" MaxChannels="10"/></Controller>)");
    REQUIRE(c);
    PowerDMX p = MakeDisconnectedPowerDMX();
    nlohmann::json data;
    CHECK(p.SetInputUniverses(data, c.get()) == -1);
    CHECK(data.is_null());
}

TEST_CASE("PowerDMX2 SetInputUniverses with no outputs returns minus one", "[vendors2][powerdmx2]") {
    OutputManager om;
    ControllerEthernet c(&om, false);
    c.SetProtocol(OUTPUT_E131);
    c.ClearOutputs();
    PowerDMX p = MakeDisconnectedPowerDMX();
    nlohmann::json data;
    CHECK(p.SetInputUniverses(data, &c) == -1);
}

TEST_CASE("PowerDMX2 UploadSequence fails for a missing file", "[vendors2][powerdmx2]") {
    PowerDMX p = MakeDisconnectedPowerDMX();
    CHECK_FALSE(p.UploadSequence("missing/file.fseq", "file.fseq", [](int, std::string) { return true; }));
}

TEST_CASE("ILightThat2 constructor connects when settings respond", "[vendors2][ilightthat2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/settings") resp.body = "{}";
        return resp;
    });
    ILightThat l(srv.Address(), "");
    CHECK(l.IsConnected());
    CHECK(l.UsesHTTP());
    CHECK(l._model_test_default_col_idx == 0);
    CHECK(l._model_test_default_colours.size() == 12);
}

TEST_CASE("ILightThat2 constructor with an empty reply is not connected", "[vendors2][ilightthat2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    ILightThat l(srv.Address(), "");
    CHECK_FALSE(l.IsConnected());
}

TEST_CASE("ILightThat2 constructor on an unreachable controller is not connected", "[vendors2][ilightthat2]") {
    VendorsTestInit();
    ILightThat l("127.0.0.1:1", "");
    CHECK_FALSE(l.IsConnected());
    CHECK(l.GetFullName().empty());
}

TEST_CASE("ILightThat2 default test colours are distinct", "[vendors2][ilightthat2]") {
    VendorsTestInit();
    ILightThat l("127.0.0.1:1", "");
    std::set<int> seen(l._model_test_default_colours.begin(), l._model_test_default_colours.end());
    CHECK(seen.size() == l._model_test_default_colours.size());
    for (int c : l._model_test_default_colours) {
        CHECK((c & 0xFF000000) == 0);
    }
}

#endif
