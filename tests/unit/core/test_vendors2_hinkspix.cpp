// The handlers are reached through ExposeVendors.h and a loopback server, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "support/ExposeVendors.h"
#include "controllers/HinksPix.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"

#include "outputs/ControllerEthernet.h"
#include "outputs/OutputManager.h"

namespace {
std::unique_ptr<HinksPix> MakeDisconnectedHinks() {
    VendorsTestInit();
    return std::make_unique<HinksPix>("127.0.0.1:1", "");
}

std::string InfoJson(const std::string& extra) {
    return "{\"Type\":\"P\",\"Controller\":\"H\"" + extra + "}";
}
} // namespace

TEST_CASE("HinksPix2 output SetConfig parses ten comma separated fields", "[vendors2][hinkspix2]") {
    HinksPixOutput o(3, 100);
    o.SetConfig("3,1,101,50,250,1,2,3,80,2");
    CHECK(o.protocol == 1);
    CHECK(o.getControllerStartChannel() == 101);
    CHECK(o.pixels == 50);
    CHECK(o.getControllerEndChannel() == 250);
    CHECK(o.direction == 1);
    CHECK(o.colorOrder == 2);
    CHECK(o.nullPixel == 3);
    CHECK(o.brightness == 80);
    CHECK(o.gamma == 2);
}

TEST_CASE("HinksPix2 output SetConfig treats undefined protocol as zero", "[vendors2][hinkspix2]") {
    HinksPixOutput o(1, 100);
    o.protocol = 5;
    o.SetConfig("1,undefined,1,0,0,0,0,0,100,1");
    CHECK(o.protocol == 0);
}

TEST_CASE("HinksPix2 output SetConfig ignores malformed or mismatched data", "[vendors2][hinkspix2]") {
    for (std::string bad : { std::string(""), std::string("1,2,3"), std::string("1,1,1,1,1,1,1,1,1"), std::string("1,1,1,1,1,1,1,1,1,1,1"), std::string("2,1,1,1,1,1,1,1,1,1"),
                             std::string("x,1,1,1,1,1,1,1,1,1"), std::string(",,,,,,,,,") }) {
        HinksPixOutput o(1, 77);
        o.pixels = 9;
        o.SetConfig(bad);
        INFO("config '" << bad << "'");
        CHECK(o.pixels == 9);
        CHECK(o.brightness == 77);
    }
}

TEST_CASE("HinksPix2 output SetConfig with garbage numbers gives zero", "[vendors2][hinkspix2]") {
    HinksPixOutput o(0, 100);
    o.SetConfig("0,a,b,c,d,e,f,g,h,i");
    CHECK(o.pixels == 0);
    CHECK(o.brightness == 0);
    CHECK(o.gamma == 0);
}

TEST_CASE("HinksPix2 output SetConfig with huge numbers truncates", "[vendors2][hinkspix2]") {
    HinksPixOutput o(1, 100);
    o.SetConfig("1,1,1,99999999999999999999,1,1,1,1,1,1");
    CHECK(o.pixels == -1); // strtol saturates at LONG_MAX which truncates to -1 as an int
}

TEST_CASE("HinksPix2 output BuildCommand formats the V field", "[vendors2][hinkspix2]") {
    HinksPixOutput o(4, 90);
    o.protocol = 1;
    o.pixels = 10;
    o.direction = 1;
    o.colorOrder = 3;
    o.nullPixel = 2;
    o.gamma = 3;
    o.setControllerChannels(31);
    CHECK(o.BuildCommand()["V"] == "4,1,31,10,60,1,3,2,90,3");
    CHECK(o.BuildCommandEasyLights() == "4,1,31,10,60,1,3,2,90,3|");
}

TEST_CASE("HinksPix2 setControllerChannels uses four channels for RGBW orders", "[vendors2][hinkspix2]") {
    HinksPixOutput o(1, 100);
    o.pixels = 10;
    o.colorOrder = 0;
    o.setControllerChannels(1);
    CHECK(o.getControllerEndChannel() == 30);
    o.colorOrder = 6;
    o.setControllerChannels(1);
    CHECK(o.getControllerEndChannel() == 40);
    o.colorOrder = 7;
    o.setControllerChannels(5);
    CHECK(o.getControllerStartChannel() == 5);
    CHECK(o.getControllerEndChannel() == 44);
    o.colorOrder = 5;
    o.setControllerChannels(1);
    CHECK(o.getControllerEndChannel() == 30);
}

