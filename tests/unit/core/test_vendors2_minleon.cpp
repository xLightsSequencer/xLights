// The handlers are reached through ExposeVendors.h and a loopback server, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "support/ExposeVendors.h"
#include "controllers/Minleon.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"

#include "outputs/Output.h"

// MinleonString is only defined inside Minleon.cpp. The data members are repeated here, in the same order, so
// the tests can read and build ports. Constructors are left implicit; the json one is exercised through ParseStringPorts.
class MinleonString {
public:
    int _port = -1;
    int _tees = 0;
    bool _reverse = false;
    int _nodes = 0;
    int _startChannel = 0;
};

namespace {
std::unique_ptr<Minleon> MakeDisconnectedMinleon() {
    VendorsTestInit();
    return std::make_unique<Minleon>("127.0.0.1:1", "", "");
}

MinleonString* Port(int port, int tees, bool rev, int nodes, int start) {
    auto* s = new MinleonString();
    s->_port = port;
    s->_tees = tees;
    s->_reverse = rev;
    s->_nodes = nodes;
    s->_startChannel = start;
    return s;
}

void Free(std::vector<MinleonString*>& v) {
    for (auto* p : v) delete p;
    v.clear();
}

const char* kConfig = R"({"config":{"nm":"255.255.255.0","gw":"10.0.0.1","t0h":350,"t1h":"650","tbit":1250.7,"tres":280,"conv":"3","protocol":"E1.31","universe":7,"chip":6,"nports":2,"rpt":2,
"ports":[{"p":1,"ts":"2","l":"50","rev":1,"ss":"1"},{"p":2,"ts":3,"l":60,"rev":0,"ss":101}]}})";
} // namespace

TEST_CASE("Minleon2 SetTimingsFromProtocol table", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    struct R {
        const char* proto;
        int t0h, t1h, tbit, tres;
    };
    const R rows[] = { { "ws2811", 350, 650, 1250, 280 }, { "ws2812b", 400, 800, 1250, 50 }, { "ws2818", 400, 800, 1250, 300 }, { "sk6812", 300, 600, 1250, 80 },
                       { "gs820x", 310, 930, 1250, 300 }, { "gs8206/8", 310, 930, 1250, 300 }, { "gs8206", 310, 930, 1250, 300 }, { "gs8208", 310, 930, 1250, 300 },
                       { "ucs2903", 250, 1000, 1250, 24 }, { "ucs2904", 400, 800, 1250, 30 }, { "tm18xx", 500, 1000, 1500, 10 }, { "tm1804", 500, 1000, 1500, 10 },
                       { "sm16703", 300, 900, 1250, 80 }, { "apa104", 400, 800, 1250, 50 }, { "ucs8903", 400, 850, 1260, 100 }, { "rgb+", 400, 850, 1260, 1000 },
                       { "rgbw+", 400, 850, 1260, 1000 }, { "rgb+2", 350, 700, 1250, 250 }, { "rgbw+2", 400, 800, 1250, 250 }, { "rm2021", 300, 900, 1250, 200 },
                       { "", 400, 850, 1260, 100 }, { "unknown", 400, 850, 1260, 100 }, { "WS2811", 400, 850, 1260, 100 } };
    for (auto const& r : rows) {
        m->_protocol = r.proto;
        m->SetTimingsFromProtocol();
        INFO(r.proto);
        CHECK(m->_t0h == r.t0h);
        CHECK(m->_t1h == r.t1h);
        CHECK(m->_tbit == r.tbit);
        CHECK(m->_tres == r.tres);
    }
}

TEST_CASE("Minleon2 max pixels per port", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    struct R {
        const char* chip;
        int p8, p16;
    };
    const R rows[] = { { "rgb+", 460, 230 }, { "rgbw+", 345, 172 }, { "rgb+2", 613, 306 }, { "tls3001", 613, 460 }, { "rgbw+2", 460, 229 }, { "ucs2904", 690, 460 },
                       { "ws2811", 920, 460 }, { "", 920, 460 }, { "RGB+", 920, 460 } };
    for (auto const& r : rows) {
        INFO(r.chip);
        CHECK(m->GetMax8PortPixels(r.chip) == r.p8);
        CHECK(m->GetMax16PortPixels(r.chip) == r.p16);
    }
}

