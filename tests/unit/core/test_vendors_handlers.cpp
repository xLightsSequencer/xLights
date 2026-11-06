#include <catch2/catch_test_macros.hpp>

#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "controllers/AlphaPix.h"
#include "controllers/ESPixelStick.h"
#include "controllers/FPP.h"
#include "controllers/Falcon.h"
#include "controllers/SanDevices.h"

TEST_CASE("Falcon DecodeModelVersion maps product ids", "[misc][vendors][falcon]") {
    struct Row {
        int id, model, version;
    };
    const Row rows[] = {
        { 1, 16, 2 }, { 2, 16, 2 }, { 3, 16, 2 }, { 4, 4, 2 }, { 5, 16, 3 }, { 6, 4, 3 }, { 7, 48, 3 },
        { 128, 16, 4 }, { 129, 48, 4 }, { 130, 16, 5 }, { 131, 48, 5 }, { 132, 32, 5 },
    };
    for (auto const& r : rows) {
        int m = -1, v = -1;
        Falcon::DecodeModelVersion(r.id, m, v);
        INFO("id " << r.id);
        CHECK(m == r.model);
        CHECK(v == r.version);
    }
}

TEST_CASE("Falcon DecodeModelVersion defaults unknown ids to a 16 port v3", "[misc][vendors][falcon]") {
    for (int id : { 0, -1, 8, 127, 133, 255, 100000 }) {
        int m = 0, v = 0;
        Falcon::DecodeModelVersion(id, m, v);
        INFO("id " << id);
        CHECK(m == 16);
        CHECK(v == 3);
    }
}

TEST_CASE("Falcon DecodeMode names the operating modes", "[misc][vendors][falcon]") {
    CHECK(Falcon::DecodeMode(0) == "E131/ArtNET");
    CHECK(Falcon::DecodeMode(2) == "Player");
    CHECK(Falcon::DecodeMode(4) == "Remote");
    CHECK(Falcon::DecodeMode(8) == "Master");
    CHECK(Falcon::DecodeMode(16) == "ZCPP");
    CHECK(Falcon::DecodeMode(64) == "DDP");
    for (int bad : { 1, 3, 5, 32, -1, 65 }) {
        CHECK(Falcon::DecodeMode(bad).empty());
    }
}

TEST_CASE("FPP DecodeReceiverType by name", "[misc][vendors][fpp]") {
    CHECK(FPP::DecodeReceiverType(std::string("Falcon v1"), true, true) == ReceiverType::v1);
    CHECK(FPP::DecodeReceiverType(std::string("receiver-v2"), false, false) == ReceiverType::v2);
    CHECK(FPP::DecodeReceiverType(std::string("v4"), true, true) == ReceiverType::FalconV4);
    CHECK(FPP::DecodeReceiverType(std::string("v4"), true, false) == ReceiverType::v2);
    CHECK(FPP::DecodeReceiverType(std::string("v5"), true, true) == ReceiverType::FalconV5);
    CHECK(FPP::DecodeReceiverType(std::string("v5"), false, true) == ReceiverType::FalconV4);
    CHECK(FPP::DecodeReceiverType(std::string("v5"), false, false) == ReceiverType::v2);
    CHECK(FPP::DecodeReceiverType(std::string(""), true, true) == ReceiverType::Standard);
    CHECK(FPP::DecodeReceiverType(std::string("standard"), true, true) == ReceiverType::Standard);
    CHECK(FPP::DecodeReceiverType(std::string("v9"), true, true) == ReceiverType::Standard);
}

TEST_CASE("FPP DecodeReceiverType name checks v1 before v2 before v4", "[misc][vendors][fpp]") {
    CHECK(FPP::DecodeReceiverType(std::string("v1v2"), true, true) == ReceiverType::v1);
    CHECK(FPP::DecodeReceiverType(std::string("v2v4"), true, true) == ReceiverType::v2);
    CHECK(FPP::DecodeReceiverType(std::string("v4v5"), true, true) == ReceiverType::FalconV4);
}

