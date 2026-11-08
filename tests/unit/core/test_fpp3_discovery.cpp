// FPP discovery: the HTTP probe chain a single address starts (system info, channel
// outputs, playlists, cape, proxies and the proxied instances), the UDP ping packet
// parser, and turning discovery results into FPP Connect instances.
//
// PrepareSingleDiscovery also opens the FPP multicast socket.  The tests take its ping
// callback and then close every socket before anything is read, so no packet from the
// network can reach them; all HTTP goes to a loopback FppServer.
#ifndef _WIN32

#include "test_fpp3_common.h"

#include <cstring>

using namespace fpp3;

namespace {

using PingCallback = std::function<void(uint8_t*, int, const std::string&)>;

// Starts a single-address discovery against srv and strips the UDP sockets, returning
// the ping handler FPP registered.
PingCallback StartSingle(Discovery& d, const std::string& address) {
    FPP::PrepareSingleDiscovery(d, address);
    REQUIRE(d.datagrams.size() == 1);
    PingCallback cb = d.datagrams.front()->callbacks.front();
    for (auto* dg : d.datagrams) {
        delete dg;
    }
    d.datagrams.clear();
    return cb;
}

struct Ping {
    uint8_t b[512] = {};
    Ping(const uint8_t ip[4], int typeId, const std::string& host, int mode = 2) {
        memcpy(b, "FPPD", 4);
        b[4] = 0x04;
        b[5] = 294 & 0xFF;
        b[6] = 294 >> 8;
        b[7] = 3;
        b[9] = (uint8_t)typeId;
        b[10] = 0;
        b[11] = 9;
        b[12] = 0;
        b[13] = 2;
        b[14] = (uint8_t)mode;
        memcpy(&b[15], ip, 4);
        strcpy((char*)&b[19], host.c_str());
        strcpy((char*)&b[84], "9.2");
        strcpy((char*)&b[125], "Raspberry Pi");
        strcpy((char*)&b[166], "1-1500");
    }
    void Uuid(const std::string& u) {
        int len = 294 + 65;
        b[5] = len & 0xFF;
        b[6] = len >> 8;
        strcpy((char*)&b[7 + 294], u.c_str());
    }
};

DiscoveredData* Result(Discovery& d, const std::string& ip, int typeId, bool http = true, const std::string& mode = "player") {
    auto* r = d.FindByIp(ip, "", true);
    r->typeId = typeId;
    r->mode = mode;
    r->extraData["httpConnected"] = http;
    return r;
}

} // namespace

