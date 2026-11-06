// FPP's universe/proxy/model/playlist configs: the universe output and input files,
// the proxy list, the overlay model, submodel and group maps, the display map and
// playlists, all against a loopback FPP.
#ifndef _WIN32

#include "test_fpp3_common.h"

#include <catch2/matchers/catch_matchers_string.hpp>

#include "models/ModelGroup.h"
#include "outputs/ArtNetOutput.h"
#include "outputs/DDPOutput.h"
#include "outputs/KinetOutput.h"
#include "support/TempDir.h"

using namespace fpp3;
using Catch::Matchers::ContainsSubstring;

namespace {

Controller* AddController(OutputManager& om, const std::string& xml) {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(xml.c_str()));
    Controller* c = Controller::Create(&om, doc.first_child(), "");
    REQUIRE(c != nullptr);
    om.AddController(c);
    om.SomethingChanged();
    return c;
}

nlohmann::json Universes(const nlohmann::json& file, const char* key = "channelOutputs") {
    return file[key][0]["universes"];
}

struct ModelShow {
    OutputManager om;
    ModelManager mm{ &om };
    ModelShow() { InitLoggers(); }
    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m != nullptr);
        return m;
    }
    Model* Line(const std::string& name, int nodes, const std::string& sc, const std::string& extra = "", const std::string& kids = "") {
        return Add("<model name=\"" + name + "\" DisplayAs=\"Single Line\" parm1=\"1\" parm2=\"" + std::to_string(nodes) + "\" parm3=\"1\" StringType=\"RGB Nodes\" StartChannel=\"" + sc +
                   "\" " + extra + ">" + kids + "</model>");
    }
    void Recalc() {
        om.SomethingChanged();
        mm.RecalcStartChannels();
    }
};

} // namespace

TEST_CASE("FPP universe file for unicast and multicast E1.31", "[fpp3][config][universes]") {
    InitLoggers();
    OutputManager om;
    auto* uni = AddController(om, R"(<Controller Id="1" Name="Uni" Type="Ethernet" Protocol="E131" IP="10.0.0.20" Priority="80" ActiveState="Active" SuppressDuplicates="1" Monitor="1">
        <network NetworkType="E131" ComPort="10.0.0.20" BaudRate="5" MaxChannels="510"/>
        <network NetworkType="E131" ComPort="10.0.0.20" BaudRate="6" MaxChannels="100"/></Controller>)");
    auto* mc = AddController(om, R"(<Controller Id="2" Name="Multi&amp;Cast" Type="Ethernet" Protocol="E131" IP="MULTICAST" ActiveState="Inactive" Monitor="1">
        <network NetworkType="E131" ComPort="MULTICAST" BaudRate="10" MaxChannels="510"/>
        <network NetworkType="E131" ComPort="MULTICAST" BaudRate="11" MaxChannels="510"/></Controller>)");
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    std::map<int, int> rngs;
    auto file = f->CreateUniverseFile(std::list<Controller*>{ uni, mc }, false, &rngs);
    auto root = file["channelOutputs"][0];
    CHECK(root["type"] == "universes");
    CHECK(root["enabled"] == 1);
    CHECK(root["timeout"] == 1000);
    CHECK(root["startChannel"] == 1);
    CHECK(root["channelCount"] == -1);
    auto u = root["universes"];
    REQUIRE(u.size() == 3);
    // different sized universes are listed one by one
    CHECK(u[0]["id"] == 5);
    CHECK(u[0]["active"] == 1);
    CHECK(u[0]["description"] == "Uni");
    CHECK(u[0]["startChannel"] == 1);
    CHECK(u[0]["channelCount"] == 510);
    CHECK(u[0]["type"] == 1);
    CHECK(u[0]["address"] == "10.0.0.20");
    CHECK(u[0]["priority"] == 80);
    CHECK(u[0]["deDuplicate"] == 1);
    CHECK(u[0]["monitor"] == 1);
    CHECK(u[0]["universeCount"] == 1);
    CHECK(u[1]["id"] == 6);
    CHECK(u[1]["startChannel"] == 511);
    CHECK(u[1]["channelCount"] == 100);
    // same sized multicast universes collapse into one entry with a count
    CHECK(u[2]["id"] == 10);
    CHECK(u[2]["type"] == 0);
    CHECK(u[2]["address"] == "");
    CHECK(u[2]["monitor"] == 0);
    CHECK(u[2]["active"] == 0);
    CHECK(u[2]["universeCount"] == 2);
    CHECK(u[2]["description"] == "Multi_Cast"); // & < > are replaced
    // only active controllers count towards the ranges
    CHECK(rngs.size() == 2);
    CHECK(rngs[1] == 510);
    CHECK(rngs[511] == 610);

    auto in = f->CreateUniverseFile(uni, true);
    CHECK(in.contains("channelOutputs")); // the single controller form always builds outputs
    auto input = f->CreateUniverseFile(std::list<Controller*>{ uni }, true);
    REQUIRE(input.contains("channelInputs"));
    auto iu = Universes(input, "channelInputs");
    CHECK(iu[0]["active"] == 1);
    CHECK(iu[0]["address"] == "");
}