TEST_CASE("FPP DecodeReceiverType by number", "[misc][vendors][fpp]") {
    CHECK(FPP::DecodeReceiverType(0, true, true) == ReceiverType::Standard);
    CHECK(FPP::DecodeReceiverType(-3, true, true) == ReceiverType::Standard);
    CHECK(FPP::DecodeReceiverType(1, true, true) == ReceiverType::v1);
    CHECK(FPP::DecodeReceiverType(3, true, true) == ReceiverType::v1);
    CHECK(FPP::DecodeReceiverType(4, true, true) == ReceiverType::v2);
    CHECK(FPP::DecodeReceiverType(9, true, true) == ReceiverType::v2);
    CHECK(FPP::DecodeReceiverType(10, true, true) == ReceiverType::FalconV5);
    CHECK(FPP::DecodeReceiverType(10, false, true) == ReceiverType::v2);
    CHECK(FPP::DecodeReceiverType(15, true, true) == ReceiverType::FalconV5);
    CHECK(FPP::DecodeReceiverType(16, true, true) == ReceiverType::FalconV4);
    CHECK(FPP::DecodeReceiverType(16, true, false) == ReceiverType::v2);
    CHECK(FPP::DecodeReceiverType(1000000, false, true) == ReceiverType::FalconV4);
}

TEST_CASE("FPP TypeIDtoControllerType classifies type ids", "[misc][vendors][fpp]") {
    struct Row {
        int id;
        FPP_TYPE type;
    };
    const Row rows[] = {
        { 1, FPP_TYPE::FPP }, { 0x7F, FPP_TYPE::FPP }, { 0x88, FPP_TYPE::FALCONV4V5 }, { 0x9F, FPP_TYPE::FALCONV4V5 },
        { 0xA0, FPP_TYPE::GENIUS }, { 0xAF, FPP_TYPE::GENIUS }, { 0xC2, FPP_TYPE::ESPIXELSTICK }, { 0xC3, FPP_TYPE::ESPIXELSTICK },
        { 0xD0, FPP_TYPE::POWERDMX }, { 0xDF, FPP_TYPE::POWERDMX },
    };
    for (auto const& r : rows) {
        FPP inst;
        FPP::TypeIDtoControllerType(r.id, &inst);
        INFO("id " << r.id);
        CHECK(inst.fppType == r.type);
    }
}

TEST_CASE("FPP TypeIDtoControllerType leaves the type alone for unknown ids", "[misc][vendors][fpp]") {
    for (int id : { 0, -1, 0x80, 0x87, 0xB0, 0xC0, 0xC4, 0xE0, 0xFFFF }) {
        FPP inst;
        inst.fppType = FPP_TYPE::GENIUS;
        FPP::TypeIDtoControllerType(id, &inst);
        INFO("id " << id);
        CHECK(inst.fppType == FPP_TYPE::GENIUS);
    }
}

TEST_CASE("FPP IsVersionAtLeast compares major minor patch in order", "[misc][vendors][fpp]") {
    FPP f;
    f.majorVersion = 7;
    f.minorVersion = 2;
    f.patchVersion = 5;
    CHECK(f.IsVersionAtLeast(7, 2, 5));
    CHECK(f.IsVersionAtLeast(7, 2, 4));
    CHECK_FALSE(f.IsVersionAtLeast(7, 2, 6));
    CHECK(f.IsVersionAtLeast(7, 1, 99));
    CHECK_FALSE(f.IsVersionAtLeast(7, 3, 0));
    CHECK(f.IsVersionAtLeast(6, 99, 99));
    CHECK_FALSE(f.IsVersionAtLeast(8, 0, 0));
    CHECK(f.IsVersionAtLeast(0, 0));
}

TEST_CASE("FPP supportedForFPPConnect needs 7.1 or a verified 6.3.3", "[misc][vendors][fpp]") {
    FPP a;
    a.majorVersion = 7;
    a.minorVersion = 1;
    CHECK(a.supportedForFPPConnect());
    FPP b;
    b.majorVersion = 6;
    b.minorVersion = 9;
    CHECK_FALSE(b.supportedForFPPConnect());
    FPP c;
    c.majorVersion = 5;
    CHECK_FALSE(c.supportedForFPPConnect());
    FPP d;
    CHECK_FALSE(d.supportedForFPPConnect());
}

TEST_CASE("FPP FillRanges parses ranges and singletons", "[misc][vendors][fpp]") {
    FPP f;
    f.ranges = "1-100,200-299,500";
    std::map<int, int> r;
    f.FillRanges(r);
    REQUIRE(r.size() == 3);
    CHECK(r[1] == 100);
    CHECK(r[200] == 100);
    CHECK(r[500] == 4);
}

TEST_CASE("FPP FillRanges of nothing gives nothing", "[misc][vendors][fpp]") {
    FPP f;
    std::map<int, int> r;
    f.FillRanges(r);
    CHECK(r.empty());
}