TEST_CASE("FPP discovery of one address follows the FPP API", "[fpp3][discovery]") {
    VendorCapsFixture caps(kFppCaps);
    InitLoggers();
    FppServer srv;
    srv.Get("/api/system/info", R"({"Platform":"BeagleBone Black","Variant":"PocketBeagle","Version":"9.2-3","HostName":"fppa","HostDescription":"Front",
        "Mode":"player","multisync":true,"uuid":"U-1","typeId":2,"zip":1,"channelRanges":"0-599","capeInfo":{"id":"TestCape","version":"2.1"}})");
    srv.Get("/api/channel/output/co-bbbStrings", R"({"channelOutputs":[{"enabled":1,"type":"BBB48String","subType":"TestCape"}]})");
    srv.Get("/api/channel/output/channelOutputsJSON", R"({"channelOutputs":[{"enabled":1,"type":"LEDPanelMatrix","panelWidth":32,"panelHeight":16,
        "panels":[{"row":0,"col":0},{"row":1,"col":1,"orientation":"W"}]}]})");
    srv.Get("/api/channel/output/co-other", R"({"channelOutputs":[{"enabled":1,"type":"VirtualMatrix"},{"enabled":0,"type":"RPIWS281X","subType":"x"}]})");
    srv.Get("/api/playlists", R"(["Main","Xmas"])");
    srv.Get("/api/cape", R"({"id":"TestCape","name":"Test"})");
    srv.Get("/api/proxies", R"(["192.0.2.50",{"host":"192.0.2.51"}])");
    srv.Get("/proxy/192.0.2.50/", "<title>Falcon Player - FPP</title>");
    srv.Get("/proxy/192.0.2.50/api/system/info", R"({"Platform":"Raspberry Pi","Version":"8.1","HostName":"behind","Mode":"remote","typeId":1})");
    srv.Get("/proxy/192.0.2.50/api/channel/output/co-pixelStrings", R"({"channelOutputs":[]})");
    srv.Get("/proxy/192.0.2.51/", "<html>a router</html>");
    srv.Get("/", "<html>FPP</html>");

    OutputManager om;
    {
        Discovery d(&om);
        // a controller already configured at this address carries the discovered type
        auto* ce = new ControllerEthernet(&om, false);
        auto* configured = d.FindByIp(srv.Address(), "", true);
        configured->controller = ce;
        StartSingle(d, srv.Address());
        d.Discover();
        d.Close(true);

        auto* r = d.FindByIp(srv.Address());
        REQUIRE(r == configured);
        CHECK(r->platform == "BeagleBone Black");
        CHECK(r->platformModel == "PocketBeagle");
        CHECK(r->version == "9.2-3");
        CHECK(r->majorVersion == 9);
        CHECK(r->minorVersion == 2);
        CHECK(r->patchVersion == 3);
        CHECK(r->hostname == "fppa");
        CHECK(r->description == "Front");
        CHECK(r->mode == "player w/multisync");
        CHECK(r->uuid == "U-1");
        CHECK(r->typeId == 2);
        CHECK(r->canZipUpload);
        CHECK(r->ranges == "0-599");
        CHECK(r->pixelControllerType == "TestCape");
        CHECK(r->panelSize == "32x64"); // 2x2 panels, turned on their side
        CHECK(r->extraData["capeVersion"] == "2.1");
        CHECK(r->extraData["httpConnected"] == true);
        CHECK(r->extraData["playlists"].size() == 2);
        CHECK(r->extraData["cape"]["name"] == "Test");
        CHECK(r->vendor == "FPP");
        CHECK(r->model == "TestCape");
        // the cape supports auto layout, so the controller is switched to it
        CHECK(ce->IsAutoLayout());

        auto* behind = d.FindByIp("192.0.2.50");
        REQUIRE(behind != nullptr);
        CHECK(behind->proxy == srv.Address());
        CHECK(behind->hostname == "behind");
        CHECK(behind->majorVersion == 8);
        CHECK(behind->minorVersion == 1);
        CHECK(behind->mode == "remote");
        CHECK(behind->extraData["httpConnected"] == true);
        auto* router = d.FindByIp("192.0.2.51");
        REQUIRE(router != nullptr);
        CHECK(router->proxy == srv.Address());
        CHECK(router->extraData["httpConnected"] == false);

        CHECK(srv.Has("GET", "/proxy/192.0.2.50/api/channel/output/co-pixelStrings"));
        CHECK(srv.Has("GET", "/proxy/192.0.2.50/api/cape"));
        // a proxied instance's own proxies are not asked for
        CHECK_FALSE(srv.Has("GET", "/proxy/192.0.2.50/api/proxies"));

        std::list<FPP*> instances;
        FPP::MapToFPPInstances(d, instances, &om);
        REQUIRE(instances.size() == 2);
        FPP* a = instances.front();
        CHECK(a->ipAddress == srv.Address());
        CHECK(a->hostName == "fppa");
        CHECK(a->uuid == "U-1");
        CHECK(a->fullVersion == "9.2-3");
        CHECK(a->pixelControllerType == "TestCape");
        CHECK(a->playlists == std::list<std::string>{ "Main", "Xmas" });
        CHECK(a->capeInfo["id"] == "TestCape");
        CHECK(a->canZipUpload);
        CHECK(a->fppType == FPP_TYPE::FPP);
        CHECK(a->solePlayer);
        CHECK(a->controllerVendor == "FPP");
        FPP* b = instances.back();
        CHECK(b->ipAddress == "192.0.2.50");
        CHECK(b->proxy() == srv.Address());
        CHECK_FALSE(b->solePlayer);
        for (auto* f : instances) delete f;
    }
}