TEST_CASE("FPP universe file for ArtNet, DDP, KiNet and Twinkly", "[fpp3][config][universes]") {
    InitLoggers();
    OutputManager om;
    auto* art = AddController(om, R"(<Controller Id="1" Name="Art" Type="Ethernet" Protocol="ArtNet" IP="10.0.0.21" ActiveState="Active">
        <network NetworkType="ArtNet" ComPort="10.0.0.21" BaudRate="1" MaxChannels="512"/>
        <network NetworkType="ArtNet" ComPort="10.0.0.21" BaudRate="2" MaxChannels="512"/></Controller>)");
    auto* ddp = AddController(om, R"(<Controller Id="3" Name="Ddp" Type="Ethernet" Protocol="DDP" IP="10.0.0.22" ActiveState="Active">
        <network NetworkType="DDP" ComPort="10.0.0.22" BaudRate="3" MaxChannels="900" KeepChannelNumbers="1"/></Controller>)");
    auto* kinet = AddController(om, R"(<Controller Id="4" Name="Kin" Type="Ethernet" Protocol="KINET" IP="10.0.0.23" ActiveState="Active">
        <network NetworkType="KINET" ComPort="10.0.0.23" BaudRate="1" MaxChannels="512" Version="2"/></Controller>)");
    auto* tw = AddController(om, R"(<Controller Id="5" Name="Tw" Type="Ethernet" Protocol="Twinkly" IP="10.0.0.24" ActiveState="Active">
        <network NetworkType="Twinkly" ComPort="10.0.0.24" BaudRate="1" MaxChannels="300"/></Controller>)");
    FppServer srv;

    auto f = MakeFpp(srv, 9, 5);
    auto u = Universes(f->CreateUniverseFile(std::list<Controller*>{ art, ddp, kinet, tw }, false));
    REQUIRE(u.size() == 4);
    CHECK(u[0]["type"] == 9); // unicast ArtNet on 9.5+
    CHECK(u[0]["universeCount"] == 2);
    CHECK(u[0]["address"] == "10.0.0.21");
    CHECK(u[1]["type"] == 4); // DDP keeping channel numbers
    CHECK(u[1]["address"] == "10.0.0.22");
    CHECK(u[1]["channelCount"] == 900);
    CHECK(u[2]["type"] == static_cast<KinetOutput*>(kinet->GetFirstOutput())->GetVersion() + 5);
    CHECK(u[2]["address"] == "10.0.0.23");
    CHECK(u[3]["type"] == 8);

    static_cast<ArtNetOutput*>(art->GetFirstOutput())->SetForceSourcePort(true);
    CHECK(Universes(f->CreateUniverseFile(std::list<Controller*>{ art }, false))[0]["type"] == 3);
    auto old = MakeFpp(srv, 9, 4);
    CHECK(Universes(old->CreateUniverseFile(std::list<Controller*>{ art }, false))[0]["type"] == 3);
    static_cast<DDPOutput*>(ddp->GetFirstOutput())->SetKeepChannelNumber(false);
    CHECK(Universes(f->CreateUniverseFile(std::list<Controller*>{ ddp }, false))[0]["type"] == 5);
    // a DDP input takes no universes
    CHECK(Universes(f->CreateUniverseFile(std::list<Controller*>{ ddp }, true), "channelInputs").empty());
}

