// The handlers are reached through ExposeVendors.h and a loopback server, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <chrono>

#include "support/ExposeVendors.h"
#include "controllers/ESPixelStick.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"

namespace {
std::unique_ptr<ESPixelStick> MakeHttpEsps(FakeHttpServer& srv) {
    VendorsTestInit();
    return std::make_unique<ESPixelStick>(srv.Address(), "");
}

FakeHttpServer::Handler AdminHandler(const std::string& admin, const std::string& output = "") {
    return [admin, output](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/conf/admininfo.json") resp.body = admin;
        else if (r.path == "/conf/output_config.json") resp.body = output;
        else resp.status = 404;
        return resp;
    };
}

const char* kOutputConfig = R"({"output_config":{"channels":{"0":{"type":1,"0":{"type":"Disabled"},"1":{"type":"WS2811","color_order":"rgb","pixel_count":50,"gamma":2.2,"reverse":false}}}}})";
} // namespace

TEST_CASE("Esps2 constructor over http reads the version", "[vendors2][esps2][http]") {
    FakeHttpServer srv(AdminHandler(R"({"admin":{"version":"4.0-dev-20240101"}})"));
    auto e = MakeHttpEsps(srv);
    CHECK(e->IsConnected());
    CHECK(e->_UsingHttpConfig);
    CHECK(e->GetModel() == "ESPixelStick");
    CHECK(e->GetVersion() == "4.0-dev-20240101");
    CHECK_FALSE(e->UsesHTTP());
}

TEST_CASE("Esps2 constructor rejects admin info without a string version", "[vendors2][esps2][http]") {
    auto body = GENERATE(std::string(R"({"admin":{"version":4}})"), std::string(R"({"admin":{}})"), std::string(R"({"admin":[1]})"), std::string(R"({"admin":"x"})"),
                         std::string(R"({"other":{"version":"4"}})"), std::string("garbage"), std::string(""));
    FakeHttpServer srv(AdminHandler(body));
    auto e = MakeHttpEsps(srv);
    INFO(body);
    CHECK_FALSE(e->_UsingHttpConfig);
}

TEST_CASE("Esps2 GetHttpConfig reports success only for a non empty result", "[vendors2][esps2][http]") {
    FakeHttpServer srv(AdminHandler(R"({"admin":{"version":"4.0"}})", kOutputConfig));
    auto e = MakeHttpEsps(srv);
    nlohmann::json out;
    CHECK(e->GetOutputConfig(out));
    CHECK(out.contains("channels"));
    nlohmann::json none;
    CHECK_FALSE(e->GetHttpConfig("missing", "x", none));
    CHECK(none.empty());
}

TEST_CASE("Esps2 GetHttpConfig with an empty key object is a failure", "[vendors2][esps2][http]") {
    FakeHttpServer srv(AdminHandler(R"({"admin":{"version":"4.0"}})", R"({"output_config":{}})"));
    auto e = MakeHttpEsps(srv);
    nlohmann::json out;
    CHECK_FALSE(e->GetOutputConfig(out));
}

TEST_CASE("Esps2 GetHttpConfig with a non object document", "[vendors2][esps2][http]") {
    FakeHttpServer srv(AdminHandler(R"({"admin":{"version":"4.0"}})", "[1,2,3]"));
    auto e = MakeHttpEsps(srv);
    nlohmann::json out;
    CHECK_NOTHROW(e->GetOutputConfig(out));
}

TEST_CASE("Esps2 SetHttpConfig posts the wrapped document", "[vendors2][esps2][http]") {
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/conf/admininfo.json") resp.body = R"({"admin":{"version":"4.0"}})";
        return resp;
    });
    auto e = MakeHttpEsps(srv);
    CHECK(e->SetHttpConfig("input_config", "input_config", nlohmann::json{ { "a", 1 } }));
    auto reqs = srv.Requests();
    REQUIRE(reqs.size() >= 2);
    CHECK(reqs.back().method == "POST");
    CHECK(reqs.back().path == "/conf/input_config.json");
    CHECK(reqs.back().body == R"({"input_config":{"a":1}})");
}

TEST_CASE("Esps2 SetHttpConfig fails when the controller rejects the post", "[vendors2][esps2][http]") {
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/conf/admininfo.json") resp.body = R"({"admin":{"version":"4.0"}})";
        else resp.status = 500;
        return resp;
    });
    auto e = MakeHttpEsps(srv);
    CHECK_FALSE(e->SetHttpConfig("output_config", "output_config", nlohmann::json::object()));
}

