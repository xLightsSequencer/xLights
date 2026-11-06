// The handler is reached through ExposeVendors.h, a loopback UDP peer and a loopback HTTP
// server, all POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <memory>
#include <thread>

#include "support/ExposeVendors.h"
#include "controllers/Pixlite16.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"
#include "support/VendorShow.h"

namespace {

constexpr uint16_t kPixlitePort = 49150;

// Byte writer for the Advatek binary config packets.
struct Pkt {
    std::vector<uint8_t> b = std::vector<uint8_t>(1500, 0);
    int pos = 0;
    void u8(int v) { b[pos++] = (uint8_t)v; }
    void u16(int v) {
        b[pos++] = (uint8_t)((v >> 8) & 0xFF);
        b[pos++] = (uint8_t)(v & 0xFF);
    }
    void str(const std::string& s, int len) {
        std::memcpy(&b[pos], s.data(), std::min<size_t>(s.size(), (size_t)len));
        pos += len;
    }
    void ip(int a, int b2, int c, int d) { u8(a); u8(b2); u8(c); u8(d); }
    void header(int version) {
        str("Advatech", 8);
        u8(0);
        u8(0);
        u8(0x02); // reply
        u8(version);
    }
};

// What a PixLite reports about itself. Per output values are derived from the index so the
// parse can be checked field by field.
struct Reply {
    std::string model = "PixLite 16 MkII";
    std::string nickname = "Garage";
    std::string firmware = "2.1.0";
    int outputs = 16;
    int dmx = 4;
    int pixels = 170;
};

std::vector<uint8_t> MakeV4(const Reply& r) {
    Pkt p;
    p.header(4);
    p.str(r.model, 20);
    p.ip(127, 0, 0, 1);
    p.u8(0); // dhcp
    p.u8(0);
    p.str(r.nickname, 40);
    p.str(r.firmware, 20);
    for (int i = 0; i < 6; ++i) p.u8(0x10 + i);
    p.u16(355);
    for (int i = 0; i < 16; ++i) p.u16(r.pixels);
    p.u8(0); // sACN
    p.pos += 2;
    p.ip(192, 0, 2, 10);
    p.ip(255, 255, 255, 0);
    p.pos += 6;
    p.u8(4); // ws2811
    p.u8(10); p.u8(10); p.u8(10);
    p.u16(120); p.u16(121);
    for (int i = 0; i < 16; ++i) p.u16(100 + i);
    for (int i = 0; i < 16; ++i) p.u16(1);
    for (int i = 0; i < 16; ++i) p.u8(3);
    for (int i = 0; i < 16; ++i) p.u16(0);
    for (int i = 0; i < 16; ++i) p.u8(1);
    p.u8(0xFF);
    p.pos += 2;
    for (int i = 0; i < 4; ++i) p.u8(1);
    for (int i = 0; i < 4; ++i) p.u16(200 + i);
    p.u8(20);
    p.u8(2);
    for (int i = 0; i < 16; ++i) p.u8(5);
    for (int i = 0; i < 16; ++i) p.u16(3);
    for (int i = 0; i < 16; ++i) p.u8(50);
    p.u8(1);
    p.u8(70);
    p.ip(255, 255, 255, 0);
    p.u8(0);
    p.u8(1); p.u8(2); p.u8(3);
    p.u8(0);
    p.u8(0);
    p.b.resize(p.pos + 4, 0xFF);
    return p.b;
}

// V5, V6 and V8 share a layout apart from the header offset, the firmware field, the gamma
// count, the DMX protocols byte (V8) and the trailing test fields.
std::vector<uint8_t> MakeV5Plus(int version, const Reply& r) {
    Pkt p;
    p.header(version);
    if (version >= 6) p.u8(0); // V6+ data starts one byte later
    for (int i = 0; i < 6; ++i) p.u8(0x20 + i);
    p.u8((int)r.model.size());
    p.str(r.model, (int)r.model.size());
    p.u8(31); // hw revision
    p.u8(4); p.u8(5); p.u8(6);
    if (version == 5) {
        p.str(r.firmware, 20);
    } else {
        // The parser copies the string from one byte past the length byte, so a lead byte is
        // sent and the length covers it plus a terminator.
        p.u8((int)r.firmware.size() + 2);
        p.u8(0x7E);
        p.str(r.firmware, (int)r.firmware.size());
        p.u8(0);
    }
    p.u8(3); // brand
    p.ip(127, 0, 0, 1);
    p.ip(255, 0, 0, 0);
    p.u8(1); // dhcp
    p.ip(198, 51, 100, 7);
    p.ip(255, 255, 0, 0);
    p.u8(1); // artnet
    p.u8(1); // hold last frame
    p.u8(1); // simple
    p.u16(1020);
    p.u8(r.outputs);
    for (int i = 0; i < r.outputs; ++i) p.u16(r.pixels + i);
    for (int i = 0; i < r.outputs; ++i) p.u16(10 + i);
    for (int i = 0; i < r.outputs; ++i) p.u16(1 + 3 * i);
    for (int i = 0; i < r.outputs; ++i) p.u8(i % 3);
    for (int i = 0; i < r.outputs; ++i) p.u16(i * 4);
    for (int i = 0; i < r.outputs; ++i) p.u8(i % 2);
    for (int i = 0; i < r.outputs; ++i) p.u8(i % 6);
    for (int i = 0; i < r.outputs; ++i) p.u16(1 + i % 2);
    for (int i = 0; i < r.outputs; ++i) p.u8(90 - i);
    p.u8(r.dmx);
    if (version >= 8) p.u8(2); // protocols on DMX
    for (int i = 0; i < r.dmx; ++i) p.u8(i % 2);
    for (int i = 0; i < r.dmx; ++i) p.u16(300 + i);
    p.u8(2); // drivers
    p.u8(8); // name length
    p.u8(4); p.u8(10);
    p.u8(1); p.u8(2);
    p.u8(1); p.u8(0);
    p.str("WS2811", 8);
    p.str("APA102", 8);
    p.u8(1); // current driver
    p.u8(4);
    p.u8(2);
    p.u8(0);
    int gammas = version == 5 ? 3 : 4;
    for (int i = 0; i < gammas; ++i) p.u8(20 + i);
    p.str(r.nickname, 40);
    p.u16(412);
    p.u8(65);
    p.u8(3);
    p.u16(118); p.u16(119); p.u16(240);
    p.u8(0); // test mode
    if (version >= 6) {
        for (int i = 0; i < 4; ++i) p.u8(9 + i);
    }
    if (version >= 8) {
        p.u8(7);
        p.u16(513);
    }
    p.b.resize(p.pos);
    return p.b;
}

bool CanBindPixlitePort() {
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return false;
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_ANY);
    a.sin_port = htons(kPixlitePort);
    bool ok = ::bind(fd, (sockaddr*)&a, sizeof(a)) == 0;
    ::close(fd);
    return ok;
}