TEST_CASE("FPP universe file stamps the controller's pacing on FPP 10", "[fpp3][config][universes]") {
    FppShow s("TestHat", OUTPUT_E131);
    s.ctl->GetFirstOutput()->SetUniverse(7);
    FppServer srv;
    auto f = MakeFpp(srv, 10);
    auto u = Universes(f->CreateUniverseFile(std::list<Controller*>{ s.ctl }, false));
    REQUIRE(u.size() == 1);
    CHECK(u[0]["pacingRate"] == 40);
    CHECK_FALSE(u[0].contains("_xlPacingAuthoritative"));
    s.ctl->SetFullxLightsControl(true);
    u = Universes(f->CreateUniverseFile(std::list<Controller*>{ s.ctl }, false));
    CHECK(u[0]["_xlPacingAuthoritative"] == true);
    auto nine = MakeFpp(srv, 9);
    CHECK_FALSE(Universes(nine->CreateUniverseFile(std::list<Controller*>{ s.ctl }, false))[0].contains("pacingRate"));
}

TEST_CASE("FPP UploadUDPOut keeps FPP's own keys and pacing", "[fpp3][config][universes]") {
    FppServer srv;
    srv.Get("/api/channel/output/universeOutputs", R"({"channelOutputs":[{"type":"universes","enabled":0,"interface":"eth0","threaded":1,"universes":[
        {"address":"10.0.0.30","pacingRate":20},{"address":"10.0.0.30","pacingRate":10},{"address":"10.0.0.31","pacingRate":0},
        {"address":"10.0.0.32","pacingRate":-1},{"address":"","pacingRate":5}]},{"type":"other","x":1}]})");
    auto f = MakeFpp(srv, 10);
    nlohmann::json udp = R"({"channelOutputs":[{"type":"universes","enabled":1,"universes":[
        {"address":"10.0.0.30","pacingRate":40},{"address":"10.0.0.31"},{"address":"10.0.0.33","pacingRate":40,"_xlPacingAuthoritative":true},{"address":""}]}]})"_json;
    CHECK_FALSE(f->UploadUDPOut(udp));
    auto posted = srv.LastJson("POST", "/api/channel/output/universeOutputs")["channelOutputs"][0];
    CHECK(posted["enabled"] == 1);
    CHECK(posted["interface"] == "eth0");
    CHECK(posted["threaded"] == 1);
    CHECK_FALSE(posted.contains("x"));
    auto u = posted["universes"];
    CHECK(u[0]["pacingRate"] == 10);  // the lowest real cap for that address
    CHECK(u[1]["pacingRate"] == 0);   // unlimited on the controller stays unlimited
    CHECK(u[2]["pacingRate"] == 40);  // authoritative
    CHECK_FALSE(u[2].contains("_xlPacingAuthoritative"));
    CHECK_FALSE(u[3].contains("pacingRate"));

    SECTION("before FPP 10 nothing about pacing is merged") {
        auto g = MakeFpp(srv, 9);
        g->UploadUDPOut(udp);
        auto p = srv.LastJson("POST", "/api/channel/output/universeOutputs")["channelOutputs"][0]["universes"];
        CHECK(p[0]["pacingRate"] == 40);
        CHECK_FALSE(p[2].contains("_xlPacingAuthoritative"));
    }
    SECTION("an unreadable config only strips the hint") {
        FppServer empty;
        auto g = MakeFpp(empty, 10);
        g->UploadUDPOut(udp);
        auto p = empty.LastJson("POST", "/api/channel/output/universeOutputs")["channelOutputs"][0];
        CHECK_FALSE(p.contains("interface"));
        CHECK(p["universes"][0]["pacingRate"] == 40);
        CHECK_FALSE(p["universes"][2].contains("_xlPacingAuthoritative"));
    }
}

TEST_CASE("FPP UploadUDPOutputsForProxy sends the controllers it proxies", "[fpp3][config][universes]") {
    InitLoggers();
    FppServer srv;
    OutputManager om;
    AddController(om, R"(<Controller Id="1" Name="Behind" Type="Ethernet" Protocol="E131" IP="198.51.100.2" FPPProxy=")" + srv.Address() +
                          R"(" ActiveState="Active"><network NetworkType="E131" ComPort="198.51.100.2" BaudRate="1" MaxChannels="510"/></Controller>)");
    AddController(om, R"(<Controller Id="2" Name="Other" Type="Ethernet" Protocol="E131" IP="198.51.100.3" ActiveState="Active"><network NetworkType="E131" ComPort="198.51.100.3" BaudRate="2" MaxChannels="510"/></Controller>)");
    auto f = MakeFpp(srv, 9);
    f->ranges = "2000-2099";
    CHECK_FALSE(f->UploadUDPOutputsForProxy(&om));
    auto u = srv.LastJson("POST", "/api/channel/output/universeOutputs")["channelOutputs"][0]["universes"];
    REQUIRE(u.size() == 1);
    CHECK(u[0]["description"] == "Behind");
    CHECK(f->ranges == "0-509,2000-2099");
}

