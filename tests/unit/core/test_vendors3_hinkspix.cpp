// The handler is reached through ExposeVendors.h and a loopback server, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>

#include <mutex>

#include "support/ExposeVendors.h"
#include "controllers/HinksPix.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"
#include "support/VendorShow.h"

namespace {

// HinksPix sends its payload in "DATA:" / "ROW:" request headers.
std::string Header(const std::string& headers, const std::string& name) {
    size_t p = headers.find(name + ": ");
    if (p == std::string::npos) return "";
    size_t e = headers.find("\r\n", p);
    return headers.substr(p + name.size() + 2, e == std::string::npos ? std::string::npos : e - p - name.size() - 2);
}

struct Posted {
    std::string path;
    std::string data;
    std::string row;
};

struct FakeHinks {
    std::string info = R"({"Type":"P","Controller":"H","BD1":"S","BD2":"L","MaxU":"12","MCPU":"VER151","PCPU":"VER002","ECPU":"VER003","WEB":"VER004"})";
    std::string mode = R"({"CMD":"DATA_MODE","MODE":"E131","DMX_ACTIVE":0,"DMX_UNIV":9,"DMX_START":1,"DMX_CHAN_CNT":16,"DDP_DMX_ACTIVE":0,"DDP_DMX_START":1,"DDP_DMX_CHAN_CNT":16})";
    std::string postReply = R"({"RESULT":"OK"})";
    std::string rowReply = "done";
    std::string easyInfo = "C,65";
    std::mutex m;
    std::vector<Posted> posts;
    std::unique_ptr<FakeHttpServer> srv;

    FakeHinks() {
        srv = std::make_unique<FakeHttpServer>([this](const FakeRequest& r) {
            FakeResponse resp;
            std::string data = Header(r.headers, "DATA");
            std::string row = Header(r.headers, "ROW");
            if (r.path == "/XLights_BoardInfo.cgi") {
                resp.body = info;
            } else if (r.path == "/Xlights_Board_Port_Config.cgi") {
                int blk = (int)std::strtol(Header(r.headers, "BLK").c_str(), nullptr, 10);
                std::string list = "{\"LIST\":[";
                for (int i = 1; i <= 16; ++i) {
                    int port = blk * 16 + i;
                    if (i > 1) list += ",";
                    // every port reports 10 pixels, GRB, brightness 70, gamma 2
                    list += "{\"V\":\"" + std::to_string(port) + ",1," + std::to_string(port * 30 - 29) + ",10," + std::to_string(port * 30) + ",0,2,0,70,2\"}";
                }
                resp.body = list + "]}";
            } else if (r.path == "/Xlights_Data_Mode.cgi") {
                resp.body = mode;
            } else if (r.path == "/Xlights_PostData.cgi" || r.path == "/Xlights_UnPack_Config.cgi") {
                std::lock_guard<std::mutex> l(m);
                posts.push_back({ r.path, data, row });
                resp.body = postReply;
            } else if (r.path == "/GetInfo.cgi") {
                std::lock_guard<std::mutex> l(m);
                posts.push_back({ r.path, data, row });
                resp.body = row == "902" ? easyInfo : rowReply;
            } else {
                resp.status = 404;
            }
            return resp;
        });
    }

    // JSON payloads posted with the given CMD (reboot commands are excluded)
    std::vector<nlohmann::json> Cmds(const std::string& cmd) {
        std::lock_guard<std::mutex> l(m);
        std::vector<nlohmann::json> r;
        for (auto& p : posts) {
            if (p.data.rfind("{", 0) != 0) continue;
            auto j = nlohmann::json::parse(p.data, nullptr, false);
            if (j.is_object() && j.value("CMD", "") == cmd) r.push_back(j);
        }
        return r;
    }
    std::vector<Posted> Path(const std::string& path) {
        std::lock_guard<std::mutex> l(m);
        std::vector<Posted> r;
        for (auto& p : posts) if (p.path == path) r.push_back(p);
        return r;
    }
    std::vector<Posted> Rows(const std::string& row) {
        std::lock_guard<std::mutex> l(m);
        std::vector<Posted> r;
        for (auto& p : posts) if (p.path == "/GetInfo.cgi" && p.row == row) r.push_back(p);
        return r;
    }
};

std::unique_ptr<HinksPix> Connect(FakeHinks& fh) {
    VendorsTestInit();
    return std::make_unique<HinksPix>(fh.srv->Address(), "");
}

std::string V(const nlohmann::json& list, size_t i) {
    return list.at(i).at("V").get<std::string>();
}

} // namespace