TEST_CASE("FPP discovery of an unconfigured address records its cape", "[fpp3][discovery]") {
    VendorCapsFixture caps(kFppCaps);
    InitLoggers();
    FppServer srv;
    srv.Get("/api/system/info", R"({"Platform":"Raspberry Pi","Version":"9.0","HostName":"x","uuid":"U-9"})");
    srv.Get("/api/channel/output/co-pixelStrings", R"({"channelOutputs":[{"enabled":1,"type":"RPIWS281X","subType":"TestHat"}]})");
    OutputManager om;
    Discovery d(&om);
    StartSingle(d, srv.Address());
    d.Discover();
    d.Close(true);
    auto* r = d.FindByIp(srv.Address());
    REQUIRE(r != nullptr);
    CHECK(r->pixelControllerType == "TestHat");
    CHECK(r->vendor == "FPP");
    CHECK(r->model == "TestHat");
    CHECK(r->controller == nullptr);
}

TEST_CASE("FPP discovery of an address that is not an FPP", "[fpp3][discovery]") {
    InitLoggers();
    FppServer srv;
    srv.Get("/api/system/info", "<html>no</html>");
    OutputManager om;
    Discovery d(&om);
    StartSingle(d, srv.Address());
    d.Discover();
    d.Close(true);
    auto* r = d.FindByIp(srv.Address());
    REQUIRE(r != nullptr);
    CHECK(r->extraData["httpConnected"] == false);
    CHECK(srv.Has("GET", "/"));
    CHECK_FALSE(srv.Has("GET", "/api/playlists"));
}

TEST_CASE("FPP ping packets create Falcon controllers", "[fpp3][discovery][ping]") {
    InitLoggers();
    FppServer srv;
    OutputManager om;
    Discovery d(&om);
    auto cb = StartSingle(d, srv.Address());

    const uint8_t ip[4] = { 192, 0, 2, 10 };
    Ping p(ip, 0x88, "falcon1", 2);
    strcpy((char*)&p.b[125], "Falcon F16V4");
    p.Uuid("UUID-F");
    cb(p.b, 7 + 294 + 65, "192.0.2.10");
    auto* r = d.FindByIp("192.0.2.10");
    REQUIRE(r != nullptr);
    CHECK(r->typeId == 0x88);
    CHECK(r->uuid == "UUID-F");
    CHECK(r->hostname == "falcon1");
    CHECK(r->platformModel == "Falcon F16V4");
    CHECK(r->version == "9.2");
    CHECK(r->majorVersion == 9);
    CHECK(r->minorVersion == 2);
    CHECK(r->mode == "player");
    CHECK(r->ranges == "1-1500");
    CHECK(r->vendor == "Falcon");
    CHECK(r->model == "F16V4");
    REQUIRE(r->controller != nullptr);
    CHECK(r->controller->GetProtocol() == OUTPUT_DDP);
    CHECK(r->controller->GetIP() == "192.0.2.10");
    CHECK(r->controller->GetName() == "falcon1");
    // the ranges size the controller, never below 512
    CHECK(r->controller->GetChannels() == 1500);

    // a second packet from the same uuid only fills in what is missing
    Ping again(ip, 0x88, "other", 1);
    again.Uuid("UUID-F");
    cb(again.b, 7 + 294 + 65, "192.0.2.10");
    CHECK(r->hostname == "falcon1");
    CHECK(r->mode == "bridge");
    CHECK(d.GetResults().size() == 1);

    // a bridge mode Falcon found by ping is E1.31
    const uint8_t ip2[4] = { 192, 0, 2, 11 };
    Ping b(ip2, 0x89, "falcon2", 1);
    cb(b.b, 300, "192.0.2.11");
    auto* r2 = d.FindByIp("192.0.2.11");
    REQUIRE(r2 != nullptr);
    CHECK(r2->mode == "bridge");
    CHECK(r2->controller->GetProtocol() == OUTPUT_E131);
    CHECK(r2->uuid.empty()); // too short to carry one

    // not an FPP packet, or no address
    Ping junk(ip2, 0x88, "x");
    junk.b[0] = 'X';
    cb(junk.b, 300, "");
    const uint8_t zero[4] = { 0, 0, 0, 0 };
    Ping noip(zero, 0x88, "x");
    cb(noip.b, 300, "");
    CHECK(d.GetResults().size() == 2);
    d.Close(true);
}