TEST_CASE("Minleon2 ConvForProtocol remaps legacy chip codes", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    auto row = GENERATE(table<std::string, std::string>({ { "1", "0" }, { "17", "13" }, { "2", "1" }, { "12", "10" }, { "15", "12" }, { "3", "KEEP" }, { "", "KEEP" },
                                                         { "0", "KEEP" }, { "ws2811", "KEEP" } }));
    CHECK(m->ConvForProtocol(std::get<0>(row), "KEEP") == std::get<1>(row));
}

TEST_CASE("Minleon2 string port protocol encode and decode", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    CHECK(m->EncodeStringPortProtocol("rgb+") == 1);
    CHECK(m->EncodeStringPortProtocol("rgbw+") == 2);
    CHECK(m->EncodeStringPortProtocol("sk6812") == 3);
    CHECK(m->EncodeStringPortProtocol("ws2812b") == 4);
    CHECK(m->EncodeStringPortProtocol("ws2811") == 6); // 6 and 13 both name ws2811, the lowest wins
    CHECK(m->EncodeStringPortProtocol("gs8206/8") == 7);
    CHECK(m->EncodeStringPortProtocol("rgbw+2") == 17);
    CHECK(m->EncodeStringPortProtocol("nonsense") == 6);
    CHECK(m->EncodeStringPortProtocol("") == 6);
    CHECK(m->EncodeStringPortProtocol("RGB+") == 6);
    CHECK(m->DecodeStringPortProtocol(1) == "rgb+");
    CHECK(m->DecodeStringPortProtocol(13) == "ws2811");
    CHECK(m->DecodeStringPortProtocol(14) == "rm2021");
    CHECK(m->DecodeStringPortProtocol(0) == "ws2811");
    CHECK(m->DecodeStringPortProtocol(5) == "ws2811");
    CHECK(m->DecodeStringPortProtocol(18) == "ws2811");
    CHECK(m->DecodeStringPortProtocol(-1) == "ws2811");
}

TEST_CASE("Minleon2 string port protocol decode then encode lands on a valid code", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    for (int code = 1; code <= 17; ++code) {
        if (code == 5) continue;
        INFO("code " << code);
        int back = m->EncodeStringPortProtocol(m->DecodeStringPortProtocol(code));
        CHECK(m->DecodeStringPortProtocol(back) == m->DecodeStringPortProtocol(code));
    }
}

TEST_CASE("Minleon2 input protocol encode and decode", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    CHECK(m->EncodeInputProtocol(OUTPUT_E131) == 2);
    CHECK(m->EncodeInputProtocol(OUTPUT_ARTNET) == 1);
    CHECK(m->EncodeInputProtocol(OUTPUT_DDP) == 0);
    CHECK(m->EncodeInputProtocol("") == -1);
    CHECK(m->EncodeInputProtocol("e131") == -1);
    CHECK(m->DecodeInputProtocol(0) == OUTPUT_DDP);
    CHECK(m->DecodeInputProtocol(1) == OUTPUT_ARTNET);
    CHECK(m->DecodeInputProtocol(2) == OUTPUT_E131);
    CHECK(m->DecodeInputProtocol(3).empty());
    CHECK(m->DecodeInputProtocol(-1).empty());
}

TEST_CASE("Minleon2 BuildStringPort pads the register numbers", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    auto* p = Port(0, 4, false, 100, 1);
    CHECK(m->BuildStringPort(p) == "&L025=4&I026=100&I027=1");
    delete p;
    p = Port(1, 2, true, 50, 511);
    CHECK(m->BuildStringPort(p) == "&L028=2&H028=0&I029=50&I030=511");
    delete p;
    p = Port(15, 0, false, 0, 0);
    CHECK(m->BuildStringPort(p) == "&L070=0&I071=0&I072=0");
    delete p;
}

TEST_CASE("Minleon2 GetPixelCount adds nodes for a port", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    std::vector<MinleonString*> v = { Port(0, 0, false, 10, 1), Port(0, 0, false, 20, 11), Port(1, 0, false, 5, 1) };
    CHECK(m->GetPixelCount(v, 0) == 30);
    CHECK(m->GetPixelCount(v, 1) == 5);
    CHECK(m->GetPixelCount(v, 2) == 0);
    CHECK(m->GetPixelCount(v, -1) == 0);
    CHECK(m->GetPixelCount({}, 0) == 0);
    Free(v);
}

TEST_CASE("Minleon2 GetMaxPixelPort is the highest port plus one", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    std::vector<MinleonString*> v = { Port(2, 0, false, 1, 1), Port(7, 0, false, 1, 1), Port(0, 0, false, 1, 1) };
    CHECK(m->GetMaxPixelPort(v) == 8);
    CHECK(m->GetMaxPixelPort({}) == 0);
    Free(v);
    std::vector<MinleonString*> neg = { Port(-1, 0, false, 1, 1) };
    CHECK(m->GetMaxPixelPort(neg) == 0);
    Free(neg);
}

