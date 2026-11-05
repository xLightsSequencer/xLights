// The handlers are reached through ExposeVendors.h and a loopback server, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "support/ExposeVendors.h"
#include "controllers/FPP.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"
#include "support/VendorCapsFixture.h"

namespace {
std::unique_ptr<FPP> MakeFpp(const std::string& address = "127.0.0.1:1") {
    VendorsTestInit();
    auto f = std::make_unique<FPP>(address, "", "");
    f->ipAddress = address;
    return f;
}
} // namespace

TEST_CASE("FPP2 parseSysInfo reads the common fields", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    auto j = nlohmann::json::parse(R"({"Platform":"Raspberry Pi","Variant":"Pi 4","Version":"7.1-3","HostName":"fpp1","HostDescription":"Roof","Mode":"player","multisync":true})");
    CHECK(f->parseSysInfo(j));
    CHECK(f->platform == "Raspberry Pi");
    CHECK(f->model == "Pi 4");
    CHECK(f->fullVersion == "7.1-3");
    CHECK(f->hostName == "fpp1");
    CHECK(f->description == "Roof");
    CHECK(f->mode == "player w/multisync");
    CHECK(f->majorVersion == 7);
    CHECK(f->minorVersion == 1);
    CHECK(f->patchVersion == 3);
}

TEST_CASE("FPP2 parseSysInfo version formats", "[vendors2][fpp2]") {
    auto row = GENERATE(table<std::string, std::vector<uint32_t>>({ { "7.1", { 7, 1, 0 } }, { "7.1.2", { 7, 1, 2 } }, { "6.3-4", { 6, 3, 4 } }, { "8.0", { 8, 0, 0 } },
                                                                     { "6.x-master", { 6, 1000, 0 } }, { "7.1-alpha", { 7, 1, 0 } },
                                                                     { "9.9.9", { 9, 9, 9 } } }));
    auto f = MakeFpp();
    nlohmann::json j = { { "Version", std::get<0>(row) } };
    CHECK(f->parseSysInfo(j));
    INFO(std::get<0>(row));
    CHECK(f->majorVersion == std::get<1>(row)[0]);
    CHECK(f->minorVersion == std::get<1>(row)[1]);
    CHECK(f->patchVersion == std::get<1>(row)[2]);
}

TEST_CASE("FPP2 parseSysInfo with an empty or missing version leaves the numbers alone", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    f->majorVersion = 3;
    nlohmann::json j = nlohmann::json::object();
    CHECK(f->parseSysInfo(j));
    CHECK(f->majorVersion == 3);
    nlohmann::json e = { { "Version", "" } };
    CHECK(f->parseSysInfo(e));
    CHECK(f->majorVersion == 3);
}

TEST_CASE("FPP2 parseSysInfo explicit major and minor override the parsed version", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    nlohmann::json j = { { "Version", "7.1" }, { "majorVersion", 9 }, { "minorVersion", 4 } };
    f->parseSysInfo(j);
    CHECK(f->majorVersion == 9);
    CHECK(f->minorVersion == 4);
}

TEST_CASE("FPP2 parseSysInfo ignores mistyped members", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    nlohmann::json j = { { "Platform", 5 }, { "Mode", nullptr }, { "multisync", "yes" }, { "majorVersion", "7" }, { "minorVersion", 1.5 } };
    CHECK(f->parseSysInfo(j));
    CHECK(f->platform.empty());
    CHECK(f->mode.empty());
    CHECK(f->majorVersion == 0);
    CHECK(f->minorVersion == 0);
}

TEST_CASE("FPP2 parseSysInfo only adds multisync for the player mode", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    nlohmann::json j = { { "Mode", "bridge" }, { "multisync", true } };
    f->parseSysInfo(j);
    CHECK(f->mode == "bridge");
    nlohmann::json p = { { "Mode", "player" }, { "multisync", false } };
    f->parseSysInfo(p);
    CHECK(f->mode == "player");
}

TEST_CASE("FPP2 parseSysInfo keeps the longest channel range list", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    nlohmann::json a = { { "channelRanges", "1-100" } };
    f->parseSysInfo(a);
    CHECK(f->ranges == "1-100");
    nlohmann::json b = { { "channelRanges", "1-5" } };
    f->parseSysInfo(b);
    CHECK(f->ranges == "1-100");
    nlohmann::json c = { { "channelRanges", "1-100,200-300" } };
    f->parseSysInfo(c);
    CHECK(f->ranges == "1-100,200-300");
}