TEST_CASE("FPP SetNewRanges writes a compact sorted list", "[misc][vendors][fpp]") {
    FPP f;
    f.SetNewRanges({});
    CHECK(f.ranges.empty());
    f.SetNewRanges({ { 1, 100 } });
    CHECK(f.ranges == "1-100");
    f.SetNewRanges({ { 1, 100 }, { 101, 50 } });
    CHECK(f.ranges == "1-150");
    f.SetNewRanges({ { 1, 10 }, { 50, 10 }, { 200, 5 } });
    CHECK(f.ranges == "1-10,50-59,200-204");
}

TEST_CASE("FPP SetNewRanges merges overlapping ranges", "[misc][vendors][fpp]") {
    FPP f;
    f.SetNewRanges({ { 10, 91 }, { 50, 10 } });
    CHECK(f.ranges == "10-100");
    f.SetNewRanges({ { 10, 91 }, { 90, 60 } });
    CHECK(f.ranges == "10-149");
}

TEST_CASE("FPP ranges round trip through Fill and Set", "[misc][vendors][fpp]") {
    FPP f;
    f.ranges = "1-512,1025-1536,3000-3002";
    std::map<int, int> r;
    f.FillRanges(r);
    f.SetNewRanges(r);
    CHECK(f.ranges == "1-512,1025-1536,3000-3002");
}

// The "start is inside the previous range" test is s < first + last - 1, which also fires for a gap
// that lies below that sum, so two separate ranges are joined into one.
TEST_CASE("FPP SetNewRanges keeps a real gap between ranges", "[misc][vendors][fpp]") {
    FPP f;
    f.SetNewRanges({ { 10, 91 }, { 105, 6 } });
    CHECK(f.ranges == "10-100,105-110");
}

TEST_CASE("FPP GetVendor and GetModel resolve legacy type names", "[misc][vendors][fpp]") {
    CHECK(FPP::GetVendor("Totally Unknown").empty());
    CHECK(FPP::GetModel("Totally Unknown").empty());
}

TEST_CASE("FPP default instance is connected-less and empty", "[misc][vendors][fpp]") {
    FPP f;
    CHECK(f.hostName.empty());
    CHECK(f.ipAddress.empty());
    CHECK(f.majorVersion == 0);
    CHECK(f.fppType == FPP_TYPE::FPP);
    CHECK(f.GetFullName().empty());
}

TEST_CASE("AlphaPix writeFunction appends curl data to a string", "[misc][vendors][alphapix]") {
    std::string out;
    const char a[] = "hello ";
    const char b[] = "world";
    CHECK(AlphaPix::writeFunction((void*)a, 1, 6, &out) == 6);
    CHECK(AlphaPix::writeFunction((void*)b, 5, 1, &out) == 5);
    CHECK(out == "hello world");
    CHECK(AlphaPix::writeFunction((void*)b, 1, 0, &out) == 0);
    CHECK(out == "hello world");
    CHECK(AlphaPix::writeFunction((void*)b, 1, 5, nullptr) == 0);
}

TEST_CASE("SanDevicesProtocol flags an upload only when a value changes", "[misc][vendors][sandevices]") {
    SanDevicesProtocol p(2, 'A');
    CHECK(p.getGroup() == 2);
    CHECK(p.getProtocol() == 'A');
    CHECK(p.getTiming() == 'A');
    CHECK_FALSE(p.shouldUpload());
    p.setProtocol('A');
    p.setTiming('A');
    CHECK_FALSE(p.shouldUpload());
    p.setTiming('C');
    CHECK(p.shouldUpload());
    CHECK(p.getTiming() == 'C');

    SanDevicesProtocol q(1, 'B', 'D');
    CHECK(q.getTiming() == 'D');
    q.setProtocol('E');
    CHECK(q.shouldUpload());
}

TEST_CASE("SanDevices output records start with their documented defaults", "[misc][vendors][sandevices]") {
    SanDevicesOutput o(1, 2, 3);
    CHECK(o.group == 1);
    CHECK(o.output == 2);
    CHECK(o.stringport == 3);
    CHECK(o.universe == 'A');
    CHECK(o.startChannel == 1);
    CHECK(o.colorOrder == 'A');
    CHECK_FALSE(o.reverse);
    CHECK_FALSE(o.upload);
    CHECK_FALSE(o.serial);
    SanDevicesOutputV4 v(5);
    CHECK(v.group == 5);
    CHECK(v.protocol == 'D');
    CHECK(v.nullPixel[3] == 0);
    CHECK_FALSE(v.reverse[2]);
    CHECK(v.colorOrder == 'A');
}

