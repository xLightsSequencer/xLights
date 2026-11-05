// The handlers are reached through ExposeVendors.h and a loopback server, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "support/ExposeVendors.h"
#include "controllers/J1Sys.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"

// J1SysPixelOutput / J1SysSerialOutput are only defined inside J1Sys.cpp; the data members are repeated here.
struct J1SysPixelOutput {
    char port;
    bool active = false;
    char protocol;
    int speed = -1;
    int universe = -1;
    int startChannel = -1;
    int pixels = -1;
};

struct J1SysSerialOutput {
    char port;
    bool active = false;
    char protocol;
    int speed = -1;
    int universe = -1;
};

namespace {
J1Sys MakeJ1() {
    VendorsTestInit();
    return J1Sys("127.0.0.1:1", "");
}

std::string Page(const std::string& model, const std::string& version) {
    return "<html><script>document.getElementById('titleRight').innerHTML = '" + model + "\";</script>"
           "<table><tr><td><b>App Version:</b></td><td>&nbsp;</td><td>" + version + "</td></tr></table></html>";
}
} // namespace

TEST_CASE("J1Sys2 string protocol encoding", "[vendors2][j1sys2]") {
    J1Sys j = MakeJ1();
    auto row = GENERATE(table<std::string, char>({ { "ws2811", '4' }, { "WS2811", '4' }, { "tm180x", '2' }, { "tm18xx", '2' }, { "TM18XX", '2' }, { "ws2801", '1' },
                                                   { "tls3001", '3' }, { "lpd6803", '0' }, { "apa102", (char)-1 }, { "", (char)-1 }, { "dmx", (char)-1 } }));
    CHECK(j.EncodeStringPortProtocol(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("J1Sys2 serial protocol encoding", "[vendors2][j1sys2]") {
    J1Sys j = MakeJ1();
    CHECK(j.EncodeSerialPortProtocol("dmx") == 'D');
    CHECK(j.EncodeSerialPortProtocol("DMX") == 'D');
    CHECK(j.EncodeSerialPortProtocol("renard") == 'R');
    CHECK(j.EncodeSerialPortProtocol("Renard") == 'R');
    CHECK(j.EncodeSerialPortProtocol("pixelnet") == (char)-1);
    CHECK(j.EncodeSerialPortProtocol("") == (char)-1);
}

TEST_CASE("J1Sys2 protocol speeds", "[vendors2][j1sys2]") {
    J1Sys j = MakeJ1();
    auto row = GENERATE(table<std::string, int>({ { "ws2811", 3200 }, { "WS2811", 3200 }, { "ws2801", 750 }, { "dmx", 5 }, { "renard", 3 }, { "tm18xx", 0 }, { "", 0 }, { "lpd6803", 0 } }));
    CHECK(j.DecodeProtocolSpeed(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("J1Sys2 bank size depends on the model", "[vendors2][j1sys2]") {
    J1Sys j = MakeJ1();
    j._outputs = 2;
    CHECK(j.GetBankSize() == 4);
    for (int o : { 0, 1, 12, -1, 100 }) {
        j._outputs = o;
        CHECK(j.GetBankSize() == 1);
    }
}

TEST_CASE("J1Sys2 BuildStringPort formats the query", "[vendors2][j1sys2]") {
    J1Sys j = MakeJ1();
    CHECK(j.BuildStringPort(true, 0, '4', 3200, 1, 5, 50) == "sAA=1&sTA=4&sBA=3200&sUA=5&sSA=1&sCA=50");
    CHECK(j.BuildStringPort(false, 2, '1', 750, 101, 9, 0) == "sAC=0&sTC=1&sBC=750&sUC=9&sSC=101&sCC=0");
    CHECK(j.BuildStringPort(true, 11, '2', 0, 0, 0, 0).substr(0, 6) == "sAL=1&");
}

TEST_CASE("J1Sys2 BuildStringPort passes negative and large numbers through", "[vendors2][j1sys2]") {
    J1Sys j = MakeJ1();
    CHECK(j.BuildStringPort(true, 0, '4', -1, -1, -1, -1) == "sAA=1&sTA=4&sBA=-1&sUA=-1&sSA=-1&sCA=-1");
    CHECK(j.BuildStringPort(true, 0, '4', 2147483647, 1, 1, 1).find("sBA=2147483647") != std::string::npos);
}

TEST_CASE("J1Sys2 BuildSerialPort formats the query", "[vendors2][j1sys2]") {
    J1Sys j = MakeJ1();
    CHECK(j.BuildSerialPort(true, 1, 'D', 5, 3) == "pA1=1&pP1=D&pB1=5&pU1=3");
    CHECK(j.BuildSerialPort(false, 2, 'R', 3, 0) == "pA2=0&pP2=R&pB2=3&pU2=0");
}

TEST_CASE("J1Sys2 disconnected state", "[vendors2][j1sys2]") {
    J1Sys j = MakeJ1();
    CHECK_FALSE(j.IsConnected());
    CHECK(j.UsesHTTP());
    CHECK(j._username == "admin");
    CHECK(j._password.empty());
    CHECK(j._outputs == 0);
}

TEST_CASE("J1Sys2 constructor parses version and model", "[vendors2][j1sys2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.contentType = "text/html";
        r.body = Page("ECG-P12S", "3.3");
        return r;
    });
    J1Sys j(srv.Address(), "");
    CHECK(j.IsConnected());
    CHECK(j.GetVersion() == "3.3");
    CHECK(j.GetModel() == "ECG-P12S");
    CHECK(j._outputs == 12);
}

TEST_CASE("J1Sys2 constructor reads the output count from the model name", "[vendors2][j1sys2][http]") {
    VendorsTestInit();
    auto row = GENERATE(table<std::string, int>({ { "ECG-P2", 2 }, { "ECG-P12R", 12 }, { "ECG-P12D", 12 }, { "ECG-PX", 0 } }));
    FakeHttpServer srv([&](const FakeRequest&) {
        FakeResponse r;
        r.body = Page(std::get<0>(row), "2.9b");
        return r;
    });
    J1Sys j(srv.Address(), "");
    CHECK(j.IsConnected());
    CHECK(j._outputs == std::get<1>(row));
}

TEST_CASE("J1Sys2 constructor with an absurd output count", "[vendors2][j1sys2][http][!mayfail]") {
    // J1Sys.cpp constructor: (int)strtol(...) truncates a number larger than INT_MAX instead of clamping or rejecting it
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = Page("ECG-P99999999999", "1");
        return r;
    });
    J1Sys j(srv.Address(), "");
    CHECK(j._outputs >= 0);
    CHECK(j._outputs == 99999999999LL);
}

TEST_CASE("J1Sys2 constructor tolerates a page without version or model", "[vendors2][j1sys2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "<html>hello</html>";
        return r;
    });
    J1Sys j(srv.Address(), "");
    CHECK(j.IsConnected());
    CHECK(j.GetVersion().empty());
    CHECK(j.GetModel().empty());
    CHECK(j._outputs == 0);
}