TEST_CASE("HinksPix3 SetOutputs uploads universes, ports, DMX and unpack map", "[vendors3][hinkspix3]") {
    FakeHinks fh;
    auto h = Connect(fh);
    REQUIRE(h->IsConnected());
    VendorShow s("HinksPix", "PRO V1/V2", "", 4);
    s.Pixels("Arch", 1, 50, 1, "nullNodes=\"2\" Brightness=\"55\" colorOrder=\"BGR\" reverse=\"1\"");
    s.Pixels("Tree", 3, 100, 511);
    s.Serial("Flood", 1, 8, 1531);
    REQUIRE_UPLOAD(s, h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    INFO(s.ui.All());

    auto modeCmds = fh.Cmds("DATA_MODE");
    REQUIRE(modeCmds.size() == 2);
    CHECK(modeCmds[0]["MODE"] == "E131");
    // the serial port moves from the controller's universe 9 to the model's universe
    CHECK(modeCmds[1]["DMX_ACTIVE"] == 1);
    CHECK(modeCmds[1]["DMX_UNIV"] == 4);
    CHECK(modeCmds[1]["DMX_START"] == 1);
    CHECK(modeCmds[1]["DMX_CHAN_CNT"] == 16);

    // twelve universes are supported: two blocks of six
    auto e131 = fh.Cmds("E131");
    REQUIRE(e131.size() == 2);
    CHECK(e131[0]["BLK"] == "0");
    CHECK(V(e131[0]["LIST"], 0) == "1,1,510,1,1,510");
    CHECK(V(e131[0]["LIST"], 1) == "2,2,510,1,511,1020");
    CHECK(V(e131[0]["LIST"], 3) == "4,4,510,1,1531,2040");
    CHECK(V(e131[0]["LIST"], 4) == "5,5,0,1,0,0");
    CHECK(V(e131[1]["LIST"], 5) == "12,12,0,1,0,0");
    auto bd = fh.Cmds("BD_INFO");
    REQUIRE(bd.size() == 1);
    CHECK(bd[0]["NumU"] == "4");

    auto pc = fh.Cmds("PCONFIG");
    REQUIRE(pc.size() == 2); // the SPI and long range boards
    CHECK(pc[0]["BOARD"] == "0");
    CHECK(pc[1]["BOARD"] == "1");
    REQUIRE(pc[0]["LIST"].size() == 16);
    // output,protocol,start,pixels,end,direction,colour,nulls,brightness,gamma
    CHECK(V(pc[0]["LIST"], 0) == "1,1,1,50,150,1,5,2,50,2");
    CHECK(V(pc[0]["LIST"], 1) == "2,1,31,10,60,0,2,0,70,2");
    CHECK(V(pc[0]["LIST"], 2) == "3,1,151,100,450,0,2,0,70,2");
    CHECK(V(pc[1]["LIST"], 0) == "17,1,481,10,510,0,2,0,70,2");

    // the first model is already where the controller expects it, only the second moves
    auto unpack = fh.Path("/Xlights_UnPack_Config.cgi");
    REQUIRE(unpack.size() == 1);
    auto uj = nlohmann::json::parse(unpack[0].data.substr(1, unpack[0].data.size() - 2));
    CHECK(uj["NUM"] == "1");
    CHECK(uj["LEFT"] == "0");
    REQUIRE(uj["LIST"].size() == 1);
    CHECK(uj["LIST"][0] == "{510,150,300}");
    CHECK(fh.Cmds("SCONFIG").empty());
}

TEST_CASE("HinksPix3 SetOutputs under full control starts from blank ports", "[vendors3][hinkspix3]") {
    FakeHinks fh;
    auto h = Connect(fh);
    VendorShow s("HinksPix", "PRO V1/V2", "", 2);
    s.ctl->SetFullxLightsControl(true);
    s.ctl->SetDefaultBrightnessUnderFullControl(80);
    s.Pixels("Arch", 2, 20, 1);
    REQUIRE_UPLOAD(s, h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(fh.srv->Count("/Xlights_Board_Port_Config.cgi") == 0);
    auto pc = fh.Cmds("PCONFIG");
    REQUIRE(pc.size() == 2);
    CHECK(V(pc[0]["LIST"], 0) == "1,0,1,0,0,0,0,0,80,1");
    CHECK(V(pc[0]["LIST"], 1) == "2,1,1,20,60,0,0,0,80,1");
    // the serial port was never configured so nothing is sent for it
    CHECK(fh.Cmds("DATA_MODE").size() == 1);
}

TEST_CASE("HinksPix3 smart receivers are uploaded for long range boards", "[vendors3][hinkspix3]") {
    FakeHinks fh;
    auto h = Connect(fh);
    VendorShow s("HinksPix", "PRO V1/V2", "", 2);
    s.Pixels("A", 18, 10, 1, "SmartRemote=\"1\" SmartRemoteType=\"hinkspix_4\"");
    s.Pixels("B", 18, 20, 31, "SmartRemote=\"2\" SmartRemoteType=\"hinkspix_4\"");
    REQUIRE_UPLOAD(s, h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    INFO(s.ui.All());
    auto sc = fh.Cmds("SCONFIG");
    REQUIRE(sc.size() == 1);
    CHECK(sc[0]["BOARD"] == "1");
    CHECK(sc[0]["Port4"] == "0");
    REQUIRE(sc[0]["LIST"].size() == 2);
    // id,type,start pixel for each of the four ports on the receiver
    CHECK(V(sc[0]["LIST"], 0) == "0,0,0,1,0,0");
    CHECK(V(sc[0]["LIST"], 1) == "1,0,0,11,0,0");
}

TEST_CASE("HinksPix3 sixteen port smart receivers fill out the group", "[vendors3][hinkspix3]") {
    FakeHinks fh;
    auto h = Connect(fh);
    VendorShow s("HinksPix", "PRO V1/V2", "", 2);
    s.Pixels("A", 17, 10, 1, "SmartRemote=\"2\" SmartRemoteType=\"hinkspix_16\"");
    REQUIRE_UPLOAD(s, h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto sc = fh.Cmds("SCONFIG");
    REQUIRE(sc.size() == 1);
    REQUIRE(sc[0]["LIST"].size() == 4);
    CHECK(V(sc[0]["LIST"], 0) == "0,1,0,0,0,0");
    CHECK(V(sc[0]["LIST"], 1) == "1,0,1,0,0,0");
    CHECK(V(sc[0]["LIST"], 3) == "3,0,0,0,0,0");
}

TEST_CASE("HinksPix3 refuses smart receivers on a board that is not long range", "[vendors3][hinkspix3]") {
    FakeHinks fh;
    auto h = Connect(fh);
    VendorShow s("HinksPix", "PRO V1/V2", "", 2);
    s.Pixels("A", 1, 10, 1, "SmartRemote=\"1\" SmartRemoteType=\"hinkspix_4\"");
    CHECK_FALSE(h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Expansion Port 1 has Smart Receivers but it is not a Long Range"));
    CHECK(fh.Cmds("PCONFIG").empty());
}

TEST_CASE("HinksPix3 refuses ports on a missing expansion board", "[vendors3][hinkspix3]") {
    FakeHinks fh;
    auto h = Connect(fh);
    VendorShow s("HinksPix", "PRO V1/V2", "", 2);
    s.Pixels("A", 40, 10, 1);
    CHECK_FALSE(h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Pixel Output 40 is being used on Expansion Port 3 and no Output Board is Connected"));
    CHECK(fh.Cmds("E131").empty());
}

TEST_CASE("HinksPix3 SetOutputs rejects failing checks and model mismatches", "[vendors3][hinkspix3]") {
    FakeHinks fh;
    SECTION("too many pixels") {
        auto h = Connect(fh);
        VendorShow s("HinksPix", "PRO V1/V2", "", 5);
        s.Pixels("A", 1, 700, 1);
        CHECK_FALSE(h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("HinksPix Upload Error"));
    }
    SECTION("controller is a PRO 80 but configured as V1/V2") {
        fh.info = R"({"Type":"8","Controller":"H","MCPU":"VER151"})";
        auto h = Connect(fh);
        VendorShow s("HinksPix", "PRO V1/V2", "", 1);
        CHECK_FALSE(h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    }
    SECTION("controller is V1/V2 but configured as PRO V3") {
        auto h = Connect(fh);
        VendorShow s("HinksPix", "PRO V3", "", 1);
        CHECK_FALSE(h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    }
    SECTION("firmware too old") {
        fh.info = R"({"Type":"P","Controller":"H","MCPU":"VER100"})";
        auto h = Connect(fh);
        VendorShow s("HinksPix", "PRO V1/V2", "", 1);
        CHECK_FALSE(h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    }
    SECTION("DDP with universe per string") {
        auto h = Connect(fh);
        VendorShow s("HinksPix", "PRO V1/V2", "", 1, OUTPUT_DDP);
        s.ctl->SetUniversePerString(true);
        CHECK_FALSE(h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(s.ui.Said("Universe Per String not allows with DDP"));
    }
    CHECK(fh.Cmds("PCONFIG").empty());
}

TEST_CASE("HinksPix3 too many universes stops the input upload", "[vendors3][hinkspix3]") {
    FakeHinks fh;
    auto h = Connect(fh);
    VendorShow s("HinksPix", "PRO V1/V2", "", 13);
    s.Pixels("A", 1, 10, 1);
    CHECK_FALSE(h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(fh.Cmds("E131").empty());
}

TEST_CASE("HinksPix3 a controller that does not answer OK fails the upload", "[vendors3][hinkspix3]") {
    FakeHinks fh;
    fh.postReply = R"({"RESULT":"FAIL"})";
    auto h = Connect(fh);
    VendorShow s("HinksPix", "PRO V1/V2", "", 1);
    s.Pixels("A", 1, 10, 1);
    CHECK_FALSE(h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
}

TEST_CASE("HinksPix3 DDP mode sends the DDP range and skips universes", "[vendors3][hinkspix3]") {
    FakeHinks fh;
    fh.info = R"({"Type":"8","Controller":"H","BD1":"S","MaxU":"12","MCPU":"VER151"})";
    auto h = Connect(fh);
    VendorShow s("HinksPix", "PRO V3", "", 1, OUTPUT_DDP);
    s.Pixels("A", 1, 10, 1);
    s.Serial("Flood", 1, 20, 31);
    REQUIRE_UPLOAD(s, h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto modeCmds = fh.Cmds("DATA_MODE");
    REQUIRE(modeCmds.size() == 2);
    CHECK(modeCmds[0]["MODE"] == "DDP");
    CHECK(modeCmds[0]["DDP_START"] == 1);
    CHECK(modeCmds[0].contains("DDP_CHAN_COUNT"));
    CHECK(modeCmds[1]["DDP_DMX_ACTIVE"] == 1);
    CHECK(modeCmds[1]["DDP_DMX_START"] == 31);
    CHECK(modeCmds[1]["DDP_DMX_CHAN_CNT"] == 20);
    CHECK(fh.Cmds("E131").empty());
}

TEST_CASE("HinksPix3 universe per string assigns universes in port order", "[vendors3][hinkspix3]") {
    FakeHinks fh;
    auto h = Connect(fh);
    VendorShow s("HinksPix", "PRO V1/V2", "", 3);
    s.ctl->SetUniversePerString(true);
    s.Pixels("A", 2, 10, 1);
    s.Pixels("B", 1, 10, 511);
    REQUIRE_UPLOAD(s, h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto e131 = fh.Cmds("E131");
    REQUIRE_FALSE(e131.empty());
    // port 1 (universe 2) is first on the controller, port 2 (universe 1) follows
    CHECK(V(e131[0]["LIST"], 0) == "1,2,510,1,1,510");
    CHECK(V(e131[0]["LIST"], 1) == "2,1,510,1,511,1020");
    CHECK(fh.Cmds("BD_INFO").at(0)["NumU"] == "2");
}

TEST_CASE("HinksPix3 EasyLights upload uses the row protocol", "[vendors3][hinkspix3]") {
    FakeHinks fh;
    fh.info = R"({"Type":"P","Controller":"E","BD1":"S","MCPU":"VER122"})";
    auto h = Connect(fh);
    REQUIRE(h->GetModel() == "EasyLights Pix16");
    VendorShow s("HinksPix", "EasyLights Pix16", "", 2);
    s.ctl->SetFullxLightsControl(true);
    s.Pixels("A", 1, 10, 1);
    s.Serial("Flood", 1, 8, 511);
    REQUIRE_UPLOAD(s, h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    INFO(s.ui.All());

    auto rows = fh.Rows("2001");
    REQUIRE(rows.size() == 10); // 65 universes, six per row
    CHECK(rows[0].data == "ROWCNT=16:ROW=0:1,1,510,1,1,510,2,2,510,1,511,1020,3,3,0,1,0,0,4,4,0,1,0,0,5,5,0,1,0,0,6,6,0,1,0,0");
    auto mode = fh.Rows("4902");
    REQUIRE(mode.size() == 1);
    CHECK(mode[0].data == "A,0,B,0,C,65,D,2,E,0");
    auto pix = fh.Rows("3041");
    REQUIRE(pix.size() == 1);
    CHECK(pix[0].data.rfind("1,1,1,10,30,0,0,0,100,1|2,0,1,0,0,0,0,0,100,1|", 0) == 0);
    CHECK(pix[0].data.size() > 2);
    CHECK(pix[0].data.substr(pix[0].data.size() - 2) == "||");
    auto serial = fh.Rows("4908");
    REQUIRE(serial.size() == 1);
    CHECK(serial[0].data == "A,0,B,1,C,0,D,2,E,1,F,16,G,1,H,512");
    CHECK(fh.Rows("1111").size() == 1);
}

TEST_CASE("HinksPix3 EasyLights failures", "[vendors3][hinkspix3]") {
    FakeHinks fh;
    fh.info = R"({"Type":"P","Controller":"E","BD1":"S","MCPU":"VER122"})";
    SECTION("no universe count") {
        fh.easyInfo = "X,1";
        auto h = Connect(fh);
        VendorShow s("HinksPix", "EasyLights Pix16", "", 1);
        s.ctl->SetFullxLightsControl(true);
        s.Pixels("A", 1, 10, 1);
        CHECK_FALSE(h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(fh.Rows("4902").empty());
    }
    SECTION("too many universes") {
        fh.easyInfo = "C,1";
        auto h = Connect(fh);
        VendorShow s("HinksPix", "EasyLights Pix16", "", 2);
        s.ctl->SetFullxLightsControl(true);
        s.Pixels("A", 1, 10, 1);
        CHECK_FALSE(h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    }
    SECTION("rows not acknowledged") {
        fh.rowReply = "busy";
        auto h = Connect(fh);
        VendorShow s("HinksPix", "EasyLights Pix16", "", 1);
        s.ctl->SetFullxLightsControl(true);
        s.Pixels("A", 1, 10, 1);
        CHECK_FALSE(h->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
        CHECK(fh.Rows("3041").size() == 1);
    }
}

#endif