TEST_CASE("FPP2 parseSysInfo with a two digit major version", "[vendors2][fpp2][!mayfail]") {
    // FPP.cpp parseSysInfo: the minor version is read from fullVersion.substr(2), which assumes a one digit major, so "10.25" gives minor 0
    auto f = MakeFpp();
    nlohmann::json j = { { "Version", "10.25" } };
    f->parseSysInfo(j);
    CHECK(f->majorVersion == 10);
    CHECK(f->minorVersion == 25);
}

TEST_CASE("FPP2 parseSysInfo with a one character version should not throw", "[vendors2][fpp2][.][known-crash]") {
    // FPP.cpp parseSysInfo: fullVersion[2] reads past the end of a one character string (undefined behaviour, valgrind flags it) and substr(2)
    // then throws out_of_range; "7.x" throws at substr(4)
    auto f = MakeFpp();
    nlohmann::json j = { { "Version", "7" } };
    CHECK_NOTHROW(f->parseSysInfo(j));
    nlohmann::json x = { { "Version", "7.x" } };
    CHECK_NOTHROW(f->parseSysInfo(x));
}

TEST_CASE("FPP2 parseControllerType selects the pixel output sub type", "[vendors2][fpp2]") {
    auto row = GENERATE(as<std::string>{}, "RPIWS281X", "BBB48String", "BBShiftString", "DPIPixels");
    auto f = MakeFpp();
    nlohmann::json j = { { "channelOutputs", { { { "enabled", 1 }, { "type", row }, { "subType", "Pi Hat" } } } } };
    f->parseControllerType(j);
    CHECK(f->pixelControllerType == "Pi Hat");
}

TEST_CASE("FPP2 parseControllerType ignores disabled outputs and unknown types", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    f->pixelControllerType = "keep";
    nlohmann::json j = { { "channelOutputs", { { { "enabled", 0 }, { "type", "RPIWS281X" }, { "subType", "x" } }, { { "enabled", 1 }, { "type", "Other" }, { "subType", "y" } } } } };
    f->parseControllerType(j);
    CHECK(f->pixelControllerType == "keep");
}

TEST_CASE("FPP2 parseControllerType handles virtual matrices", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    nlohmann::json j = { { "channelOutputs", { { { "enabled", 1 }, { "type", "VirtualMatrix" } } } } };
    f->parseControllerType(j);
    CHECK(f->pixelControllerType == "Virtual Matrix");
}

TEST_CASE("FPP2 parseControllerType computes the LED panel size", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    nlohmann::json j = { { "channelOutputs", { { { "enabled", 1 }, { "type", "LEDPanelMatrix" }, { "panelWidth", 32 }, { "panelHeight", 16 },
                                                   { "panels", { { { "row", 0 }, { "col", 0 } }, { { "row", 1 }, { "col", 2 } } } } } } } };
    f->parseControllerType(j);
    CHECK(f->pixelControllerType == "LED Panels");
    CHECK(f->panelSize == "96x32");
}

TEST_CASE("FPP2 parseControllerType swaps width and height for rotated panels", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    nlohmann::json j = { { "channelOutputs", { { { "enabled", 1 }, { "type", "LEDPanelMatrix" }, { "panelWidth", 32 }, { "panelHeight", 16 },
                                                   { "panels", { { { "row", 0 }, { "col", 0 }, { "orientation", "E" } } } } } } } };
    f->parseControllerType(j);
    CHECK(f->panelSize == "16x32");
}

TEST_CASE("FPP2 parseControllerType with no panels is a single panel", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    nlohmann::json j = { { "channelOutputs", { { { "enabled", 1 }, { "type", "LEDPanelMatrix" }, { "panelWidth", 64 }, { "panelHeight", 32 } } } } };
    f->parseControllerType(j);
    CHECK(f->panelSize == "64x32");
}

TEST_CASE("FPP2 parseControllerType copes with missing or mistyped documents", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    nlohmann::json empty = nlohmann::json::object();
    CHECK_NOTHROW(f->parseControllerType(empty));
    nlohmann::json nulls = { { "channelOutputs", { nullptr, 5 } } };
    CHECK_NOTHROW(f->parseControllerType(nulls));
    CHECK(f->pixelControllerType.empty());
}

