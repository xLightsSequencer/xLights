// The channel output configs FPP Connect / controller upload writes to an FPP: the pixel
// string file (co-bbbStrings / co-pixelStrings), serial outputs (BBBSerial and co-other),
// panel and virtual matrices, and PWM, all against a loopback FPP.
#ifndef _WIN32

#include "test_fpp3_common.h"

#include <catch2/matchers/catch_matchers_string.hpp>

using namespace fpp3;
using Catch::Matchers::ContainsSubstring;

namespace {

const std::string kStrings = "/api/channel/output/co-bbbStrings";
const std::string kPiStrings = "/api/channel/output/co-pixelStrings";
const std::string kOther = "/api/configfile/co-other.json";

// A Beagle cape whose pinout FPP knows.
void CapeKnown(FppServer& srv, const std::string& id = "TestCape", const std::string& driver = "BBB48String") {
    srv.GetJson("/api/cape/strings", nlohmann::json::array({ "Other", id }));
    nlohmann::json pin = nlohmann::json::object();
    if (!driver.empty()) {
        pin["driver"] = driver;
    }
    srv.GetJson("/api/cape/strings/" + id, pin);
}

nlohmann::json Vs(const nlohmann::json& root, int port, int idx = 0, const std::string& key = "virtualStrings") {
    return root["channelOutputs"][0]["outputs"][port - 1][key][idx];
}

} // namespace

TEST_CASE("FPP pixel upload writes the strings file for a Beagle cape", "[fpp3][outputs][pixel]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.OnPort("B", "ws2811", 1, 5, "!Ctl:31", "Brightness=\"48\" gamma=\"2.2\" colorOrder=\"GRB\" Reverse=\"1\" nullNodes=\"1\" endNullNodes=\"2\" groupCount=\"2\" ZigZag=\"3\"");
    s.OnPort("C", "ws2811", 3, 4, "!Ctl:100");
    s.OnPort("W", "ws2811", 2, 6, "!Ctl:200", "", "", 1, "Single Color White");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv);
    auto f = MakeFpp(srv, 9);

    CHECK_FALSE(f->UploadPixelOutputs(&s.mm, &s.om, s.ctl));
    REQUIRE(srv.Count("POST", kStrings) == 1);
    auto root = srv.LastJson("POST", kStrings);
    // the cape's serial output is always written, here with nothing on it
    REQUIRE(root["channelOutputs"].size() == 2);
    CHECK(root["channelOutputs"][1]["type"] == "BBBSerial");
    CHECK(root["channelOutputs"][1]["outputs"].size() == 2);
    auto so = root["channelOutputs"][0];
    CHECK(so["type"] == "BBB48String");
    CHECK(so["subType"] == "TestCape");
    CHECK(so["pinoutVersion"] == "1.x");
    CHECK(so["enabled"] == 1);
    CHECK(so["startChannel"] == 1);
    CHECK(so["channelCount"] == -1);
    CHECK(so["outputCount"] == 3);
    CHECK(so["serialInUse"] == false);
    REQUIRE(so["outputs"].size() == 8);
    for (int p = 0; p < 8; p++) {
        CHECK(so["outputs"][p]["portNumber"] == p);
    }
    CHECK(so["outputs"][0]["protocol"] == "ws2811");
    CHECK_FALSE(so["outputs"][3].contains("protocol"));

    auto a = Vs(root, 1, 0);
    CHECK(a["description"] == "A");
    CHECK(a["startChannel"] == 0);
    CHECK(a["pixelCount"] == 10);
    CHECK(a["brightness"] == 100);
    CHECK(a["gamma"] == "1.0");
    CHECK(a["colorOrder"] == "RGB");
    CHECK(a["reverse"] == 0);
    CHECK(a["groupCount"] == 0);
    CHECK(a["nullNodes"] == 0);
    CHECK(a["endNulls"] == 0);
    CHECK(a["zigZag"] == 0);

    auto b = Vs(root, 1, 1);
    CHECK(b["description"] == "B");
    CHECK(b["startChannel"] == 30);
    CHECK(b["pixelCount"] == 10);       // 5 pixels, grouped by 2
    CHECK(b["brightness"] == 50);       // rounded to the nearest 5
    CHECK(b["gamma"] == "2.2");
    CHECK(b["colorOrder"] == "GRB");
    CHECK(b["reverse"] == 1);
    CHECK(b["nullNodes"] == 1);
    CHECK(b["endNulls"] == 2);
    CHECK(b["groupCount"] == 2);
    CHECK(b["zigZag"] == 3);

    auto w = Vs(root, 2, 0);
    CHECK(w["colorOrder"] == "W");
    // a single colour string is one channel however long it is
    CHECK(w["pixelCount"] == 1);
    CHECK(w["startChannel"] == 199);

    auto c = Vs(root, 3, 0);
    CHECK(c["startChannel"] == 99);
    CHECK(c["pixelCount"] == 4);

    // unused ports still get one empty string
    auto e = Vs(root, 4, 0);
    CHECK(e["description"] == "");
    CHECK(e["pixelCount"] == 0);
    CHECK(e["brightness"] == 100);
    // port 5 starts a 4 port expansion with no smart receivers
    CHECK(so["outputs"][4]["expansionType"] == 0);
    CHECK_FALSE(so["outputs"][0].contains("expansionType"));

    // FPP 9 restarts itself, so no restart flag is set; the ranges now cover the strings
    CHECK_FALSE(srv.Has("PUT", "/api/settings/restartFlag"));
    CHECK(f->ranges == "0-44,99-110,199-199");
    CHECK_FALSE(srv.Has("POST", kOther));
}

TEST_CASE("FPP pixel upload does not resend an unchanged strings file", "[fpp3][outputs][pixel]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv);
    auto f = MakeFpp(srv, 8);
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    REQUIRE(srv.Count("POST", kStrings) == 1);
    // FPP 8 needs the restart flag
    CHECK(srv.Count("PUT", "/api/settings/restartFlag") == 1);
    CHECK(f->restartNeeded);

    srv.Get(kStrings, srv.Sent("POST", kStrings).back().body);
    auto f2 = MakeFpp(srv, 8);
    f2->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    CHECK(srv.Count("POST", kStrings) == 1);
    CHECK(f2->ranges == "0-29");
}