TEST_CASE("FPP ping packets from a known FPP set its type and modes", "[fpp3][discovery][ping]") {
    VendorCapsFixture caps(kFppCaps);
    InitLoggers();
    FppServer srv;
    OutputManager om;
    Discovery d(&om);
    auto cb = StartSingle(d, srv.Address());
    auto row = GENERATE(table<int, std::string>({ { 1, "bridge" }, { 2, "player" }, { 4, "master" }, { 6, "master" }, { 8, "remote" } }));
    const uint8_t ip[4] = { 192, 0, 2, 20 };
    // known already (as an HTTP probe would have made it), so the ping adds no requests
    auto* known = d.FindByIp("192.0.2.20", "", true);
    known->pixelControllerType = "TestHat";
    known->mode = "";
    Ping p(ip, 0x03, "pi", std::get<0>(row));
    cb(p.b, 300, "192.0.2.20");
    INFO(std::get<0>(row));
    CHECK(known->mode == std::get<1>(row));
    CHECK(known->typeId == 3);
    REQUIRE(known->controller != nullptr);
    CHECK(known->controller->GetProtocol() == OUTPUT_DDP);
    CHECK(known->vendor == "FPP");
    CHECK(known->model == "TestHat");
    CHECK(known->controller->IsAutoLayout());
    d.Close(true);
    CHECK(srv.Requests().size() <= 2); // only the probe of the address PrepareSingleDiscovery was given
}

TEST_CASE("FPP ping from an ESPixelStick uses DDP without channel numbers", "[fpp3][discovery][ping]") {
    InitLoggers();
    FppServer srv;
    OutputManager om;
    Discovery d(&om);
    auto cb = StartSingle(d, srv.Address());
    const uint8_t ip[4] = { 192, 0, 2, 30 };
    Ping p(ip, 0xC2, "esps", 8);
    strcpy((char*)&p.b[125], "ESPixelStick-32");
    strcpy((char*)&p.b[166], "0-0");
    cb(p.b, 300, "");
    auto* r = d.FindByIp("192.0.2.30");
    REQUIRE(r != nullptr);
    CHECK(r->mode == "remote");
    CHECK(r->ranges.empty());
    CHECK(r->pixelControllerType == "ESPixelStick-32");
    CHECK(r->controller->GetProtocol() == OUTPUT_DDP);
    CHECK(r->controller->GetChannels() == 512);
    d.Close(true);
}