TEST_CASE("Minleon2 FindPort returns the matching string", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    std::vector<MinleonString*> v = { Port(0, 0, false, 1, 1), Port(3, 0, false, 9, 1) };
    CHECK(m->FindPort(v, 3) == v[1]);
    CHECK(m->FindPort(v, 0) == v[0]);
    Free(v);
}

TEST_CASE("Minleon2 FindPort for a missing port asserts", "[vendors2][minleon2]") {
    // Minleon.cpp FindPort: assert(false) before returning nullptr
    auto m = MakeDisconnectedMinleon();
    std::vector<MinleonString*> v;
    CHECK(m->FindPort(v, 3) == nullptr);
}

TEST_CASE("Minleon2 InitialiseStrings fills the gaps and drops out of range ports", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    std::vector<MinleonString*> v = { Port(2, 1, false, 5, 1), Port(9, 1, false, 5, 1) };
    MinleonString* kept = v[0];
    MinleonString* dropped = v[1];
    m->InitialiseStrings(v, 4);
    REQUIRE(v.size() == 4);
    for (int i = 0; i < 4; ++i) {
        CHECK(v[i]->_port == i);
    }
    CHECK(v[2] == kept);
    CHECK(v[0]->_nodes == 0);
    CHECK_FALSE(v[1]->_reverse);
    delete dropped; // InitialiseStrings forgets (leaks) strings beyond max, so the test releases it
    Free(v);
}

TEST_CASE("Minleon2 InitialiseStrings with a zero max empties the list", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    std::vector<MinleonString*> v = { Port(0, 1, false, 5, 1) };
    MinleonString* orphan = v[0];
    m->InitialiseStrings(v, 0);
    CHECK(v.empty());
    delete orphan;
}

TEST_CASE("Minleon2 InitialiseStrings duplicates a port that appears twice", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    std::vector<MinleonString*> v = { Port(1, 1, false, 5, 1), Port(1, 2, false, 6, 1) };
    m->InitialiseStrings(v, 2);
    CHECK(v.size() == 3);
    Free(v);
}

TEST_CASE("Minleon2 ParseStringPorts reads the ddp and api shapes", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    std::vector<MinleonString*> v;
    auto j = nlohmann::json::parse(R"([{"port":"3","ts":"2","rev":1,"l":"40","ss":"7"},{"p":4,"ts":1,"l":5,"ss":"9"}])");
    m->ParseStringPorts(v, j);
    REQUIRE(v.size() == 2);
    CHECK(v[0]->_port == 2);
    CHECK(v[0]->_tees == 2);
    CHECK(v[0]->_reverse);
    CHECK(v[0]->_nodes == 40);
    CHECK(v[0]->_startChannel == 7);
    CHECK(v[1]->_port == 3);
    CHECK_FALSE(v[1]->_reverse);
    CHECK(v[1]->_startChannel == 9);
    m->ParseStringPorts(v, j); // replaces and frees the previous list
    CHECK(v.size() == 2);
    nlohmann::json empty = nlohmann::json::array();
    m->ParseStringPorts(v, empty);
    CHECK(v.empty());
}

TEST_CASE("Minleon2 ParseStringPorts of a null document gives nothing", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    std::vector<MinleonString*> v = { Port(0, 0, false, 0, 0) };
    nlohmann::json null;
    m->ParseStringPorts(v, null);
    CHECK(v.empty());
}

TEST_CASE("Minleon2 ParseStringPorts with a string rev should not throw", "[vendors2][minleon2]") {
    // Minleon.cpp MinleonString(json): rev is read with get<int>() while every other member accepts a string, but the NDB+ upload
    // itself posts "rev" as a string, so a round trip through the controller throws
    auto m = MakeDisconnectedMinleon();
    std::vector<MinleonString*> v;
    auto j = nlohmann::json::parse(R"([{"p":1,"ts":"1","l":"1","rev":"1","ss":"1"}])");
    CHECK_NOTHROW(m->ParseStringPorts(v, j));
    Free(v);
}

TEST_CASE("Minleon2 ParseStringPorts with missing members should not throw", "[vendors2][minleon2]") {
    // Minleon.cpp MinleonString(json): val["ts"] / ["l"] / ["ss"] on a missing key is null and get<int>() throws type_error
    auto m = MakeDisconnectedMinleon();
    std::vector<MinleonString*> v;
    auto j = nlohmann::json::parse(R"([{"p":1}])");
    CHECK_NOTHROW(m->ParseStringPorts(v, j));
    Free(v);
}