TEST_CASE("FPP SetInputUniversesBridge", "[fpp3][config][universes]") {
    FppShow s("TestCape", OUTPUT_E131);
    s.ctl->GetFirstOutput()->SetUniverse(3);
    FppServer srv;
    SECTION("an E1.31 controller sends its universes as inputs") {
        srv.Get("/api/configfile/ci-universes.json", R"({"channelInputs":[{"enabled":1}]})");
        auto f = MakeFpp(srv, 9);
        CHECK_FALSE(f->SetInputUniversesBridge(s.ctl));
        auto u = Universes(srv.LastJson("POST", "/api/channel/output/universeInputs"), "channelInputs");
        REQUIRE(u.size() == 1);
        CHECK(u[0]["id"] == 3);
        CHECK_FALSE(f->restartNeeded);
    }
    SECTION("with inputs off a DDP controller still turns them on") {
        FppShow d;
        auto f = MakeFpp(srv, 9);
        f->SetInputUniversesBridge(d.ctl);
        CHECK(srv.Has("POST", "/api/channel/output/universeInputs"));
        CHECK(f->restartNeeded);
    }
    SECTION("with inputs on and a DDP controller nothing is sent") {
        srv.Get("/api/configfile/ci-universes.json", R"({"channelInputs":[{"enabled":1}]})");
        FppShow d;
        auto f = MakeFpp(srv, 9);
        f->SetInputUniversesBridge(d.ctl);
        CHECK_FALSE(srv.Has("POST", "/api/channel/output/universeInputs"));
    }
}

TEST_CASE("FPP SetInputUniverses authenticates then sends the inputs", "[fpp3][config][universes]") {
    FppShow s("TestCape", OUTPUT_E131);
    FppServer srv;
    srv.Get("/config.php", "x");
    srv.Get("/api/system/info", R"({"Version":"9.0"})");
    auto f = MakeFpp(srv, 0);
    f->sysInfoLoaded = false;
    CHECK(f->SetInputUniverses(s.ctl, nullptr));
    CHECK(srv.Has("POST", "/api/channel/output/universeInputs"));
}

TEST_CASE("FPP UploadControllerProxies on FPP 8 posts the merged list", "[fpp3][config][proxies]") {
    InitLoggers();
    FppServer srv;
    srv.Get("/api/proxies", R"([{"host":"198.51.100.9","description":"existing"},"198.51.100.2"])");
    OutputManager om;
    AddController(om, R"(<Controller Id="1" Name="Behind" Type="Ethernet" Protocol="DDP" IP="198.51.100.2" FPPProxy=")" + srv.Address() +
                          R"(" ActiveState="Active"><network NetworkType="DDP" ComPort="198.51.100.2" BaudRate="1" MaxChannels="510"/></Controller>)");
    AddController(om, R"(<Controller Id="2" Name="New" Type="Ethernet" Protocol="DDP" IP="198.51.100.3" FPPProxy=")" + srv.Address() +
                          R"(" ActiveState="Active"><network NetworkType="DDP" ComPort="198.51.100.3" BaudRate="2" MaxChannels="510"/></Controller>)");
    auto f = MakeFpp(srv, 8);
    CHECK_FALSE(f->UploadControllerProxies(&om));
    auto p = srv.LastJson("POST", "/api/proxies");
    REQUIRE(p.size() == 3);
    CHECK(p[0]["host"] == "198.51.100.9");
    CHECK(p[0]["description"] == "existing");
    CHECK(p[1]["host"] == "198.51.100.2");
    CHECK(p[1]["description"] == "");
    CHECK(p[2]["host"] == "198.51.100.3");
    CHECK(p[2]["description"] == "New");
}

// Proxies before FPP 8 post an empty body.
TEST_CASE("FPP UploadControllerProxies before FPP 8 adds each new proxy", "[fpp3]") {
    InitLoggers();
    FppServer srv;
    srv.Get("/api/proxies", R"(["198.51.100.2"])");
    OutputManager om;
    AddController(om, R"(<Controller Id="1" Name="Behind" Type="Ethernet" Protocol="DDP" IP="198.51.100.2" FPPProxy=")" + srv.Address() +
                          R"(" ActiveState="Active"><network NetworkType="DDP" ComPort="198.51.100.2" BaudRate="1" MaxChannels="510"/></Controller>)");
    AddController(om, R"(<Controller Id="2" Name="New" Type="Ethernet" Protocol="DDP" IP="198.51.100.3" FPPProxy=")" + srv.Address() +
                          R"(" ActiveState="Active"><network NetworkType="DDP" ComPort="198.51.100.3" BaudRate="2" MaxChannels="510"/></Controller>)");
    auto f = MakeFpp(srv, 7);
    f->UploadControllerProxies(&om);
    CHECK(srv.Has("POST", "/api/proxies/198.51.100.3"));
    CHECK_FALSE(srv.Has("POST", "/api/proxies/198.51.100.2"));
}