TEST_CASE("MapToFPPInstances keeps the instances FPP Connect can use", "[fpp3][discovery][map]") {
    InitLoggers();
    OutputManager om;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<Controller Id="1" Name="Esp" Type="Ethernet" Protocol="DDP" IP="192.0.2.40" ActiveState="Active">
        <network NetworkType="DDP" ComPort="192.0.2.40" BaudRate="1" MaxChannels="300"/></Controller>)"));
    om.AddController(Controller::Create(&om, doc.first_child(), ""));
    om.SomethingChanged();
    Discovery d(&om);

    auto* fpp = Result(d, "192.0.2.1", 2);
    fpp->hostname = "one";
    fpp->uuid = "U1";
    fpp->version = "9.1";
    fpp->majorVersion = 9;
    fpp->minorVersion = 1;
    fpp->extraData["playlists"] = { "P" };
    Result(d, "192.0.2.2", 2, false);                         // no HTTP: skipped
    Result(d, "192.0.2.3", 0);                                // no type: skipped
    Result(d, "192.0.2.4", 0x88, true, "bridge");             // bridge Falcon: skipped
    Result(d, "192.0.2.5", 0x8A, true, "remote");             // Falcon v5
    Result(d, "192.0.2.6", 0xA0, true, "remote");             // Genius
    Result(d, "192.0.2.7", 0xD0, true, "player")->hostname = "pdmx"; // PowerDMX
    auto* esp = Result(d, "192.0.2.40", 0xC2, true, "remote"); // ESPixelStick, sized from the controller
    esp->majorVersion = 4;
    auto* espOld = Result(d, "192.0.2.41", 0xC2, true, "remote");
    espOld->majorVersion = 3;
    espOld->ranges = "0-10";
    Result(d, "192.0.2.42", 0xC3, true, "remote");             // no ranges, no controller: skipped
    Result(d, "192.0.2.8", 0xF0, true, "player");              // unknown type
    // the same device again at a stale link-local address, seen first
    auto* stale = d.FindByIp("169.254.1.1", "", true);
    stale->typeId = 0;
    stale->uuid = "U1";
    auto* staleTyped = d.FindByIp("169.254.1.2", "", true);
    staleTyped->typeId = 2;
    staleTyped->uuid = "U9";
    staleTyped->extraData["httpConnected"] = true;
    auto* sameDevice = Result(d, "192.0.2.9", 0, true, "player");
    sameDevice->uuid = "U9";

    std::list<FPP*> instances;
    FPP::MapToFPPInstances(d, instances, &om);
    std::map<std::string, FPP*> byIp;
    for (auto* f : instances) byIp[f->ipAddress] = f;
    CHECK(byIp.count("192.0.2.1") == 1);
    CHECK(byIp.count("192.0.2.2") == 0);
    CHECK(byIp.count("192.0.2.3") == 0);
    CHECK(byIp.count("192.0.2.4") == 0);
    CHECK(byIp.count("192.0.2.41") == 0);
    CHECK(byIp.count("192.0.2.42") == 0);
    CHECK(byIp.count("192.0.2.8") == 0);
    CHECK(byIp.count("169.254.1.1") == 0); // its uuid is already in use by a routable address
    REQUIRE(byIp.count("192.0.2.9") == 1);  // took its type from the stale record of the same uuid
    CHECK(byIp.count("169.254.1.2") == 0);
    CHECK(byIp["192.0.2.9"]->uuid == "U9");
    CHECK(byIp["192.0.2.1"]->playlists == std::list<std::string>{ "P" });
    CHECK(byIp["192.0.2.1"]->majorVersion == 9);
    CHECK(byIp["192.0.2.5"]->fppType == FPP_TYPE::FALCONV4V5);
    CHECK(byIp["192.0.2.6"]->fppType == FPP_TYPE::GENIUS);
    CHECK(byIp["192.0.2.7"]->fppType == FPP_TYPE::POWERDMX);
    REQUIRE(byIp.count("192.0.2.40") == 1);
    CHECK(byIp["192.0.2.40"]->fppType == FPP_TYPE::ESPIXELSTICK);
    CHECK(byIp["192.0.2.40"]->ranges == "0-299");
    // three players: none is the only one
    for (auto* f : instances) CHECK_FALSE(f->solePlayer);

    // a second pass merges into the instances already listed
    byIp["192.0.2.7"]->hostName = "";
    byIp["192.0.2.7"]->fppType = FPP_TYPE::FPP;
    size_t before = instances.size();
    FPP::MapToFPPInstances(d, instances, &om);
    CHECK(instances.size() == before);
    CHECK(byIp["192.0.2.7"]->hostName == "pdmx");
    CHECK(byIp["192.0.2.7"]->fppType == FPP_TYPE::POWERDMX);
    for (auto* f : instances) delete f;
}