TEST_CASE("Minleon2 ParseNDBHTML finds the value of a numbered input", "[vendors2][minleon2]") {
    // Minleon.cpp ParseNDBHTML: the pattern is built as name="I" + "I004", i.e. name="II004", so the real name="I004" input is never matched
    auto m = MakeDisconnectedMinleon();
    std::string html = "<input name=\"I004\"value=\"255\"><input name=\"I005\"value=\"128\">";
    CHECK(m->ParseNDBHTML(html, 4) == "255");
    CHECK(m->ParseNDBHTML(html, 5) == "128");
}

TEST_CASE("Minleon2 ParseNDBHTML returns empty for no match", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    CHECK(m->ParseNDBHTML("", 4).empty());
    CHECK(m->ParseNDBHTML("<html></html>", 255).empty());
    CHECK(m->ParseNDBHTML(std::string(100000, 'x'), 7).empty());
}

TEST_CASE("Minleon2 disconnected defaults", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    CHECK_FALSE(m->IsConnected());
    CHECK(m->UsesHTTP());
    CHECK(m->_chip == "ws2811");
    CHECK(m->_startUniverse == -1);
    CHECK(m->_stringPorts.empty());
    CHECK_FALSE(m->_ndbPro);
    CHECK_FALSE(m->_ndbOrig);
}

TEST_CASE("Minleon2 constructor parses a 01.html style config", "[vendors2][minleon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/") resp.body = "<html><script>x</script>NDB+ v2.2\r\n<p>";
        if (r.path == "/01.html") resp.body = kConfig;
        return resp;
    });
    Minleon m(srv.Address(), "", "");
    CHECK(m.IsConnected());
    CHECK_FALSE(m._ndbPro);
    CHECK(m._version.find("NDB+ v2.2") != std::string::npos);
    CHECK(m._nm == "255.255.255.0");
    CHECK(m._gw == "10.0.0.1");
    CHECK(m._t0h == 350);
    CHECK(m._t1h == 650);
    CHECK(m._tbit == 1250);
    CHECK(m._tres == 280);
    CHECK(m._conv == "3");
    CHECK(m._protocol == OUTPUT_E131);
    CHECK(m._startUniverse == 7);
    CHECK(m._chip == "ws2811");
    CHECK(m._ports == 2);
    CHECK(m._grouping == 2);
    REQUIRE(m._stringPorts.size() == 2);
    CHECK(m._stringPorts[0]->_port == 0);
    CHECK(m._stringPorts[0]->_nodes == 50);
}

TEST_CASE("Minleon2 constructor falls back to api config and flags NDB Pro", "[vendors2][minleon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/01.html") resp.body = "This URI does not exist";
        if (r.path == "/api/config") resp.body = kConfig;
        return resp;
    });
    Minleon m(srv.Address(), "", "");
    CHECK(m.IsConnected());
    CHECK(m._ndbPro);
}

TEST_CASE("Minleon2 constructor maps protocol numbers and DDP skips the universe", "[vendors2][minleon2][http]") {
    VendorsTestInit();
    auto row = GENERATE(table<std::string, std::string>({ { "0", OUTPUT_DDP }, { "1", OUTPUT_ARTNET }, { "2", OUTPUT_E131 }, { "Art-Net", OUTPUT_ARTNET },
                                                         { "DDP", OUTPUT_DDP }, { "Mystery", "Mystery" } }));
    FakeHttpServer srv([&](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/01.html")
            resp.body = "{\"config\":{\"conv\":1,\"protocol\":\"" + std::get<0>(row) + "\",\"universe\":9,\"chip\":4,\"ports\":[]}}";
        return resp;
    });
    Minleon m(srv.Address(), "", "");
    CHECK(m._protocol == std::get<1>(row));
    CHECK(m._startUniverse == (std::get<1>(row) == OUTPUT_DDP ? -1 : 9));
    CHECK(m._chip == "ws2812b");
    CHECK(m._conv == "1");
    CHECK(m._grouping == 1);
}

TEST_CASE("Minleon2 constructor with nothing reachable is disconnected", "[vendors2][minleon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    Minleon m(srv.Address(), "", "");
    CHECK_FALSE(m.IsConnected());
    CHECK(m._stringPorts.empty());
}