// Plays the controller: keeps sending its config reply from 127.0.0.1 to the PixLite port
// so the reply is queued whenever the handler's discovery socket is listening.
class FakePixliteUdp {
public:
    explicit FakePixliteUdp(std::vector<uint8_t> reply) : _reply(std::move(reply)) {
        _fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        sockaddr_in a{};
        a.sin_family = AF_INET;
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        ::bind(_fd, (sockaddr*)&a, sizeof(a));
        _thread = std::thread([this] {
            sockaddr_in to{};
            to.sin_family = AF_INET;
            to.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            to.sin_port = htons(kPixlitePort);
            while (!_stop) {
                ::sendto(_fd, _reply.data(), _reply.size(), 0, (sockaddr*)&to, sizeof(to));
                std::this_thread::sleep_for(std::chrono::microseconds(200));
            }
        });
    }
    ~FakePixliteUdp() {
        _stop = true;
        _thread.join();
        ::close(_fd);
    }

private:
    std::vector<uint8_t> _reply;
    int _fd = -1;
    std::atomic<bool> _stop{ false };
    std::thread _thread;
};

// Constructs the handler against the fake controller. Returns null (and the caller skips)
// when the PixLite port is taken: the handler would then fall back to broadcasting on the
// real network, which a unit test must not do.
std::unique_ptr<Pixlite16> Connect(const std::vector<uint8_t>& reply) {
    VendorsTestInit();
    if (!CanBindPixlitePort()) return nullptr;
    FakePixliteUdp peer(reply);
    return std::make_unique<Pixlite16>("127.0.0.1");
}

#define CONNECT_OR_SKIP(var, reply)                                                   \
    auto var = Connect(reply);                                                        \
    if (!var) {                                                                       \
        SKIP("UDP port 49150 is in use; the PixLite handler would broadcast instead"); \
    }                                                                                 \
    REQUIRE(var->IsConnected())