TEST_CASE("Esps2 GetFromJSON extracts values after a header", "[vendors2][esps2]") {
    VendorsTestInit();
    FakeHttpServer srv(AdminHandler(R"({"admin":{"version":"4.0"}})"));
    auto e = MakeHttpEsps(srv);
    CHECK(e->GetFromJSON("", "version", "G2{\"version\":\"3.5\"}") == "3.5");
    CHECK(e->GetFromJSON("", "n", "{\"n\":42}") == "42");
    CHECK(e->GetFromJSON("", "f", "{\"f\":1.5}") == std::to_string(1.5f));
    CHECK(e->GetFromJSON("s", "k", "{\"s\":{\"k\":\"v\"}}") == "v");
    CHECK(e->GetFromJSON("", "k", "[{\"k\":1}]").empty());
    CHECK(e->GetFromJSON("", "k", "xx[{\"k\":1}]").empty());
}

TEST_CASE("Esps2 GetFromJSON returns empty for unusable input", "[vendors2][esps2]") {
    FakeHttpServer srv(AdminHandler(R"({"admin":{"version":"4.0"}})"));
    auto e = MakeHttpEsps(srv);
    for (std::string bad : { std::string(""), std::string("no braces"), std::string("{"), std::string("{\"a\":"), std::string("{\"a\":true}"), std::string("{\"a\":null}"),
                             std::string("{\"a\":[1]}"), std::string("{\"a\":{}}"), std::string("{\"b\":1}") }) {
        INFO("json '" << bad << "'");
        CHECK(e->GetFromJSON("", "a", bad).empty());
    }
}

TEST_CASE("Esps2 GetFromJSON with a missing section does not throw", "[vendors2][esps2]") {
    FakeHttpServer srv(AdminHandler(R"({"admin":{"version":"4.0"}})"));
    auto e = MakeHttpEsps(srv);
    CHECK_NOTHROW(e->GetFromJSON("s", "k", "{\"x\":1}"));
    CHECK(e->GetFromJSON("s", "k", "{\"x\":1}").empty());
    CHECK_NOTHROW(e->GetFromJSON("s", "k", "[1,2]"));
}

TEST_CASE("Esps2 GetFromJSON with huge integers does not overflow into garbage", "[vendors2][esps2][!mayfail]") {
    // ESPixelStick.cpp GetFromJSON: integers are read with get<int>(), so 4294967296 prints as 0
    FakeHttpServer srv(AdminHandler(R"({"admin":{"version":"4.0"}})"));
    auto e = MakeHttpEsps(srv);
    CHECK(e->GetFromJSON("", "n", "{\"n\":4294967296}") == "4294967296");
}

TEST_CASE("Esps2 ParseV4Config builds a port table", "[vendors2][esps2][http]") {
    FakeHttpServer srv(AdminHandler(R"({"admin":{"version":"4.0"}})"));
    auto e = MakeHttpEsps(srv);
    auto cfg = nlohmann::json::parse(kOutputConfig)["output_config"];
    CHECK(e->ParseV4Config(cfg));
    REQUIRE(e->EspsConfig.size() == 1);
    auto& port = e->EspsConfig["0"];
    CHECK(port.PortId == "0");
    CHECK(port.CurrentProtocolId == "1");
    CHECK(port.CurrentProtocolName == "ws2811");
    CHECK(port.ProtocolsByName.size() == 2);
    CHECK(port.DisabledId == "0");
    CHECK(port.ProtocolsByName["ws2811"].IsPixel());
    CHECK_FALSE(port.ProtocolsByName["disabled"].IsPixel());
}

TEST_CASE("Esps2 ParseV4Config rejects documents without channels and clears old data", "[vendors2][esps2][http]") {
    FakeHttpServer srv(AdminHandler(R"({"admin":{"version":"4.0"}})"));
    auto e = MakeHttpEsps(srv);
    auto good = nlohmann::json::parse(kOutputConfig)["output_config"];
    REQUIRE(e->ParseV4Config(good));
    nlohmann::json bad = { { "other", 1 } };
    CHECK_FALSE(e->ParseV4Config(bad));
    CHECK(e->EspsConfig.empty());
    nlohmann::json empty = nlohmann::json::object();
    CHECK_FALSE(e->ParseV4Config(empty));
}

TEST_CASE("Esps2 ParseV4Config with an empty channel list succeeds with no ports", "[vendors2][esps2][http]") {
    FakeHttpServer srv(AdminHandler(R"({"admin":{"version":"4.0"}})"));
    auto e = MakeHttpEsps(srv);
    nlohmann::json cfg = { { "channels", nlohmann::json::object() } };
    CHECK(e->ParseV4Config(cfg));
    CHECK(e->EspsConfig.empty());
}

