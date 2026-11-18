// The handlers are reached through ExposeVendors.h and a loopback server, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <mutex>

#include "support/ExposeVendors.h"
#include "controllers/Falcon.h"
#include "support/UnexposeVendors.h"
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

// A V3 Falcon: status.xml, strings.xml and the form posts it accepts.
struct FalconV3 {
    std::string status = "<status><fv>2.60</fv><a>1</a><n>Falcon</n><p>5</p><m>0</m></status>";
    std::string strings;
    std::string home = "<html><input type=\"hidden\" name=\"m\" id=\"m\"  value=\"0\" /></html>";
    std::mutex m;
    std::vector<FakeRequest> posts;
    std::unique_ptr<FakeHttpServer> srv;

    FalconV3() {
        // four configured ports with distinctive settings
        strings = "<strings m=\"0\" k0=\"340\" k1=\"340\" k2=\"344\">";
        for (int p = 0; p < 4; ++p) {
            strings += "<vs y=\"Old" + std::to_string(p) + "\" p=\"" + std::to_string(p) + "\" u=\"" + std::to_string(2 + p) +
                       "\" us=\"9\" c=\"77\" g=\"1\" t=\"0\" d=\"1\" o=\"3\" n=\"0\" z=\"0\" b=\"4\" ga=\"2\"/>";
        }
        strings += "</strings>";
        srv = std::make_unique<FakeHttpServer>([this](const FakeRequest& r) {
            FakeResponse resp;
            resp.contentType = "text/xml";
            if (r.method == "POST") {
                std::lock_guard<std::mutex> l(m);
                posts.push_back(r);
                resp.body = "OK";
            } else if (r.path == "/status.xml") {
                resp.body = status;
            } else if (r.path == "/strings.xml") {
                resp.body = strings;
            } else if (r.path == "/") {
                resp.body = home;
            } else {
                resp.status = 404;
            }
            return resp;
        });
    }
    std::vector<FakeRequest> Posts(const std::string& path) {
        std::lock_guard<std::mutex> l(m);
        std::vector<FakeRequest> r;
        for (auto& p : posts) if (p.path == path) r.push_back(p);
        return r;
    }
};

std::unique_ptr<Falcon> ConnectV3(FalconV3& fv) {
    VendorsTestInit();
    return std::make_unique<Falcon>(fv.srv->Address(), "");
}

} // namespace