TEST_CASE("HinksPix2 setControllerChannels with no pixels ends before it starts", "[vendors2][hinkspix2]") {
    HinksPixOutput o(1, 100);
    o.pixels = 0;
    o.setControllerChannels(10);
    CHECK(o.getControllerEndChannel() == 9);
}

TEST_CASE("HinksPix2 setControllerChannels with a huge pixel count overflows", "[vendors2][hinkspix2][.][known-crash]") {
    // HinksPix.cpp setControllerChannels: pixels * chanPerPix is a signed int multiply, so a pixel count above ~700 million is undefined behaviour
    HinksPixOutput o(1, 100);
    o.pixels = 2000000000;
    o.setControllerChannels(1);
    CHECK(o.getControllerEndChannel() > 0);
}

TEST_CASE("HinksPix2 output defaults", "[vendors2][hinkspix2]") {
    HinksPixOutput o(7, 55);
    CHECK(o.output == 7);
    CHECK(o.brightness == 55);
    CHECK(o.universe == 1);
    CHECK(o.startChannel == 1);
    CHECK(o.gamma == 1);
    CHECK_FALSE(o.used);
    CHECK(o.getControllerStartChannel() == 1);
    CHECK(o.getControllerEndChannel() == 0);
}

TEST_CASE("HinksPix2 serial SetConfig reads the DMX settings", "[vendors2][hinkspix2]") {
    HinksPixSerial s;
    s.SetConfig(nlohmann::json::parse(R"({"CMD":"DATA_MODE","DMX_ACTIVE":1,"DMX_UNIV":5,"DMX_START":3,"DMX_CHAN_CNT":64,"DDP_DMX_ACTIVE":0,"DDP_DMX_START":9,"DDP_DMX_CHAN_CNT":32})"));
    CHECK(s.e131Enabled);
    CHECK(s.e131Universe == 5);
    CHECK(s.e131StartChannel == 3);
    CHECK(s.e131NumOfChan == 64);
    CHECK_FALSE(s.ddpDMXEnabled);
    CHECK(s.ddpDMXStartChannel == 9);
    CHECK(s.ddpDMXNumOfChan == 32);
}

TEST_CASE("HinksPix2 serial SetConfig throws on missing keys", "[vendors2][hinkspix2]") {
    HinksPixSerial s;
    CHECK_THROWS(s.SetConfig(nlohmann::json::parse("{}")));
    CHECK_THROWS(s.SetConfig(nlohmann::json::parse(R"({"DMX_ACTIVE":"1"})")));
    CHECK_THROWS(s.SetConfig(nlohmann::json::array()));
}

TEST_CASE("HinksPix2 serial BuildCommand is a DATA line with json", "[vendors2][hinkspix2]") {
    HinksPixSerial s;
    s.e131Enabled = true;
    s.e131Universe = 4;
    s.e131StartChannel = 2;
    s.e131NumOfChan = 100;
    s.ddpDMXEnabled = false;
    s.ddpDMXStartChannel = 6;
    s.ddpDMXNumOfChan = 16;
    auto cmd = s.BuildCommand();
    REQUIRE(cmd.substr(0, 6) == "DATA: ");
    auto j = nlohmann::json::parse(cmd.substr(6));
    CHECK(j["CMD"] == "DATA_MODE");
    CHECK(j["DMX_ACTIVE"] == 1);
    CHECK(j["DMX_UNIV"] == 4);
    CHECK(j["DDP_DMX_ACTIVE"] == 0);
    CHECK(j["DDP_DMX_CHAN_CNT"] == 16);
}

TEST_CASE("HinksPix2 serial EasyLights command", "[vendors2][hinkspix2]") {
    HinksPixSerial s;
    CHECK(s.BuildCommandEasyLights(0) == "A,0,B,0,C,0,D,1,E,1,F,512,G,1,H,512");
    s.e131Enabled = true;
    s.ddpDMXEnabled = true;
    CHECK(s.BuildCommandEasyLights(2) == "A,2,B,1,C,1,D,1,E,1,F,512,G,1,H,512");
}