int Rd16(const uint8_t* d, int pos) {
    return (d[pos] << 8) | d[pos + 1];
}

} // namespace

TEST_CASE("Pixlite3 connects to a V4 controller over the discovery protocol", "[vendors3][pixlite3]") {
    Reply r;
    CONNECT_OR_SKIP(p, MakeV4(r));
    CHECK(p->_protocolVersion == 4);
    CHECK(p->GetModel() == "PixLite 16 MkII");
    CHECK(p->GetVersion() == "2.1.0");
    CHECK(p->_config._nickname == "Garage");
    CHECK(p->_config._numOutputs == 16);
    CHECK(p->_config._realOutputs == 16);
    CHECK(p->_config._dmxUniverse[3] == 203);
    CHECK(p->_config._outputColourOrder[0] == 5);
    CHECK_FALSE(p->UsesHTTP());
}

TEST_CASE("Pixlite3 parses a V5 reply", "[vendors3][pixlite3]") {
    Reply r;
    r.outputs = 4;
    r.dmx = 1;
    r.model = "PixLite 4 MkII";
    CONNECT_OR_SKIP(p, MakeV5Plus(5, r));
    auto& c = p->_config;
    CHECK(p->_protocolVersion == 5);
    CHECK(c._modelName == "PixLite 4 MkII");
    CHECK(c._firmwareVersion == "2.1.0");
    CHECK(c._mac[0] == 0x20);
    CHECK(c._mac[5] == 0x25);
    CHECK(c._hwRevision == 31);
    CHECK(c._minAssistantVer[2] == 6);
    CHECK(c._brand == 3);
    CHECK(c._currentSubnetMask[0] == 255);
    CHECK(c._currentSubnetMask[1] == 0);
    CHECK(c._dhcp == 1);
    CHECK(c._staticIP[0] == 198);
    CHECK(c._staticIP[3] == 7);
    CHECK(c._staticSubnetMask[2] == 0);
    CHECK(c._protocol == 1);
    CHECK(c._holdLastFrame == 1);
    CHECK(c._simpleConfig == 1);
    CHECK(c._maxPixelsPerOutput == 1020);
    REQUIRE(c._numOutputs == 4);
    CHECK(c._realOutputs == 4);
    for (int i = 0; i < 4; ++i) {
        INFO("output " << i);
        CHECK(c._outputPixels[i] == 170 + i);
        CHECK(c._outputUniverse[i] == 10 + i);
        CHECK(c._outputStartChannel[i] == 1 + 3 * i);
        CHECK(c._outputNullPixels[i] == i % 3);
        CHECK(c._outputZigZag[i] == i * 4);
        CHECK(c._outputReverse[i] == i % 2);
        CHECK(c._outputColourOrder[i] == i % 6);
        CHECK(c._outputGrouping[i] == 1 + i % 2);
        CHECK(c._outputBrightness[i] == 90 - i);
    }
    REQUIRE(c._numDMX == 1);
    CHECK(c._dmxUniverse[0] == 300);
    REQUIRE(c._numDrivers == 2);
    CHECK(c._driverType[1] == 10);
    CHECK(c._driverSpeed[0] == 1);
    CHECK(c._driverExpandable[0] == 1);
    CHECK(c._driverName[0] == "WS2811");
    CHECK(c._driverName[1] == "APA102");
    CHECK(c._currentDriver == 1);
    CHECK(c._currentDriverType == 4);
    CHECK(c._currentDriverSpeed == 2);
    REQUIRE(c._gamma.size() == 3);
    CHECK(c._gamma[2] == 22);
    CHECK(c._nickname == "Garage");
    CHECK(c._temperature == 412);
    CHECK(c._maxTargetTemp == 65);
    REQUIRE(c._numBanks == 3);
    CHECK(c._bankVoltage[2] == 240);
    CHECK(c._testParameters.empty());
}

