// The handlers are reached through ExposeVendors.h and a loopback server, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "support/ExposeVendors.h"
#include "controllers/Experience.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"

#include "outputs/ControllerEthernet.h"
#include "outputs/DDPOutput.h"
#include "outputs/OutputManager.h"

namespace {
Experience MakeDisconnectedExperience() {
    VendorsTestInit();
    return Experience("127.0.0.1:1", "");
}

std::string StateJson(const std::string& system) {
    return "{\"system\":" + system + "}";
}

const char* kGoodSystem = R"({"controller_model_name":"Genius 16","controller_model":"G16_2023","firmware_version":"2.3.1-45","controller_line":"Genius","number_of_local_outputs":16,"number_of_long_range_pixel_ports":8,"number_of_long_range_dmx_ports":2})";

std::unique_ptr<Controller> LoadController(OutputManager& om, const std::string& xml) {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(xml.c_str()));
    return std::unique_ptr<Controller>(Controller::Create(&om, doc.first_child(), ""));
}
} // namespace

TEST_CASE("Experience2 DecodeFirmwareInformation parses major minor patch and build", "[vendors2][experience2]") {
    Experience e = MakeDisconnectedExperience();
    auto row = GENERATE(table<std::string, std::vector<int>>({ { "1.2.3-4", { 1, 2, 3, 4 } }, { "v10.20.30-40", { 10, 20, 30, 40 } },
                                                              { "Genius 2.3.1-45", { 2, 3, 1, 45 } }, { "1.2.-4", { 1, 2, 0, 4 } },
                                                              { "0.0.0-0", { 0, 0, 0, 0 } }, { "1.2.3-4-beta", { 1, 2, 3, 4 } },
                                                              { "7.8.9-10.11.12-13", { 7, 8, 9, 10 } } }));
    CHECK(e.DecodeFirmwareInformation(std::get<0>(row)));
    CHECK(e._firmwareMajor == std::get<1>(row)[0]);
    CHECK(e._firmwareMinor == std::get<1>(row)[1]);
    CHECK(e._firmwarePatch == std::get<1>(row)[2]);
    CHECK(e._firmwareBuild == std::get<1>(row)[3]);
}

TEST_CASE("Experience2 DecodeFirmwareInformation rejects strings without a build number", "[vendors2][experience2]") {
    Experience e = MakeDisconnectedExperience();
    for (std::string s : { std::string(""), std::string("1.2.3"), std::string("1.2"), std::string("abc"), std::string("1-2-3"), std::string("..-"), std::string("1.2.3-") }) {
        INFO("firmware '" << s << "'");
        CHECK_FALSE(e.DecodeFirmwareInformation(s));
        CHECK(e._firmwareMajor == 0);
    }
}

TEST_CASE("Experience2 DecodeFirmwareInformation keeps old values when the parse fails", "[vendors2][experience2]") {
    Experience e = MakeDisconnectedExperience();
    REQUIRE(e.DecodeFirmwareInformation("3.4.5-6"));
    CHECK_FALSE(e.DecodeFirmwareInformation("garbage"));
    CHECK(e._firmwareMajor == 3);
    CHECK(e._firmwareBuild == 6);
}

TEST_CASE("Experience2 DecodeFirmwareInformation with huge components does not crash", "[vendors2][experience2]") {
    Experience e = MakeDisconnectedExperience();
    CHECK(e.DecodeFirmwareInformation("99999999999999999999.1.1-1"));
    CHECK(e.DecodeFirmwareInformation(std::string(5000, '9') + ".1.1-1"));
}

TEST_CASE("Experience2 DecodeModelInformation splits model and year", "[vendors2][experience2]") {
    Experience e = MakeDisconnectedExperience();
    CHECK(e.DecodeModelInformation("G16_2023"));
    CHECK(e._controllerModel == "G16");
    CHECK(e._modelYear == 2023);
    CHECK(e.DecodeModelInformation("G16_V2_2024"));
    CHECK(e._controllerModel == "G16");
    CHECK(e._modelYear == 0); // "V2_2024" is not a number
    CHECK(e.DecodeModelInformation("_2020"));
    CHECK(e._controllerModel == "");
    CHECK(e._modelYear == 2020);
    CHECK(e.DecodeModelInformation("G_"));
    CHECK(e._controllerModel == "G");
    CHECK(e._modelYear == 0);
}