TEST_CASE("MapToFPPInstances marks proxies and prefers a configured address", "[fpp3][discovery][map]") {
    InitLoggers();
    OutputManager om;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<Controller Id="1" Name="Behind" Type="Ethernet" Protocol="DDP" IP="192.0.2.60" FPPProxy="192.0.2.61" ActiveState="Active">
        <network NetworkType="DDP" ComPort="192.0.2.60" BaudRate="1" MaxChannels="300"/></Controller>)"));
    om.AddController(Controller::Create(&om, doc.first_child(), ""));
    pugi::xml_document doc2;
    REQUIRE(doc2.load_string(R"(<Controller Id="2" Name="Proxy" Type="Ethernet" Protocol="DDP" IP="192.0.2.61" ActiveState="Active">
        <network NetworkType="DDP" ComPort="192.0.2.61" BaudRate="2" MaxChannels="300"/></Controller>)"));
    om.AddController(Controller::Create(&om, doc2.first_child(), ""));
    om.SomethingChanged();
    om.ResolveControllerIPs(true, true);
    Discovery d(&om);
    auto* unconfigured = Result(d, "192.0.2.70", 2);
    unconfigured->uuid = "SAME";
    auto* proxy = Result(d, "192.0.2.61", 2);
    proxy->uuid = "SAME";
    std::list<FPP*> instances;
    FPP::MapToFPPInstances(d, instances, &om);
    REQUIRE(instances.size() == 1);
    // the configured address of the same device replaces the unconfigured one
    CHECK(instances.front()->ipAddress == "192.0.2.61");
    for (auto* f : instances) delete f;

    std::list<FPP*> again;
    Discovery d2(&om);
    Result(d2, "192.0.2.61", 2);
    FPP::MapToFPPInstances(d2, again, &om);
    REQUIRE(again.size() == 1);
    CHECK(again.front()->isaProxy);
    CHECK(again.front()->solePlayer);
    for (auto* f : again) delete f;
}

TEST_CASE("FPP controller seeding lists the configured addresses", "[fpp3][discovery]") {
    InitLoggers();
    OutputManager om;
    for (auto xml : { R"(<Controller Id="1" Name="A" Type="Ethernet" Protocol="DDP" IP="192.0.2.80" FPPProxy="192.0.2.82" ActiveState="Active"><network NetworkType="DDP" ComPort="192.0.2.80" BaudRate="1" MaxChannels="3"/></Controller>)",
                      R"(<Controller Id="2" Name="B" Type="Ethernet" Protocol="DDP" IP="192.0.2.81" ActiveState="Inactive"><network NetworkType="DDP" ComPort="192.0.2.81" BaudRate="2" MaxChannels="3"/></Controller>)",
                      R"(<Controller Id="3" Name="M" Type="Ethernet" Protocol="E131" IP="MULTICAST" ActiveState="Active"><network NetworkType="E131" ComPort="MULTICAST" BaudRate="3" MaxChannels="3"/></Controller>)",
                      R"(<Controller Id="4" Name="C" Type="Ethernet" Protocol="DDP" IP="192.0.2.80" ActiveState="Active"><network NetworkType="DDP" ComPort="192.0.2.80" BaudRate="4" MaxChannels="3"/></Controller>)" }) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml));
        om.AddController(Controller::Create(&om, doc.first_child(), ""));
    }
    om.SomethingChanged();
    auto seeded = FPP::StartControllerSeeding(&om);
    CHECK(seeded == std::list<std::string>{ "192.0.2.80", "192.0.2.82" });

    // every active controller was seeded or found, so there is nothing left to probe
    Discovery d(&om);
    FPP::FinishControllerSeeding(d, seeded);
    CHECK(d.GetResults().empty());
}

#endif