TEST_CASE("Pixlite3 parses V6 and V8 replies", "[vendors3][pixlite3]") {
    for (int version : { 6, 8 }) {
        INFO("version " << version);
        Reply r;
        r.outputs = 16;
        r.dmx = 4;
        auto p = Connect(MakeV5Plus(version, r));
        if (!p) SKIP("UDP port 49150 is in use");
        REQUIRE(p->IsConnected());
        auto& c = p->_config;
        CHECK(p->_protocolVersion == version);
        CHECK(c._modelName == "PixLite 16 MkII");
        CHECK(c._firmwareVersion == "2.1.0");
        CHECK(c._brand == 3);
        CHECK(c._currentIP[0] == 127);
        CHECK(c._mac[1] == 0x21);
        REQUIRE(c._numOutputs == 16);
        CHECK(c._outputPixels[15] == 185);
        CHECK(c._outputBrightness[15] == 75);
        REQUIRE(c._numDMX == 4);
        CHECK(c._dmxOn[1] == 1);
        CHECK(c._dmxUniverse[3] == 303);
        CHECK(c._protocolsOnDMX == (version >= 8 ? 2 : 0));
        REQUIRE(c._gamma.size() == 4);
        CHECK(c._gamma[3] == 23);
        CHECK(c._nickname == "Garage");
        CHECK(c._bankVoltage[0] == 118);
        REQUIRE(c._testParameters.size() == 4);
        CHECK(c._testParameters[3] == 12);
        CHECK(c._testOutputNum == (version >= 8 ? 7 : 0));
        CHECK(c._testPixelNum == (version >= 8 ? 513 : 0));
    }
}

TEST_CASE("Pixlite3 V4 SetOutputs writes ports, models settings and DMX", "[vendors3][pixlite3]") {
    Reply r;
    CONNECT_OR_SKIP(p, MakeV4(r));
    VendorShow s("Advatek", "PixLite 16 MkII", "Normal", 4);
    s.Pixels("Arch", 1, 50, 1, "nullNodes=\"2\" Brightness=\"60\" colorOrder=\"GRB\" groupCount=\"2\" zigZag=\"5\" reverse=\"1\"");
    s.Pixels("Tree", 3, 100, 511);
    s.Serial("Flood", 1, 6, 1531);
    REQUIRE_UPLOAD(s, p->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    INFO(s.ui.All());
    auto& c = p->_config;
    CHECK(c._nickname == "Ctl");
    CHECK(c._protocolName == "ws2811");
    CHECK(c._currentDriver == 4);
    CHECK(c._outputPixels[0] == 50);
    CHECK(c._outputUniverse[0] == 1);
    CHECK(c._outputStartChannel[0] == 1);
    CHECK(c._outputNullPixels[0] == 2);
    CHECK(c._outputBrightness[0] == 60);
    CHECK(c._outputColourOrder[0] == 2);
    CHECK(c._outputGrouping[0] == 2);
    CHECK(c._outputZigZag[0] == 5);
    CHECK(c._outputReverse[0] == 1);
    CHECK(c._outputPixels[2] == 100);
    CHECK(c._outputUniverse[2] == 2);
    CHECK(c._outputStartChannel[2] == 1);
    CHECK(c._outputReverse[2] == 0);
    // ports with no models keep what the controller reported
    CHECK(c._outputPixels[1] == 170);
    CHECK(c._outputUniverse[1] == 101);
    CHECK(c._dmxOn[0] == 1);
    CHECK(c._dmxUniverse[0] == 4);
    CHECK(c._dmxUniverse[1] == 201);
    CHECK_FALSE(c._forceExpanded);
}

TEST_CASE("Pixlite3 V4 config packet carries the uploaded values", "[vendors3][pixlite3]") {
    Reply r;
    r.pixels = 0;
    CONNECT_OR_SKIP(p, MakeV4(r));
    VendorShow s("Advatek", "PixLite 16 MkII", "Normal", 2);
    s.Pixels("Arch", 2, 40, 1, "Brightness=\"70\"");
    REQUIRE_UPLOAD(s, p->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));

    uint8_t d[1500] = {};
    int len = p->PrepareV4Config(d);
    CHECK(std::string((char*)d, 8) == "Advatech");
    CHECK(Rd16(d, 9) == 5);
    CHECK(d[11] == 0x02);
    CHECK(std::string((char*)d + 12) == "PixLite 16 MkII");
    int pos = 12 + 20;
    CHECK(d[pos] == 0); // dhcp
    CHECK(std::string((char*)d + pos + 1) == "Ctl");
    pos += 1 + 40;
    CHECK(Rd16(d, pos) == 0);
    CHECK(Rd16(d, pos + 2) == 40);
    pos += 32;
    CHECK(d[pos] == 0); // sACN
    // brightness table: after the per output and DMX tables
    int bright = pos + 1 + 2 + 4 + 4 + 2 + 4 + 1 + 3 + 32 + 32 + 16 + 32 + 16 + 3 + 4 + 8 + 1 + 16 + 32;
    CHECK(d[bright + 1] == 70);
    CHECK(len == bright + 16 + 4);
    CHECK(d[len - 2] == 0); // not expanded: nothing on the upper half
}

