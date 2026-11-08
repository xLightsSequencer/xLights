// The handlers are reached through ExposeVendors.h and a loopback server, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <mutex>

#include "support/ExposeVendors.h"
#include "controllers/Experience.h"
#include "controllers/J1Sys.h"
#include "controllers/PowerDMX.h"
#include "controllers/WLED.h"
#include "support/UnexposeVendors.h"
#include "outputs/DDPOutput.h"
#include "support/FakeHttpServer.h"
#include "support/VendorShow.h"

namespace {

std::map<std::string, std::string> Form(const std::string& body) {
    std::map<std::string, std::string> r;
    size_t pos = 0;
    while (pos <= body.size()) {
        size_t amp = body.find('&', pos);
        if (amp == std::string::npos) amp = body.size();
        std::string kv = body.substr(pos, amp - pos);
        size_t eq = kv.find('=');
        if (!kv.empty()) r[kv.substr(0, eq)] = eq == std::string::npos ? "" : kv.substr(eq + 1);
        pos = amp + 1;
    }
    return r;
}

// Records POST bodies by path and serves canned GET bodies.
struct Recorder {
    std::map<std::string, std::string> pages;
    std::string postReply = "OK";
    std::mutex m;
    std::vector<FakeRequest> posts;
    std::unique_ptr<FakeHttpServer> srv;
    Recorder() {
        srv = std::make_unique<FakeHttpServer>([this](const FakeRequest& r) {
            std::lock_guard<std::mutex> l(m);
            FakeResponse resp;
            if (r.method == "POST") {
                posts.push_back(r);
                resp.body = postReply;
                return resp;
            }
            std::string path = r.path.substr(0, r.path.find('?'));
            auto it = pages.find(path);
            if (it == pages.end()) {
                resp.status = 404;
            } else {
                resp.body = it->second;
            }
            return resp;
        });
    }
    std::vector<std::string> Bodies(const std::string& path) {
        std::lock_guard<std::mutex> l(m);
        std::vector<std::string> r;
        for (auto& p : posts) if (p.path == path) r.push_back(p.body);
        return r;
    }
};

} // namespace

// ---------------------------------------------------------------- J1Sys

namespace {
std::string SysInfo(const std::string& model, const std::string& version) {
    return "<html><script>document.getElementById('titleRight').innerHTML = '" + model + "\";</script>"
           "<table><tr><td><b>App Version:</b></td><td>&nbsp;</td><td>" + version + "</td></tr></table></html>";
}

std::unique_ptr<J1Sys> ConnectJ1(Recorder& r, const std::string& model, const std::string& version = "3.3") {
    VendorsTestInit();
    r.pages["/sysinfo.htm"] = SysInfo(model, version);
    r.pages["/protect/reboot.htm"] = "rebooting";
    // existing serial setup: both ports DMX at speed 5 on universes 7 and 8
    r.pages["/protect/portConfig.htm"] =
        "<input name=pA1 checked><select name=pP1><option value=\"D\" selected></select><select name=pB1><option value=\"5\" selected></select><input name=pU1 value=\"7\">"
        "<input name=pA2><select name=pP2><option value=\"R\" selected></select><select name=pB2><option value=\"3\" selected></select><input name=pU2 value=\"8\">";
    r.pages["/protect/stringConfig.htm"] = "";
    return std::make_unique<J1Sys>(r.srv->Address(), "");
}
} // namespace