namespace {

nlohmann::json MakeEspsPort() {
    return nlohmann::json::parse(R"({
        "0": {"type": "disabled"},
        "1": {"type": "ws2811", "color_order": "rgb", "pixel_count": 50, "gamma_value": 2.2, "brightness": 100, "reverse": false},
        "type": 1
    })");
}

} // namespace

TEST_CASE("EspsV4Protocol parses typed settings into strings", "[misc][vendors][esps]") {
    auto j = nlohmann::json::parse(R"({"type":"WS2811","color_order":"rgb","pixel_count":50,"gamma_value":2.5,"reverse":true})");
    EspsV4Protocol p;
    p.ParseV4Settings("1", j);
    CHECK(p.Id() == "1");
    CHECK(p.Name() == "ws2811");
    CHECK(p.IsPixel());
    std::string v;
    CHECK(p.GetSetting("color_order", v));
    CHECK(v == "rgb");
    CHECK(p.GetSetting("pixel_count", v));
    CHECK(v == "50");
    CHECK(p.GetSetting("reverse", v));
    CHECK(v == "1");
    CHECK(p.GetSetting("gamma_value", v));
    CHECK(v.rfind("2.5", 0) == 0);
    CHECK_FALSE(p.GetSetting("nope", v));
}

TEST_CASE("EspsV4Protocol without color_order is not a pixel protocol", "[misc][vendors][esps]") {
    EspsV4Protocol p;
    p.ParseV4Settings("2", nlohmann::json::parse(R"({"type":"DMX","channel_count":512})"));
    CHECK_FALSE(p.IsPixel());
    CHECK(p.Name() == "dmx");
}

TEST_CASE("EspsV4Protocol skips unsupported json value types", "[misc][vendors][esps]") {
    EspsV4Protocol p;
    p.ParseV4Settings("3", nlohmann::json::parse(R"({"type":"x","arr":[1,2],"obj":{"a":1},"nul":null,"ok":"yes"})"));
    std::string v;
    CHECK_FALSE(p.GetSetting("arr", v));
    CHECK_FALSE(p.GetSetting("obj", v));
    CHECK_FALSE(p.GetSetting("nul", v));
    CHECK(p.GetSetting("ok", v));
}

TEST_CASE("EspsV4Protocol PutSetting only counts real changes", "[misc][vendors][esps]") {
    EspsV4Protocol p;
    p.ParseV4Settings("1", nlohmann::json::parse(R"({"type":"ws2811","color_order":"rgb","pixel_count":50,"gamma_value":2.0})"));
    nlohmann::json out = nlohmann::json::parse(R"({"type":"ws2811","color_order":"rgb","pixel_count":50,"gamma_value":2.0})");
    CHECK(p.PutSetting("color_order", std::string("rgb"), "grb"));
    CHECK(p.PutSetting("pixel_count", 50, 0));
    CHECK(p.WriteConfigToJson(out) == 0);
    CHECK(p.PutSetting("color_order", std::string("grb"), "rgb"));
    CHECK(p.PutSetting("pixel_count", 60, 0));
    CHECK(p.WriteConfigToJson(out) == 2);
    CHECK(out["color_order"] == "grb");
    CHECK(out["pixel_count"] == 60);
    CHECK(out["type"] == "ws2811");
}

TEST_CASE("EspsV4Protocol empty values keep the controller setting unless in full control", "[misc][vendors][esps]") {
    auto j = nlohmann::json::parse(R"({"type":"ws2811","color_order":"rgb","pixel_count":50,"gamma_value":2.0})");
    {
        EspsV4Protocol p;
        p.ParseV4Settings("1", j);
        CHECK_FALSE(p.PutSetting("color_order", std::string(""), "grb"));
        CHECK_FALSE(p.PutSetting("pixel_count", -1, 170));
        CHECK_FALSE(p.PutSetting("gamma_value", -1.0f, 2.2f));
        std::string v;
        p.GetSetting("color_order", v);
        CHECK(v == "rgb");
        p.GetSetting("pixel_count", v);
        CHECK(v == "50");
    }
    {
        EspsV4Protocol p;
        p.ParseV4Settings("1", j);
        p.SetIsFullxLightsControl(true);
        CHECK(p.PutSetting("color_order", std::string(""), "grb"));
        CHECK(p.PutSetting("pixel_count", -1, 170));
        CHECK(p.PutSetting("gamma_value", -1.0f, 2.2f));
        std::string v;
        p.GetSetting("color_order", v);
        CHECK(v == "grb");
        p.GetSetting("pixel_count", v);
        CHECK(v == "170");
    }
}