TEST_CASE("Experience2 DecodeModelInformation without a year keeps just the name", "[vendors2][experience2]") {
    Experience e = MakeDisconnectedExperience();
    CHECK_FALSE(e.DecodeModelInformation("G16"));
    CHECK(e._controllerModel == "G16");
    e._modelYear = 5;
    CHECK_FALSE(e.DecodeModelInformation(""));
    CHECK(e._controllerModel == "G16"); // empty input leaves the model alone
    CHECK(e._modelYear == 5);
}

TEST_CASE("Experience2 IsVersionAtLeast compares major minor patch", "[vendors2][experience2]") {
    Experience e = MakeDisconnectedExperience();
    e._firmwareMajor = 2;
    e._firmwareMinor = 3;
    e._firmwarePatch = 4;
    struct R {
        int a, b, c;
        bool expect;
    };
    const R rows[] = { { 2, 3, 4, true }, { 2, 3, 5, false }, { 2, 3, 3, true }, { 2, 4, 0, false }, { 2, 2, 99, true }, { 1, 99, 99, true },
                       { 3, 0, 0, false }, { 0, 0, 0, true }, { 2, 3, 0, true }, { -1, -1, -1, true } };
    for (auto const& r : rows) {
        INFO(r.a << "." << r.b << "." << r.c);
        CHECK(e.IsVersionAtLeast(r.a, r.b, r.c) == r.expect);
    }
}

TEST_CASE("Experience2 version strings", "[vendors2][experience2]") {
    Experience e = MakeDisconnectedExperience();
    CHECK(e.GetVersionStr() == "0.0.0.0");
    e.DecodeFirmwareInformation("1.2.3-4");
    CHECK(e.GetVersionStr() == "1.2.3.4");
    e._model = "Genius";
    CHECK(e.GetFullName() == "Genius v1.2.3.4");
}