TEST_CASE("FPP pixel upload keeps string settings made on the controller", "[fpp3][outputs][pixel]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.OnPort("B", "ws2811", 2, 10, "!Ctl:31", "Brightness=\"30\"");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv);
    nlohmann::json orig = R"({"channelOutputs":[{"type":"BBB48String","subType":"TestCape","pinoutVersion":"2.x","outputs":[
        {"portNumber":0,"virtualStrings":[{"description":"A","groupCount":3,"reverse":1,"colorOrder":"BGR","nullNodes":4,"endNulls":5,"zigZag":6,"brightness":70,"gamma":"1.8"}]},
        {"portNumber":1,"virtualStrings":[{"description":"B","groupCount":0,"reverse":0,"colorOrder":"RBG","nullNodes":0,"zigZag":0,"brightness":90,"gamma":"2.0"},{"description":""}]}]}]})"_json;
    srv.GetJson(kStrings, orig);
    auto f = MakeFpp(srv, 9);
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    auto root = srv.LastJson("POST", kStrings);
    CHECK(root["channelOutputs"][0]["pinoutVersion"] == "2.x");
    auto a = Vs(root, 1);
    CHECK(a["groupCount"] == 3);
    CHECK(a["pixelCount"] == 30);
    CHECK(a["reverse"] == 1);
    CHECK(a["colorOrder"] == "BGR");
    CHECK(a["nullNodes"] == 4);
    CHECK(a["endNulls"] == 5);
    CHECK(a["zigZag"] == 6);
    CHECK(a["brightness"] == 70);
    CHECK(a["gamma"] == "1.8");
    // a model's own setting wins over the controller's
    auto b = Vs(root, 2);
    CHECK(b["brightness"] == 30);
    CHECK(b["colorOrder"] == "RBG");
    CHECK(b["endNulls"] == 0);

    SECTION("under full xLights control the controller's settings are not kept") {
        s.ctl->SetFullxLightsControl(true);
        s.ctl->SetDefaultBrightnessUnderFullControl(62);
        s.ctl->SetDefaultGammaUnderFullControl(2.5f);
        auto g = MakeFpp(srv, 9);
        g->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
        auto r2 = srv.LastJson("POST", kStrings);
        auto a2 = Vs(r2, 1);
        CHECK(a2["groupCount"] == 0);
        CHECK(a2["colorOrder"] == "RGB");
        CHECK(a2["brightness"] == 60);
        CHECK(a2["gamma"] == "2.5");
        CHECK(Vs(r2, 3)["brightness"] == 60);
    }
}

TEST_CASE("FPP pixel upload smart receivers on FPP 10", "[fpp3][outputs][pixel][smart]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1", "SmartRemote=\"1\" SmartRemoteType=\"falcon_v4\"");
    s.OnPort("B", "ws2811", 1, 10, "!Ctl:31", "SmartRemote=\"2\" SmartRemoteType=\"falcon_v4\"");
    s.OnPort("C", "ws2811", 2, 10, "!Ctl:61", "SmartRemote=\"1\" SmartRemoteType=\"falcon_v4\"");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv);
    auto f = MakeFpp(srv, 10);
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    auto root = srv.LastJson("POST", kStrings);
    auto outs = root["channelOutputs"][0]["outputs"];
    CHECK(Vs(root, 1, 0)["description"] == "A");
    CHECK(Vs(root, 1, 0, "virtualStringsB")["description"] == "B");
    CHECK(Vs(root, 1, 0, "virtualStringsB")["startChannel"] == 30);
    // two receivers deep on a Falcon v4 chain: 2 + 15, for every used port of the block
    CHECK(outs[0]["differentialType"] == 17);
    CHECK(outs[1]["differentialType"] == 17);
    CHECK_FALSE(outs[2].contains("differentialType"));
    CHECK(outs[4]["expansionType"] == 0);
}

TEST_CASE("FPP pixel upload smart receivers before FPP 10 fall back to v2", "[fpp3][outputs][pixel][smart]") {
    auto row = GENERATE(table<std::string, int, int>({ { "falcon_v4", 2, 5 }, { "falcon_v1", 1, 1 }, { "fpp_v2", 1, 4 }, { "falcon_v5", 3, 6 } }));
    FppShow s;
    std::string type = std::get<0>(row);
    for (int r = 1; r <= std::get<1>(row); r++) {
        s.OnPort("R" + std::to_string(r), "ws2811", 1, 10, "!Ctl:" + std::to_string(1 + (r - 1) * 30),
                 "SmartRemote=\"" + std::to_string(r) + "\" SmartRemoteType=\"" + type + "\"");
    }
    s.Recalc();
    FppServer srv;
    CapeKnown(srv);
    auto f = MakeFpp(srv, 9);
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    auto root = srv.LastJson("POST", kStrings);
    INFO(type);
    CHECK(root["channelOutputs"][0]["outputs"][0]["differentialType"] == std::get<2>(row));
}

TEST_CASE("FPP pixel upload Falcon v5 receivers need the cape's listener", "[fpp3][outputs][pixel][smart]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1", "SmartRemote=\"1\" SmartRemoteType=\"falcon_v5\"");
    s.Recalc();
    FppServer srv;
    srv.GetJson("/api/cape/strings", nlohmann::json::array({ "TestCape" }));
    srv.Get("/api/cape/strings/TestCape", R"({"driver":"BBB48String","falconV5ListenerConfig":{}})");
    auto f = MakeFpp(srv, 10);
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    auto root = srv.LastJson("POST", kStrings);
    CHECK(root["channelOutputs"][0]["outputs"][0]["differentialType"] == 10);
    // and the expansion block reports a receiver on it
    CHECK(root["channelOutputs"][0]["outputs"][4]["expansionType"] == 0);
}

TEST_CASE("FPP pixel upload marks an expansion with receivers", "[fpp3][outputs][pixel][smart]") {
    FppShow s;
    s.OnPort("A", "ws2811", 6, 10, "!Ctl:1", "SmartRemote=\"1\" SmartRemoteType=\"falcon_v1\"");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv);
    auto f = MakeFpp(srv, 9);
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    auto root = srv.LastJson("POST", kStrings);
    CHECK(root["channelOutputs"][0]["outputs"][5]["differentialType"] == 1);
    CHECK(root["channelOutputs"][0]["outputs"][4]["differentialType"] == 1);
    CHECK(root["channelOutputs"][0]["outputs"][4]["expansionType"] == 1);
}