TEST_CASE("J1Sys3 P12S SetOutputs posts one row per port and the serial ports", "[vendors3][j1sys3]") {
    Recorder r;
    auto j = ConnectJ1(r, "ECG-P12S");
    REQUIRE(j->IsConnected());
    REQUIRE(j->_outputs == 12);
    VendorShow s("J1Sys", "P12S", "", 4);
    s.Pixels("Arch", 1, 50, 1);
    s.Pixels("A", 3, 10, 511);
    s.Pixels("B", 3, 10, 541);
    s.Serial("Flood", 1, 8, 1531);
    REQUIRE_UPLOAD(s, j->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));

    auto ip = r.Bodies("/protect/ipConfig.htm");
    REQUIRE(ip.size() == 1);
    CHECK(ip[0] == "an=0&e1en=1&anen=0");

    auto str = r.Bodies("/protect/stringConfig.htm");
    REQUIRE(str.size() == 1);
    auto f = Form(str[0]);
    CHECK(f["sAA"] == "1");
    CHECK(f["sTA"] == "4");
    CHECK(f["sBA"] == "3200");
    CHECK(f["sUA"] == "1");
    CHECK(f["sSA"] == "1");
    CHECK(f["sCA"] == "50");
    CHECK(f["sAB"] == "0");
    // two back to back models on port 3 share one row
    CHECK(f["sAC"] == "1");
    CHECK(f["sUC"] == "2");
    CHECK(f["sSC"] == "1");
    CHECK(f["sCC"] == "20");
    CHECK(f.count("sAL") == 1);
    CHECK(f.count("sAM") == 0);

    auto ser = r.Bodies("/protect/portConfig.htm");
    REQUIRE(ser.size() == 1);
    auto sf = Form(ser[0]);
    CHECK(sf["pA1"] == "1");
    CHECK(sf["pP1"] == "D");
    CHECK(sf["pB1"] == "5");
    CHECK(sf["pU1"] == "4");
    // port 2 is resent as read from the controller
    CHECK(sf["pA2"] == "0");
    CHECK(sf["pP2"] == "R");
    CHECK(sf["pB2"] == "3");
    CHECK(sf["pU2"] == "8");
    CHECK(r.srv->Count("/protect/reboot.htm?") == 1);
}