TEST_CASE("Experience2 EncodeBrightness rounds to supported steps", "[vendors2][experience2]") {
    Experience e = MakeDisconnectedExperience();
    auto row = GENERATE(table<int, int>({ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 5 }, { 7, 5 }, { 8, 10 }, { 12, 10 }, { 13, 15 }, { 27, 25 }, { 28, 30 }, { 29, 30 },
                                          { 30, 30 }, { 34, 30 }, { 35, 40 }, { 44, 40 }, { 45, 50 }, { 95, 100 }, { 100, 100 }, { 104, 100 }, { 105, 110 } }));
    INFO("brightness " << std::get<0>(row));
    CHECK(e.EncodeBrightness(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("Experience2 EncodeBrightness of negative values", "[vendors2][experience2]") {
    Experience e = MakeDisconnectedExperience();
    CHECK(e.EncodeBrightness(-2) == 0);
    CHECK(e.EncodeBrightness(-3) == 0);
    CHECK(e.EncodeBrightness(-10) == -5);
    CHECK(e.EncodeBrightness(INT32_MIN + 10) <= 0);
}

TEST_CASE("Experience2 EncodeBrightness near INT_MAX overflows", "[vendors2][experience2]") {
    // Experience.cpp EncodeBrightness: brightness + 5 overflows a signed int for values near INT_MAX (UB)
    Experience e = MakeDisconnectedExperience();
    CHECK(e.EncodeBrightness(INT32_MAX) > 0);
}

TEST_CASE("Experience2 EncodeGamma maps to hundredths", "[vendors2][experience2]") {
    Experience e = MakeDisconnectedExperience();
    auto row = GENERATE(table<double, int>({ { 0.0, 10 }, { 1.0, 10 }, { 1.49, 10 }, { 1.5, 20 }, { 2.0, 20 }, { 2.149, 20 }, { 2.15, 23 }, { 2.3, 23 }, { 2.399, 23 },
                                             { 2.4, 25 }, { 2.5, 25 }, { 2.649, 25 }, { 2.65, 28 }, { 2.8, 28 }, { 2.899, 28 }, { 2.9, 30 }, { 3.0, 30 }, { 1000.0, 30 },
                                             { -3.0, 10 } }));
    INFO("gamma " << std::get<0>(row));
    CHECK(e.EncodeGamma(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("Experience2 EncodeGamma of NaN picks the top step", "[vendors2][experience2]") {
    Experience e = MakeDisconnectedExperience();
    CHECK(e.EncodeGamma(std::numeric_limits<double>::quiet_NaN()) == 30);
}

TEST_CASE("Experience2 EncodeColorOrder lower cases", "[vendors2][experience2]") {
    Experience e = MakeDisconnectedExperience();
    CHECK(e.EncodeColorOrder("RGB") == "rgb");
    CHECK(e.EncodeColorOrder("GrBw") == "grbw");
    CHECK(e.EncodeColorOrder("") == "");
}

TEST_CASE("Experience2 static urls", "[vendors2][experience2]") {
    CHECK(Experience::GetStateURL() == "/api/state");
    CHECK(Experience::GetConfigURL() == "/api/config");
}

TEST_CASE("Experience2 disconnected defaults", "[vendors2][experience2]") {
    Experience e = MakeDisconnectedExperience();
    CHECK_FALSE(e.IsConnected());
    CHECK(e.UsesHTTP());
    CHECK(e.GetNumberOfPixelOutputs() == 0);
    CHECK(e.GetNumberOfRemoteOutputs() == 0);
    CHECK(e.GetNumberOfSerial() == 0);
    CHECK(e._opMode == "ddp");
    CHECK_FALSE(e._has_efuses);
}

TEST_CASE("Experience2 constructor parses state and config", "[vendors2][experience2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/api/state") {
            resp.body = StateJson(kGoodSystem);
        } else if (r.path == "/api/config") {
            resp.body = R"({"system":{"operating_mode":"standalone"}})";
        } else {
            resp.status = 404;
        }
        return resp;
    });
    Experience e(srv.Address(), "");
    CHECK(e.IsConnected());
    CHECK(e._model == "Genius 16");
    CHECK(e._version == "2.3.1-45");
    CHECK(e._controllerType == "Genius");
    CHECK(e._controllerModel == "G16");
    CHECK(e._modelYear == 2023);
    CHECK(e.GetNumberOfPixelOutputs() == 16);
    CHECK(e.GetNumberOfRemoteOutputs() == 8);
    CHECK(e.GetNumberOfSerial() == 2);
    CHECK(e._firmwareMajor == 2);
    CHECK(e._firmwareBuild == 45);
    CHECK(e._opMode == "standalone");
    CHECK_FALSE(e._has_efuses);
    CHECK(e.GetFullName() == "Genius 16 v2.3.1.45");
}

TEST_CASE("Experience2 constructor reads has_efuses", "[vendors2][experience2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/api/state") {
            resp.body = StateJson(R"({"controller_model_name":"G","controller_model":"G_1","firmware_version":"1.0.0-1","controller_line":"L","number_of_local_outputs":1,"number_of_long_range_pixel_ports":0,"number_of_long_range_dmx_ports":0,"has_efuses":true})");
        } else {
            resp.body = R"({"system":{"operating_mode":"ddp"}})";
        }
        return resp;
    });
    Experience e(srv.Address(), "");
    CHECK(e.IsConnected());
    CHECK(e._has_efuses);
}

TEST_CASE("Experience2 constructor needs both documents to be json", "[vendors2][experience2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/api/state") {
            resp.body = StateJson(kGoodSystem);
        } else {
            resp.body = "<html>error</html>";
        }
        return resp;
    });
    Experience e(srv.Address(), "");
    CHECK_FALSE(e.IsConnected());
}

TEST_CASE("Experience2 constructor with a non json state is not connected", "[vendors2][experience2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "not json";
        return r;
    });
    Experience e(srv.Address(), "");
    CHECK_FALSE(e.IsConnected());
    CHECK(srv.Requests().size() == 1);
}

TEST_CASE("Experience2 constructor with a state missing members should not throw", "[vendors2][experience2][http]") {
    // Experience.cpp constructor: data["system"][...].get<>() on missing/mistyped members throws out of the constructor
    VendorsTestInit();
    auto state = GENERATE(std::string(R"({"system":{}})"), std::string(R"({"x":1})"), std::string(R"({"system":5})"),
                          std::string(R"({"system":{"controller_model_name":5}})"), std::string("[1]"));
    FakeHttpServer srv([&](const FakeRequest& r) {
        FakeResponse resp;
        resp.body = r.path == "/api/state" ? state : std::string(R"({"system":{"operating_mode":"ddp"}})");
        return resp;
    });
    INFO(state);
    CHECK_NOTHROW(Experience(srv.Address(), ""));
}