TEST_CASE("FPP2 parseControllerType with a string channelOutputs should not throw", "[vendors2][fpp2][!mayfail]") {
    // FPP.cpp parseControllerType: a scalar has size() 1 so val["channelOutputs"][0] is attempted on a string and throws type_error
    auto f = MakeFpp();
    nlohmann::json bad = { { "channelOutputs", "none" } };
    CHECK_NOTHROW(f->parseControllerType(bad));
}

TEST_CASE("FPP2 parseControllerType with a null document should not throw", "[vendors2][fpp2][!mayfail]") {
    // FPP.cpp parseControllerType: val["channelOutputs"] on a json array or scalar throws type_error; callers pass whatever the controller returned
    auto f = MakeFpp();
    nlohmann::json arr = nlohmann::json::array();
    CHECK_NOTHROW(f->parseControllerType(arr));
}

TEST_CASE("FPP2 parseProxies collects host names", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    nlohmann::json j = nlohmann::json::array({ "10.0.0.2", "10.0.0.3", "10.0.0.2" });
    f->parseProxies(j);
    CHECK(f->proxies.size() == 2);
    CHECK(f->proxies.count("10.0.0.3") == 1);
    nlohmann::json none = nlohmann::json::array();
    f->parseProxies(none);
    CHECK(f->proxies.size() == 2);
}

TEST_CASE("FPP2 parseProxies with FPP 8 objects should not throw", "[vendors2][fpp2][!mayfail]") {
    // FPP.cpp parseProxies: val[x].get<std::string>() throws for the FPP 8 object form ({"host":..}) that GetProxies already handles
    auto f = MakeFpp();
    nlohmann::json j = nlohmann::json::array({ nlohmann::json{ { "host", "10.0.0.2" } } });
    CHECK_NOTHROW(f->parseProxies(j));
}

TEST_CASE("FPP2 parseConfig recognises the falcon player title", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    f->fppType = FPP_TYPE::GENIUS;
    f->parseConfig("var x;\n  settings['Title'] = \"Falcon Player - FPP\";\n");
    CHECK(f->fppType == FPP_TYPE::FPP);
}

TEST_CASE("FPP2 parseConfig leaves the type for other titles", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    f->fppType = FPP_TYPE::GENIUS;
    f->parseConfig("settings['Title'] = \"Something Else\";\n");
    CHECK(f->fppType == FPP_TYPE::GENIUS);
    f->parseConfig("");
    CHECK(f->fppType == FPP_TYPE::GENIUS);
    f->parseConfig("no settings here\nat all");
    CHECK(f->fppType == FPP_TYPE::GENIUS);
}

TEST_CASE("FPP2 parseConfig with a truncated settings line should not throw", "[vendors2][fpp2][!mayfail]") {
    // FPP.cpp parseConfig: to.substr(10) throws std::out_of_range for a line that starts with "settings" but is shorter than 10 characters
    auto f = MakeFpp();
    CHECK_NOTHROW(f->parseConfig("settings\n"));
    CHECK_NOTHROW(f->parseConfig("settings["));
}

TEST_CASE("FPP2 FillRanges tolerates odd range text", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    std::map<int, int> r;
    f->ranges = "1-10,20,30-35";
    f->FillRanges(r);
    CHECK(r[1] == 10);
    CHECK(r[20] == 4);
    CHECK(r[30] == 6);
    r.clear();
    f->ranges = "abc";
    f->FillRanges(r);
    CHECK(r[0] == 4);
    r.clear();
    f->ranges = "5-";
    f->FillRanges(r);
    CHECK(r.size() == 1);
}

TEST_CASE("FPP2 FillRanges with empty list items should not misbehave", "[vendors2][fpp2][!mayfail]") {
    // FPP.cpp FillRanges: Split() of an empty item can yield an empty vector and r[0] is then read out of bounds
    auto f = MakeFpp();
    std::map<int, int> r;
    f->ranges = "1-5,,7";
    CHECK_NOTHROW(f->FillRanges(r));
    CHECK(r.size() == 2);
}