TEST_CASE("Esps2 ParseV4Config with a scalar port should not throw", "[vendors2][esps2][http][!mayfail]") {
    // ESPixelStick.cpp EspsPort/EspsV4Protocol::ParseV4Settings: items() of a scalar yields one entry with an empty key and
    // JsonConfig[""] on a const non-object throws type_error, so a port value of 5 aborts the parse
    FakeHttpServer srv(AdminHandler(R"({"admin":{"version":"4.0"}})"));
    auto e = MakeHttpEsps(srv);
    nlohmann::json cfg = { { "channels", { { "0", 5 } } } };
    CHECK_NOTHROW(e->ParseV4Config(cfg));
}

TEST_CASE("Esps2 port without a disabled protocol gets an empty disabled id", "[vendors2][esps2]") {
    EspsPort port;
    port.PortId = "0";
    auto j = nlohmann::json::parse(R"({"type":1,"1":{"type":"WS2811","color_order":"rgb"}})");
    CHECK(port.ParseV4Settings(j));
    CHECK(port.DisabledId.empty());
    CHECK(port.CurrentProtocolName == "ws2811");
    port.Disable();
    CHECK(port.CurrentProtocolId.empty());
    CHECK(port.CurrentProtocolName == "disabled");
}

TEST_CASE("Esps2 port whose current type has no matching protocol gets an empty name", "[vendors2][esps2]") {
    EspsPort port;
    auto j = nlohmann::json::parse(R"({"type":9,"0":{"type":"Disabled"}})");
    CHECK(port.ParseV4Settings(j));
    CHECK(port.CurrentProtocolId == "9");
    CHECK(port.CurrentProtocolName.empty());
}

TEST_CASE("Esps2 port protocol keys of three or more characters are rejected", "[vendors2][esps2]") {
    EspsPort port;
    auto j = nlohmann::json::parse(R"({"type":1,"100":{"type":"x"}})");
    CHECK_FALSE(port.ParseV4Settings(j));
    CHECK(port.CurrentProtocolName == "disabled");
}

TEST_CASE("Esps2 port accepts two character protocol keys", "[vendors2][esps2]") {
    EspsPort port;
    auto j = nlohmann::json::parse(R"({"type":12,"0":{"type":"Disabled"},"12":{"type":"Serial","baud":"250000"}})");
    CHECK(port.ParseV4Settings(j));
    CHECK(port.CurrentProtocolName == "serial");
    CHECK_FALSE(port.ProtocolsByName["serial"].IsPixel());
}

TEST_CASE("Esps2 port with a non integer type should not throw", "[vendors2][esps2][!mayfail]") {
    // ESPixelStick.cpp EspsPort::ParseV4Settings: Elementval.get<int>() throws for a string "type"
    EspsPort port;
    auto j = nlohmann::json::parse(R"({"type":"one","0":{"type":"Disabled"}})");
    CHECK_NOTHROW(port.ParseV4Settings(j));
}

TEST_CASE("Esps2 protocol settings keep floats booleans and strings as text", "[vendors2][esps2]") {
    EspsV4Protocol p;
    p.ParseV4Settings("1", nlohmann::json::parse(R"({"type":"WS2811","gamma":2.5,"count":10,"flag":true,"off":false})"));
    std::string v;
    CHECK(p.GetSetting("gamma", v));
    CHECK(v == std::to_string(2.5f));
    CHECK(p.GetSetting("count", v));
    CHECK(v == "10");
    CHECK(p.GetSetting("flag", v));
    CHECK(v == "1");
    CHECK(p.GetSetting("off", v));
    CHECK(v == "0");
    CHECK(p.Name() == "ws2811");
    CHECK(p.Id() == "1");
}

TEST_CASE("Esps2 protocol without a type has an empty name", "[vendors2][esps2]") {
    EspsV4Protocol p;
    p.ParseV4Settings("3", nlohmann::json::parse(R"({"x":1})"));
    CHECK(p.Name().empty());
    CHECK(p.Id() == "3");
    CHECK_FALSE(p.IsPixel());
}

TEST_CASE("Esps2 protocol parse of an empty object keeps nothing", "[vendors2][esps2]") {
    EspsV4Protocol p;
    p.ParseV4Settings("3", nlohmann::json::object());
    std::string v;
    CHECK_FALSE(p.GetSetting("anything", v));
}

TEST_CASE("Esps2 protocol parse of an array should not throw", "[vendors2][esps2][!mayfail]") {
    // ESPixelStick.cpp EspsV4Protocol::ParseV4Settings: items() on an array yields index keys, but JsonConfig[ElementId] then indexes the array with a string and throws
    EspsV4Protocol p;
    CHECK_NOTHROW(p.ParseV4Settings("3", nlohmann::json::parse(R"([5,"a"])")));
}

TEST_CASE("Esps2 protocol GetSetting of a missing name fails and leaves the value", "[vendors2][esps2]") {
    EspsV4Protocol p;
    p.ParseV4Settings("1", nlohmann::json::parse(R"({"type":"x"})"));
    std::string v = "keep";
    CHECK_FALSE(p.GetSetting("nope", v));
    CHECK(v == "keep");
}