// Quotes in a name are left for the JSON writer to escape (once).
TEST_CASE("FPP universe description keeps quotes as they are", "[fpp3]") {
    InitLoggers();
    OutputManager om;
    auto* c = AddController(om, R"(<Controller Id="1" Name="Big &quot;Tree&quot;" Type="Ethernet" Protocol="E131" IP="10.0.0.20" ActiveState="Active">
        <network NetworkType="E131" ComPort="10.0.0.20" BaudRate="5" MaxChannels="510"/></Controller>)");
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    CHECK(Universes(f->CreateUniverseFile(std::list<Controller*>{ c }, false))[0]["description"] == "Big \"Tree\"");
}

TEST_CASE("FPP ValidateProxy asks the proxy for its list", "[fpp3][config][proxies]") {
    FppServer srv;
    srv.Get("/api/proxies", R"(["198.51.100.2"])");
    InitLoggers();
    CHECK(FPP::ValidateProxy("198.51.100.2", srv.Address()));
    CHECK_FALSE(FPP::ValidateProxy("198.51.100.7", srv.Address()));
}

TEST_CASE("FPP restart flag, restart and description", "[fpp3][config]") {
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    CHECK(f->SetRestartFlag());
    CHECK_FALSE(srv.Has("PUT", "/api/settings/restartFlag"));
    CHECK_FALSE(f->restartNeeded);
    f->SetRestartFlag(true);
    CHECK(f->restartNeeded);
    CHECK(srv.Count("PUT", "/api/settings/restartFlag") == 1);

    CHECK_FALSE(f->Restart(true));
    CHECK(srv.Has("GET", "/api/system/fppd/restart?quick=1"));
    CHECK_FALSE(f->restartNeeded);
    int before = (int)srv.Requests().size();
    f->Restart(true); // not needed any more
    CHECK((int)srv.Requests().size() == before);

    f->SetDescription("Yard");
    CHECK(srv.Sent("PUT", "/api/settings/HostDescription").back().body.size() > 0);
}

TEST_CASE("FPP vendor and model from an old controller type name", "[fpp3][config]") {
    CHECK(FPP::GetVendor("") == "");
    std::string v, m, var;
    Controller::ConvertOldTypeToVendorModel("F16V3", v, m, var);
    CHECK(FPP::GetVendor("F16V3") == v);
    CHECK(FPP::GetModel("F16V3") == m);
}