TEST_CASE("HinksPix2 smart output SetConfig", "[vendors2][hinkspix2]") {
    HinksSmartOutput so(0);
    so.SetConfig("2,1,10,20,30,40");
    CHECK(so.id == 2);
    CHECK(so.type == 1);
    CHECK(so.portStartPixel[0] == 10);
    CHECK(so.portStartPixel[3] == 40);
    nlohmann::ordered_json j = so.BuildCommand();
    CHECK(j["V"] == "2,1,10,20,30,40");
}

TEST_CASE("HinksPix2 smart output SetConfig rejects the wrong field count", "[vendors2][hinkspix2]") {
    for (std::string bad : { std::string(""), std::string("1,2,3,4,5"), std::string("1,2,3,4,5,6,7") }) {
        HinksSmartOutput so(9);
        so.SetConfig(bad);
        INFO(bad);
        CHECK(so.id == 9);
    }
}

TEST_CASE("HinksPix2 smart outputs compare by id only", "[vendors2][hinkspix2]") {
    HinksSmartOutput a(1), b(1), c(2);
    b.type = 2;
    b.portStartPixel = { 9, 9, 9, 9 };
    CHECK(a == b);
    CHECK_FALSE(a == c);
}

TEST_CASE("HinksPix2 smart output default command", "[vendors2][hinkspix2]") {
    HinksSmartOutput so(3);
    CHECK(so.BuildCommand()["V"] == "3,0,0,0,0,0");
}

TEST_CASE("HinksPix2 input universe commands", "[vendors2][hinkspix2]") {
    HinksPixInputUniverse u(10, 510);
    u.index = 2;
    u.hinksPixStartChannel = 1021;
    CHECK(u.BuildCommand()["V"] == "2,10,510,1,1021,1530");
    CHECK(u.BuildCommandEasyLights() == "2,10,510,1,1021,1530");
}

TEST_CASE("HinksPix2 input universes compare by universe number", "[vendors2][hinkspix2]") {
    HinksPixInputUniverse a(5, 100), b(5, 200), c(6, 100);
    CHECK(a == b);
    CHECK_FALSE(a == c);
    CHECK(a.index == -1);
}

TEST_CASE("HinksPix2 input universe with zero channels ends one before it starts", "[vendors2][hinkspix2]") {
    HinksPixInputUniverse u(1, 0);
    u.index = 0;
    CHECK(u.BuildCommandEasyLights() == "0,1,0,1,1,0");
}