TEST_CASE("FPP pixel upload with an unknown pinout asks before continuing", "[fpp3][outputs][pixel]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.Recalc();
    FppServer srv;
    srv.GetJson("/api/cape/strings", nlohmann::json::array({ "Other" }));
    RecordingUi ui;
    ui.answer = false;
    auto f = MakeFpp(srv, 9);
    f->_ui = &ui;
    CHECK(f->UploadPixelOutputs(&s.mm, &s.om, s.ctl));
    REQUIRE(ui.prompts.size() == 1);
    CHECK_THAT(ui.prompts[0], ContainsSubstring("Could not detect a pinout for TestCape for controller type TestCape."));
    CHECK_FALSE(srv.Has("POST", kStrings));

    ui.answer = true;
    CHECK_FALSE(f->UploadPixelOutputs(&s.mm, &s.om, s.ctl));
    CHECK(srv.Has("POST", kStrings));
}

TEST_CASE("FPP pixel upload finds versioned pinout files", "[fpp3][outputs][pixel]") {
    auto suffix = GENERATE(as<std::string>{}, "_v2", "_v3", "-v2", "-v3");
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.Recalc();
    FppServer srv;
    srv.GetJson("/api/cape/strings", nlohmann::json::array({ "TestCape" + suffix }));
    srv.Get("/api/cape/strings/TestCape" + suffix, R"({"driver":"BBShiftString"})");
    RecordingUi ui;
    auto f = MakeFpp(srv, 9);
    f->_ui = &ui;
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    INFO(suffix);
    CHECK(ui.prompts.empty());
    CHECK(srv.LastJson("POST", kStrings)["channelOutputs"][0]["type"] == "BBShiftString");
}

TEST_CASE("FPP pixel upload follows the cape's driver and says so", "[fpp3][outputs][pixel]") {
    FppShow s("TestShift");
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv, "TestShift", "");
    RecordingUi ui;
    auto f = MakeFpp(srv, 9);
    f->_ui = &ui;
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    // a cape that names no driver is a BBB48String board
    CHECK(srv.LastJson("POST", kStrings)["channelOutputs"][0]["type"] == "BBB48String");
    REQUIRE(ui.messages.size() == 1);
    CHECK_THAT(ui.messages[0], ContainsSubstring("is running the BBB48String output but the configured variant TestShift  expects BBShiftString."));
}

TEST_CASE("FPP pixel upload asks before replacing a different controller type", "[fpp3][outputs][pixel]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv);
    srv.Get(kStrings, R"({"channelOutputs":[{"type":"BBB48String","subType":"F8-B","outputs":[]}]})");
    RecordingUi ui;
    ui.answer = false;
    auto f = MakeFpp(srv, 9);
    f->_ui = &ui;
    f->controllerModel = "OtherCape";
    CHECK(f->UploadPixelOutputs(&s.mm, &s.om, s.ctl));
    REQUIRE(ui.prompts.size() == 1);
    CHECK_THAT(ui.prompts[0], ContainsSubstring("Configured controller type TestCape for " + srv.Address() + " is not compatible with type already configured: OtherCape."));
}

TEST_CASE("FPP pixel upload warns about protocols the string driver cannot send", "[fpp3][outputs][pixel]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.OnPort("T", "tm1814", 2, 10, "!Ctl:31");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv);
    RecordingUi ui;
    ui.answer = false;
    auto f = MakeFpp(srv, 9);
    f->_ui = &ui;
    CHECK(f->UploadPixelOutputs(&s.mm, &s.om, s.ctl));
    REQUIRE(ui.prompts.size() == 1);
    CHECK_THAT(ui.prompts[0], ContainsSubstring("Port 2 is set to tm1814, but this controller is running the BBB48String output"));
    CHECK_FALSE(srv.Has("POST", kStrings));

    // headless there is nobody to ask, so the upload goes ahead
    auto g = MakeFpp(srv, 9);
    CHECK_FALSE(g->UploadPixelOutputs(&s.mm, &s.om, s.ctl));
    CHECK(srv.LastJson("POST", kStrings)["channelOutputs"][0]["outputs"][1]["protocol"] == "tm1814");
}

TEST_CASE("FPP pixel upload on a shift string cape checks receivers against the protocol", "[fpp3][outputs][pixel][smart]") {
    auto row = GENERATE(table<std::string, std::string, std::string>({
        { "tm1814", "falcon_v4", "Port 1 is set to tm1814, which drives an inverted line that a Falcon smart receiver cannot be configured through." },
        { "ucs8904", "fpp_v2", "Port 1 is set to ucs8904, which is 16 bit. v1/v2 smart receivers cannot carry it and it will be sent as 8 bit." } }));
    FppShow s("TestShift");
    s.OnPort("T", std::get<0>(row), 1, 10, "!Ctl:1", "SmartRemote=\"1\" SmartRemoteType=\"" + std::get<1>(row) + "\"");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv, "TestShift", "BBShiftString");
    RecordingUi ui;
    ui.answer = true;
    auto f = MakeFpp(srv, 10);
    f->_ui = &ui;
    CHECK_FALSE(f->UploadPixelOutputs(&s.mm, &s.om, s.ctl));
    REQUIRE(ui.prompts.size() == 1);
    CHECK_THAT(ui.prompts[0], ContainsSubstring(std::get<2>(row)));
    auto root = srv.LastJson("POST", kStrings);
    CHECK(root["channelOutputs"][0]["type"] == "BBShiftString");
    CHECK(root["channelOutputs"][0]["outputs"][0]["protocol"] == std::get<0>(row));
}

TEST_CASE("FPP pixel upload on a shift string cape uses FPP's protocol names", "[fpp3][outputs][pixel]") {
    FppShow s("TestShift");
    s.OnPort("S", "ws2811 slow", 1, 10, "!Ctl:1");
    s.OnPort("W", "ws2812", 2, 10, "!Ctl:31");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv, "TestShift", "BBShiftString");
    RecordingUi ui;
    auto f = MakeFpp(srv, 10);
    f->_ui = &ui;
    CHECK_FALSE(f->UploadPixelOutputs(&s.mm, &s.om, s.ctl));
    CHECK(ui.prompts.empty());
    auto outs = srv.LastJson("POST", kStrings)["channelOutputs"][0]["outputs"];
    // FPP spells the slow ws2811 without the space, and ws2812 is driven as the cape's ws2811
    CHECK(outs[0]["protocol"] == "ws2811slow");
    CHECK(outs[1]["protocol"] == "ws2811");
}

TEST_CASE("FPP pixel upload leaves an unused cape serial output off", "[fpp3]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv);
    auto f = MakeFpp(srv, 9);
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    auto ser = srv.LastJson("POST", kStrings)["channelOutputs"][1];
    CHECK(ser["enabled"] == 0);
    CHECK(ser["subType"] == "off");
}