TEST_CASE("J1Sys2 ReadCurrentConfig reads each output of a single bank controller", "[vendors2][j1sys2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/protect/stringConfig.htm") {
            resp.body =
                "<input name=sAA type=checkbox checked><select name=sTA><option value=0>LPD6803</option><option value=4 selected>WS2811</option></select>#"
                "<input name=sBA value=\"3200\"><input name=sUA value=\"12\"><input name=sSA value=\"1\"><input name=sCA value=\"50\">"
                "<input name=sAB type=checkbox><select name=sTB><option value=1 selected>WS2801</option></select>#"
                "<input name=sBB value=\"750\"><input name=sUB value=\"13\"><input name=sSB value=\"101\"><input name=sCB value=\"7\">";
        }
        return resp;
    });
    J1Sys j = MakeJ1();
    j._ip = srv.Address();
    j._outputs = 12;
    std::vector<J1SysPixelOutput> out(2);
    j.ReadCurrentConfig(out);
    CHECK(out[0].active);
    CHECK(out[0].protocol == '4');
    CHECK(out[0].speed == 3200);
    CHECK(out[0].universe == 12);
    CHECK(out[0].startChannel == 1);
    CHECK(out[0].pixels == 50);
    CHECK_FALSE(out[1].active);
    CHECK(out[1].protocol == '1');
    CHECK(out[1].speed == 750);
    CHECK(out[1].universe == 13);
    CHECK(out[1].startChannel == 101);
    CHECK(out[1].pixels == 7);
}

TEST_CASE("J1Sys2 ReadCurrentConfig copies protocol and speed within a bank", "[vendors2][j1sys2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse resp;
        resp.body = "<input name=sAA checked><select name=sTA><option selected>WS2801</option></select><input name=sBA value=\"750\">"
                    "<input name=sUB value=\"4\">";
        return resp;
    });
    J1Sys j = MakeJ1();
    j._ip = srv.Address();
    j._outputs = 2;
    std::vector<J1SysPixelOutput> out(8);
    j.ReadCurrentConfig(out);
    for (int i = 1; i < 4; ++i) {
        CHECK(out[i].protocol == out[0].protocol);
        CHECK(out[i].speed == 750);
    }
    CHECK(out[1].universe == 4);
    CHECK(out[2].universe == -1);
    // the second bank takes its own protocol, which is missing from the page
    CHECK(out[4].speed == -1);
}