TEST_CASE("FPP overlay models for the models on its channels", "[fpp3][config][models]") {
    ModelShow s;
    s.Line("Arch One", 10, "1", "Dir=\"R\"");
    s.Add(R"(<model name="Mat" DisplayAs="Vert Matrix" parm1="4" parm2="5" parm3="1" StringType="RGB Nodes" StartChannel="31"/>)");
    s.Add(R"(<model name="HMat" DisplayAs="Horiz Matrix" parm1="2" parm2="3" parm3="1" StringType="RGB Nodes" StartChannel="200"/>)");
    s.Add(R"(<model name="Cust" DisplayAs="Custom" parm1="3" parm2="2" StringType="RGB Nodes" StartChannel="300" CustomModel="1,,2;,3,"/>)");
    s.Line("Far", 5, "5000");
    s.Line("Off", 5, "40", "Active=\"0\"");
    s.Recalc();
    FppServer srv;
    srv.Get("/api/models", R"([{"Name":"Theirs","StartChannel":5},{"Name":"Auto","autoCreated":true},{"Name":"Old_xL","xLights":true},
        {"Name":"Arch_One","StartChannel":1},{"NoName":1},{"Name":7},{"Name":"Elsewhere","StartChannel":9000}])");
    auto f = MakeFpp(srv, 9);
    auto j = f->CreateModelMemoryMap(&s.mm, 1, 1000);
    auto models = j["models"];
    std::map<std::string, nlohmann::json> byName;
    for (auto& m : models) {
        byName[m["Name"].get<std::string>()] = m;
    }
    CHECK(byName.count("Far") == 0);
    CHECK(byName.count("Off") == 0);
    CHECK(byName.count("Auto") == 0);
    CHECK(byName.count("Old_xL") == 0);
    CHECK(byName.count("Theirs") == 1);
    CHECK(byName.count("Elsewhere") == 1); // FPP 8+ keeps non-xLights models wherever they are
    REQUIRE(byName.count("Arch_One") == 1);
    auto a = byName["Arch_One"];
    CHECK(a["xLights"] == true); // ours, not the controller's model of the same name
    CHECK(a["ChannelCount"] == 30);
    CHECK(a["StartChannel"] == 1);
    CHECK(a["ChannelCountPerNode"] == 3);
    CHECK(a["Orientation"] == "horizontal");
    CHECK(a["StringCount"] == 1);
    CHECK(a["StrandsPerString"] == 1);
    CHECK(a["Type"] == "Channel");
    CHECK(a["StartCorner"].get<std::string>().size() == 2);
    CHECK(byName["Mat"]["Orientation"] == "vertical");
    CHECK(byName["Mat"]["StringCount"] == 4);
    CHECK(byName["HMat"]["Orientation"] == "horizontal");
    auto c = byName["Cust"];
    CHECK(c["Orientation"] == "custom");
    CHECK(c["StringCount"] == 1);
    CHECK(c.contains("data"));
    CHECK_FALSE(c.contains("compressedData"));
    CHECK(models.size() == 6);

    auto ten = MakeFpp(srv, 10);
    auto j10 = ten->CreateModelMemoryMap(&s.mm, 1, 1000);
    for (auto& m : j10["models"]) {
        if (m["Name"] == "Cust") {
            CHECK(m.contains("compressedData"));
        }
    }

    // before FPP 8 controller models outside the range are dropped
    auto seven = MakeFpp(srv, 7);
    bool found = false;
    for (auto& m : seven->CreateModelMemoryMap(&s.mm, 1, 1000)["models"]) {
        found |= m["Name"] == "Elsewhere";
    }
    CHECK_FALSE(found);
}

TEST_CASE("FPP overlay models when the controller's list is not an array", "[fpp3][config][models]") {
    ModelShow s;
    s.Line("A", 10, "1");
    s.Recalc();
    FppServer srv;
    srv.Get("/api/models", R"({"not":"a list"})");
    auto f = MakeFpp(srv, 9);
    CHECK(f->CreateModelMemoryMap(&s.mm, 1, 100)["models"].size() == 1);
}

TEST_CASE("FPP finds xLights overlay models that no longer match the layout", "[fpp3][config][models]") {
    ModelShow s;
    s.Line("Same", 10, "1");
    s.Line("Moved", 10, "31");
    s.Recalc();
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    auto current = f->CreateModelMemoryMap(&s.mm, 1, 100)["models"];
    nlohmann::json onFpp = nlohmann::json::array();
    for (auto m : current) {
        if (m["Name"] == "Moved") {
            m["StartChannel"] = 100;
        }
        if (m["Name"] == "Same") {
            m.erase("ChannelCountPerNode"); // FPP treats a missing value as 3
        }
        onFpp.push_back(m);
    }
    onFpp.push_back(R"({"Name":"Gone","xLights":true,"StartChannel":5000})"_json);
    onFpp.push_back(R"({"Name":"Hand","StartChannel":1})"_json);
    onFpp.push_back(R"({"Name":"Sub","xLights":true,"Type":"Sub"})"_json);
    onFpp.push_back(R"({"Name":"Auto","xLights":true,"autoCreated":true})"_json);
    onFpp.push_back(R"("junk")"_json);
    srv.GetJson("/api/models", onFpp);
    bool allInRange = true;
    auto out = f->FindOutdatedXLightsModels(&s.mm, 1, 1000, allInRange);
    CHECK(out == std::vector<std::string>{ "Moved", "Gone" });
    CHECK_FALSE(allInRange);

    FppServer none;
    auto g = MakeFpp(none, 9);
    CHECK(g->FindOutdatedXLightsModels(&s.mm, 1, 1000, allInRange).empty());
    CHECK(allInRange);
}

TEST_CASE("FPP UploadModels posts only a changed overlay file", "[fpp3][config][models]") {
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    nlohmann::json models = { { "models", nlohmann::json::array({ { { "Name", "A" } } }) } };
    CHECK_FALSE(f->UploadModels(models));
    CHECK(srv.Count("POST", "/api/models") == 1);
    CHECK(f->restartNeeded);
    srv.Get("/api/configfile/model-overlays.json", models.dump(3, ' ', false, nlohmann::json::error_handler_t::replace));
    auto g = MakeFpp(srv, 9);
    g->UploadModels(models);
    CHECK(srv.Count("POST", "/api/models") == 1);
    CHECK_FALSE(g->restartNeeded);
}