TEST_CASE("FPP2 SetNewRanges handles adjacent and single channel entries", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    f->SetNewRanges({ { 1, 10 }, { 11, 5 }, { 100, 1 } });
    CHECK(f->ranges == "1-15,100-100");
    f->SetNewRanges({});
    CHECK(f->ranges.empty());
    f->SetNewRanges({ { 1, 100 }, { 10, 5 } });
    CHECK(f->ranges == "1-100");
    f->SetNewRanges({ { 1, 100 }, { 50, 100 } });
    CHECK(f->ranges == "1-149");
}

TEST_CASE("FPP2 SetNewRanges with a negative length should not produce an inverted range", "[vendors2][fpp2][!mayfail]") {
    // FPP.cpp FillRanges/SetNewRanges: "10-5" gives length -4 and the next SetNewRanges prints "10-5"
    auto f = MakeFpp();
    f->ranges = "10-5";
    std::map<int, int> r;
    f->FillRanges(r);
    f->SetNewRanges(r);
    CHECK(f->ranges != "10-5");
}

TEST_CASE("FPP2 SetNewRanges near INT_MAX does not wrap", "[vendors2][fpp2][!mayfail]") {
    // FPP.cpp SetNewRanges: s + l - 1 is a signed int addition, so a range ending at INT_MAX overflows
    auto f = MakeFpp();
    f->SetNewRanges({ { 2147483000, 2000 } });
    CHECK(f->ranges.find('-') != std::string::npos);
    CHECK(std::strtoll(f->ranges.substr(f->ranges.find('-') + 1).c_str(), nullptr, 10) > 2147483000LL);
}

TEST_CASE("FPP2 GetExpansionPorts reads the cape port table", "[vendors2][fpp2]") {
    VendorCapsFixture fx(R"(<Vendor Name="V"><Controller Name="M"><Variant Name="" ID="v-m"><fpp>3</fpp><fpp1>1,16</fpp1><fpp2>0,0</fpp2><fpp3>2,8</fpp3></Variant></Controller></Vendor>)");
    auto* caps = ControllerCaps::GetControllerConfig("V", "M", "");
    REQUIRE(caps != nullptr);
    auto f = MakeFpp();
    auto ports = f->GetExpansionPorts(caps);
    CHECK(ports.size() == 2);
    CHECK(ports[1] == 16);
    CHECK(ports[2] == 8);
    CHECK(ports.count(3) == 0);
}

TEST_CASE("FPP2 GetExpansionPorts without a table is empty", "[vendors2][fpp2]") {
    VendorCapsFixture fx(R"(<Vendor Name="V"><Controller Name="M"><Variant Name="" ID="v-m"><fpp>2</fpp><fpp1>bogus</fpp1></Variant></Controller></Vendor>)");
    auto* caps = ControllerCaps::GetControllerConfig("V", "M", "");
    auto f = MakeFpp();
    auto ports = f->GetExpansionPorts(caps);
    CHECK(ports.empty());
}

TEST_CASE("FPP2 MakeFullURL builds direct proxied and espixelstick urls", "[vendors2][fpp2]") {
    auto f = MakeFpp("10.0.0.5");
    std::string ip;
    CHECK(f->MakeFullURL("/api/x", ip) == "http://10.0.0.5/api/x");
    CHECK(ip == "10.0.0.5");
    f->_fppProxy = "10.0.0.9";
    CHECK(f->MakeFullURL("/api/x", ip) == "http://10.0.0.9/proxy/10.0.0.5/api/x");
    CHECK(ip == "10.0.0.9");
    f->_fppProxy = "";
    f->fppType = FPP_TYPE::ESPIXELSTICK;
    CHECK(f->MakeFullURL("/api/x", ip) == "http://10.0.0.5/fpp?path=/api/x");
}

TEST_CASE("FPP2 MakeFullURL brackets IPv6 addresses", "[vendors2][fpp2]") {
    auto f = MakeFpp("fe80::1");
    std::string ip;
    CHECK(f->MakeFullURL("/a", ip) == "http://[fe80::1]/a");
    f->_fppProxy = "fe80::2";
    CHECK(f->MakeFullURL("/a", ip) == "http://[fe80::2]/proxy/[fe80::1]/a");
}