TEST_CASE("HinksPix2 string protocol encoding", "[vendors2][hinkspix2]") {
    auto h = MakeDisconnectedHinks();
    auto row = GENERATE(table<std::string, int>({ { "ws2811", 1 }, { "WS2811", 1 }, { "ws2812", 2 }, { "ws2812b", 3 }, { "ws2813", 4 }, { "ws2801", 5 }, { "tls3001", 6 }, { "apa102", 7 } }));
    CHECK(h->EncodeStringPortProtocol(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("HinksPix2 unknown string protocol asserts", "[vendors2][hinkspix2][.][known-crash]") {
    // HinksPix.cpp EncodeStringPortProtocol: assert(false) for e.g. tm1814; UpdatePortData compares the result with -1 which can never happen
    auto h = MakeDisconnectedHinks();
    CHECK(h->EncodeStringPortProtocol("tm1814") == -1);
}

TEST_CASE("HinksPix2 colour order encoding", "[vendors2][hinkspix2]") {
    auto h = MakeDisconnectedHinks();
    auto row = GENERATE(table<std::string, int>({ { "rgb", 0 }, { "RGB", 0 }, { "rbg", 1 }, { "grb", 2 }, { "gbr", 3 }, { "brg", 4 }, { "bgr", 5 }, { "rgbw", 6 }, { "WRGB", 7 } }));
    CHECK(h->EncodeColorOrder(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("HinksPix2 direction brightness and gamma encoding", "[vendors2][hinkspix2]") {
    auto h = MakeDisconnectedHinks();
    CHECK(h->EncodeDirection("Reverse") == 1);
    CHECK(h->EncodeDirection("reverse") == 0);
    CHECK(h->EncodeDirection("Forward") == 0);
    CHECK(h->EncodeDirection("") == 0);
    auto br = GENERATE(table<int, int>({ { 100, 100 }, { 99, 90 }, { 95, 90 }, { 90, 90 }, { 85, 80 }, { 25, 20 }, { 20, 20 }, { 19, 15 }, { 10, 15 }, { 0, 15 }, { -50, 15 }, { 150, 150 } }));
    INFO(std::get<0>(br));
    CHECK(h->EncodeBrightness(std::get<0>(br)) == std::get<1>(br));
    CHECK(h->EncodeGamma(1) == 1);
    CHECK(h->EncodeGamma(4) == 4);
    CHECK(h->EncodeGamma(5) == 4);
    CHECK(h->EncodeGamma(1000) == 4);
    CHECK(h->EncodeGamma(0) == 0);
    CHECK(h->EncodeGamma(-3) == -3);
}

TEST_CASE("HinksPix2 expansion type decoding", "[vendors2][hinkspix2]") {
    auto h = MakeDisconnectedHinks();
    CHECK(h->DecodeExpansionType("S") == HinksPix::EXPType::Local_SPI);
    CHECK(h->DecodeExpansionType("L") == HinksPix::EXPType::Long_Range);
    CHECK(h->DecodeExpansionType("A") == HinksPix::EXPType::Local_AC);
    CHECK(h->DecodeExpansionType("N") == HinksPix::EXPType::Not_Present);
    CHECK(h->DecodeExpansionType("") == HinksPix::EXPType::Not_Present);
    CHECK(h->DecodeExpansionType("s") == HinksPix::EXPType::Not_Present);
}

TEST_CASE("HinksPix2 StringToMap pairs up keys and values", "[vendors2][hinkspix2]") {
    auto h = MakeDisconnectedHinks();
    auto m = h->StringToMap("a,1,b,2,c,3");
    CHECK(m.size() == 3);
    CHECK(m["b"] == "2");
    CHECK(h->StringToMap("").empty());
    CHECK(h->StringToMap("a,1,b").empty()); // an odd number of items is rejected
    CHECK(h->StringToMap("a,1,a,2")["a"] == "2");
}

TEST_CASE("HinksPix2 firmware upload support depends on hardware and version", "[vendors2][hinkspix2]") {
    auto h = MakeDisconnectedHinks();
    struct R {
        const char* type;
        bool v3;
        int ver;
        bool expect;
    };
    const R rows[] = { { "H", false, 151, true }, { "H", false, 150, false }, { "H", true, 129, true }, { "H", true, 128, false }, { "H", true, 151, true },
                       { "H", false, 129, false }, { "E", false, 999, false }, { "E", true, 999, false }, { "", false, 0, false }, { "H", false, -1, false } };
    for (auto const& r : rows) {
        h->_controllerType = r.type;
        h->_hardwareV3 = r.v3;
        h->_MCPU_Version = r.ver;
        INFO(r.type << " v3=" << r.v3 << " ver=" << r.ver);
        CHECK(h->FirmwareSupportsUpload() == r.expect);
    }
}

TEST_CASE("HinksPix2 CheckPixelOutputs flags used ports without a board", "[vendors2][hinkspix2]") {
    auto h = MakeDisconnectedHinks();
    h->InitControllerOutputData(true, 100);
    REQUIRE(h->_pixelOutputs.size() == 80);
    std::string msg;
    CHECK(h->CheckPixelOutputs(msg));
    h->_pixelOutputs[20].used = true; // output 21 is on expansion board 2
    CHECK_FALSE(h->CheckPixelOutputs(msg));
    CHECK(msg.find("Pixel Output 21") != std::string::npos);
    CHECK(msg.find("Expansion Port 2") != std::string::npos);
    h->_EXP_Outputs[1] = HinksPix::EXPType::Local_SPI;
    msg.clear();
    CHECK(h->CheckPixelOutputs(msg));
    CHECK(msg.empty());
}

TEST_CASE("HinksPix2 CheckPixelOutputs checks every expansion port boundary", "[vendors2][hinkspix2]") {
    auto h = MakeDisconnectedHinks();
    h->InitControllerOutputData(true, 100);
    for (int i = 0; i < 80; ++i) {
        for (auto& o : h->_pixelOutputs) o.used = false;
        h->_pixelOutputs[i].used = true;
        for (auto& e : h->_EXP_Outputs) e = HinksPix::EXPType::Not_Present;
        h->_EXP_Outputs[i / 16] = HinksPix::EXPType::Local_SPI;
        std::string msg;
        INFO("output " << i + 1);
        CHECK(h->CheckPixelOutputs(msg));
    }
}

TEST_CASE("HinksPix2 CheckSmartReceivers needs long range boards", "[vendors2][hinkspix2]") {
    auto h = MakeDisconnectedHinks();
    std::string msg;
    CHECK(h->CheckSmartReceivers(msg));
    h->_smartOutputs[2][1].emplace_back(0);
    CHECK_FALSE(h->CheckSmartReceivers(msg));
    CHECK(msg.find("Expansion Port 3") != std::string::npos);
    h->_EXP_Outputs[2] = HinksPix::EXPType::Long_Range;
    msg.clear();
    CHECK(h->CheckSmartReceivers(msg));
}

TEST_CASE("HinksPix2 InitControllerOutputData builds eighty outputs", "[vendors2][hinkspix2]") {
    auto h = MakeDisconnectedHinks();
    CHECK(h->InitControllerOutputData(true, 60));
    REQUIRE(h->_pixelOutputs.size() == 80);
    CHECK(h->_pixelOutputs.front().output == 1);
    CHECK(h->_pixelOutputs.back().output == 80);
    CHECK(h->_pixelOutputs[10].brightness == 60);
    CHECK(h->_serialOutput != nullptr);
    CHECK(h->InitControllerOutputData(true, 70));
    CHECK(h->_pixelOutputs.size() == 80);
    CHECK(h->_pixelOutputs[0].brightness == 70);
}

TEST_CASE("HinksPix2 IsUnPackSupported_Hinks depends on model and mcpu", "[vendors2][hinkspix2]") {
    auto h = MakeDisconnectedHinks();
    OutputManager om;
    ControllerEthernet c(&om, false);
    struct R {
        const char* model;
        int ver;
        bool expect;
    };
    const R rows[] = { { "PRO V1/V2", 151, true }, { "PRO V1/V2", 150, false }, { "PRO V3", 129, true }, { "PRO V3", 128, false }, { "PRO V3", 500, true },
                       { "EasyLights", 500, false }, { "", 500, false }, { "pro v3", 500, false } };
    for (auto const& r : rows) {
        c.SetModel(r.model);
        h->_MCPU_Version = r.ver;
        INFO(r.model << " " << r.ver);
        CHECK(h->IsUnPackSupported_Hinks(&c) == r.expect);
    }
}

TEST_CASE("HinksPix2 disconnected defaults", "[vendors2][hinkspix2]") {
    auto h = MakeDisconnectedHinks();
    CHECK_FALSE(h->IsConnected());
    CHECK(h->UsesHTTP());
    CHECK(h->GetMPUVersion() == 0);
    CHECK_FALSE(h->IsHardwareV3());
    for (auto e : h->_EXP_Outputs) {
        CHECK(e == HinksPix::EXPType::Not_Present);
    }
}

TEST_CASE("HinksPix2 static urls", "[vendors2][hinkspix2]") {
    CHECK(HinksPix::GetJSONPostURL() == "/Xlights_PostData.cgi");
    CHECK(HinksPix::GetJSONInfoURL() == "/XLights_BoardInfo.cgi");
    CHECK(HinksPix::GetJSONPortURL() == "/Xlights_Board_Port_Config.cgi");
    CHECK(HinksPix::GetJSONUnPackURL() == "/Xlights_UnPack_Config.cgi");
    CHECK(HinksPix::GetJSONModeURL() == "/Xlights_Data_Mode.cgi");
    CHECK(HinksPix::GetE131URL() == "/GetE131Data.cgi");
    CHECK(HinksPix::GetInfoURL() == "/GetInfo.cgi");
}

TEST_CASE("HinksPix2 constructor identifies a PRO controller", "[vendors2][hinkspix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = InfoJson(R"(,"BD1":"S","BD2":"L","BD3":"A","BD4":"X","MaxU":"96","MCPU":"VER151","PCPU":"VER002","ECPU":"VER003","WEB":"VER004")");
        return r;
    });
    HinksPix h(srv.Address(), "");
    CHECK(h.IsConnected());
    CHECK(h.GetModel() == "HinksPix PRO");
    CHECK(h.GetVersion() == "MAIN:151,POWER:002,WIFI:003,WEB:004");
    CHECK(h.GetMPUVersion() == 151);
    CHECK(h._numberOfUniverses == 96);
    CHECK_FALSE(h.IsHardwareV3());
    CHECK(h._EXP_Outputs[0] == HinksPix::EXPType::Local_SPI);
    CHECK(h._EXP_Outputs[1] == HinksPix::EXPType::Long_Range);
    CHECK(h._EXP_Outputs[2] == HinksPix::EXPType::Local_AC);
    CHECK(h._EXP_Outputs[3] == HinksPix::EXPType::Not_Present);
    CHECK(h._EXP_Outputs[4] == HinksPix::EXPType::Not_Present);
    CHECK(h.FirmwareSupportsUpload());
}

TEST_CASE("HinksPix2 constructor identifies controller models", "[vendors2][hinkspix2][http]") {
    VendorsTestInit();
    auto row = GENERATE(table<std::string, std::string>({ { "P:H", "HinksPix PRO" }, { "8:H", "HinksPix PRO 80" }, { "P:E", "EasyLights Pix16" }, { "P:Q", "Unknown" } }));
    FakeHttpServer srv([&](const FakeRequest&) {
        FakeResponse r;
        r.body = "{\"Type\":\"" + std::get<0>(row).substr(0, 1) + "\",\"Controller\":\"" + std::get<0>(row).substr(2, 1) + "\"}";
        return r;
    });
    HinksPix h(srv.Address(), "");
    CHECK(h.IsConnected());
    CHECK(h.GetModel() == std::get<1>(row));
    CHECK(h.IsHardwareV3() == (std::get<0>(row)[0] == '8'));
}

TEST_CASE("HinksPix2 constructor leaves the version blank without all CPU fields", "[vendors2][hinkspix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = InfoJson(R"(,"MCPU":"VER129")");
        return r;
    });
    HinksPix h(srv.Address(), "");
    CHECK(h.IsConnected());
    CHECK(h.GetVersion().empty());
    CHECK(h.GetMPUVersion() == 129);
}

TEST_CASE("HinksPix2 constructor retries the info request three times", "[vendors2][hinkspix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    HinksPix h(srv.Address(), "");
    CHECK_FALSE(h.IsConnected());
    CHECK(srv.Count("/XLights_BoardInfo.cgi") == 3);
}

TEST_CASE("HinksPix2 constructor with a non json reply is not connected", "[vendors2][hinkspix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "<html>nope</html>";
        return r;
    });
    HinksPix h(srv.Address(), "");
    CHECK_FALSE(h.IsConnected());
}