TEST_CASE("Pixlite3 PixCon and long range names are rewritten for the V4 packet", "[vendors3][pixlite3]") {
    Reply r;
    CONNECT_OR_SKIP(p, MakeV4(r));
    uint8_t d[1500] = {};
    p->_config._modelName = "PixLite 16 MkII LR";
    p->PrepareV4Config(d);
    CHECK(std::string((char*)d + 12) == "PixLite 16 MkII");
    std::memset(d, 0, sizeof(d));
    p->_config._modelName = "PixCon 16";
    p->PrepareV4Config(d);
    CHECK(std::string((char*)d + 12) == "PixLite 16");
}

TEST_CASE("Pixlite3 V5, V6 and V8 packets carry the uploaded values", "[vendors3][pixlite3]") {
    Reply r;
    r.outputs = 16;
    r.pixels = 0;
    CONNECT_OR_SKIP(p, MakeV5Plus(8, r));
    VendorShow s("Advatek", "PixLite 16 MkII", "Normal", 4);
    s.Pixels("Arch", 1, 30, 1, "Brightness=\"40\"");
    s.Pixels("High", 12, 20, 511);
    s.Serial("Flood", 2, 6, 1531);
    REQUIRE_UPLOAD(s, p->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));

    struct V {
        int version;
        int (Pixlite16::*prep)(uint8_t*) const;
    };
    for (auto v : { V{ 5, &Pixlite16::PrepareV5Config }, V{ 6, &Pixlite16::PrepareV6Config }, V{ 8, &Pixlite16::PrepareV8Config } }) {
        INFO("version " << v.version);
        uint8_t d[1500] = {};
        int len = ((*p).*(v.prep))(d);
        CHECK(d[11] == v.version);
        CHECK(d[12] == 0x20); // mac
        CHECK(d[18] == 1);    // dhcp
        CHECK(d[19] == 198);
        int pos = 12 + 6 + 1 + 4 + 4;
        CHECK(d[pos] == 1);     // artnet kept from the controller
        CHECK(d[pos + 2] == 0); // always advanced config
        pos += 3;
        CHECK(Rd16(d, pos) == 30);
        CHECK(Rd16(d, pos + 22) == 20);
        int uni = pos + 32;
        CHECK(Rd16(d, uni) == 1);
        CHECK(Rd16(d, uni + 22) == 2);
        int bright = uni + 32 + 32 + 16 + 32 + 16 + 16 + 32;
        CHECK(d[bright] == 40);
        int dmx = bright + 16;
        CHECK(d[dmx + 1] == 1); // serial port 2 turned on
        CHECK(Rd16(d, dmx + 4 + 2) == 4);
        int drv = dmx + 4 + 8;
        CHECK(d[drv] == 4); // ws2811
        CHECK(d[drv + 3] == 1); // port 12 is in the upper half so expanded
        CHECK(std::string((char*)d + drv + 4 + 4) == "Ctl");
        CHECK(len == drv + 4 + 4 + 40 + 1);
    }
}