TEST_CASE("FPP submodel map uses node numbers when it can", "[fpp3][config][models]") {
    ModelShow s;
    s.Line("Line A", 6, "11", "",
           R"(<subModel name="Left/Half" layout="horizontal" type="ranges" bufferstyle="Default" line0="1-3"/>)"
           R"(<subModel name="Pair" layout="horizontal" type="ranges" bufferstyle="Default" line0="5,2"/>)");
    s.Line("Plain", 4, "100");
    s.Recalc();
    auto j = FPP::CreateSubModelMap(&s.mm, 1, 1000);
    CHECK(j["source"] == "xlights");
    CHECK(j["version"] == 1);
    REQUIRE(j["submodels"].size() == 2);
    auto a = j["submodels"][0];
    CHECK(a["Name"] == "Line_A_Left_Half");
    CHECK(a["DisplayName"] == "Left/Half");
    CHECK(a["Type"] == "Sub");
    CHECK(a["SubType"] == "grid");
    CHECK(a["Parent"] == "Line_A");
    CHECK(a["ParentStartChannel"] == 11);
    CHECK(a["ChannelCountPerNode"] == 3);
    CHECK(a["Width"] == 3);
    CHECK(a["Height"] == 1);
    CHECK(a["Grid"] == "1,2,3");
    auto b = j["submodels"][1];
    CHECK(b["Grid"] == "5,2");
    CHECK(FPP::CreateSubModelMap(&s.mm, 500, 1000)["submodels"].empty());
}

TEST_CASE("FPP model group map lists groups wholly inside the upload", "[fpp3][config][models]") {
    ModelShow s;
    s.Line("A", 3, "1");
    s.Line("B", 3, "10");
    s.Line("Far", 3, "5000");
    s.Add(R"(<modelGroup name="AB" models="A,B" layout="horizontal" GridSize="6"/>)");
    s.Add(R"(<modelGroup name="WithFar" models="A,Far" layout="horizontal" GridSize="6"/>)");
    s.Add(R"(<modelGroup name="Nested" models="AB" layout="horizontal" GridSize="6"/>)");
    s.Recalc();
    auto j = FPP::CreateModelGroupMap(&s.mm, 1, 1000);
    CHECK(j["source"] == "xlights");
    std::map<std::string, nlohmann::json> byName;
    for (auto& g : j["modelgroups"]) {
        byName[g["Name"].get<std::string>()] = g;
    }
    CHECK(byName.count("WithFar") == 0);
    REQUIRE(byName.count("AB") == 1);
    auto ab = byName["AB"];
    CHECK(ab["IsGroup"] == true);
    CHECK(ab["SubType"] == "channelgrid");
    CHECK(ab["GroupType"] == "model");
    CHECK(ab["MemberCount"] == 2);
    CHECK(ab["PixelCount"] == 6);
    CHECK(ab["ChannelCountPerNode"] == 3);
    // every pixel of both models is somewhere in the grid, by its 1 based channel
    std::string grid = ab["Grid"];
    for (int ch : { 1, 4, 7, 10, 13, 16 }) {
        CHECK_THAT(grid, ContainsSubstring(std::to_string(ch)));
    }
    CHECK(byName.count("Nested") == 1);
}

TEST_CASE("FPP UploadSubModelsAndGroups needs FPP 10 and posts only changes", "[fpp3][config][models]") {
    ModelShow s;
    s.Line("A", 3, "1");
    s.Recalc();
    FppServer srv;
    auto nine = MakeFpp(srv, 9);
    CHECK_FALSE(nine->UploadSubModelsAndGroups(&s.mm, 1, 100));
    CHECK(srv.Requests().empty());
    auto f = MakeFpp(srv, 10);
    f->UploadSubModelsAndGroups(&s.mm, 1, 100);
    CHECK(srv.Count("POST", "/api/configfile/xlights-submodels.json") == 1);
    CHECK(srv.Count("POST", "/api/configfile/xlights-modelgroups.json") == 1);
    CHECK(f->restartNeeded);
    srv.Get("/api/configfile/xlights-submodels.json", srv.Sent("POST", "/api/configfile/xlights-submodels.json").back().body);
    srv.Get("/api/configfile/xlights-modelgroups.json", srv.Sent("POST", "/api/configfile/xlights-modelgroups.json").back().body);
    auto g = MakeFpp(srv, 10);
    g->UploadSubModelsAndGroups(&s.mm, 1, 100);
    CHECK(srv.Count("POST", "/api/configfile/xlights-submodels.json") == 1);
    CHECK_FALSE(g->restartNeeded);
}