TEST_CASE("HinksPix2 constructor with a short MCPU string should not throw", "[vendors2][hinkspix2][http][!mayfail]") {
    // HinksPix.cpp constructor: data.at("MCPU").get<std::string>().substr(3) throws std::out_of_range when the string has fewer than 3 characters
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = InfoJson(R"(,"MCPU":"V1")");
        return r;
    });
    CHECK_NOTHROW(HinksPix(srv.Address(), ""));
}

TEST_CASE("HinksPix2 constructor with an info document lacking Type or Controller should not throw", "[vendors2][hinkspix2][http][!mayfail]") {
    // HinksPix.cpp constructor: data.at("Type") / data.at("Controller") throw out_of_range and _connected is already true
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = R"({"BD1":"S"})";
        return r;
    });
    CHECK_NOTHROW(HinksPix(srv.Address(), ""));
}

TEST_CASE("HinksPix2 constructor with a numeric MaxU should not throw", "[vendors2][hinkspix2][http][!mayfail]") {
    // HinksPix.cpp constructor: MaxU is read with get<std::string>() so a controller that reports a number throws
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = InfoJson(R"(,"MaxU":96)");
        return r;
    });
    CHECK_NOTHROW(HinksPix(srv.Address(), ""));
}

TEST_CASE("HinksPix2 InitExpansionBoardData fills outputs from the controller list", "[vendors2][hinkspix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/Xlights_Board_Port_Config.cgi") {
            std::string list = "{\"LIST\":[";
            for (int i = 1; i <= 16; ++i) {
                if (i > 1) list += ",";
                list += "{\"V\":\"" + std::to_string(i) + ",1,1," + std::to_string(i) + ",1,0,0,0,100,1\"}";
            }
            resp.body = list + "]}";
        }
        return resp;
    });
    auto h = MakeDisconnectedHinks();
    h->_ip = srv.Address();
    h->_curl = curl_easy_init();
    h->InitControllerOutputData(true, 100);
    h->InitExpansionBoardData(1, 1, 16);
    CHECK(h->_pixelOutputs[0].pixels == 1);
    CHECK(h->_pixelOutputs[15].pixels == 16);
    CHECK(h->_pixelOutputs[16].pixels == 0);
}