TEST_CASE("FPP pixel upload writes the cape's serial ports as BBBSerial", "[fpp3][outputs][serial]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 2, "!Ctl:1");
    s.OnPort("D", "dmx", 2, 4, "!Ctl:101");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv);
    auto f = MakeFpp(srv, 9);
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    auto root = srv.LastJson("POST", kStrings);
    REQUIRE(root["channelOutputs"].size() == 2);
    CHECK(root["channelOutputs"][0]["serialInUse"] == true);
    auto ser = root["channelOutputs"][1];
    CHECK(ser["type"] == "BBBSerial");
    CHECK(ser["subType"] == "DMX");
    CHECK(ser["device"] == "TestCape");
    CHECK(ser["enabled"] == 1);
    CHECK(ser["channelCount"] == 16); // never fewer than 16 DMX channels
    REQUIRE(ser["outputs"].size() == 2);
    CHECK(ser["outputs"][0]["outputNumber"] == 0);
    CHECK(ser["outputs"][0]["startChannel"] == 0);
    CHECK(ser["outputs"][0]["channelCount"] == 0);
    CHECK(ser["outputs"][1]["outputNumber"] == 1);
    CHECK(ser["outputs"][1]["startChannel"] == 101);
    CHECK(ser["outputs"][1]["outputType"] == "DMX");
    CHECK(f->ranges == "0-5,101-116");
}

TEST_CASE("FPP pixel upload pixelnet on the cape's serial port", "[fpp3][outputs][serial]") {
    FppShow s;
    s.OnPort("P", "pixelnet", 1, 4, "!Ctl:1");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv);
    auto f = MakeFpp(srv, 9);
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    auto ser = srv.LastJson("POST", kStrings)["channelOutputs"][1];
    CHECK(ser["subType"] == "PixelNet");
    CHECK(ser["channelCount"] == 4096);
    CHECK(ser["outputs"][0]["outputType"] == "Pixelnet");
    CHECK(ser["outputs"][0]["channelCount"] == 4096);
}

TEST_CASE("FPP pixel upload on a Pi hat writes co-pixelStrings and the uart to co-other", "[fpp3][outputs][serial]") {
    FppShow s("TestHat");
    s.OnPort("A", "ws2811", 2, 10, "!Ctl:1");
    s.OnPort("D", "dmx", 1, 2, "!Ctl:101");
    s.Recalc();
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    // the Pi hat has no Beagle pinout to look up
    CHECK_FALSE(srv.Has("GET", "/api/cape/strings"));
    auto root = srv.LastJson("POST", kPiStrings);
    REQUIRE(root["channelOutputs"].size() == 1);
    CHECK(root["channelOutputs"][0]["type"] == "RPIWS281X");
    CHECK(root["channelOutputs"][0]["subType"] == "TestHat");
    CHECK(root["channelOutputs"][0]["outputCount"] == 2);
    CHECK(Vs(root, 1)["description"] == "");
    CHECK(Vs(root, 2)["description"] == "A");

    // no co-other.json on the controller yet: ours is written as is
    auto other = srv.LastJson("POST", kOther);
    REQUIRE(other["channelOutputs"].size() == 1);
    auto o = other["channelOutputs"][0];
    CHECK(o["device"] == "ttyAMA0");
    CHECK(o["type"] == "DMX-Open");
    CHECK(o["enabled"] == 1);
    CHECK(o["startChannel"] == 101);
    CHECK(o["channelCount"] == 16);
    CHECK(o["description"] == "");
    CHECK_FALSE(o.contains("xLightsManaged"));
}

TEST_CASE("FPP pixel upload merges serial outputs into the controller's co-other", "[fpp3][outputs][serial]") {
    FppShow s("TestHat");
    s.OnPort("D", "dmx", 1, 20, "!Ctl:101");
    s.Recalc();
    FppServer srv;
    srv.Get(kOther, R"({"channelOutputs":[
        {"type":"DMX-Open","device":"ttyAMA0","enabled":0,"startChannel":1,"channelCount":16,"description":"old"},
        {"type":"Renard","device":"ttyAMA0","enabled":1,"startChannel":1,"channelCount":16},
        {"type":"GenericSerial","device":"ttyS5","enabled":1}]})");
    auto f = MakeFpp(srv, 9);
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    auto other = srv.LastJson("POST", kOther);
    REQUIRE(other["channelOutputs"].size() == 3);
    auto dmx = other["channelOutputs"][0];
    CHECK(dmx["enabled"] == 1);
    CHECK(dmx["startChannel"] == 101);
    CHECK(dmx["channelCount"] == 60);
    CHECK(dmx["description"] == "");
    CHECK(dmx["xLightsManaged"] == 1);
    // another output on the same device is turned off; one elsewhere is left alone
    CHECK(other["channelOutputs"][1]["enabled"] == 0);
    CHECK(other["channelOutputs"][2]["enabled"] == 1);

    // the same upload again changes nothing
    srv.GetJson(kOther, other);
    auto g = MakeFpp(srv, 9);
    g->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    CHECK(srv.Count("POST", kOther) == 1);
}

TEST_CASE("FPP pixel upload turns off an xLights output whose port is now empty", "[fpp3][outputs][serial]") {
    FppShow s("TestHat");
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.Recalc();
    FppServer srv;
    srv.Get(kOther, R"({"channelOutputs":[
        {"type":"DMX-Open","device":"ttyAMA0","enabled":1,"xLightsManaged":1,"startChannel":1,"channelCount":16},
        {"type":"DMX-Open","device":"ttyAMA0","enabled":1,"startChannel":1,"channelCount":16,"note":"set up by hand"}]})");
    auto f = MakeFpp(srv, 9);
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    auto other = srv.LastJson("POST", kOther);
    CHECK(other["channelOutputs"][0]["enabled"] == 0);
    CHECK(other["channelOutputs"][1]["enabled"] == 1);
}