TEST_CASE("Pixlite3 expanded variant forces the expanded flag", "[vendors3][pixlite3]") {
    Reply r;
    r.pixels = 0;
    CONNECT_OR_SKIP(p, MakeV4(r));
    VendorShow s("Advatek", "PixLite 16 MkII", "Expanded", 2);
    s.Pixels("Arch", 1, 30, 1);
    REQUIRE_UPLOAD(s, p->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(p->_config._forceExpanded);
    uint8_t d[1500] = {};
    int len = p->PrepareV4Config(d);
    // only port 1 has pixels, so the flag comes from the variant
    CHECK(d[len - 2] == 1);
}

TEST_CASE("Pixlite3 full xLights control resets ports without models", "[vendors3][pixlite3]") {
    Reply r;
    CONNECT_OR_SKIP(p, MakeV4(r));
    VendorShow s("Advatek", "PixLite 16 MkII", "Normal", 2);
    s.ctl->SetFullxLightsControl(true);
    s.Pixels("Arch", 2, 25, 1);
    REQUIRE_UPLOAD(s, p->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    auto& c = p->_config;
    CHECK(c._outputPixels[0] == 0);
    CHECK(c._outputPixels[1] == 25);
    CHECK(c._outputPixels[5] == 0);
    CHECK(c._outputNullPixels[0] == 0);
    CHECK(c._outputZigZag[0] == 1);
    CHECK(c._outputGrouping[0] == 1);
    CHECK(c._outputColourOrder[0] == 0);
    CHECK(c._outputReverse[0] == 0);
    CHECK(c._outputBrightness[0] == 100);
    for (auto on : c._dmxOn) CHECK(on == 0);
}

TEST_CASE("Pixlite3 refuses ports beyond what the controller reported", "[vendors3][pixlite3]") {
    Reply r;
    r.outputs = 4;
    r.dmx = 1;
    CONNECT_OR_SKIP(p, MakeV5Plus(5, r));
    VendorShow s("Advatek", "PixLite 16 MkII", "Normal", 2);
    s.Pixels("Far", 8, 10, 1);
    CHECK_FALSE(p->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Controller reported 4 pixel outputs but ports up to 8 are in use"));
    CHECK(s.ui.Said("Not uploaded due to errors"));
    CHECK(p->_config._outputPixels[3] == 173);
}

TEST_CASE("Pixlite3 refuses serial ports beyond what the controller reported", "[vendors3][pixlite3]") {
    Reply r;
    r.outputs = 16;
    r.dmx = 1;
    CONNECT_OR_SKIP(p, MakeV5Plus(5, r));
    VendorShow s("Advatek", "PixLite 16 MkII", "Normal", 2);
    s.Serial("Flood", 3, 6, 1);
    CHECK_FALSE(p->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Controller reported 1 serial outputs but ports up to 3 are in use"));
}

TEST_CASE("Pixlite3 refuses a port with too many pixels", "[vendors3][pixlite3]") {
    Reply r;
    CONNECT_OR_SKIP(p, MakeV4(r));
    VendorShow s("Advatek", "PixLite 16 MkII", "Normal", 8);
    s.Pixels("Long", 1, 1100, 1);
    CHECK_FALSE(p->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Not uploaded due to errors"));
    CHECK(s.ui.captions.back() == "Error");
}

TEST_CASE("Pixlite3 SetOutputs without caps for the controller fails cleanly", "[vendors3][pixlite3]") {
    Reply r;
    CONNECT_OR_SKIP(p, MakeV4(r));
    VendorShow s("Advatek", "No Such Board", "Normal", 1);
    s.Pixels("Arch", 1, 10, 1);
    CHECK_FALSE(p->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    CHECK(s.ui.Said("Not uploaded due to errors"));
}

namespace {
const char* kMk3Ver = R"({"result":{"apiVer":[{"maj":"v1","min":[0,3]}],"prodName":"PixLite E16-S Mk3","fwVer":"1.4.2","nickname":"Porch","oem":2}})";

std::string Mk3Config(bool gammaOn) {
    nlohmann::json j;
    auto& c = j["result"]["config"];
    c["pix"]["dataSrc"] = "Art-Net";
    c["pix"]["holdLastFrm"] = true;
    c["pix"]["expand"] = false;
    c["pix"]["pixType"] = "WS2811";
    c["pix"]["freq"] = 800;
    c["pix"]["pixsSpanUni"] = true;
    c["pix"]["gammaOn"] = gammaOn;
    c["pix"]["gamma"] = { 2.2, 2.3, 2.4 };
    for (int i = 0; i < 16; ++i) {
        c["pixPort"]["pixCount"].push_back(100 + i);
        c["pixPort"]["startUni"].push_back(1 + i);
        c["pixPort"]["startCh"].push_back(1);
        c["pixPort"]["nullPix"].push_back(i % 2);
        c["pixPort"]["zigZag"].push_back(0);
        c["pixPort"]["reverse"].push_back(i == 3);
        c["pixPort"]["colorOrder"].push_back(i == 1 ? "BGR" : "RGB");
        c["pixPort"]["group"].push_back(1);
        c["pixPort"]["intensity"].push_back(80);
    }
    c["auxPort"]["uni"] = { 40 };
    c["auxPort"]["mode"] = { "DMX512Out" };
    return j.dump();
}

struct Mk3Server {
    std::string lastConfigChange;
    std::string saveReply = R"({"result":{"saved":true}})";
    std::string configReply;
    std::unique_ptr<FakeHttpServer> srv;
    explicit Mk3Server(bool gammaOn = true) : configReply(Mk3Config(gammaOn)) {
        srv = std::make_unique<FakeHttpServer>([this](const FakeRequest& r) {
            FakeResponse resp;
            if (r.path == "/ver") {
                resp.body = kMk3Ver;
            } else if (r.path == "/v1.3") {
                if (r.body.find("configRead") != std::string::npos) resp.body = configReply;
                else if (r.body.find("constantRead") != std::string::npos) resp.body = R"({"result":{}})";
                else {
                    lastConfigChange = r.body;
                    resp.body = saveReply;
                }
            } else {
                resp.status = 404;
            }
            return resp;
        });
    }
};
} // namespace

TEST_CASE("Pixlite3 reads a Mk3 configuration over HTTP", "[vendors3][pixlite3]") {
    Reply r;
    CONNECT_OR_SKIP(p, MakeV4(r));
    Mk3Server m;
    REQUIRE(m.srv->ok());
    p->_ip = m.srv->Address();
    p->_config = Pixlite16::Config();
    REQUIRE(p->GetMK3Config());
    auto& c = p->_config;
    CHECK(p->_mk3APIVersion == "v1.3");
    CHECK(c._modelName == "PixLite E16-S Mk3");
    CHECK(c._firmwareVersion == "1.4.2");
    CHECK(c._nickname == "Porch");
    CHECK(c._brand == 2);
    CHECK(c._protocol == 1);
    CHECK(c._holdLastFrame == 1);
    CHECK(c._numOutputs == 16);
    CHECK(c._realOutputs == 8);
    CHECK(c._outputPixels[15] == 115);
    CHECK(c._outputUniverse[2] == 3);
    CHECK(c._outputNullPixels[1] == 1);
    CHECK(c._outputReverse[3] == 1);
    CHECK(c._outputColourOrder[1] == 5);
    CHECK(c._outputBrightness[0] == 80);
    REQUIRE(c._numDMX == 1);
    CHECK(c._realDMX == 1);
    CHECK(c._dmxUniverse[0] == 40);
    CHECK(c._protocolName == "WS2811");
    CHECK(c._currentDriverSpeed == (uint8_t)800);
    CHECK(c._pixelsCanBeSplit);
    CHECK(c._gamma[0] == 22);
    CHECK(c._gamma[2] == 24);
    CHECK(m.srv->Count("/ver") == 1);
    CHECK(m.srv->Count("/v1.3") == 2);
}

TEST_CASE("Pixlite3 Mk3 with gamma off reports zero gamma", "[vendors3][pixlite3]") {
    Reply r;
    CONNECT_OR_SKIP(p, MakeV4(r));
    Mk3Server m(false);
    p->_ip = m.srv->Address();
    REQUIRE(p->GetMK3Config());
    for (auto g : p->_config._gamma) CHECK(g == 0);
}

TEST_CASE("Pixlite3 Mk3 bad replies are not a configuration", "[vendors3][pixlite3]") {
    Reply r;
    CONNECT_OR_SKIP(p, MakeV4(r));
    SECTION("version endpoint is not JSON") {
        FakeHttpServer srv([](const FakeRequest&) { return FakeResponse{ 200, "<html>nope</html>" }; });
        p->_ip = srv.Address();
        CHECK_FALSE(p->GetMK3Config());
        CHECK(p->_mk3APIVersion.empty());
    }
    SECTION("config read is missing fields") {
        FakeHttpServer srv([](const FakeRequest& rq) { return FakeResponse{ 200, rq.path == "/ver" ? kMk3Ver : R"({"result":{"config":{}}})" }; });
        p->_ip = srv.Address();
        CHECK_FALSE(p->GetMK3Config());
        CHECK(p->_mk3APIVersion == "v1.3");
    }
}

TEST_CASE("Pixlite3 Mk3 SetOutputs posts a JSON config change", "[vendors3][pixlite3]") {
    Reply r;
    CONNECT_OR_SKIP(p, MakeV4(r));
    Mk3Server m;
    p->_ip = m.srv->Address();
    REQUIRE(p->GetMK3Config());
    p->_mk3Ver = kMk3Ver;
    VendorShow s("Advatek", "PixLite E16-S Mk3", "Normal", 4);
    s.ctl->SetFullxLightsControl(true);
    s.Pixels("Arch", 1, 50, 1, "Brightness=\"55\" colorOrder=\"GBR\" reverse=\"1\" nullNodes=\"3\" groupCount=\"2\"", "tm1814");
    s.Pixels("Tree", 2, 100, 511, "", "tm1814");
    s.Serial("Flood", 1, 6, 1531);
    REQUIRE_UPLOAD(s, p->SetOutputs(&s.mm, &s.om, s.ctl, &s.ui));
    INFO(m.lastConfigChange);
    auto j = nlohmann::json::parse(m.lastConfigChange);
    CHECK(j["req"] == "configChange");
    CHECK(j["params"]["action"] == "save");
    auto& c = j["params"]["config"];
    CHECK(c["dev"]["nickname"] == "Ctl");
    CHECK(c["pix"]["dataSrc"] == "Art-Net");
    CHECK(c["pix"]["pixType"] == "TM1814");
    CHECK(c["pix"]["freq"] == 900);
    CHECK(c["pix"]["expand"] == false);
    auto& pp = c["pixPort"];
    REQUIRE(pp["pixCount"].size() == 8);
    // tm1814 is a four channel pixel, so the RGB models' channels make fewer pixels
    CHECK(pp["pixCount"][0] == 38);
    CHECK(pp["pixCount"][1] == 75);
    CHECK(pp["pixCount"][2] == 0);
    CHECK(pp["startUni"][1] == 2);
    CHECK(pp["startCh"][0] == 1);
    CHECK(pp["nullPix"][0] == 3);
    CHECK(pp["zigZag"][0] == 1); // 0 is sent as 1
    CHECK(pp["group"][0] == 2);
    CHECK(pp["reverse"][0] == true);
    CHECK(pp["reverse"][1] == false);
    CHECK(pp["colorOrder"][0] == "GBR");
    CHECK(pp["intensity"][0] == 55);
    CHECK(c["auxPort"]["mode"][0] == "DMX512Out");
    CHECK(c["auxPort"]["dataSrc"][0] == "Art-Net");
    CHECK(c["auxPort"]["uni"][0] == 4);
}

TEST_CASE("Pixlite3 Mk3 config change reports the controller's answer", "[vendors3][pixlite3]") {
    Reply r;
    CONNECT_OR_SKIP(p, MakeV4(r));
    Mk3Server m;
    p->_ip = m.srv->Address();
    REQUIRE(p->GetMK3Config());
    p->_mk3Ver = kMk3Ver;
    for (auto& d : p->_config._dmxOn) d = 0;

    m.saveReply = R"({"result":{"saved":true}})";
    CHECK(p->SendConfig());
    auto j = nlohmann::json::parse(m.lastConfigChange);
    // every DMX port off: no data source or universe list is sent
    CHECK_FALSE(j["params"]["config"]["auxPort"].contains("dataSrc"));
    CHECK_FALSE(j["params"]["config"]["auxPort"].contains("uni"));
    CHECK(j["params"]["config"]["auxPort"]["mode"][0] == "Off");
    // upper half has pixels so all 16 ports go out expanded
    CHECK(j["params"]["config"]["pixPort"]["pixCount"].size() == 16);
    CHECK(j["params"]["config"]["pix"]["expand"] == true);

    m.saveReply = R"({"result":{"saved":false}})";
    CHECK_FALSE(p->SendConfig());
    m.saveReply = R"({"err":{"msg":"bad value"}})";
    CHECK_FALSE(p->SendConfig());
    m.saveReply = "not json";
    CHECK_FALSE(p->SendConfig());
}

TEST_CASE("Pixlite3 Mk3 unknown pixel type sends no frequency", "[vendors3][pixlite3]") {
    Reply r;
    CONNECT_OR_SKIP(p, MakeV4(r));
    Mk3Server m;
    p->_ip = m.srv->Address();
    REQUIRE(p->GetMK3Config());
    p->_mk3Ver = kMk3Ver;
    p->_config._protocolName = "gs8208";
    p->_config._protocol = 0;
    REQUIRE(p->SendConfig());
    auto j = nlohmann::json::parse(m.lastConfigChange);
    CHECK(j["params"]["config"]["pix"]["pixType"] == "GS8208");
    CHECK_FALSE(j["params"]["config"]["pix"].contains("freq"));
    CHECK(j["params"]["config"]["pix"]["dataSrc"] == "sACN");
}

#endif