TEST_CASE("FPP2 GetURLAsJSON and GetURLAsString", "[vendors2][fpp2][http]") {
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/json") resp.body = R"({"a":[1,2]})";
        else if (r.path == "/bad") resp.body = "{";
        else if (r.path == "/text") resp.body = "plain";
        else resp.status = 404;
        return resp;
    });
    auto f = MakeFpp(srv.Address());
    nlohmann::json v;
    CHECK(f->GetURLAsJSON("/json", v));
    CHECK(v["a"].size() == 2);
    CHECK_FALSE(f->GetURLAsJSON("/bad", v));
    CHECK(v.is_null());
    std::string s;
    CHECK(f->GetURLAsString("/text", s));
    CHECK(s == "plain");
    CHECK_FALSE(f->GetURLAsString("/missing", s));
    REQUIRE_FALSE(f->messages.empty());
    CHECK(f->messages.back().find("Response Code: 404") != std::string::npos);
    f->messages.clear();
    CHECK_FALSE(f->GetURLAsString("/missing", s, false));
    CHECK(f->messages.empty());
}

TEST_CASE("FPP2 GetURLAsJSON with a body of just a scalar succeeds", "[vendors2][fpp2][http]") {
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse resp;
        resp.body = "5";
        return resp;
    });
    auto f = MakeFpp(srv.Address());
    nlohmann::json v;
    CHECK(f->GetURLAsJSON("/x", v));
    CHECK(v == 5);
}

TEST_CASE("FPP2 URLContentMatches and PostToURLIfChanged", "[vendors2][fpp2][http]") {
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.method == "GET") resp.body = "current";
        return resp;
    });
    auto f = MakeFpp(srv.Address());
    CHECK(f->URLContentMatches("/c", "current"));
    CHECK_FALSE(f->URLContentMatches("/c", "other"));
    CHECK_FALSE(f->URLContentMatches("/c", ""));
    auto before = srv.Requests().size();
    CHECK_FALSE(f->PostToURLIfChanged("/c", "current")); // unchanged, so nothing is sent
    CHECK(srv.Requests().size() == before + 1);           // only the GET
    CHECK(f->PostToURLIfChanged("/c", "different"));
    CHECK(srv.Requests().back().method == "POST");
    CHECK(srv.Requests().back().body == "different");
}

TEST_CASE("FPP2 PostJSONToURL and PostJSONToURLAsFormData", "[vendors2][fpp2][http]") {
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    auto f = MakeFpp(srv.Address());
    CHECK(f->PostJSONToURL("/p", nlohmann::json{ { "a", 1 } }) == 200);
    CHECK(nlohmann::json::parse(srv.Requests().back().body)["a"] == 1);
    CHECK(f->PostJSONToURLAsFormData("/q", "x=1", nlohmann::json{ { "b", 2 } }) == 200);
    auto body = srv.Requests().back().body;
    CHECK(body.find("x=1&data={") == 0);
    CHECK(body.back() == '}');
}

TEST_CASE("FPP2 JSON posts with invalid utf8 are replaced rather than thrown", "[vendors2][fpp2][http]") {
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    auto f = MakeFpp(srv.Address());
    CHECK_NOTHROW(f->PostJSONToURL("/p", nlohmann::json{ { "a", std::string("bad\xff") } }));
}

TEST_CASE("FPP2 PostToURL returns the response code or zero", "[vendors2][fpp2][http]") {
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        resp.status = r.path == "/err" ? 500 : 200;
        return resp;
    });
    auto f = MakeFpp(srv.Address());
    CHECK(f->PostToURL("/ok", "x") == 200);
    CHECK(f->PostToURL("/err", "x") == 500);
    CHECK(f->PutToURL("/ok", "x") == 200);
    auto dead = MakeFpp();
    CHECK(dead->PostToURL("/ok", "x") == 0);
}

TEST_CASE("FPP2 AuthenticateAndUpdateVersions loads config and system info once", "[vendors2][fpp2][http]") {
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/config.php") resp.body = "settings['Title'] = \"Falcon Player\";\n";
        if (r.path == "/api/system/info") resp.body = R"({"Version":"8.0","Platform":"Raspberry Pi","HostName":"fpp8"})";
        return resp;
    });
    auto f = MakeFpp(srv.Address());
    CHECK(f->AuthenticateAndUpdateVersions());
    CHECK(f->majorVersion == 8);
    CHECK(f->hostName == "fpp8");
    CHECK(f->sysInfoLoaded);
    CHECK(f->AuthenticateAndUpdateVersions());
    CHECK(srv.Count("/config.php") == 1);
}