TEST_CASE("FPP pixel upload writes USB adapter ports under their stable names", "[fpp3][outputs][serial]") {
    FppShow s("TestShift");
    s.OnPort("A", "ws2811", 1, 2, "!Ctl:1");
    s.OnPort("D", "DMX-Open", 17, 4, "!Ctl:101");      // ttyUSB0
    s.OnPort("R", "Renard", 18, 4, "!Ctl:201");        // ttyUSB1
    s.Recalc();
    FppServer srv;
    CapeKnown(srv, "TestShift", "BBShiftString");
    srv.Get("/api/channel/output/serialDevices", R"({"devices":[
        {"device":"ttyUSB0","stable":"serial/by-id/usb-A","byId":"serial/by-id/usb-A"},
        {"device":"ttyUSB1","byPath":"serial/by-path/p1"}]})");
    // an existing co-other.json, so new outputs get FPP's defaults
    srv.Get(kOther, R"({"channelOutputs":[]})");
    auto f = MakeFpp(srv, 9);
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    auto other = srv.LastJson("POST", kOther);
    REQUIRE(other["channelOutputs"].size() == 2);
    auto d = other["channelOutputs"][0];
    CHECK(d["device"] == "serial/by-id/usb-A");
    CHECK(d["xLightsPort"] == "ttyUSB0");
    CHECK(d["type"] == "DMX-Open");
    CHECK(d["channelCount"] == 16);
    auto r = other["channelOutputs"][1];
    CHECK(r["device"] == "ttyUSB1"); // no stable name offered
    CHECK(r["type"] == "Renard");
    CHECK(r["renardparm"] == "8N1");
    CHECK(r["renardspeed"] == 57600);
    // adapters are not the cape's own serial output
    CHECK(srv.LastJson("POST", kStrings)["channelOutputs"].size() == 1);
}

TEST_CASE("FPP pixel upload keeps an adapter written before under the name it had", "[fpp3][outputs][serial]") {
    FppShow s("TestShift");
    s.OnPort("D", "DMX-Open", 17, 4, "!Ctl:101");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv, "TestShift", "BBShiftString");
    srv.Get("/api/channel/output/serialDevices", R"({"devices":[{"device":"ttyUSB0","stable":"serial/by-id/usb-B"}]})");

    SECTION("recorded port keeps its device") {
        srv.Get(kOther, R"({"channelOutputs":[{"type":"DMX-Open","device":"serial/by-id/usb-A","xLightsPort":"ttyUSB0","xLightsManaged":1,"enabled":1,"startChannel":1,"channelCount":16}]})");
        auto f = MakeFpp(srv, 9);
        f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
        auto o = srv.LastJson("POST", kOther)["channelOutputs"];
        REQUIRE(o.size() == 1);
        CHECK(o[0]["device"] == "serial/by-id/usb-A");
        CHECK(o[0]["startChannel"] == 101);
    }
    SECTION("an output written under the tty moves to the stable name") {
        srv.Get(kOther, R"({"channelOutputs":[{"type":"DMX-Open","device":"ttyUSB0","xLightsManaged":1,"enabled":1,"startChannel":1,"channelCount":16}]})");
        auto f = MakeFpp(srv, 9);
        f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
        auto o = srv.LastJson("POST", kOther)["channelOutputs"];
        REQUIRE(o.size() == 1);
        CHECK(o[0]["device"] == "serial/by-id/usb-B");
        CHECK(o[0]["xLightsPort"] == "ttyUSB0");
    }
    SECTION("an output set up on the controller under the adapter's name is reused") {
        srv.Get(kOther, R"({"channelOutputs":[{"type":"DMX-Open","device":"serial/by-id/usb-B","enabled":0,"startChannel":1,"channelCount":16}]})");
        auto f = MakeFpp(srv, 9);
        f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
        auto o = srv.LastJson("POST", kOther)["channelOutputs"];
        REQUIRE(o.size() == 1);
        CHECK(o[0]["enabled"] == 1);
        CHECK(o[0]["xLightsPort"] == "ttyUSB0");
    }
}

TEST_CASE("FPP pixel upload refuses an adapter another port was set up on", "[fpp3][outputs][serial]") {
    FppShow s("TestShift");
    s.OnPort("D0", "DMX-Open", 17, 4, "!Ctl:101");
    s.OnPort("D1", "DMX-Open", 18, 4, "!Ctl:201");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv, "TestShift", "BBShiftString");
    // the adapter ttyUSB1 was set up on is now numbered ttyUSB0
    srv.Get("/api/channel/output/serialDevices", R"({"devices":[{"device":"ttyUSB0","stable":"serial/by-id/usb-X"},{"device":"ttyUSB1","stable":"serial/by-id/usb-Y"}]})");
    srv.Get(kOther, R"({"channelOutputs":[{"type":"DMX-Open","device":"serial/by-id/usb-X","xLightsPort":"ttyUSB1","xLightsManaged":1,"enabled":1,"startChannel":1,"channelCount":16}]})");
    RecordingUi ui;
    auto f = MakeFpp(srv, 9);
    f->_ui = &ui;
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    REQUIRE(ui.messages.size() == 1);
    CHECK_THAT(ui.messages[0], ContainsSubstring("ttyUSB0 is now the adapter ttyUSB1 was set up on (serial/by-id/usb-X)"));
    auto o = srv.LastJson("POST", kOther)["channelOutputs"];
    REQUIRE(o.size() == 1);
    CHECK(o[0]["startChannel"] == 201);
}

TEST_CASE("FPP pixel upload writes LOR enhanced units for an adapter", "[fpp3][outputs][serial]") {
    FppShow s("TestShift");
    s.OnPort("L", "LOR Enhanced", 17, 10, "!Ctl:1", "Speed=\"500000\"");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv, "TestShift", "BBShiftString");
    auto f = MakeFpp(srv, 9);
    f->UploadPixelOutputs(&s.mm, &s.om, s.ctl);
    auto o = srv.LastJson("POST", kOther)["channelOutputs"][0];
    CHECK(o["type"] == "LOREnhanced");
    CHECK(o["device"] == "ttyUSB0");
    CHECK(o["speed"] == 500000);
    REQUIRE(o["units"].size() == 1);
    CHECK(o["units"][0]["unitId"] == 1);
    CHECK(o["units"][0]["numOfPixels"] == 10);
    CHECK(o["units"][0]["lorStartPixel"] == 1);
    CHECK(o["units"][0]["fppStartAddr"] == 1);
    CHECK(o["channelCount"] == 30);
}

TEST_CASE("FPP pixel upload with nothing on the controller does nothing", "[fpp3][outputs][pixel]") {
    FppShow s;
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    CHECK_FALSE(f->UploadPixelOutputs(&s.mm, &s.om, s.ctl));
    CHECK(srv.Requests().empty());

    FppShow none("NoSuchCape");
    CHECK_FALSE(f->UploadPixelOutputs(&none.mm, &none.om, none.ctl));
    FppShow panels("TestPanels");
    CHECK_FALSE(f->UploadPixelOutputs(&panels.mm, &panels.om, panels.ctl));
    CHECK(srv.Requests().empty());
}