TEST_CASE("HinksPix2 InitExpansionBoardData ignores a list of the wrong length", "[vendors2][hinkspix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse resp;
        resp.body = R"({"LIST":[{"V":"1,1,1,5,1,0,0,0,100,1"}]})";
        return resp;
    });
    auto h = MakeDisconnectedHinks();
    h->_ip = srv.Address();
    h->_curl = curl_easy_init();
    h->InitControllerOutputData(true, 100);
    h->InitExpansionBoardData(1, 1, 16);
    CHECK(h->_pixelOutputs[0].pixels == 0);
}

TEST_CASE("HinksPix2 InitExpansionBoardData with list members that are not objects should not throw", "[vendors2][hinkspix2][http][!mayfail]") {
    // HinksPix.cpp InitExpansionBoardData: ports.at(i)["V"].get<std::string>() throws when an entry has no V or is not an object
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse resp;
        std::string list = "{\"LIST\":[";
        for (int i = 0; i < 16; ++i) list += std::string(i ? "," : "") + "{}";
        resp.body = list + "]}";
        return resp;
    });
    auto h = MakeDisconnectedHinks();
    h->_ip = srv.Address();
    h->_curl = curl_easy_init();
    h->InitControllerOutputData(true, 100);
    CHECK_NOTHROW(h->InitExpansionBoardData(1, 1, 16));
}