TEST_CASE("Experience2 constructor with an empty state object is not connected", "[vendors2][experience2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        resp.body = r.path == "/api/state" ? "{}" : R"({"system":{"operating_mode":"ddp"}})";
        return resp;
    });
    Experience e(srv.Address(), "");
    CHECK_FALSE(e.IsConnected());
}

TEST_CASE("Experience2 constructor with a config lacking operating_mode should not throw", "[vendors2][experience2][http]") {
    // Experience.cpp constructor: config["system"]["operating_mode"].get<std::string>() throws when the member is absent
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        resp.body = r.path == "/api/state" ? StateJson(kGoodSystem) : std::string(R"({"system":{}})");
        return resp;
    });
    CHECK_NOTHROW(Experience(srv.Address(), ""));
}

TEST_CASE("Experience2 constructor keeps the default mode for an empty config", "[vendors2][experience2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        resp.body = r.path == "/api/state" ? StateJson(kGoodSystem) : std::string("{}");
        return resp;
    });
    Experience e(srv.Address(), "");
    CHECK(e.IsConnected());
    CHECK(e._opMode == "ddp");
}

TEST_CASE("Experience2 GetJSONData parses and reports failure", "[vendors2][experience2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/ok") resp.body = R"({"a":[1,2,3]})";
        if (r.path == "/bad") resp.body = "{";
        if (r.path == "/empty") resp.body = "";
        return resp;
    });
    Experience e = MakeDisconnectedExperience();
    e._ip = srv.Address();
    nlohmann::json v;
    CHECK(e.GetJSONData("/ok", v));
    CHECK(v["a"].size() == 3);
    CHECK_FALSE(e.GetJSONData("/bad", v));
    CHECK_FALSE(e.GetJSONData("/empty", v));
}

TEST_CASE("Experience2 PostJSONToURL sends json", "[vendors2][experience2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse resp;
        resp.body = "done";
        return resp;
    });
    Experience e = MakeDisconnectedExperience();
    e._ip = srv.Address();
    CHECK(e.PostJSONToURL("/api/config", nlohmann::json{ { "a", 1 } }) == "done");
    auto reqs = srv.Requests();
    REQUIRE(reqs.size() == 1);
    CHECK(reqs[0].method == "POST");
    CHECK(reqs[0].body == "{\"a\":1}");
}

TEST_CASE("Experience2 SetInputUniverses writes universes for same sized E1.31 outputs", "[vendors2][experience2]") {
    OutputManager om;
    auto c = LoadController(om, R"(<Controller Id="1" Name="Genius A" Type="Ethernet" Protocol="E131" IP="10.1.2.3" ActiveState="Active">
        <network NetworkType="E131" ComPort="10.1.2.3" BaudRate="5" MaxChannels="510"/>
        <network NetworkType="E131" ComPort="10.1.2.3" BaudRate="6" MaxChannels="510"/>
        <network NetworkType="E131" ComPort="10.1.2.3" BaudRate="7" MaxChannels="510"/></Controller>)");
    REQUIRE(c);
    Experience e = MakeDisconnectedExperience();
    nlohmann::json data;
    int32_t sc = e.SetInputUniverses(data, c.get());
    CHECK(sc == (int32_t)c->GetOutputs().front()->GetStartChannel());
    CHECK(data["system"]["friendly_name"] == "Genius A");
    CHECK(data["system"]["auto_chain_all_outputs"] == false);
    CHECK(data["system"]["operating_mode"] == "e1.31");
    REQUIRE(data["inputs"].size() == 1);
    CHECK(data["inputs"][0]["start_universe"] == 5);
    CHECK(data["inputs"][0]["number_of_universes"] == 3);
    CHECK(data["inputs"][0]["channels_per_universe"] == 510);
    CHECK(data["inputs"][0]["start_channel"] == 1);
}