TEST_CASE("FPP serial controller upload writes co-other.json", "[fpp3][outputs][serial]") {
    auto row = GENERATE(table<std::string, std::string, int>({ { OUTPUT_DMX, "DMX-Pro", 16 }, { OUTPUT_OPENDMX, "DMX-Open", 16 }, { OUTPUT_PIXELNET, "Pixelnet-Lynx", 4096 },
                                                               { OUTPUT_OPENPIXELNET, "Pixelnet-Open", 4096 }, { OUTPUT_LOR, "LOR", 12 }, { OUTPUT_RENARD, "Renard", 12 },
                                                               { OUTPUT_GENERICSERIAL, "GenericSerial", 12 } }));
    VendorCapsFixture caps(kFppCaps);
    InitLoggers();
    OutputManager om;
    ModelManager mm{ &om };
    auto* c = new ControllerSerial(&om);
    c->SetName("Ser");
    c->SetProtocol(std::get<0>(row));
    c->SetPort("COM:ttyUSB3");
    c->SetSpeed(115200);
    c->SetVendor("FPP");
    c->SetModel("TestSerial");
    c->SetVariant("");
    om.AddController(c);
    om.SomethingChanged();
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<model name="S" DisplayAs="Single Line" parm1="1" parm2="4" parm3="1" StringType="RGB Nodes" StartChannel="!Ser:1" Controller="Ser"><ControllerConnection Protocol="dmx" Port="1"/></model>)"));
    mm.createAndAddModel(doc.first_child(), 100, 100);
    om.SomethingChanged();
    mm.RecalcStartChannels();

    FppServer srv;
    auto f = MakeFpp(srv, 9);
    INFO(std::get<0>(row));
    CHECK_FALSE(f->UploadSerialOutputs(&mm, &om, c));
    auto o = srv.LastJson("POST", kOther)["channelOutputs"];
    REQUIRE(o.size() == 1);
    CHECK(o[0]["type"] == std::get<1>(row));
    CHECK(o[0]["device"] == "ttyUSB3");
    CHECK(o[0]["startChannel"] == 1);
    CHECK(o[0]["channelCount"] == std::get<2>(row));
    CHECK(o[0]["description"] == "Ser");
    if (std::get<1>(row) == "LOR" || std::get<1>(row) == "Renard" || std::get<1>(row) == "GenericSerial") {
        CHECK(o[0]["speed"] == 115200);
    }
    CHECK(f->ranges == "0-" + std::to_string(std::get<2>(row) - 1));

    // a pixel controller is handled by the pixel upload instead
    FppShow s;
    CHECK_FALSE(f->UploadSerialOutputs(&s.mm, &s.om, s.ctl));
}

TEST_CASE("FPP panel upload moves the configured matrix to the model", "[fpp3][outputs][panel]") {
    FppShow s("TestPanels");
    s.Add(R"(<model name="P" DisplayAs="Matrix" parm1="16" parm2="32" parm3="1" StringType="RGB Nodes" StartChannel="!Ctl:101" Controller="Ctl"><ControllerConnection Protocol="LED Panel Matrix" Port="2"/></model>)");
    s.Recalc();
    FppServer srv;
    srv.Get("/api/channel/output/channelOutputsJSON", R"({"channelOutputs":[
        {"type":"LEDPanelMatrix","panelMatrixID":"1","subType":"ColorLight5a75","enabled":1,"startChannel":1,"channelCount":512},
        {"type":"LEDPanelMatrix","panelMatrixID":"2","subType":"RGBMatrix","enabled":0,"startChannel":1,"channelCount":1536},
        {"type":"Other"}]})");
    auto f = MakeFpp(srv, 9);
    CHECK_FALSE(f->UploadPanelOutputs(&s.mm, &s.om, s.ctl));
    auto j = srv.LastJson("POST", "/api/channel/output/channelOutputsJSON");
    CHECK(j["channelOutputs"][1]["startChannel"] == 101);
    CHECK(j["channelOutputs"][1]["enabled"] == 1);
    // matrix 1 has no models but xLights does not own the whole config, so it is left
    CHECK(j["channelOutputs"][0]["enabled"] == 1);
    CHECK(f->ranges == "100-1635");

    SECTION("under full control a matrix with no models is turned off") {
        s.ctl->SetFullxLightsControl(true);
        auto g = MakeFpp(srv, 9);
        g->UploadPanelOutputs(&s.mm, &s.om, s.ctl);
        CHECK(srv.LastJson("POST", "/api/channel/output/channelOutputsJSON")["channelOutputs"][0]["enabled"] == 0);
    }
}

TEST_CASE("FPP panel upload refuses a matrix of the wrong family or one that is missing", "[fpp3][outputs][panel]") {
    FppShow s("TestPanels");
    s.Add(R"(<model name="P" DisplayAs="Matrix" parm1="16" parm2="32" parm3="1" StringType="RGB Nodes" StartChannel="!Ctl:1" Controller="Ctl"><ControllerConnection Protocol="LED Panel Matrix - Hat/Cap/Cape" Port="1"/></model>)");
    s.Add(R"(<model name="Q" DisplayAs="Matrix" parm1="4" parm2="4" parm3="1" StringType="RGB Nodes" StartChannel="!Ctl:2000" Controller="Ctl"><ControllerConnection Protocol="LED Panel Matrix" Port="3"/></model>)");
    s.Recalc();
    FppServer srv;
    srv.Get("/api/channel/output/channelOutputsJSON", R"({"channelOutputs":[{"type":"LEDPanelMatrix","subType":"ColorLight5a75","enabled":1,"startChannel":1,"channelCount":512}]})");
    RecordingUi ui;
    auto f = MakeFpp(srv, 9);
    f->_ui = &ui;
    f->UploadPanelOutputs(&s.mm, &s.om, s.ctl);
    REQUIRE(ui.messages.size() == 2);
    CHECK_THAT(ui.messages[0], ContainsSubstring("LED Panel Matrix port 1 is set to 'LED Panel Matrix - Hat/Cap/Cape' but panel matrix 1 on"));
    CHECK_THAT(ui.messages[1], ContainsSubstring("Models are assigned to LED Panel Matrix port 3 but"));
    CHECK_FALSE(srv.Has("POST", "/api/channel/output/channelOutputsJSON"));
}