TEST_CASE("HinksPix2 InitSerialData reads the mode document unless in full control", "[vendors2][hinkspix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse resp;
        resp.body = R"({"CMD":"DATA_MODE","DMX_ACTIVE":1,"DMX_UNIV":9,"DMX_START":1,"DMX_CHAN_CNT":16,"DDP_DMX_ACTIVE":0,"DDP_DMX_START":1,"DDP_DMX_CHAN_CNT":16})";
        return resp;
    });
    auto h = MakeDisconnectedHinks();
    h->_ip = srv.Address();
    h->_curl = curl_easy_init();
    h->_controllerType = "H";
    auto full = h->InitSerialData(true);
    CHECK(full->e131Universe == 1);
    CHECK(srv.Requests().empty());
    auto live = h->InitSerialData(false);
    CHECK(live->e131Universe == 9);
    CHECK(live->e131Enabled);
}

TEST_CASE("HinksPix2 InitSerialData skips EasyLights controllers", "[vendors2][hinkspix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    auto h = MakeDisconnectedHinks();
    h->_ip = srv.Address();
    h->_curl = curl_easy_init();
    h->_controllerType = "E";
    auto s = h->InitSerialData(false);
    CHECK(s != nullptr);
    CHECK(srv.Requests().empty());
}

TEST_CASE("HinksPix2 InitSerialData with a reply lacking CMD keeps the defaults", "[vendors2][hinkspix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse resp;
        resp.body = R"({"x":1})";
        return resp;
    });
    auto h = MakeDisconnectedHinks();
    h->_ip = srv.Address();
    h->_curl = curl_easy_init();
    h->_controllerType = "H";
    auto s = h->InitSerialData(false);
    CHECK(s->e131Universe == 1);
}

TEST_CASE("HinksPix2 GetControllerDataJSON parses or fails", "[vendors2][hinkspix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/good") resp.body = R"({"a":1})";
        if (r.path == "/bad") resp.body = "{";
        return resp;
    });
    auto h = MakeDisconnectedHinks();
    h->_ip = srv.Address();
    h->_curl = curl_easy_init();
    nlohmann::json v;
    CHECK(h->GetControllerDataJSON("/good", v, ""));
    CHECK(v["a"] == 1);
    CHECK_FALSE(h->GetControllerDataJSON("/bad", v, ""));
    CHECK_FALSE(h->GetControllerDataJSON("/none", v, ""));
}

TEST_CASE("HinksPix2 requests without a curl handle return nothing", "[vendors2][hinkspix2]") {
    auto h = MakeDisconnectedHinks();
    curl_easy_cleanup(h->_curl);
    h->_curl = nullptr;
    CHECK(h->GetJSONControllerData("/x", "").empty());
    nlohmann::json v;
    CHECK_FALSE(h->GetControllerDataJSON("/x", v, ""));
    h->PostToControllerNoResponse("/x", "d");
}

#endif