TEST_CASE("Experience2 SetInputUniverses lists universes of different sizes individually", "[vendors2][experience2]") {
    OutputManager om;
    auto c = LoadController(om, R"(<Controller Id="1" Name="G" Type="Ethernet" Protocol="ArtNet" IP="10.1.2.3" ActiveState="Active">
        <network NetworkType="ArtNet" ComPort="10.1.2.3" BaudRate="1" MaxChannels="510"/>
        <network NetworkType="ArtNet" ComPort="10.1.2.3" BaudRate="2" MaxChannels="100"/></Controller>)");
    REQUIRE(c);
    Experience e = MakeDisconnectedExperience();
    nlohmann::json data;
    static_cast<void>(e.SetInputUniverses(data, c.get()));
    CHECK(data["system"]["operating_mode"] == "artnet");
    REQUIRE(data["inputs"].size() == 2);
    CHECK(data["inputs"][0]["number_of_universes"] == 1);
    CHECK(data["inputs"][1]["channels_per_universe"] == 100);
    CHECK(data["inputs"][1]["start_channel"].get<int>() >= 1);
}

TEST_CASE("Experience2 SetInputUniverses keeps standalone and remote modes", "[vendors2][experience2]") {
    OutputManager om;
    auto c = LoadController(om, R"(<Controller Id="1" Name="G" Type="Ethernet" Protocol="E131" IP="10.1.2.3" ActiveState="Active">
        <network NetworkType="E131" ComPort="10.1.2.3" BaudRate="1" MaxChannels="510"/></Controller>)");
    REQUIRE(c);
    for (std::string mode : { "standalone", "remote" }) {
        Experience e = MakeDisconnectedExperience();
        e._opMode = mode;
        nlohmann::json data;
        static_cast<void>(e.SetInputUniverses(data, c.get()));
        CHECK(data["system"]["operating_mode"] == mode);
    }
}

TEST_CASE("Experience2 SetInputUniverses for DDP sets the start channel", "[vendors2][experience2]") {
    OutputManager om;
    auto c = LoadController(om, R"(<Controller Id="1" Name="G" Type="Ethernet" Protocol="DDP" IP="10.1.2.3" ActiveState="Active">
        <network NetworkType="DDP" ComPort="10.1.2.3" BaudRate="1" MaxChannels="1500"/></Controller>)");
    REQUIRE(c);
    Experience e = MakeDisconnectedExperience();
    nlohmann::json data;
    static_cast<void>(e.SetInputUniverses(data, c.get()));
    CHECK(data["system"]["operating_mode"] == "ddp");
    CHECK_FALSE(data.contains("inputs"));
    auto* ddp = dynamic_cast<DDPOutput*>(c->GetFirstOutput());
    REQUIRE(ddp != nullptr);
    if (ddp->IsKeepChannelNumbers()) {
        CHECK(data["system"]["start_channel"] == (int)ddp->GetStartChannel());
    } else {
        CHECK(data["system"]["start_channel"] == 1);
    }
}

TEST_CASE("Experience2 SetInputUniverses rejects non ethernet controllers", "[vendors2][experience2]") {
    OutputManager om;
    auto c = LoadController(om, R"(<Controller Name="n" Type="Null" ActiveState="Inactive"><network NetworkType="NULL" Id="3" MaxChannels="10"/></Controller>)");
    REQUIRE(c);
    Experience e = MakeDisconnectedExperience();
    nlohmann::json data;
    CHECK(e.SetInputUniverses(data, c.get()) == -1);
    CHECK(data.is_null());
}

TEST_CASE("Experience2 SetInputUniverses with an ethernet controller that has no outputs should not crash", "[vendors2][experience2]") {
    // Experience.cpp SetInputUniverses: outputs.front() is dereferenced without checking for an empty output list
    OutputManager om;
    ControllerEthernet c(&om, false);
    c.SetProtocol(OUTPUT_E131);
    c.ClearOutputs();
    Experience e = MakeDisconnectedExperience();
    nlohmann::json data;
    CHECK(e.SetInputUniverses(data, &c) == -1);
}

TEST_CASE("Experience2 UploadSequence fails for an unreadable local file", "[vendors2][experience2][http]") {
    VendorsTestInit();
    Experience e = MakeDisconnectedExperience();
    bool called = false;
    CHECK_FALSE(e.UploadSequence("no/such/file.fseq", "x.fseq", [&](int, std::string) {
        called = true;
        return true;
    }));
    (void)called;
}

#endif