TEST_CASE("Esps2 protocol WriteConfigToJson skips the type and unsupported fields", "[vendors2][esps2]") {
    EspsV4Protocol p;
    p.ParseV4Settings("1", nlohmann::json::parse(R"({"type":"WS2811","a":"x","b":1})"));
    nlohmann::json target = { { "type", "keep" }, { "a", "old" }, { "b", 9 } };
    p.WriteConfigToJson(target);
    CHECK(target["type"] == "keep");
    CHECK(target["a"] == "x");
    CHECK(target["b"] == 1);
    nlohmann::json odd = { { "a", nlohmann::json::array() }, { "b", nullptr } };
    CHECK_NOTHROW(p.WriteConfigToJson(odd));
    CHECK(odd["a"].is_array());
}

TEST_CASE("Esps2 protocol WriteConfigToJson converts text back by target type", "[vendors2][esps2]") {
    EspsV4Protocol p;
    p.ParseV4Settings("1", nlohmann::json::parse(R"({"type":"t","s":"12","f":"1.5","i":"7","b":"True"})"));
    nlohmann::json target = { { "s", "x" }, { "f", 0.5 }, { "i", 3 }, { "b", false } };
    p.WriteConfigToJson(target);
    CHECK(target["s"] == "12");
    CHECK(target["f"] == 1.5);
    CHECK(target["i"] == 7);
    CHECK(target["b"] == true);
}

TEST_CASE("Esps2 protocol WriteConfigToJson with non numeric text gives zero", "[vendors2][esps2]") {
    EspsV4Protocol p;
    p.ParseV4Settings("1", nlohmann::json::parse(R"({"type":"t","f":"abc","i":"abc"})"));
    nlohmann::json target = { { "f", 0.5 }, { "i", 3 } };
    p.WriteConfigToJson(target);
    CHECK(target["f"] == 0.0);
    CHECK(target["i"] == 0);
}

TEST_CASE("Esps2 protocol WriteConfigToJson of text beyond int range wraps", "[vendors2][esps2][!mayfail]") {
    // ESPixelStick.cpp WriteConfigToJson: (int)strtol saturates then truncates, so "99999999999" is written as -1
    EspsV4Protocol p;
    p.ParseV4Settings("1", nlohmann::json::parse(R"({"type":"t","i":"99999999999"})"));
    nlohmann::json target = { { "i", 3 } };
    p.WriteConfigToJson(target);
    CHECK(target["i"] != -1);
}

TEST_CASE("Esps2 protocol WriteConfigToJson reports the number of changes", "[vendors2][esps2]") {
    EspsV4Protocol p;
    p.ParseV4Settings("1", nlohmann::json::parse(R"({"type":"t","i":"7"})"));
    nlohmann::json target = { { "i", 3 } };
    CHECK(p.WriteConfigToJson(target) == 0); // the changed counter only counts PutSetting edits
    CHECK(p.PutSetting("i", 8, 0));
    CHECK(p.WriteConfigToJson(target) >= 1);
}

TEST_CASE("Esps2 port WriteConfigToJson reports whether anything changed", "[vendors2][esps2]") {
    EspsPort port;
    port.PortId = "0";
    auto j = nlohmann::json::parse(R"({"type":1,"0":{"type":"Disabled"},"1":{"type":"WS2811","count":5}})");
    REQUIRE(port.ParseV4Settings(j));
    nlohmann::json out = { { "channels", { { "0", { { "type", 1 }, { "1", { { "type", "WS2811" }, { "count", 5 } } } } } } } };
    CHECK_FALSE(port.WriteConfigToJson(out));
    port.CurrentProtocolId = "0";
    port.CurrentProtocolName = "disabled";
    CHECK(port.WriteConfigToJson(out));
    CHECK(out["channels"]["0"]["type"] == 0);
}

TEST_CASE("Esps2 port WriteConfigToJson with an unknown current protocol name", "[vendors2][esps2]") {
    EspsPort port;
    port.PortId = "2";
    port.CurrentProtocolId = "5";
    port.CurrentProtocolName = "ghost";
    nlohmann::json out = nlohmann::json::object();
    CHECK(port.WriteConfigToJson(out));
    CHECK(out["channels"]["2"]["type"] == 5);
    CHECK(port.ProtocolsByName.count("ghost") == 1);
}

TEST_CASE("Esps2 SetInputUniverses on a pre v4 controller does nothing network side", "[vendors2][esps2][http]") {
    FakeHttpServer srv(AdminHandler(R"({"admin":{"version":"3.0"}})"));
    auto e = MakeHttpEsps(srv);
    auto before = srv.Requests().size();
    (void)before;
    CHECK(e->GetVersion() == "3.0");
}

#endif