TEST_CASE("FPP panel upload with no panel models or panel support stays quiet", "[fpp3][outputs][panel]") {
    FppShow s("TestPanels");
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    CHECK_FALSE(f->UploadPanelOutputs(&s.mm, &s.om, s.ctl));
    FppShow cape;
    CHECK_FALSE(f->UploadPanelOutputs(&cape.mm, &cape.om, cape.ctl));
    CHECK(srv.Requests().empty());
}

TEST_CASE("FPP virtual matrix upload adds an HDMI output per model", "[fpp3][outputs][vmatrix]") {
    FppShow s("TestPanels");
    s.Add(R"(<model name="VM1" DisplayAs="Matrix" parm1="8" parm2="4" parm3="1" StringType="RGB Nodes" StartChannel="!Ctl:1" Controller="Ctl"><ControllerConnection Protocol="Virtual Matrix" Port="1"/></model>)");
    s.Add(R"(<model name="VM2" DisplayAs="Matrix" parm1="2" parm2="3" parm3="1" StringType="RGB Nodes" StartChannel="!Ctl:200" Controller="Ctl" Dir="L" ><ControllerConnection Protocol="Virtual Matrix" Port="1"/></model>)");
    s.Recalc();
    FppServer srv;
    srv.Get("/api/channel/output/co-other", R"({"channelOutputs":[{"type":"VirtualMatrix","description":"Gone","device":"HDMI-A-1","enabled":1}]})");
    auto f = MakeFpp(srv, 8);
    CHECK_FALSE(f->UploadVirtualMatrixOutputs(&s.mm, &s.om, s.ctl));
    auto j = srv.LastJson("POST", "/api/channel/output/co-other")["channelOutputs"];
    REQUIRE(j.size() == 3);
    CHECK(j[0]["description"] == "Gone");
    auto v1 = j[1];
    CHECK(v1["description"] == "VM1");
    CHECK(v1["type"] == "VirtualMatrix");
    CHECK(v1["device"] == "HDMI-A-1");
    CHECK(v1["startChannel"] == 1);
    CHECK(v1["channelCount"] == 96);
    CHECK(v1["width"] == 4);
    CHECK(v1["height"] == 8);
    CHECK(v1["layout"] == "4x8");
    CHECK(v1["scaling"] == "None");
    CHECK(v1["yoff"] == 0);
    CHECK(j[2]["yoff"] == 8);
    CHECK(j[2]["channelCount"] == 18);
    CHECK(srv.Count("PUT", "/api/settings/restartFlag") == 1);
}

TEST_CASE("FPP virtual matrix upload before FPP 8 names the framebuffer", "[fpp3][outputs][vmatrix]") {
    FppShow s("TestPanels");
    s.Add(R"(<model name="VM" DisplayAs="Matrix" parm1="8" parm2="4" parm3="1" StringType="RGB Nodes" StartChannel="!Ctl:1" Controller="Ctl"><ControllerConnection Protocol="Virtual Matrix" Port="1"/></model>)");
    s.Recalc();
    FppServer srv;
    auto f = MakeFpp(srv, 7);
    f->UploadVirtualMatrixOutputs(&s.mm, &s.om, s.ctl);
    auto v = srv.LastJson("POST", "/api/channel/output/co-other")["channelOutputs"][0];
    CHECK(v["device"] == "fb0");
    CHECK(v["scaling"] == "Hardware");
}

TEST_CASE("FPP virtual matrix upload updates a matching output in place", "[fpp3][outputs][vmatrix]") {
    FppShow s("TestPanels");
    s.Add(R"(<model name="VM" DisplayAs="Matrix" parm1="8" parm2="4" parm3="1" StringType="RGB Nodes" StartChannel="!Ctl:11" Controller="Ctl"><ControllerConnection Protocol="Virtual Matrix" Port="1"/></model>)");
    s.Recalc();
    FppServer srv;
    srv.Get("/api/channel/output/co-other", R"({"channelOutputs":[{"type":"VirtualMatrix","description":"VM","device":"fb0","enabled":0,"startChannel":1,"channelCount":3,"width":1,"height":1,"layout":"","yoff":3,"scaling":"None","colorOrder":"BGR"},
        {"type":"VirtualMatrix","description":"Stale","device":"fb0","enabled":1}]})");
    auto f = MakeFpp(srv, 7);
    f->UploadVirtualMatrixOutputs(&s.mm, &s.om, s.ctl);
    auto j = srv.LastJson("POST", "/api/channel/output/co-other")["channelOutputs"];
    REQUIRE(j.size() == 2);
    CHECK(j[0]["enabled"] == 1);
    // another matrix on the same framebuffer would fight it, so it is turned off
    CHECK(j[1]["enabled"] == 0);
    CHECK(j[0]["startChannel"] == 11);
    CHECK(j[0]["width"] == 4);
    CHECK(j[0]["yoff"] == 0);
    CHECK(j[0]["scaling"] == "Hardware");
    CHECK(j[0]["colorOrder"] == "BGR");

    SECTION("full control replaces every virtual matrix") {
        s.ctl->SetFullxLightsControl(true);
        auto g = MakeFpp(srv, 9);
        g->UploadVirtualMatrixOutputs(&s.mm, &s.om, s.ctl);
        auto k = srv.LastJson("POST", "/api/channel/output/co-other")["channelOutputs"];
        REQUIRE(k.size() == 1);
        CHECK(k[0]["colorOrder"] == "RGB");
        CHECK(k[0]["device"] == "HDMI-A-1");
    }
}

// Not under full control, matrices of other models on the port are disabled; FPP 8+ names
// the device "HDMI-A-1" rather than "fb0".
TEST_CASE("FPP virtual matrix upload on FPP 8+ keeps the matrix it updates enabled", "[fpp3]") {
    FppShow s("TestPanels");
    s.Add(R"(<model name="VM" DisplayAs="Matrix" parm1="8" parm2="4" parm3="1" StringType="RGB Nodes" StartChannel="!Ctl:11" Controller="Ctl"><ControllerConnection Protocol="Virtual Matrix" Port="1"/></model>)");
    s.Recalc();
    FppServer srv;
    srv.Get("/api/channel/output/co-other", R"({"channelOutputs":[{"type":"VirtualMatrix","description":"VM","device":"HDMI-A-1","enabled":0,"startChannel":1}]})");
    auto f = MakeFpp(srv, 9);
    f->UploadVirtualMatrixOutputs(&s.mm, &s.om, s.ctl);
    auto j = srv.LastJson("POST", "/api/channel/output/co-other")["channelOutputs"];
    CHECK(j[0]["enabled"] == 1);
}