// SetOutputs sorts the strings with FalconString::operator>; it must be a strict weak
// ordering or libc++'s debug check aborts the upload.
TEST_CASE("Falcon3 V3 SetOutputs posts string ports built from the models", "[vendors3]") {
    FalconV3 fv;
    auto f = ConnectV3(fv);
    REQUIRE(f->IsConnected());
    VendorShow s("Falcon", "F16V3", "No Expansion Boards", 4);
    s.Pixels("Arch", 1, 50, 1, "nullNodes=\"2\" Brightness=\"50\" colorOrder=\"GRB\" groupCount=\"2\" reverse=\"1\" gamma=\"2.3\"");
    s.Pixels("Tree", 2, 100, 511);
    REQUIRE_UPLOAD(s, f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    INFO(s.ui.All());

    auto posts = fv.Posts("/StringPorts.htm");
    REQUIRE(posts.size() == 1);
    auto form = Form(posts[0].body);
    CHECK(form["r"] == "1");
    CHECK(form["m"] == "0");
    CHECK(form["S"] == "16");
    CHECK(form["q"] == "0");
    // V3 firmware supports variable expansion: the unused banks are folded into the main one
    CHECK(form["k0"] == "1024");
    CHECK(form["k1"] == "0");
    CHECK(form["k2"] == "0");

    // port 1: the model's settings
    CHECK(form["p0"] == "0");
    CHECK(form["x0"] == "0");
    CHECK(form["t0"] == "0");
    CHECK(form["u0"] == "1");
    CHECK(form["s0"] == "1");
    CHECK(form["c0"] == "100"); // 50 pixels grouped by 2
    CHECK(form["y0"] == "Arch");
    CHECK(form["b0"] == "9");
    CHECK(form["n0"] == "2");
    CHECK(form["G0"] == "2");
    CHECK(form["o0"] == "2");
    CHECK(form["d0"] == "1");
    CHECK(form["g0"] == "2");
    CHECK(form["w0"] == "0");

    // port 2: unset settings come from what the controller had on that port
    CHECK(form["p1"] == "1");
    CHECK(form["u1"] == "2");
    CHECK(form["s1"] == "1");
    CHECK(form["c1"] == "100");
    CHECK(form["y1"] == "Tree");
    CHECK(form["b1"] == "4");
    CHECK(form["G1"] == "2");
    CHECK(form["o1"] == "3");
    CHECK(form["d1"] == "1");
    CHECK(form["n1"] == "0");
    CHECK(form["g1"] == "1");

    // port 3 has no model so the controller's string is sent back unchanged
    CHECK(form["p2"] == "2");
    CHECK(form["y2"] == "Old2");
    CHECK(form["u2"] == "4");
    CHECK(form["s2"] == "10");
    CHECK(form["c2"] == "77");

    // ports the controller did not report are filled with one pixel defaults
    CHECK(form["p15"] == "15");
    CHECK(form["c15"] == "1");
    CHECK(form["u15"] == "1");
    CHECK(form["y15"] == "");
    CHECK(fv.Posts("/SerialOutputs.htm").empty());
}

TEST_CASE("Falcon3 V3 SetOutputs in absolute mode sends absolute start channels and serial ports", "[vendors3]") {
    FalconV3 fv;
    fv.status = "<status><fv>2.60</fv><a>0</a><n>Falcon</n><p>5</p></status>";
    auto f = ConnectV3(fv);
    REQUIRE(f->_usingAbsolute);
    VendorShow s("Falcon", "F16V3", "No Expansion Boards", 4);
    s.Pixels("Tree", 2, 100, 511);
    s.Serial("Flood", 2, 8, 1531);
    REQUIRE_UPLOAD(s, f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto form = Form(fv.Posts("/StringPorts.htm").at(0).body);
    CHECK(form["p1"] == "1");
    CHECK(form["s1"] == "511");
    CHECK(form["u1"] == "2");

    auto serial = fv.Posts("/SerialOutputs.htm");
    REQUIRE(serial.size() == 1);
    auto sf = Form(serial[0].body);
    CHECK(sf["a"] == "0");
    CHECK(sf["btnSave"] == "Save");
    CHECK(sf["t1"] == "0");
    CHECK(sf["s1"] == "1531");
    CHECK(sf.count("u1") == 0);
}

TEST_CASE("Falcon3 V3 serial ports in universe mode name the universe", "[vendors3]") {
    FalconV3 fv;
    auto f = ConnectV3(fv);
    VendorShow s("Falcon", "F16V3", "No Expansion Boards", 4);
    s.Serial("Flood", 1, 8, 1531, "Renard");
    REQUIRE_UPLOAD(s, f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto sf = Form(fv.Posts("/SerialOutputs.htm").at(0).body);
    CHECK(sf["a"] == "1");
    CHECK(sf["t0"] == "2");
    CHECK(sf["u0"] == "4");
    CHECK(sf["s0"] == "1");
}

TEST_CASE("Falcon3 V3 SetOutputs under full control ignores the controller's strings", "[vendors3]") {
    FalconV3 fv;
    auto f = ConnectV3(fv);
    VendorShow s("Falcon", "F16V3", "No Expansion Boards", 2);
    s.ctl->SetFullxLightsControl(true);
    s.ctl->SetDefaultBrightnessUnderFullControl(80);
    s.Pixels("Arch", 1, 20, 1);
    REQUIRE_UPLOAD(s, f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(fv.srv->Count("/strings.xml") == 0);
    auto form = Form(fv.Posts("/StringPorts.htm").at(0).body);
    CHECK(form["S"] == "16");
    CHECK(form["c0"] == "20");
    CHECK(form["b0"] == "4");
    CHECK(form["b3"] == "4");
    CHECK(form["y3"] == "");
    CHECK(form["c3"] == "1");
}

TEST_CASE("Falcon3 V3 SetOutputs with smart remotes fills every port in the group", "[vendors3]") {
    FalconV3 fv;
    auto f = ConnectV3(fv);
    VendorShow s("Falcon", "F16V3", "No Expansion Boards", 2);
    s.Pixels("Arch", 1, 20, 1, "SmartRemote=\"2\" SmartRemoteType=\"falcon_v1\"");
    REQUIRE_UPLOAD(s, f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    INFO(s.ui.All());
    auto form = Form(fv.Posts("/StringPorts.htm").at(0).body);
    // ports 1-4 each get remotes A and B, the rest of the board stays plain
    int remotes = 0;
    for (int i = 0; i < 40; ++i) {
        auto k = "p" + std::to_string(i);
        if (!form.count(k)) break;
        if (std::strtol(form[k].c_str(), nullptr, 10) < 4) {
            ++remotes;
            CHECK(form["w" + std::to_string(i)] != "0");
        }
    }
    CHECK(remotes == 8);
    // strings are sorted by port group, then remote, then port
    CHECK(form["p0"] == "0");
    CHECK(form["w0"] == "1");
    CHECK(form["p1"] == "1");
    CHECK(form["w1"] == "1");
    CHECK(form["p4"] == "0");
    CHECK(form["w4"] == "2");
    CHECK(form["c4"] == "20");
}

TEST_CASE("Falcon3 V3 SetOutputs refuses ports with too many pixels", "[vendors3]") {
    FalconV3 fv;
    auto f = ConnectV3(fv);
    VendorShow s("Falcon", "F16V3", "No Expansion Boards", 8);
    s.Pixels("Long", 1, 1100, 1);
    CHECK_FALSE(f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Not uploaded due to errors"));
    CHECK(fv.Posts("/StringPorts.htm").empty());
}

TEST_CASE("Falcon3 V3 SetOutputs refuses to mix smart and plain remotes in a group", "[vendors3]") {
    FalconV3 fv;
    auto f = ConnectV3(fv);
    VendorShow s("Falcon", "F16V3", "No Expansion Boards", 2);
    s.Pixels("Smart", 1, 10, 1, "SmartRemote=\"1\" SmartRemoteType=\"falcon_v1\"");
    s.Pixels("Plain", 2, 10, 31);
    CHECK_FALSE(f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Not uploaded due to errors"));
    CHECK(fv.Posts("/StringPorts.htm").empty());
}

TEST_CASE("Falcon3 V3 SetOutputs failure paths", "[vendors3][falcon3]") {
    FalconV3 fv;
    auto f = ConnectV3(fv);
    SECTION("board model does not match") {
        VendorShow s("Falcon", "F16V4", "16 Local Ports", 1);
        CHECK_FALSE(f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("Board version does not match"));
        CHECK_FALSE(f->SetInputUniverses(s.ctl, &s.ui));
    }
    SECTION("strings.xml missing") {
        fv.strings.clear();
        VendorShow s("Falcon", "F16V3", "No Expansion Boards", 1);
        s.Pixels("Arch", 1, 10, 1);
        CHECK_FALSE(f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("strings.xml could not be retrieved"));
    }
    SECTION("strings.xml is not xml") {
        fv.strings = "<strings><vs";
        VendorShow s("Falcon", "F16V3", "No Expansion Boards", 1);
        s.Pixels("Arch", 1, 10, 1);
        CHECK_FALSE(f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("Could not parse Falcon strings.xml"));
    }
    CHECK(fv.Posts("/StringPorts.htm").empty());
}

TEST_CASE("Falcon3 V2 SetOutputs splits pixels between main and expansion", "[vendors3]") {
    FalconV3 fv;
    fv.status = "<status><fv>1.10</fv><a>1</a><n>Falcon</n><p>1</p></status>";
    auto f = ConnectV3(fv);
    REQUIRE(f->IsV2());
    VendorShow s("Falcon", "F16V2", "One Expansion Board", 4);
    s.Pixels("Low", 1, 100, 1);
    s.Pixels("High", 20, 100, 511);
    REQUIRE_UPLOAD(s, f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    INFO(s.ui.All());
    auto form = Form(fv.Posts("/StringPorts.htm").at(0).body);
    CHECK(form["m"] == "1");
    CHECK(form["S"] == "32");
    // fixed split on old V2 firmware: no k values
    CHECK(form.count("k0") == 0);
}

TEST_CASE("Falcon3 V2 SetOutputs rejects more than half the pixels with an expansion", "[vendors3]") {
    FalconV3 fv;
    fv.status = "<status><fv>1.10</fv><a>1</a><n>Falcon</n><p>1</p></status>";
    auto f = ConnectV3(fv);
    VendorShow s("Falcon", "F16V2", "One Expansion Board", 4);
    s.Pixels("Low", 1, 400, 1);
    s.Pixels("High", 20, 10, 1201);
    CHECK_FALSE(f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("only supports 340/340 pixel split"));
    CHECK(fv.Posts("/StringPorts.htm").empty());
}

TEST_CASE("Falcon3 V3 SetInputUniverses posts the universe table", "[vendors3][falcon3]") {
    FalconV3 fv;
    auto f = ConnectV3(fv);
    VendorShow s("Falcon", "F16V3", "No Expansion Boards", 3, OUTPUT_E131, 510, 10);
    REQUIRE_UPLOAD(s, f->SetInputUniverses(s.ctl, &s.ui));
    auto posts = fv.Posts("/E131.htm");
    REQUIRE(posts.size() == 1);
    auto form = Form(posts[0].body);
    CHECK(form["z"] == "3");
    CHECK(form["a"] == "1");
    CHECK(form["u0"] == "10");
    CHECK(form["u2"] == "12");
    CHECK(form["s1"] == "510");
    CHECK(form["c1"] == "511");
    CHECK(form["t0"] == "0");
    CHECK(fv.Posts("/index.htm").empty());
}

TEST_CASE("Falcon3 V3 SetInputUniverses marks artnet universes", "[vendors3][falcon3]") {
    FalconV3 fv;
    auto f = ConnectV3(fv);
    VendorShow s("Falcon", "F16V3", "No Expansion Boards", 2, OUTPUT_ARTNET, 510, 0);
    REQUIRE_UPLOAD(s, f->SetInputUniverses(s.ctl, &s.ui));
    auto form = Form(fv.Posts("/E131.htm").at(0).body);
    CHECK(form["t0"] == "1");
    CHECK(form["t1"] == "1");
}

TEST_CASE("Falcon3 V3 SetInputUniverses refuses more than 96 universes", "[vendors3][falcon3]") {
    FalconV3 fv;
    auto f = ConnectV3(fv);
    VendorShow s("Falcon", "F16V3", "No Expansion Boards", 97);
    CHECK_FALSE(f->SetInputUniverses(s.ctl, &s.ui));
    CHECK(s.ui.Said("only 96 are supported"));
    CHECK(fv.Posts("/E131.htm").empty());
}

TEST_CASE("Falcon3 V3 SetInputUniverses refuses DDP on old firmware", "[vendors3][falcon3]") {
    FalconV3 fv;
    fv.status = "<status><fv>2.50</fv><a>1</a><n>Falcon</n><p>5</p></status>";
    auto f = ConnectV3(fv);
    VendorShow s("Falcon", "F16V3", "No Expansion Boards", 1, OUTPUT_DDP);
    CHECK_FALSE(f->SetInputUniverses(s.ctl, &s.ui));
    CHECK(s.ui.Said("Install 2.58 or higher"));
}

TEST_CASE("Falcon3 V3 SetInputUniverses switches an already DDP board with one post", "[vendors3][falcon3]") {
    FalconV3 fv;
    fv.home = "<html><input type=\"hidden\" name=\"m\" id=\"m\"  value=\"64\" /></html>";
    auto f = ConnectV3(fv);
    VendorShow s("Falcon", "F16V3", "No Expansion Boards", 1, OUTPUT_DDP);
    REQUIRE_UPLOAD(s, f->SetInputUniverses(s.ctl, &s.ui));
    auto posts = fv.Posts("/index.htm");
    REQUIRE(posts.size() == 1);
    CHECK(posts[0].body == "c=64&d=1");
    CHECK(f->_usingAbsolute);
}

// ---------------------------------------------------------------- V4

namespace {

std::string V4Reply(const std::string& payload, int r = 200, int f = 1, int b = 0) {
    return "{\"R\":" + std::to_string(r) + ",\"T\":\"Q\",\"F\":" + std::to_string(f) + ",\"B\":" + std::to_string(b) +
           ",\"M\":\"X\",\"RB\":0,\"P\":" + payload + ",\"W\":\" \",\"L\":\"\"}";
}

struct FalconV4 {
    nlohmann::json status = { { "BR", 16 }, { "O", 0 }, { "A", 1 }, { "B", 0 }, { "ps", 0 }, { "I", "198.51.100.1" }, { "sr", 250000 } };
    nlohmann::json strings = nlohmann::json::array();
    nlohmann::json inputs = nlohmann::json::array({ { { "u", 1 }, { "c", 510 }, { "uc", 4 }, { "p", "e" } } });
    std::mutex m;
    std::vector<nlohmann::json> sets; // every "S" request
    std::unique_ptr<FakeHttpServer> srv;

    FalconV4() {
        for (int p = 0; p < 16; ++p) {
            strings.push_back({ { "p", p }, { "s", 0 }, { "r", 0 }, { "u", 1 + p / 4 }, { "sc", 0 }, { "n", 30 + p }, { "z", 0 }, { "ns", 0 }, { "ne", 0 },
                                { "g", 23 }, { "o", 1 }, { "v", 0 }, { "b", 70 }, { "gp", 1 }, { "nm", "Old " + std::to_string(p) }, { "bl", 0 }, { "l", 14 } });
        }
        srv = std::make_unique<FakeHttpServer>([this](const FakeRequest& r) {
            FakeResponse resp;
            if (r.path == "/status.xml") {
                resp.contentType = "text/xml";
                resp.body = "<status><v>1.0</v><n>v4</n><p>128</p></status>";
                return resp;
            }
            auto j = nlohmann::json::parse(r.body, nullptr, false);
            std::string t = j.value("T", "");
            std::string mth = j.value("M", "");
            std::lock_guard<std::mutex> l(m);
            if (t == "S") {
                sets.push_back(j);
                resp.body = V4Reply("{}");
            } else if (mth == "ST") {
                resp.body = V4Reply(status.dump());
            } else if (mth == "SP") {
                resp.body = V4Reply(nlohmann::json({ { "A", strings } }).dump());
            } else if (mth == "IN") {
                resp.body = V4Reply(nlohmann::json({ { "A", inputs } }).dump());
            } else {
                resp.status = 404;
            }
            return resp;
        });
    }
    std::vector<nlohmann::json> Sets(const std::string& method) {
        std::lock_guard<std::mutex> l(m);
        std::vector<nlohmann::json> r;
        for (auto& s : sets) if (s["M"] == method) r.push_back(s);
        return r;
    }
};

std::unique_ptr<Falcon> ConnectV4(FalconV4& fv) {
    VendorsTestInit();
    auto f = std::make_unique<Falcon>(fv.srv->Address(), "");
    return f;
}

} // namespace

TEST_CASE("Falcon3 V4 SetOutputs sends one string per call built from models and the board", "[vendors3][falcon3]") {
    FalconV4 fv;
    auto f = ConnectV4(fv);
    REQUIRE(f->IsConnected());
    REQUIRE(f->IsV4());
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 4);
    s.Pixels("Arch", 1, 50, 1, "nullNodes=\"2\" Brightness=\"50\" colorOrder=\"GRB\" groupCount=\"2\" reverse=\"1\" endNullNodes=\"3\"");
    s.Pixels("Tree", 2, 100, 511, "gamma=\"2.5\"");
    REQUIRE_UPLOAD(s, f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    INFO(s.ui.All());

    CHECK(fv.Sets("BM").empty()); // board already in the right mode
    auto sp = fv.Sets("SP");
    REQUIRE(sp.size() == 16);
    for (size_t i = 0; i < sp.size(); ++i) {
        CHECK(sp[i]["B"] == (int)i);
        CHECK(sp[i]["E"] == 16);
        CHECK(sp[i]["P"]["AD"] == 1);
        CHECK(sp[i]["P"]["B"] == 255);
        REQUIRE(sp[i]["P"]["A"].size() == 1);
    }
    auto a0 = sp[0]["P"]["A"][0];
    CHECK(a0["p"] == 0);
    CHECK(a0["nm"] == "Arch");
    CHECK(a0["n"] == 100);
    CHECK(a0["gp"] == 2);
    CHECK(a0["ns"] == 2);
    CHECK(a0["ne"] == 3);
    CHECK(a0["b"] == 50);
    CHECK(a0["o"] == 2);
    CHECK(a0["v"] == 1);
    CHECK(a0["g"] == 23); // not set on the model so the board's value is kept
    CHECK(a0["u"] == 1);
    CHECK(a0["sc"] == 0);
    CHECK(a0["l"] == 14);
    auto a1 = sp[1]["P"]["A"][0];
    CHECK(a1["nm"] == "Tree");
    CHECK(a1["u"] == 2);
    CHECK(a1["sc"] == 0);
    CHECK(a1["n"] == 100);
    CHECK(a1["g"] == 25);
    CHECK(a1["b"] == 70);
    // ports without models are resent as the board reported them
    auto a5 = sp[5]["P"]["A"][0];
    CHECK(a5["nm"] == "Old 5");
    CHECK(a5["n"] == 35);
    CHECK(a5["u"] == 2);
    CHECK(fv.Sets("SE").empty());
}

TEST_CASE("Falcon3 V4 SetOutputs uploads the serial port", "[vendors3][falcon3]") {
    FalconV4 fv;
    auto f = ConnectV4(fv);
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 4);
    s.Serial("Flood", 1, 8, 1531, "DMX", "Speed=\"250000\"");
    REQUIRE_UPLOAD(s, f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto se = fv.Sets("SE");
    REQUIRE(se.size() == 1);
    CHECK(se[0]["P"]["sm"] == 0);
    CHECK(se[0]["P"]["su"] == 4);
    CHECK(se[0]["P"]["ssc"] == 0);
    CHECK(se[0]["P"]["sr"] == 250000);
}

TEST_CASE("Falcon3 V4 renard at the DMX rate keeps the board's serial rate", "[vendors3][falcon3]") {
    // each serial upload waits a second, so only the 250000 case is run: it means "keep the
    // board's rate"; other rates outside 19200/38400/57600/115200 become 57600
    FalconV4 fv;
    fv.status["sr"] = 115200;
    auto f = ConnectV4(fv);
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 4);
    s.Serial("Flood", 1, 8, 1531, "Renard", "Speed=\"250000\"");
    REQUIRE_UPLOAD(s, f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto se = fv.Sets("SE");
    REQUIRE(se.size() == 1);
    CHECK(se[0]["P"]["sm"] == 1);
    CHECK(se[0]["P"]["sr"] == 115200);
}

TEST_CASE("Falcon3 V4 DMX serial port without a speed uses the DMX rate", "[vendors3]") {
    FalconV4 fv;
    auto f = ConnectV4(fv);
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 4);
    s.Serial("Flood", 1, 8, 1531);
    REQUIRE_UPLOAD(s, f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto se = fv.Sets("SE");
    REQUIRE(se.size() == 1);
    CHECK(se[0]["P"]["sr"] == 250000);
}

TEST_CASE("Falcon3 V4 SetOutputs in absolute mode sends zero based absolute channels", "[vendors3][falcon3]") {
    FalconV4 fv;
    fv.status["A"] = 0;
    for (auto& st : fv.strings) {
        st["u"] = 0;
        st["sc"] = 0;
    }
    auto f = ConnectV4(fv);
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 4);
    s.Pixels("Tree", 3, 100, 600);
    REQUIRE_UPLOAD(s, f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto sp = fv.Sets("SP");
    REQUIRE(sp.size() == 16);
    CHECK(sp[2]["P"]["AD"] == 0);
    CHECK(sp[2]["P"]["A"][0]["u"] == 0);
    CHECK(sp[2]["P"]["A"][0]["sc"] == 599);
}

TEST_CASE("Falcon3 V4 SetOutputs refuses a board in the wrong mode", "[vendors3][falcon3]") {
    FalconV4 fv;
    fv.status["B"] = 1;
    auto f = ConnectV4(fv);
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 4);
    s.Pixels("Arch", 1, 10, 1);
    CHECK_FALSE(f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Board is currently set to the wrong mode"));
    CHECK(fv.Sets("SP").empty());
}

TEST_CASE("Falcon3 V4 SetOutputs refuses mixed protocols within a bank", "[vendors3][falcon3]") {
    FalconV4 fv;
    fv.strings[3]["l"] = 0;
    auto f = ConnectV4(fv);
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 4);
    s.Pixels("Arch", 1, 10, 1);
    CHECK_FALSE(f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Port 4 has an invalid protocol configuration"));
    CHECK(fv.Sets("SP").empty());
}

TEST_CASE("Falcon3 V4 SetOutputs refuses a model protocol different from the bank", "[vendors3][falcon3]") {
    FalconV4 fv;
    auto f = ConnectV4(fv);
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 4);
    s.Pixels("Arch", 1, 10, 1, "", "apa102");
    CHECK_FALSE(f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Port 1 has an invalid protocol configuration"));
}

TEST_CASE("Falcon3 V4 SetOutputs aborts when the string query fails", "[vendors3][falcon3]") {
    FalconV4 fv;
    fv.strings = nlohmann::json::array({ 5 });
    auto f = ConnectV4(fv);
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 4);
    s.Pixels("Arch", 1, 10, 1);
    CHECK_FALSE(f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Failed to retrieve current string configuration"));
}

TEST_CASE("Falcon3 V4 SetOutputs with checks failing does not touch the board", "[vendors3][falcon3]") {
    FalconV4 fv;
    auto f = ConnectV4(fv);
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 8);
    s.Pixels("Long", 1, 1100, 1);
    CHECK_FALSE(f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Not uploaded due to errors"));
    CHECK(fv.Sets("SP").empty());
    CHECK(fv.Sets("BM").empty());
}

TEST_CASE("Falcon3 V4 SetOutputs under full control checks the inputs", "[vendors3][falcon3]") {
    FalconV4 fv;
    fv.inputs[0]["uc"] = 2;
    auto f = ConnectV4(fv);
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 4);
    s.ctl->SetFullxLightsControl(true);
    s.Pixels("Arch", 1, 10, 1);
    CHECK_FALSE(f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Falcon inputs not as expected"));
}

TEST_CASE("Falcon3 V4 SetOutputs under full control uses defaults for empty ports", "[vendors3][falcon3]") {
    FalconV4 fv;
    auto f = ConnectV4(fv);
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 4);
    s.ctl->SetFullxLightsControl(true);
    s.ctl->SetDefaultBrightnessUnderFullControl(40);
    s.ctl->SetDefaultGammaUnderFullControl(2.8f);
    s.Pixels("Arch", 1, 10, 1);
    REQUIRE_UPLOAD(s, f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto sp = fv.Sets("SP");
    REQUIRE(sp.size() == 16);
    CHECK(sp[0]["P"]["A"][0]["b"] == 40);
    CHECK(sp[0]["P"]["A"][0]["g"] == 28);
    auto a9 = sp[9]["P"]["A"][0];
    CHECK(a9["nm"] == "Port 10");
    CHECK(a9["n"] == 0);
    CHECK(a9["b"] == 40);
    CHECK(a9["g"] == 28);
    CHECK(a9["u"] == 1);
    CHECK(a9["sc"] == 0);
}

TEST_CASE("Falcon3 V4 SetInputUniverses sends one input block", "[vendors3][falcon3]") {
    FalconV4 fv;
    auto f = ConnectV4(fv);
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 6, OUTPUT_ARTNET, 510, 3);
    REQUIRE_UPLOAD(s, f->SetInputUniverses(s.ctl, &s.ui));
    auto in = fv.Sets("IN");
    REQUIRE(in.size() == 1);
    auto a = in[0]["P"]["A"][0];
    CHECK(a["u"] == 3);
    CHECK(a["c"] == 510);
    CHECK(a["uc"] == 6);
    CHECK(a["p"] == "a");
}

TEST_CASE("Falcon3 V4 SetInputUniverses refuses more than 192 universes", "[vendors3][falcon3]") {
    FalconV4 fv;
    auto f = ConnectV4(fv);
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 193);
    CHECK_FALSE(f->SetInputUniverses(s.ctl, &s.ui));
    CHECK(s.ui.Said("only 192 are supported"));
}

TEST_CASE("Falcon3 V4 in player mode only fixes the start channel", "[vendors3][falcon3]") {
    FalconV4 fv;
    fv.status["O"] = 5;
    fv.status["ps"] = 99;
    auto f = ConnectV4(fv);
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 1);
    REQUIRE_UPLOAD(s, f->SetInputUniverses(s.ctl, &s.ui));
    auto bm = fv.Sets("BM");
    REQUIRE(bm.size() == 1);
    CHECK(bm[0]["P"]["O"] == 5);
    CHECK(bm[0]["P"]["ps"] == 0);
    CHECK(fv.Sets("IN").empty());
}

TEST_CASE("Falcon3 V4 handler without status refuses uploads", "[vendors3][falcon3]") {
    FalconV4 fv;
    auto f = ConnectV4(fv);
    f->_v4status = nlohmann::json();
    VendorShow s("Falcon", "F16V4", "16 Local Ports", 1);
    CHECK_FALSE(f->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Failed to retrieve current configuration"));
    s.ui.messages.clear();
    CHECK_FALSE(f->SetInputUniverses(s.ctl, &s.ui));
    CHECK(s.ui.Said("Failed to retrieve current configuration"));
}

#endif