TEST_CASE("J1Sys2 ReadCurrentConfig leaves defaults for an empty page", "[vendors2][j1sys2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    J1Sys j = MakeJ1();
    j._ip = srv.Address();
    std::vector<J1SysPixelOutput> out(3);
    j.ReadCurrentConfig(out);
    for (auto const& o : out) {
        CHECK(o.port == 0);
        CHECK_FALSE(o.active);
        CHECK(o.universe == -1);
    }
}

TEST_CASE("J1Sys2 ReadCurrentConfig with garbage fields yields zeros", "[vendors2][j1sys2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse resp;
        resp.body = "<input name=sUA value=\"\"><input name=sSA value=\"99999999999999999999\"><input name=sCA value=\"-5\">";
        return resp;
    });
    J1Sys j = MakeJ1();
    j._ip = srv.Address();
    std::vector<J1SysPixelOutput> out(1);
    j.ReadCurrentConfig(out);
    CHECK(out[0].universe == 0);
    CHECK(out[0].startChannel == -1);
    CHECK(out[0].pixels == 0);
}

TEST_CASE("J1Sys2 ReadCurrentConfig with an empty vector does nothing", "[vendors2][j1sys2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    J1Sys j = MakeJ1();
    j._ip = srv.Address();
    std::vector<J1SysPixelOutput> out;
    j.ReadCurrentConfig(out);
    CHECK(out.empty());
}

TEST_CASE("J1Sys2 ReadCurrentSerialConfig parses serial ports", "[vendors2][j1sys2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/protect/portConfig.htm") {
            resp.body = "<input name=pA1 type=checkbox checked><select name=pP1><option value=\"D\" selected>DMX</option></select>"
                        "<select name=pB1><option value=\"5\" selected>x</option></select><input name=pU1 value=\"21\">"
                        "<input name=pA2 type=checkbox><select name=pP2><option value=\"R\" selected>Renard</option></select>"
                        "<select name=pB2><option value=\"3\" selected>x</option></select><input name=pU2 value=\"22\">";
        }
        return resp;
    });
    J1Sys j = MakeJ1();
    j._ip = srv.Address();
    std::vector<J1SysSerialOutput> out(2);
    j.ReadCurrentSerialConfig(out);
    CHECK(out[0].active);
    CHECK(out[0].protocol == 'D');
    CHECK(out[0].speed == 5);
    CHECK(out[0].universe == 21);
    CHECK_FALSE(out[1].active);
    CHECK(out[1].protocol == 'R');
    CHECK(out[1].speed == 3);
    CHECK(out[1].universe == 22);
}

TEST_CASE("J1Sys2 ReadCurrentSerialConfig on an unreachable controller changes nothing", "[vendors2][j1sys2]") {
    J1Sys j = MakeJ1();
    std::vector<J1SysSerialOutput> out(2);
    j.ReadCurrentSerialConfig(out);
    CHECK(out[0].speed == -1);
    CHECK(out[1].universe == -1);
}

TEST_CASE("J1Sys2 ResetSerialOutputs only posts for the 12 output model", "[vendors2][j1sys2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    J1Sys j = MakeJ1();
    j._ip = srv.Address();
    j._outputs = 2;
    j.ResetSerialOutputs();
    CHECK(srv.Requests().empty());
    j._outputs = 12;
    j.ResetSerialOutputs();
    REQUIRE(srv.Requests().size() == 1);
    CHECK(srv.Requests()[0].path == "/protect/portConfig.htm");
}

TEST_CASE("J1Sys2 ResetStringOutputs and Reboot hit the expected urls", "[vendors2][j1sys2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    J1Sys j = MakeJ1();
    j._ip = srv.Address();
    j.ResetStringOutputs();
    j.Reboot();
    auto reqs = srv.Requests();
    REQUIRE(reqs.size() == 2);
    CHECK(reqs[0].path == "/protect/stringConfig.htm");
    CHECK(reqs[0].method == "POST");
    CHECK(reqs[1].path == "/protect/reboot.htm?");
}

TEST_CASE("J1Sys2 ReadCurrentConfig with more than 26 outputs uses non letter port ids", "[vendors2][j1sys2][http][!mayfail]") {
    // J1Sys.cpp ReadCurrentConfig: (char)(i + 65) is used as a regex fragment, so port 27+ becomes '[' '\\' ']' ... and can build an invalid regex
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    srv.ok();
    FakeHttpServer srv2([](const FakeRequest&) {
        FakeResponse r;
        r.body = "x";
        return r;
    });
    J1Sys j = MakeJ1();
    j._ip = srv2.Address();
    std::vector<J1SysPixelOutput> out(40);
    CHECK_NOTHROW(j.ReadCurrentConfig(out));
}

#endif