TEST_CASE("FPP UploadDisplayMap", "[fpp3][config][models]") {
    FppServer srv;
    TempDir dir;
    dir.Write("asset.png", "PNGDATA");
    std::map<std::string, std::string> data{ { "/api/configfile/virtualdisplaymap", "map" },
                                             { "/api/configfile/virtdisplay.json", "{}" },
                                             { "bg.png", (dir.path / "asset.png").string() } };
    auto nine = MakeFpp(srv, 9);
    nine->UploadDisplayMap(data);
    CHECK(srv.Count("POST", "/api/configfile/virtualdisplaymap") == 1);
    CHECK_FALSE(srv.Has("POST", "/api/configfile/virtdisplay.json"));
    CHECK(nine->restartNeeded);

    auto ten = MakeFpp(srv, 10);
    ten->UploadDisplayMap(data);
    DrainCurls();
    CHECK(srv.Count("POST", "/api/configfile/virtualdisplaymap") == 2);
    CHECK(srv.Count("POST", "/api/configfile/virtdisplay.json") == 1);
    auto up = srv.Sent("PATCH", "/api/file/virtualdisplay_assets");
    REQUIRE(up.size() == 1);
    CHECK(up[0].body == "PNGDATA");
    CHECK(ten->faileduploads.empty());
}

TEST_CASE("FPP UploadPlaylist adds the uploaded sequences once", "[fpp3][config][playlist]") {
    FppServer srv;
    srv.Get("/api/playlist/My%20Show", R"({"name":"My Show","random":1,"playlistInfo":{"x":1},"mainPlaylist":[
        {"type":"both","sequenceName":"a.fseq","mediaName":"a.mp3","duration":10.5},{"type":"pause","duration":2}]})");
    auto f = MakeFpp(srv, 9);
    f->sequences["a.fseq"] = { "a.fseq", "a.mp3", 10.5f };
    f->sequences["b.fseq"] = { "b.fseq", "", 4.0f };
    f->sequences["c.fseq"] = { "c.fseq", "c.mp3", 1.0f };
    CHECK_FALSE(f->UploadPlaylist("My Show"));
    auto pl = srv.LastJson("POST", "/api/playlist/My%20Show");
    CHECK(pl["name"] == "My Show");
    CHECK(pl["random"] == 1);
    auto main = pl["mainPlaylist"];
    REQUIRE(main.size() == 4);
    CHECK(main[2]["type"] == "sequence");
    CHECK(main[2]["sequenceName"] == "b.fseq");
    CHECK(main[3]["type"] == "both");
    CHECK(main[3]["mediaName"] == "c.mp3");
    CHECK(main[3]["videoOut"] == "--Default--");
    CHECK(pl["playlistInfo"]["total_items"] == 4);
    CHECK_FALSE(pl["playlistInfo"].contains("x"));

    CHECK(f->GetPlaylistItems("My Show") == std::vector<std::string>{ "a.fseq", "" });
    CHECK(f->GetPlaylistItems("missing").empty());
}

TEST_CASE("FPP UploadPlaylist totals the durations", "[fpp3]") {
    FppServer srv;
    srv.Get("/api/playlist/P", R"({"mainPlaylist":[{"type":"pause","duration":2.5}]})");
    auto f = MakeFpp(srv, 9);
    f->sequences["b.fseq"] = { "b.fseq", "", 4.0f };
    f->UploadPlaylist("P");
    CHECK(srv.LastJson("POST", "/api/playlist/P")["playlistInfo"]["total_duration"].get<double>() == 6.5);
}

TEST_CASE("FPP UploadPlaylist creates a new playlist", "[fpp3][config][playlist]") {
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    f->sequences["b.fseq"] = { "b.fseq", "", 4.0f };
    f->UploadPlaylist("New");
    auto pl = srv.LastJson("POST", "/api/playlist/New");
    CHECK(pl["random"] == 0);
    REQUIRE(pl["mainPlaylist"].size() == 1);
    CHECK(pl["mainPlaylist"][0]["sequenceName"] == "b.fseq");
}

#endif