TEST_CASE("FPP2 AuthenticateAndUpdateVersions fails when the config page is unavailable", "[vendors2][fpp2][http]") {
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse resp;
        resp.status = 404;
        return resp;
    });
    auto f = MakeFpp(srv.Address());
    CHECK_FALSE(f->AuthenticateAndUpdateVersions());
    CHECK_FALSE(f->sysInfoLoaded);
}

TEST_CASE("FPP2 AuthenticateAndUpdateVersions fails when system info is not json", "[vendors2][fpp2][http]") {
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        resp.body = r.path == "/config.php" ? "x" : "<html>";
        return resp;
    });
    auto f = MakeFpp(srv.Address());
    CHECK_FALSE(f->AuthenticateAndUpdateVersions());
}

TEST_CASE("FPP2 AuthenticateAndUpdateVersions on a non fpp device reports false", "[vendors2][fpp2][http]") {
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        resp.body = r.path == "/config.php" ? "settings['Title'] = \"Other\";\n" : R"({"Version":"1.0"})";
        return resp;
    });
    auto f = MakeFpp(srv.Address());
    f->fppType = FPP_TYPE::GENIUS;
    CHECK_FALSE(f->AuthenticateAndUpdateVersions());
}

TEST_CASE("FPP2 IsDDPInputEnabled looks at the first channel input", "[vendors2][fpp2][http]") {
    auto row = GENERATE(table<std::string, bool>({ { R"({"channelInputs":[{"enabled":1}]})", true }, { R"({"channelInputs":[{"enabled":0}]})", false },
                                                   { R"({"channelInputs":[]})", false }, { R"({"channelInputs":[{"enabled":"1"}]})", false }, { "{}", false },
                                                   { "[]", false } }));
    FakeHttpServer srv([&](const FakeRequest&) {
        FakeResponse resp;
        resp.body = std::get<0>(row);
        return resp;
    });
    auto f = MakeFpp(srv.Address());
    INFO(std::get<0>(row));
    CHECK(f->IsDDPInputEnabled() == std::get<1>(row));
}

TEST_CASE("FPP2 IsDDPInputEnabled with a scalar channelInputs should not throw", "[vendors2][fpp2][http][!mayfail]") {
    // FPP.cpp IsDDPInputEnabled: size() of a scalar is 1 so .at(0) is attempted on a number and throws type_error
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse resp;
        resp.body = R"({"channelInputs":5})";
        return resp;
    });
    auto f = MakeFpp(srv.Address());
    CHECK_NOTHROW(f->IsDDPInputEnabled());
}

TEST_CASE("FPP2 IsDDPInputEnabled when the controller is unreachable", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    CHECK_FALSE(f->IsDDPInputEnabled());
}

TEST_CASE("FPP2 UpdateChannelRanges merges the new ranges", "[vendors2][fpp2][http]") {
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse resp;
        resp.body = R"({"channelRanges":"100-199"})";
        return resp;
    });
    auto f = MakeFpp(srv.Address());
    f->ranges = "1-99";
    f->UpdateChannelRanges();
    CHECK(f->ranges == "1-199");
}

TEST_CASE("FPP2 UpdateChannelRanges does nothing for non fpp devices", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    f->fppType = FPP_TYPE::GENIUS;
    f->ranges = "1-5";
    f->UpdateChannelRanges();
    CHECK(f->ranges == "1-5");
}

TEST_CASE("FPP2 GetProxies handles strings and FPP 8 objects", "[vendors2][fpp2][http]") {
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse resp;
        resp.body = R"(["10.0.0.2",{"host":"10.0.0.3","description":"Roof"},5,null])";
        return resp;
    });
    auto f = MakeFpp(srv.Address());
    auto p = f->GetProxies();
    REQUIRE(p.size() == 2);
    CHECK(std::get<0>(p[0]) == "10.0.0.2");
    CHECK(std::get<1>(p[0]).empty());
    CHECK(std::get<0>(p[1]) == "10.0.0.3");
    CHECK(std::get<1>(p[1]) == "Roof");
    auto names = f->GetProxyList();
    CHECK(names.size() == 2);
}