TEST_CASE("FPP PWM upload builds the PCA9685 output", "[fpp3][outputs][pwm]") {
    FppShow s("TestHat");
    s.Add(R"(<model name="PW" DisplayAs="Single Line" parm1="1" parm2="2" parm3="1" StringType="Node Single Color" StartChannel="!Ctl:10" Controller="Ctl"><ControllerConnection Protocol="PWM" Port="1"/></model>)");
    s.Recalc();
    FppServer srv;
    srv.Get("/api/cape", R"({"id":"TestHat","provides":["strings","pwm"]})");
    auto f = MakeFpp(srv, 8);
    CHECK_FALSE(f->UploadPWMOutputs(&s.mm, &s.om, s.ctl));
    auto j = srv.LastJson("POST", "/api/configfile/co-pwm.json");
    auto pca = j["channelOutputs"][0];
    CHECK(pca["type"] == "PCA9685");
    CHECK(pca["subType"] == "TestHat");
    CHECK(pca["frequency"] == "50hz");
    REQUIRE(pca["outputs"].size() == 3);
    CHECK(pca["outputs"][0]["type"] == "LED");
    // PWM channels are 0 based, like the other FPP channel outputs
    CHECK(pca["outputs"][0]["startChannel"] == 9);
    CHECK(pca["outputs"][0]["is16bit"] == 0);
    CHECK(pca["outputs"][1]["startChannel"] == 10);
    CHECK_THAT(pca["outputs"][0]["description"].get<std::string>(), ContainsSubstring("PW - "));
    // the unused port is a default servo
    CHECK(pca["outputs"][2]["type"] == "Servo");
    CHECK(pca["outputs"][2]["min"] == 1000);
    CHECK(pca["outputs"][2]["zero"] == "Hold");
}

TEST_CASE("FPP PWM upload needs FPP 8 and a cape that provides PWM", "[fpp3][outputs][pwm]") {
    FppShow s("TestHat");
    s.Add(R"(<model name="PW" DisplayAs="Single Line" parm1="1" parm2="1" parm3="1" StringType="Single Color White" StartChannel="!Ctl:10" Controller="Ctl"><ControllerConnection Protocol="PWM" Port="1"/></model>)");
    s.Recalc();
    FppServer srv;
    srv.Get("/api/cape", R"({"id":"TestHat","provides":["strings"]})");
    auto old = MakeFpp(srv, 7);
    CHECK(old->UploadPWMOutputs(&s.mm, &s.om, s.ctl));
    auto f = MakeFpp(srv, 8);
    CHECK(f->UploadPWMOutputs(&s.mm, &s.om, s.ctl));
    CHECK_FALSE(srv.Has("POST", "/api/configfile/co-pwm.json"));
    FppShow cape;
    CHECK_FALSE(f->UploadPWMOutputs(&cape.mm, &cape.om, cape.ctl));
}

TEST_CASE("FPP PWM upload updates an existing PCA9685 output", "[fpp3][outputs][pwm]") {
    FppShow s("TestHat");
    s.Add(R"(<model name="PW" DisplayAs="Single Line" parm1="1" parm2="1" parm3="1" StringType="Single Color White" StartChannel="!Ctl:10" Controller="Ctl"><ControllerConnection Protocol="PWM" Port="2"/></model>)");
    s.Recalc();
    FppServer srv;
    srv.Get("/api/cape", R"({"id":"TestHat","provides":["pwm"]})");
    srv.Get("/api/configfile/co-pwm.json", R"({"channelOutputs":[{"type":"Other"},{"type":"PCA9685","frequency":"200hz","outputs":[{"type":"Servo","description":"keep"}]}]})");
    auto f = MakeFpp(srv, 8);
    f->UploadPWMOutputs(&s.mm, &s.om, s.ctl);
    auto j = srv.LastJson("POST", "/api/configfile/co-pwm.json")["channelOutputs"];
    REQUIRE(j.size() == 2);
    CHECK(j[1]["frequency"] == "200hz");
    REQUIRE(j[1]["outputs"].size() == 3);
    CHECK(j[1]["outputs"][0]["description"] == "keep");
    CHECK(j[1]["outputs"][1]["type"] == "LED");
    CHECK(j[1]["outputs"][1]["startChannel"] == 9);
}

TEST_CASE("FPP SetOutputs runs every upload after authenticating", "[fpp3][outputs]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv);
    srv.Get("/config.php", "settings['Title'] = \"Falcon Player\";\n");
    srv.Get("/api/system/info", R"({"Version":"8.2","Platform":"BeagleBone Black"})");
    auto f = MakeFpp(srv, 0);
    f->sysInfoLoaded = false;
    RecordingUi ui;
    CHECK(f->SetOutputs(&s.mm, &s.om, s.ctl, &ui));
    CHECK(f->majorVersion == 8);
    CHECK(srv.Has("POST", kStrings));
    // FPP 8 sets the restart flag, then Restart clears it
    CHECK(srv.Has("GET", "/api/system/fppd/restart?quick=1"));
    CHECK(srv.Count("PUT", "/api/settings/restartFlag") == 2);
    CHECK_FALSE(f->restartNeeded);
}

TEST_CASE("FPP UploadForImmediateOutput uploads and restarts only when needed", "[fpp3][outputs]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.Recalc();
    FppServer srv;
    CapeKnown(srv);
    srv.Get("/config.php", "x");
    srv.Get("/api/system/info", R"({"Version":"9.1"})");
    srv.Get("/api/configfile/ci-universes.json", R"({"channelInputs":[{"enabled":1}]})");
    auto f = MakeFpp(srv, 0);
    f->sysInfoLoaded = false;
    CHECK(f->UploadForImmediateOutput(&s.mm, &s.om, s.ctl, nullptr));
    CHECK(srv.Has("POST", kStrings));
    // DDP input already on and a DDP controller: no inputs to send, no restart
    CHECK_FALSE(srv.Has("POST", "/api/channel/output/universeInputs"));
    CHECK_FALSE(srv.Has("GET", "/api/system/fppd/restart?quick=1"));

    FppServer down;
    auto g = MakeFpp(down, 0);
    g->sysInfoLoaded = false;
    CHECK_FALSE(g->UploadForImmediateOutput(&s.mm, &s.om, s.ctl, nullptr));
}

#endif