TEST_CASE("Minleon2 constructor with an invalid config document should not throw", "[vendors2][minleon2][http]") {
    // Minleon.cpp constructor: nlohmann::json::parse(configJSON) is not guarded, so html/text where JSON is expected throws out of the constructor
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/01.html") resp.body = "<html>not json</html>";
        return resp;
    });
    CHECK_NOTHROW(Minleon(srv.Address(), "", ""));
}

TEST_CASE("Minleon2 constructor with a config lacking conv should not throw", "[vendors2][minleon2][http]") {
    // Minleon.cpp constructor: val["config"]["conv"].get<int>() throws when conv is absent
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/01.html") resp.body = R"({"config":{"protocol":"DDP","ports":[]}})";
        return resp;
    });
    CHECK_NOTHROW(Minleon(srv.Address(), "", ""));
}

TEST_CASE("Minleon2 PostURL and Upload do nothing harmful against a closed port", "[vendors2][minleon2]") {
    auto m = MakeDisconnectedMinleon();
    m->_ip = "127.0.0.1:1";
    m->PostURL("/00.html", "op=Save");
    SUCCEED();
}

TEST_CASE("Minleon2 UploadNDPPlus builds the legacy form post", "[vendors2][minleon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    auto m = MakeDisconnectedMinleon();
    m->_ip = srv.Address();
    m->_protocol = OUTPUT_E131;
    m->_startUniverse = 4;
    m->_chip = "ws2811";
    m->_conv = "0";
    m->_grouping = 1;
    m->_nm = "255.255.255.0";
    m->_gw = "10.0.0.1";
    m->_stringPorts.push_back(Port(0, 2, true, 30, 1));
    m->UploadNDPPlus(true);
    auto reqs = srv.Requests();
    REQUIRE(reqs.size() == 2);
    CHECK(reqs[0].path == "/00.html");
    CHECK(reqs[0].body.find("I013=4") != std::string::npos);
    CHECK(reqs[0].body.find("I012=2") != std::string::npos);
    CHECK(reqs[0].body.find("I004=255") != std::string::npos);
    CHECK(reqs[0].body.find("H025=0") != std::string::npos);
    CHECK(reqs[0].body.find("L025=2") != std::string::npos);
    CHECK(reqs[0].body.find("I026=30") != std::string::npos);
    CHECK(reqs[0].body.find("op=Save") != std::string::npos);
    CHECK(reqs[1].body == "op=Reboot");
    Free(m->_stringPorts);
}

TEST_CASE("Minleon2 UploadNDPPlus with an ip that has fewer than four octets pads with zero", "[vendors2][minleon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    auto m = MakeDisconnectedMinleon();
    m->_ip = srv.Address(); // "127.0.0.1:PORT" splits into 4 parts, the last one carrying the port
    m->_protocol = OUTPUT_DDP;
    m->UploadNDPPlus(false);
    auto reqs = srv.Requests();
    REQUIRE(reqs.size() == 1);
    CHECK(reqs[0].body.find("I000=127") != std::string::npos);
    CHECK(reqs[0].body.find("I013=0") != std::string::npos);
    CHECK(reqs[0].body.find("I004=0&") != std::string::npos); // an empty netmask posts 0, not a blank field
}

TEST_CASE("Minleon2 UploadNDBPro posts a json document", "[vendors2][minleon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    auto m = MakeDisconnectedMinleon();
    m->_ip = srv.Address();
    m->_protocol = OUTPUT_ARTNET;
    m->_chip = "sk6812";
    m->_conv = "1";
    m->_startUniverse = 3;
    m->_ndbPro = true;
    m->_stringPorts.push_back(Port(0, 1, true, 10, 1));
    m->_stringPorts.push_back(Port(1, 2, false, 20, 11));
    m->UploadNDPPlus(false);
    auto reqs = srv.Requests();
    REQUIRE(reqs.size() == 1);
    CHECK(reqs[0].path == "/api/config");
    auto j = nlohmann::json::parse(reqs[0].body);
    CHECK(j["chip"] == "3");
    CHECK(j["conv"] == "1"); // the chip name is not a legacy numeric code so conv is unchanged
    CHECK(j["proto"] == "1");
    CHECK(j["univ"] == "3");
    CHECK(j["universe"] == "3");
    REQUIRE(j["ports"].size() == 2);
    CHECK(j["ports"][0]["rev"] == "1");
    CHECK(j["ports"][1]["l"] == "20");
    CHECK(j["ports"][1]["ss"] == "11");
    Free(m->_stringPorts);
}

#endif