TEST_CASE("FPP2 GetProxies with an object missing its host should not throw", "[vendors2][fpp2][http][!mayfail]") {
    // FPP.cpp GetProxies: val[x]["host"].get<std::string>() throws when an FPP 8 proxy object lacks host or description
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse resp;
        resp.body = R"([{"x":1}])";
        return resp;
    });
    auto f = MakeFpp(srv.Address());
    CHECK_NOTHROW(f->GetProxies());
}

TEST_CASE("FPP2 GetProxies when disconnected is empty", "[vendors2][fpp2]") {
    auto f = MakeFpp();
    f->_connected = false;
    CHECK(f->GetProxies().empty());
}

TEST_CASE("FPP2 copy constructor copies the descriptive fields", "[vendors2][fpp2]") {
    auto f = MakeFpp("10.0.0.7");
    f->hostName = "h";
    f->description = "d";
    f->fullVersion = "7.1";
    f->platform = "p";
    f->model = "m";
    f->majorVersion = 7;
    f->minorVersion = 1;
    f->patchVersion = 2;
    f->ranges = "1-5";
    f->mode = "player";
    f->pixelControllerType = "x";
    f->username = "u";
    f->password = "pw";
    f->fppType = FPP_TYPE::GENIUS;
    FPP c(*f);
    CHECK(c.hostName == "h");
    CHECK(c.ipAddress == "10.0.0.7");
    CHECK(c.fullVersion == "7.1");
    CHECK(c.majorVersion == 7);
    CHECK(c.patchVersion == 2);
    CHECK(c.ranges == "1-5");
    CHECK(c.username == "u");
    CHECK(c.fppType == FPP_TYPE::GENIUS);
}

TEST_CASE("FPP2 copy constructor keeps the connection details of the base", "[vendors2][fpp2][!mayfail]") {
    // FPP.cpp FPP(const FPP&): the base class is default constructed, so the copy has an empty _ip / _fppProxy and reports not connected
    auto f = MakeFpp("10.0.0.7");
    f->_fppProxy = "10.0.0.9";
    FPP c(*f);
    CHECK(c._ip == f->_ip);
    CHECK(c.IsConnected() == f->IsConnected());
}

TEST_CASE("FPP2 default constructed instance leaves upload uninitialised", "[vendors2][fpp2][.][known-crash]") {
    // FPP.h: `bool upload;` has no initialiser and none of the constructors set it, so sortByUpload and any read of it is undefined behaviour
    FPP f;
    CHECK_FALSE(f.upload);
}

TEST_CASE("FPP2 sort helpers order by name and address", "[vendors2][fpp2]") {
    FPP a, b;
    a.hostName = "Alpha";
    b.hostName = "beta";
    a.ipAddress = "10.0.0.20";
    b.ipAddress = "10.0.0.3";
    CHECK(sortByName(&a, &b));
    CHECK_FALSE(sortByName(&b, &a));
    CHECK_FALSE(sortByName(&a, &a));
    CHECK(sortByIP(&a, &b)); // plain string compare, so "10.0.0.20" < "10.0.0.3"
    CHECK(case_insensitive_match("AbC", "aBc") == 1);
    CHECK(case_insensitive_match("abc", "abd") == 0);
    CHECK(case_insensitive_match("", "") == 1);
}

TEST_CASE("FPP2 sortByUpload puts uploading instances first", "[vendors2][fpp2]") {
    FPP a, b;
    a.upload = true;
    b.upload = false;
    CHECK(sortByUpload(&a, &b));
    CHECK_FALSE(sortByUpload(&b, &a));
    CHECK_FALSE(sortByUpload(&a, &a));
}

TEST_CASE("FPP2 constructor from an address marks the instance connected", "[vendors2][fpp2]") {
    FPP f("10.9.8.7");
    CHECK(f.IsConnected());
    CHECK(f.ipAddress == "10.9.8.7");
    CHECK(f.UsesHTTP());
    CHECK(f.majorVersion == 0);
    CHECK(f.fppType == FPP_TYPE::FPP);
}

TEST_CASE("FPP2 three argument constructor remembers the pixel controller model", "[vendors2][fpp2]") {
    FPP f("10.9.8.7", "10.9.8.1", "F16V5");
    CHECK(f.pixelControllerType == "F16V5");
    CHECK(f.proxy() == "10.9.8.1");
    CHECK(f._baseUrl == "/proxy/10.9.8.7");
}

#endif