TEST_CASE("EspsV4Protocol PutSetting of an unknown setting fails", "[misc][vendors][esps]") {
    EspsV4Protocol p;
    p.ParseV4Settings("1", nlohmann::json::parse(R"({"type":"ws2811"})"));
    CHECK_FALSE(p.PutSetting("missing", std::string("a"), ""));
    CHECK_FALSE(p.PutSetting("missing", 1, 0));
    CHECK_FALSE(p.PutSetting("missing", 1.0f, 0.0f));
}

TEST_CASE("EspsV4Protocol write keeps each field's json type", "[misc][vendors][esps]") {
    EspsV4Protocol p;
    p.ParseV4Settings("1", nlohmann::json::parse(R"({"type":"ws2811","s":"a","i":1,"f":1.5,"b":false})"));
    REQUIRE(p.PutSetting("s", std::string("z"), ""));
    REQUIRE(p.PutSetting("i", 7, 0));
    REQUIRE(p.PutSetting("f", 2.25f, 0.0f));
    REQUIRE(p.PutSetting("b", std::string("true"), ""));
    nlohmann::json out = nlohmann::json::parse(R"({"type":"ws2811","s":"a","i":1,"f":1.5,"b":false})");
    p.WriteConfigToJson(out);
    CHECK(out["s"].is_string());
    CHECK(out["s"] == "z");
    CHECK(out["i"].is_number_integer());
    CHECK(out["i"] == 7);
    CHECK(out["f"].is_number_float());
    CHECK(out["f"].get<double>() > 2.2);
    CHECK(out["b"].is_boolean());
    CHECK(out["b"] == true);
}

TEST_CASE("EspsPort parses protocols and the current selection", "[misc][vendors][esps]") {
    EspsPort port;
    port.PortId = "0";
    REQUIRE(port.ParseV4Settings(MakeEspsPort()));
    CHECK(port.CurrentProtocolId == "1");
    CHECK(port.CurrentProtocolName == "ws2811");
    CHECK(port.DisabledId == "0");
    CHECK(port.DisabledName == "disabled");
    CHECK(port.ProtocolsByName.count("ws2811") == 1);
    CHECK(port.ProtocolsByName.count("disabled") == 1);
    CHECK(port.ProtocolIdToProtocolName["1"] == "ws2811");
}

TEST_CASE("EspsPort Disable switches to the disabled protocol", "[misc][vendors][esps]") {
    EspsPort port;
    REQUIRE(port.ParseV4Settings(MakeEspsPort()));
    port.Disable();
    CHECK(port.CurrentProtocolId == "0");
    CHECK(port.CurrentProtocolName == "disabled");
}

TEST_CASE("EspsPort rejects unexpected members and falls back to disabled", "[misc][vendors][esps]") {
    EspsPort port;
    auto j = MakeEspsPort();
    j["bogus_member"] = 5;
    CHECK_FALSE(port.ParseV4Settings(j));
    CHECK(port.CurrentProtocolId == port.DisabledId);
    CHECK(port.CurrentProtocolName == "disabled");
}

TEST_CASE("EspsPort WriteConfigToJson switches the type and updates the protocol", "[misc][vendors][esps]") {
    EspsPort port;
    port.PortId = "0";
    auto portJson = MakeEspsPort();
    REQUIRE(port.ParseV4Settings(portJson));
    nlohmann::json config;
    config["channels"]["0"] = portJson;
    CHECK_FALSE(port.WriteConfigToJson(config));

    port.CurrentProtocolId = "0";
    port.CurrentProtocolName = "disabled";
    CHECK(port.WriteConfigToJson(config));
    CHECK(config["channels"]["0"]["type"] == 0);

    port.ProtocolsByName["ws2811"].PutSetting("pixel_count", 99, 0);
    port.CurrentProtocolId = "1";
    port.CurrentProtocolName = "ws2811";
    CHECK(port.WriteConfigToJson(config));
    CHECK(config["channels"]["0"]["type"] == 1);
    CHECK(config["channels"]["0"]["1"]["pixel_count"] == 99);
}

// A port without a numeric "type" makes WriteConfigToJson throw a json type_error nobody catches.
TEST_CASE("EspsPort WriteConfigToJson tolerates a port with no type", "[misc][vendors][esps]") {
    EspsPort port;
    port.PortId = "0";
    REQUIRE(port.ParseV4Settings(MakeEspsPort()));
    nlohmann::json config = nlohmann::json::parse(R"({"channels":{"0":{}}})");
    CHECK_NOTHROW(port.WriteConfigToJson(config));
}