TEST_CASE("J1Sys3 P2 spreads a port over the rows of its bank", "[vendors3][j1sys3]") {
    Recorder r;
    auto j = ConnectJ1(r, "ECG-P2");
    REQUIRE(j->_outputs == 2);
    REQUIRE(j->GetBankSize() == 4);
    VendorShow s("J1Sys", "P2", "", 4);
    // 300 pixels from channel 1 run over universes 1 and 2; the next model leaves a gap
    s.Pixels("Long", 1, 300, 1);
    s.Pixels("After", 1, 20, 1100);
    REQUIRE_UPLOAD(s, j->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto f = Form(r.Bodies("/protect/stringConfig.htm").at(0));
    CHECK(f["sAA"] == "1");
    CHECK(f["sUA"] == "1");
    CHECK(f["sCA"] == "170");
    CHECK(f["sAB"] == "1");
    CHECK(f["sUB"] == "2");
    CHECK(f["sSB"] == "1");
    CHECK(f["sCB"] == "130");
    CHECK(f["sAC"] == "1");
    CHECK(f["sUC"] == "3");
    CHECK(f["sSC"] == "80");
    CHECK(f["sCC"] == "20");
    CHECK(f["sAD"] == "0");
    // every row in the bank carries the bank's protocol
    CHECK(f["sTD"] == "4");
    CHECK(f["sBD"] == "3200");
    CHECK(f.count("sAH") == 1);
    CHECK(r.Bodies("/protect/portConfig.htm").empty());
}

TEST_CASE("J1Sys3 SetOutputs refusals", "[vendors3][j1sys3]") {
    Recorder r;
    SECTION("a port that needs more rows than its bank has") {
        auto j = ConnectJ1(r, "ECG-P12S");
        VendorShow s("J1Sys", "P12S", "", 4);
        s.Pixels("Span", 1, 300, 401);
        CHECK_FALSE(j->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("too many outputs required for port 1"));
        CHECK(r.Bodies("/protect/stringConfig.htm").empty());
    }
    SECTION("a serial port that does not start a universe") {
        auto j = ConnectJ1(r, "ECG-P12S");
        VendorShow s("J1Sys", "P12S", "", 4);
        s.Serial("Flood", 1, 8, 1535);
        CHECK_FALSE(j->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("does not start on channel 1 of universe 4"));
    }
    SECTION("a serial port over two universes") {
        auto j = ConnectJ1(r, "ECG-P12S");
        VendorShow s("J1Sys", "P12S", "", 4);
        s.Serial("Flood", 1, 512, 1021);
        CHECK_FALSE(j->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("requires more than 1 universe"));
    }
    SECTION("too many universes for a P12") {
        auto j = ConnectJ1(r, "ECG-P12S");
        VendorShow s("J1Sys", "P12S", "", 13);
        CHECK_FALSE(j->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(r.Bodies("/protect/ipConfig.htm").empty());
    }
    SECTION("too many universes for a P2") {
        auto j = ConnectJ1(r, "ECG-P2");
        VendorShow s("J1Sys", "P2", "", 9);
        CHECK_FALSE(j->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    }
    SECTION("universes larger than 510") {
        auto j = ConnectJ1(r, "ECG-P12S");
        VendorShow s("J1Sys", "P12S", "", 1, OUTPUT_E131, 512);
        CHECK_FALSE(j->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    }
    SECTION("the controller rejects the login") {
        r.postReply = "401 Unauthorized";
        auto j = ConnectJ1(r, "ECG-P12S");
        VendorShow s("J1Sys", "P12S", "", 1);
        s.Pixels("Arch", 1, 10, 1);
        CHECK_FALSE(j->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(r.Bodies("/protect/stringConfig.htm").empty());
    }
    CHECK(r.srv->Count("/protect/reboot.htm?") == 0);
}

TEST_CASE("J1Sys3 newer P12 firmware takes 26 universes and artnet", "[vendors3][j1sys3]") {
    Recorder r;
    auto j = ConnectJ1(r, "ECG-P12S", "3.4");
    VendorShow s("J1Sys", "P12S", "", 20, OUTPUT_ARTNET);
    CHECK(j->SetInputUniverses(s.ctl, &s.om));
    CHECK(r.Bodies("/protect/ipConfig.htm").at(0) == "an=0&e1en=0&anen=1");
}

// ---------------------------------------------------------------- Experience

namespace {
std::string GeniusState(const std::string& model = "G16_2023", const std::string& fw = "2.3.1-45", bool efuses = false) {
    return std::string(R"({"system":{"controller_model_name":"Genius 16","controller_model":")") + model + R"(","firmware_version":")" + fw +
           R"(","controller_line":"Genius","number_of_local_outputs":16,"number_of_long_range_pixel_ports":2,"number_of_long_range_dmx_ports":2,"has_efuses":)" +
           (efuses ? "true" : "false") + "}}";
}

std::unique_ptr<Experience> ConnectGenius(Recorder& r, const std::string& state = GeniusState(), const std::string& mode = "ddp") {
    VendorsTestInit();
    r.pages["/api/state"] = state;
    r.pages["/api/config"] = R"({"system":{"operating_mode":")" + mode + R"(","friendly_name":"old"},"outputs":[]})";
    return std::make_unique<Experience>(r.srv->Address(), "");
}

nlohmann::json Posted(Recorder& r) {
    auto b = r.Bodies("/api/config");
    REQUIRE(b.size() == 1);
    return nlohmann::json::parse(b[0]);
}
} // namespace

TEST_CASE("Experience3 SetOutputs posts virtual strings, remotes and DMX", "[vendors3][experience3]") {
    Recorder r;
    auto e = ConnectGenius(r);
    REQUIRE(e->IsConnected());
    VendorShow s("Experience Lights", "Genius Pixel 16", "", 4);
    s.Pixels("Arch", 1, 50, 1, "Brightness=\"55\" gamma=\"2.2\" nullNodes=\"2\" endNullNodes=\"1\" reverse=\"1\" colorOrder=\"GRB\" groupCount=\"2\"");
    s.Pixels("Rem", 17, 10, 151, "SmartRemote=\"2\" SmartRemoteType=\"falcon_v2\"");
    s.Serial("Flood", 1, 8, 1531);
    REQUIRE_UPLOAD(s, e->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto j = Posted(r);
    INFO(j.dump());
    CHECK(j["system"]["friendly_name"] == "Ctl");
    CHECK(j["system"]["auto_chain_all_outputs"] == false);
    CHECK(j["system"]["operating_mode"] == "e1.31");
    REQUIRE(j["inputs"].size() == 1);
    CHECK(j["inputs"][0]["start_universe"] == 1);
    CHECK(j["inputs"][0]["number_of_universes"] == 4);
    CHECK(j["inputs"][0]["channels_per_universe"] == 510);

    auto& o = j["outputs"];
    REQUIRE(o.size() == 26);
    auto& vs = o[0]["virtual_strings"][0];
    CHECK(vs["n"] == "Arch");
    CHECK(vs["sc"] == 1);
    CHECK(vs["ec"] == 50);
    CHECK(vs["r"] == true);
    CHECK(vs["g"] == 23);
    CHECK(vs["b"] == 60);
    CHECK(vs["sn"] == 2);
    CHECK(vs["en"] == 1);
    CHECK(vs["st"] == "grb");
    CHECK(vs["gc"] == 2);
    CHECK(o[0]["disabled"] == false);
    CHECK(o[1]["virtual_strings"][0]["ec"] == 0);
    CHECK_FALSE(o[1].contains("disabled"));

    // remote B on the first long range port gets a placeholder for remote A
    auto& lr = o[16]["virtual_strings"];
    REQUIRE(lr.size() == 2);
    CHECK(lr[0]["ri"] == 0);
    CHECK(lr[0]["ec"] == 0);
    CHECK(lr[1]["ri"] == 1);
    CHECK(lr[1]["n"] == "Rem");
    CHECK(lr[1]["sc"] == 151);
    CHECK(lr[1]["ec"] == 10);
    CHECK(o[16]["long_range_port_index"] == 0);
    // the other ports on that receiver are padded to the same number of remotes
    CHECK(o[17]["virtual_strings"].size() == 2);
    CHECK(o[17]["virtual_strings"][1]["ri"] == 1);
    CHECK(j["long_range_ports"][0]["number_of_receivers"] == 2);
    CHECK(j["long_range_ports"][1]["number_of_receivers"] == 1);
    CHECK(o[20]["virtual_strings"][0]["ri"] == 0);

    auto& dmx = o[24];
    CHECK(dmx["long_range_port_index"] == 2);
    CHECK(dmx["virtual_strings"][0]["sc"] == 1531);
    CHECK(dmx["virtual_strings"][0]["ec"] == 8);
    CHECK(j["long_range_ports"][2]["type"] == "dmx");
    CHECK(o[25]["virtual_strings"][0]["ec"] == 0);
}

TEST_CASE("Experience3 full control and eFuse boards", "[vendors3][experience3]") {
    Recorder r;
    auto e = ConnectGenius(r, GeniusState("G16_2023", "2.3.1-45", true), "standalone");
    VendorShow s("Experience Lights", "Genius Pro 16 Port", "", 2);
    s.ctl->SetFullxLightsControl(true);
    s.ctl->SetDefaultBrightnessUnderFullControl(42);
    s.ctl->SetDefaultGammaUnderFullControl(2.8f);
    s.Pixels("Arch", 1, 10, 1);
    REQUIRE_UPLOAD(s, e->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto j = Posted(r);
    CHECK(j["system"]["operating_mode"] == "standalone");
    CHECK(j["outputs"][0]["power_enabled"] == true);
    CHECK(j["outputs"][5]["power_enabled"] == true);
    CHECK(j["outputs"][0]["virtual_strings"][0]["b"] == 40);
    CHECK(j["outputs"][0]["virtual_strings"][0]["g"] == 28);
}

TEST_CASE("Experience3 DDP sends the start channel instead of universes", "[vendors3][experience3]") {
    Recorder r;
    auto e = ConnectGenius(r);
    VendorShow s("Experience Lights", "Genius Pixel 16", "", 1, OUTPUT_DDP);
    s.Pixels("Arch", 1, 10, 1);
    REQUIRE_UPLOAD(s, e->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto j = Posted(r);
    CHECK(j["system"]["operating_mode"] == "ddp");
    CHECK(j["system"]["start_channel"] == 1);
    CHECK_FALSE(j.contains("inputs"));
    CHECK(j["outputs"][0]["virtual_strings"][0]["sc"] == 1);
}

TEST_CASE("Experience3 universes of different sizes are listed one by one", "[vendors3][experience3]") {
    Recorder r;
    auto e = ConnectGenius(r);
    VendorShow s("Experience Lights", "Genius Pixel 16", "", 2, OUTPUT_ARTNET);
    s.ctl->GetOutputs().back()->SetChannels(300);
    REQUIRE_UPLOAD(s, e->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto j = Posted(r);
    CHECK(j["system"]["operating_mode"] == "artnet");
    REQUIRE(j["inputs"].size() == 2);
    CHECK(j["inputs"][1]["start_universe"] == 2);
    CHECK(j["inputs"][1]["channels_per_universe"] == 300);
    CHECK(j["inputs"][1]["start_channel"] == 511);
}

TEST_CASE("Experience3 refusals", "[vendors3][experience3]") {
    Recorder r;
    SECTION("group count on a 2022 controller") {
        auto e = ConnectGenius(r, GeniusState("G16_2022"));
        VendorShow s("Experience Lights", "Genius Pixel 16", "", 1);
        s.Pixels("Arch", 1, 10, 1, "groupCount=\"2\"");
        CHECK_FALSE(e->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("Group Count is not supported"));
    }
    SECTION("group count on old firmware") {
        auto e = ConnectGenius(r, GeniusState("G16_2023", "1.4.0-1"));
        VendorShow s("Experience Lights", "Genius Pixel 16", "", 1);
        s.Pixels("Arch", 1, 10, 1, "groupCount=\"2\"");
        CHECK_FALSE(e->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("Group Count is not supported"));
    }
    SECTION("failed checks") {
        auto e = ConnectGenius(r);
        VendorShow s("Experience Lights", "Genius Pixel 16", "", 1);
        s.Pixels("Arch", 30, 10, 1);
        CHECK_FALSE(e->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("Experience Upload Error"));
    }
    SECTION("config document unavailable") {
        auto e = ConnectGenius(r);
        r.pages.erase("/api/config");
        VendorShow s("Experience Lights", "Genius Pixel 16", "", 1);
        CHECK_FALSE(e->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    }
    CHECK(r.Bodies("/api/config").empty());
}

// ---------------------------------------------------------------- PowerDMX

namespace {
std::unique_ptr<PowerDMX> ConnectPowerDMX(Recorder& r) {
    VendorsTestInit();
    r.pages["/api/powerdmx/status"] = R"({"system":{"variant":"PowerDMX-7","version":"1.0.0"},"capabilities":{"max_ports":7}})";
    return std::make_unique<PowerDMX>(r.srv->Address(), "");
}
} // namespace

TEST_CASE("PowerDMX3 SetOutputs posts every port", "[vendors3][powerdmx3]") {
    Recorder r;
    auto p = ConnectPowerDMX(r);
    REQUIRE(p->IsConnected());
    VendorShow s("PowerDMX", "PowerDMX-7", "", 4);
    s.Pixels("Arch", 1, 50, 1);
    s.Pixels("Tree", 3, 100, 511);
    REQUIRE_UPLOAD(s, p->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto b = r.Bodies("/api/powerdmx/config");
    REQUIRE(b.size() == 1);
    auto j = nlohmann::json::parse(b[0]);
    CHECK(j["inputs"] == "e1.31");
    REQUIRE(j["ports"].size() == 7);
    CHECK(j["ports"][0] == nlohmann::json({ { "port", 1 }, { "pixels", 50 }, { "universe", 1 } }));
    CHECK(j["ports"][1]["pixels"] == 0);
    CHECK(j["ports"][1]["universe"] == 0);
    CHECK(j["ports"][2]["pixels"] == 100);
    CHECK(j["ports"][2]["universe"] == 2);
    CHECK(j["ports"][6]["port"] == 7);
}

TEST_CASE("PowerDMX3 DDP input and refusals", "[vendors3][powerdmx3]") {
    Recorder r;
    auto p = ConnectPowerDMX(r);
    SECTION("ddp") {
        VendorShow s("PowerDMX", "PowerDMX-7", "", 1, OUTPUT_DDP);
        s.Pixels("Arch", 1, 10, 1);
        REQUIRE_UPLOAD(s, p->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(nlohmann::json::parse(r.Bodies("/api/powerdmx/config").at(0))["inputs"] == "ddp");
    }
    SECTION("port beyond the board") {
        VendorShow s("PowerDMX", "PowerDMX-7", "", 1);
        s.Pixels("Arch", 8, 10, 1);
        CHECK_FALSE(p->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("PowerDMX Upload Error"));
        CHECK(r.Bodies("/api/powerdmx/config").empty());
    }
}

// ---------------------------------------------------------------- WLED

namespace {
const char* kWledCfg = R"({"hw":{"led":{"total":10,"ins":[{"len":10,"start":0,"pin":[16],"type":22,"order":1,"rev":false,"skip":0}]}},"if":{"live":{"en":false}},"light":{"scale-bri":100}})";

std::unique_ptr<WLED> ConnectWled(Recorder& r, int vid = 2310130) {
    VendorsTestInit();
    r.pages["/json/info"] = R"({"ver":"0.14.0","vid":)" + std::to_string(vid) + R"(,"arch":"esp32","name":"W"})";
    r.pages["/json/cfg"] = kWledCfg;
    return std::make_unique<WLED>(r.srv->Address(), "");
}

nlohmann::json WledPosted(Recorder& r) {
    auto b = r.Bodies("/json/cfg");
    REQUIRE(b.size() == 1);
    return nlohmann::json::parse(b[0]);
}
} // namespace

TEST_CASE("WLED3 SetOutputs posts the LED outputs and live input", "[vendors3][wled3]") {
    Recorder r;
    auto w = ConnectWled(r);
    REQUIRE(w->IsConnected());
    VendorShow s("WLED", "WLED", "Generic ESP32", 1);
    s.Pixels("Arch", 1, 50, 1, "colorOrder=\"GRB\" reverse=\"1\" nullNodes=\"2\"");
    s.Pixels("Tree", 3, 20, 151);
    REQUIRE_UPLOAD(s, w->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto j = WledPosted(r);
    INFO(j.dump());
    CHECK(j["rb"] == true);
    CHECK(j["hw"]["led"]["total"] == 70);
    auto& ins = j["hw"]["led"]["ins"];
    REQUIRE(ins.size() == 2);
    CHECK(ins[0]["len"] == 50);
    CHECK(ins[0]["start"] == 0);
    CHECK(ins[0]["pin"][0] == 16); // kept from the controller
    CHECK(ins[0]["type"] == 22);
    CHECK(ins[0]["order"] == 0);
    CHECK(ins[0]["rev"] == true);
    CHECK(ins[0]["skip"] == 2);
    CHECK(ins[1]["len"] == 20);
    CHECK(ins[1]["start"] == 50);
    CHECK(ins[1]["pin"][0] == 12); // the caps default for port 3
    CHECK(ins[1]["rev"] == false);
    auto& live = j["if"]["live"];
    CHECK(live["en"] == true);
    CHECK(live["port"] == 5568);
    CHECK(live["dmx"]["uni"] == 1);
    CHECK(live["dmx"]["addr"] == 1);
    CHECK(j["light"]["scale-bri"] == 100);
}

TEST_CASE("WLED3 artnet, multicast and full control", "[vendors3][wled3]") {
    Recorder r;
    auto w = ConnectWled(r);
    VendorShow s("WLED", "WLED", "Generic ESP32", 1, OUTPUT_ARTNET, 510, 4, "MULTICAST");
    s.ctl->SetFullxLightsControl(true);
    s.ctl->SetDefaultBrightnessUnderFullControl(60);
    s.Pixels("Arch", 2, 10, 1);
    REQUIRE_UPLOAD(s, w->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto j = WledPosted(r);
    CHECK(j["if"]["live"]["port"] == 6454);
    CHECK(j["if"]["live"]["mc"] == true);
    CHECK(j["if"]["live"]["dmx"]["uni"] == 4);
    CHECK(j["light"]["scale-bri"] == 60);
    // full control ignores what the controller had
    auto& ins = j["hw"]["led"]["ins"];
    REQUIRE(ins.size() == 1);
    CHECK(ins[0]["pin"][0] == 13);
    CHECK(ins[0]["order"] == 0);
}

TEST_CASE("WLED3 DDP leaves the live port alone on newer firmware", "[vendors3][wled3]") {
    Recorder r;
    auto w = ConnectWled(r);
    VendorShow s("WLED", "WLED", "Generic ESP32", 1, OUTPUT_DDP);
    dynamic_cast<DDPOutput*>(s.ctl->GetFirstOutput())->SetKeepChannelNumber(false);
    s.Pixels("Arch", 1, 10, 1);
    REQUIRE_UPLOAD(s, w->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto j = WledPosted(r);
    CHECK(j["if"]["live"]["en"] == true);
    CHECK_FALSE(j["if"]["live"].contains("port"));
}

TEST_CASE("WLED3 DDP on older firmware sets the live port", "[vendors3][wled3]") {
    Recorder r;
    auto w = ConnectWled(r, 2106000);
    VendorShow s("WLED", "WLED", "Generic ESP32", 1, OUTPUT_DDP);
    dynamic_cast<DDPOutput*>(s.ctl->GetFirstOutput())->SetKeepChannelNumber(false);
    s.Pixels("Arch", 1, 10, 1);
    REQUIRE_UPLOAD(s, w->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto j = WledPosted(r);
    CHECK(j["if"]["live"]["port"] == 4048);
    CHECK(j["if"]["live"]["dmx"]["addr"] == 1);
}

TEST_CASE("WLED3 refusals", "[vendors3][wled3]") {
    Recorder r;
    SECTION("firmware too old") {
        auto w = ConnectWled(r, 2105000);
        VendorShow s("WLED", "WLED", "Generic ESP32", 1);
        CHECK_FALSE(w->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("0.13b5 or newer is required"));
    }
    SECTION("broken 0.13 firmware") {
        auto w = ConnectWled(r, 2200000);
        VendorShow s("WLED", "WLED", "Generic ESP32", 1);
        CHECK_FALSE(w->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("0.13.1 is broken"));
    }
    SECTION("failed checks") {
        auto w = ConnectWled(r);
        VendorShow s("WLED", "WLED", "Generic ESP32", 1);
        s.Pixels("Arch", 9, 10, 1);
        CHECK_FALSE(w->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("WLED Upload Error"));
    }
    SECTION("config is not json") {
        auto w = ConnectWled(r);
        r.pages["/json/cfg"] = "<html>";
        VendorShow s("WLED", "WLED", "Generic ESP32", 1);
        s.Pixels("Arch", 1, 10, 1);
        CHECK_FALSE(w->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("JSON Parse Error"));
    }
    SECTION("DDP keeping channel numbers") {
        auto w = ConnectWled(r);
        VendorShow s("WLED", "WLED", "Generic ESP32", 1, OUTPUT_DDP);
        dynamic_cast<DDPOutput*>(s.ctl->GetFirstOutput())->SetKeepChannelNumber(true);
        s.Pixels("Arch", 1, 10, 1);
        CHECK_FALSE(w->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    }
    SECTION("universes of different sizes") {
        auto w = ConnectWled(r);
        VendorShow s("WLED", "WLED", "Generic ESP32", 2);
        s.ctl->GetOutputs().back()->SetChannels(300);
        s.Pixels("Arch", 1, 10, 1);
        CHECK_FALSE(w->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    }
    CHECK(r.Bodies("/json/cfg").empty());
}

TEST_CASE("WLED3 an error reply fails the upload", "[vendors3][wled3]") {
    Recorder r;
    r.postReply = R"({"error":9})";
    auto w = ConnectWled(r);
    VendorShow s("WLED", "WLED", "Generic ESP32", 1);
    s.Pixels("Arch", 1, 10, 1);
    CHECK_FALSE(w->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(r.Bodies("/json/cfg").size() == 1);
}

#endif
