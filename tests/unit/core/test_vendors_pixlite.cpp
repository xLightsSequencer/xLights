#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "controllers/Pixlite16.h"

namespace {

struct PixliteAccess : Pixlite16 {
    using Pixlite16::DecodeColourOrder;
    using Pixlite16::DecodeSerialOutputProtocol;
    using Pixlite16::DecodeStringPortProtocol;
    using Pixlite16::EncodeColourOrder;
    using Pixlite16::Mk3FrequencyForProtocol;
    using Pixlite16::ParseV4Config;
    using Pixlite16::Read16;
    using Pixlite16::Write16;
    using Pixlite16::WriteString;
};

struct Cursor {
    std::vector<uint8_t>& b;
    int pos;
    void u8(int v) { b[pos++] = (uint8_t)v; }
    void u16(int v) {
        b[pos++] = (uint8_t)(v >> 8);
        b[pos++] = (uint8_t)(v & 0xFF);
    }
    void str(const std::string& s, int len) {
        std::memcpy(&b[pos], s.data(), std::min<size_t>(s.size(), (size_t)len));
        pos += len;
    }
    void skip(int n) { pos += n; }
};

std::vector<uint8_t> MakeV4(const std::string& model, bool advancedColour, int bankVoltage = 120) {
    std::vector<uint8_t> b(600, 0);
    Cursor c{ b, 12 };
    c.str(model, 20);
    c.u8(10); c.u8(0); c.u8(0); c.u8(7);
    c.u8(1);
    c.skip(1);
    c.str("Front Yard", 40);
    c.str("v1.2.3", 20);
    for (int i = 0; i < 6; ++i) c.u8(0xA0 + i);
    c.u16(412);
    for (int i = 0; i < 16; ++i) c.u16(100 + i);
    c.u8(4);
    c.skip(2);
    c.u8(192); c.u8(168); c.u8(1); c.u8(50);
    c.u8(255); c.u8(255); c.u8(255); c.u8(0);
    c.skip(2);
    c.skip(4);
    c.u8(3);
    c.u8(22); c.u8(23); c.u8(24);
    c.u16(bankVoltage); c.u16(bankVoltage + 1);
    for (int i = 0; i < 16; ++i) c.u16(1 + i);
    for (int i = 0; i < 16; ++i) c.u16(1000 + i);
    for (int i = 0; i < 16; ++i) c.u8(i);
    for (int i = 0; i < 16; ++i) c.u16(i * 2);
    for (int i = 0; i < 16; ++i) c.u8(i & 1);
    c.u8(advancedColour ? 0xFF : 2);
    c.skip(2);
    for (int i = 0; i < 4; ++i) c.u8(1);
    for (int i = 0; i < 4; ++i) c.u16(50 + i);
    c.u8(9);
    c.u8(6);
    for (int i = 0; i < 16; ++i) c.u8(i % 6);
    for (int i = 0; i < 16; ++i) c.u16(1 + i);
    for (int i = 0; i < 16; ++i) c.u8(200);
    c.u8(1);
    c.u8(60);
    c.u8(255); c.u8(255); c.u8(255); c.u8(0);
    c.u8(1);
    c.u8(2); c.u8(3); c.u8(4);
    c.u8(0);
    c.u8(8);
    return b;
}

} // namespace

TEST_CASE("Pixlite16 maps pixel protocol names to codes case insensitively", "[misc][vendors][pixlite]") {
    CHECK(PixliteAccess::DecodeStringPortProtocol("tls3001") == 0);
    CHECK(PixliteAccess::DecodeStringPortProtocol("WS2811") == 4);
    CHECK(PixliteAccess::DecodeStringPortProtocol("Ws2811") == 4);
    CHECK(PixliteAccess::DecodeStringPortProtocol("apa102") == 10);
    CHECK(PixliteAccess::DecodeStringPortProtocol("sk6812") == 12);
    CHECK(PixliteAccess::DecodeStringPortProtocol("apa109") == 31);
    CHECK(PixliteAccess::DecodeStringPortProtocol("ucs1903") == 15);
}

TEST_CASE("Pixlite16 unknown pixel protocols decode to minus one", "[misc][vendors][pixlite]") {
    CHECK(PixliteAccess::DecodeStringPortProtocol("") == -1);
    CHECK(PixliteAccess::DecodeStringPortProtocol("nonsense") == -1);
    CHECK(PixliteAccess::DecodeStringPortProtocol("ws2811 ") == -1);
    CHECK(PixliteAccess::DecodeStringPortProtocol(std::string(10000, 'x')) == -1);
}

TEST_CASE("Pixlite16 protocol codes are unique", "[misc][vendors][pixlite]") {
    const char* names[] = { "tls3001", "sm16716", "ws2801", "lpd6803", "ws2811", "mb16020", "tm1803", "tm1804", "tm1809",
                            "my9231", "apa102", "my9221", "sk6812", "ucs2903", "p9813", "ucs1903", "apa109" };
    std::vector<int> seen;
    for (auto n : names) {
        int v = PixliteAccess::DecodeStringPortProtocol(n);
        CHECK(v >= 0);
        CHECK(std::find(seen.begin(), seen.end(), v) == seen.end());
        seen.push_back(v);
    }
}

TEST_CASE("Pixlite16 serial protocol only knows dmx", "[misc][vendors][pixlite]") {
    CHECK(PixliteAccess::DecodeSerialOutputProtocol("dmx") == 0);
    CHECK(PixliteAccess::DecodeSerialOutputProtocol("DMX") == 0);
    CHECK(PixliteAccess::DecodeSerialOutputProtocol("renard") == -1);
    CHECK(PixliteAccess::DecodeSerialOutputProtocol("") == -1);
}

TEST_CASE("Pixlite16 colour order encode and decode round trip", "[misc][vendors][pixlite]") {
    const char* orders[] = { "RGB", "RBG", "GRB", "GBR", "BRG", "BGR" };
    for (int i = 0; i < 6; ++i) {
        CHECK(PixliteAccess::EncodeColourOrder(orders[i]) == i);
        CHECK(PixliteAccess::DecodeColourOrder(i) == orders[i]);
    }
}

TEST_CASE("Pixlite16 colour order falls back to RGB for bad input", "[misc][vendors][pixlite]") {
    CHECK(PixliteAccess::EncodeColourOrder("rgb") == 0);
    CHECK(PixliteAccess::EncodeColourOrder("XYZ") == 0);
    CHECK(PixliteAccess::EncodeColourOrder("") == 0);
    CHECK(PixliteAccess::DecodeColourOrder(-1) == "RGB");
    CHECK(PixliteAccess::DecodeColourOrder(6) == "RGB");
    CHECK(PixliteAccess::DecodeColourOrder(1000000) == "RGB");
}

TEST_CASE("Pixlite16 Mk3 frequencies group compatible chips", "[misc][vendors][pixlite]") {
    for (auto p : { "apa102", "lpd6803", "mbi6020", "my9221", "my9231", "my9291", "p9813", "sk9822", "sm16716", "ws2801" }) {
        CHECK(PixliteAccess::Mk3FrequencyForProtocol(p) == 1600);
    }
    CHECK(PixliteAccess::Mk3FrequencyForProtocol("dmx512") == 750);
    CHECK(PixliteAccess::Mk3FrequencyForProtocol("sk6822") == 900);
    CHECK(PixliteAccess::Mk3FrequencyForProtocol("tm1814") == 900);
    CHECK(PixliteAccess::Mk3FrequencyForProtocol("tm1803") == 980);
    CHECK(PixliteAccess::Mk3FrequencyForProtocol("tm1804") == 666);
    CHECK(PixliteAccess::Mk3FrequencyForProtocol("tm1809") == 1110);
    CHECK(PixliteAccess::Mk3FrequencyForProtocol("ws2811") == 800);
    CHECK(PixliteAccess::Mk3FrequencyForProtocol("ucs1903") == 800);
    CHECK(PixliteAccess::Mk3FrequencyForProtocol("tm1914") == 800);
    CHECK(PixliteAccess::Mk3FrequencyForProtocol("tls3001") == 1000);
}

TEST_CASE("Pixlite16 Mk3 frequency is zero for unknown or differently cased names", "[misc][vendors][pixlite]") {
    CHECK(PixliteAccess::Mk3FrequencyForProtocol("") == 0);
    CHECK(PixliteAccess::Mk3FrequencyForProtocol("bogus") == 0);
    CHECK(PixliteAccess::Mk3FrequencyForProtocol("WS2811") == 0);
}

TEST_CASE("Pixlite16 Read16 is big endian and advances", "[misc][vendors][pixlite]") {
    uint8_t d[] = { 0x12, 0x34, 0xFF, 0xFF, 0x00, 0x01 };
    int pos = 0;
    CHECK(PixliteAccess::Read16(d, pos) == 0x1234);
    CHECK(pos == 2);
    CHECK(PixliteAccess::Read16(d, pos) == 0xFFFF);
    CHECK(PixliteAccess::Read16(d, pos) == 1);
    CHECK(pos == 6);
}

TEST_CASE("Pixlite16 Write16 then Read16 round trips and truncates to 16 bits", "[misc][vendors][pixlite]") {
    uint8_t d[8] = {};
    int pos = 0;
    PixliteAccess::Write16(d, pos, 0xABCD);
    PixliteAccess::Write16(d, pos, 0);
    PixliteAccess::Write16(d, pos, 0x1FFFF);
    CHECK(pos == 6);
    CHECK(d[0] == 0xAB);
    CHECK(d[1] == 0xCD);
    int r = 0;
    CHECK(PixliteAccess::Read16(d, r) == 0xABCD);
    CHECK(PixliteAccess::Read16(d, r) == 0);
    CHECK(PixliteAccess::Read16(d, r) == 0xFFFF);
}

TEST_CASE("Pixlite16 WriteString pads with zeros and truncates", "[misc][vendors][pixlite]") {
    uint8_t d[16];
    std::memset(d, 0xEE, sizeof(d));
    int pos = 2;
    PixliteAccess::WriteString(d, pos, 6, "abc");
    CHECK(pos == 8);
    CHECK(d[1] == 0xEE);
    CHECK(d[2] == 'a');
    CHECK(d[4] == 'c');
    CHECK(d[5] == 0);
    CHECK(d[7] == 0);
    CHECK(d[8] == 0xEE);

    pos = 0;
    PixliteAccess::WriteString(d, pos, 3, "abcdef");
    CHECK(pos == 3);
    CHECK(d[2] == 'c');

    pos = 0;
    PixliteAccess::WriteString(d, pos, 4, "");
    CHECK(pos == 4);
    CHECK(d[0] == 0);
    CHECK(d[3] == 0);
}

TEST_CASE("Pixlite16 Config has the documented defaults", "[misc][vendors][pixlite]") {
    Pixlite16::Config c;
    CHECK(c._numOutputs == 0);
    CHECK(c._currentSubnetMask[0] == 255);
    CHECK(c._currentSubnetMask[3] == 0);
    CHECK(c._staticSubnetMask[2] == 255);
    CHECK(c._gamma.size() == 3);
    CHECK(c._modelName.empty());
    CHECK_FALSE(c._pixelsCanBeSplit);
    CHECK_FALSE(c._forceExpanded);
}

TEST_CASE("Pixlite16 parses a V4 configuration packet", "[misc][vendors][pixlite]") {
    auto buf = MakeV4("PixLite 16 Mk2", true);
    Pixlite16::Config c;
    REQUIRE(PixliteAccess::ParseV4Config(buf.data(), c));
    CHECK(c._modelName == "PixLite 16 Mk2");
    CHECK(c._nickname == "Front Yard");
    CHECK(c._firmwareVersion == "v1.2.3");
    CHECK(c._currentIP[0] == 10);
    CHECK(c._currentIP[3] == 7);
    CHECK(c._dhcp == 1);
    CHECK(c._mac[0] == 0xA0);
    CHECK(c._mac[5] == 0xA5);
    CHECK(c._temperature == 412);
    CHECK(c._numOutputs == 16);
    CHECK(c._realOutputs == 16);
    CHECK(c._maxPixelsPerOutput == 340);
    REQUIRE(c._outputPixels.size() == 16);
    CHECK(c._outputPixels[0] == 100);
    CHECK(c._outputPixels[15] == 115);
    CHECK(c._protocol == 4);
    CHECK(c._staticIP[0] == 192);
    CHECK(c._staticIP[3] == 50);
    CHECK(c._staticSubnetMask[3] == 0);
    CHECK(c._currentDriver == 3);
    REQUIRE(c._gamma.size() == 3);
    CHECK(c._gamma[0] == 22);
    CHECK(c._gamma[2] == 24);
    REQUIRE(c._bankVoltage.size() == 2);
    CHECK(c._bankVoltage[0] == 120);
    CHECK(c._bankVoltage[1] == 121);
}

TEST_CASE("Pixlite16 V4 per output tables line up", "[misc][vendors][pixlite]") {
    auto buf = MakeV4("PixLite 16", true);
    Pixlite16::Config c;
    REQUIRE(PixliteAccess::ParseV4Config(buf.data(), c));
    REQUIRE(c._outputUniverse.size() == 16);
    REQUIRE(c._outputStartChannel.size() == 16);
    REQUIRE(c._outputNullPixels.size() == 16);
    REQUIRE(c._outputZigZag.size() == 16);
    REQUIRE(c._outputReverse.size() == 16);
    REQUIRE(c._outputColourOrder.size() == 16);
    REQUIRE(c._outputGrouping.size() == 16);
    REQUIRE(c._outputBrightness.size() == 16);
    for (int i = 0; i < 16; ++i) {
        CHECK(c._outputUniverse[i] == 1 + i);
        CHECK(c._outputStartChannel[i] == 1000 + i);
        CHECK(c._outputNullPixels[i] == i);
        CHECK(c._outputZigZag[i] == i * 2);
        CHECK(c._outputReverse[i] == (i & 1));
        CHECK(c._outputColourOrder[i] == i % 6);
        CHECK(c._outputGrouping[i] == 1 + i);
        CHECK(c._outputBrightness[i] == 200);
    }
    CHECK(c._numDMX == 4);
    CHECK(c._realDMX == 4);
    REQUIRE(c._dmxUniverse.size() == 4);
    CHECK(c._dmxUniverse[3] == 53);
    CHECK(c._hwRevision == 9);
    CHECK(c._currentDriverSpeed == 6);
    CHECK(c._holdLastFrame == 1);
    CHECK(c._maxTargetTemp == 60);
    CHECK(c._currentSubnetMask[0] == 255);
    CHECK(c._currentDriverExpanded == 1);
    CHECK(c._minAssistantVer[0] == 2);
    CHECK(c._minAssistantVer[2] == 4);
    CHECK(c._testMode == 0);
    CHECK(c._currentDriverType == 8);
}

TEST_CASE("Pixlite16 V4 without advanced colour copies one order to every output", "[misc][vendors][pixlite]") {
    auto buf = MakeV4("PixLite 16", false);
    Pixlite16::Config c;
    REQUIRE(PixliteAccess::ParseV4Config(buf.data(), c));
    REQUIRE(c._outputColourOrder.size() == 16);
    for (auto v : c._outputColourOrder) {
        CHECK(v == 2);
    }
    CHECK(c._outputGrouping[0] == 1);
    CHECK(c._outputBrightness[0] == 200);
}

TEST_CASE("Pixlite16 V4 small models report four outputs and one DMX", "[misc][vendors][pixlite]") {
    auto buf = MakeV4("PixLite 4 Mk2", true);
    Pixlite16::Config c;
    REQUIRE(PixliteAccess::ParseV4Config(buf.data(), c));
    CHECK(c._realOutputs == 4);
    CHECK(c._realDMX == 1);
    CHECK(c._numOutputs == 16);
}

TEST_CASE("Pixlite16 V4 strings stop at the first terminator", "[misc][vendors][pixlite]") {
    auto buf = MakeV4("Short", true);
    buf[12 + 5] = 0;
    buf[12 + 6] = 'X';
    Pixlite16::Config c;
    REQUIRE(PixliteAccess::ParseV4Config(buf.data(), c));
    CHECK(c._modelName == "Short");
}

TEST_CASE("Pixlite16 V4 parse of an all zero packet gives empty strings", "[misc][vendors][pixlite]") {
    std::vector<uint8_t> buf(600, 0);
    Pixlite16::Config c;
    REQUIRE(PixliteAccess::ParseV4Config(buf.data(), c));
    CHECK(c._modelName.empty());
    CHECK(c._nickname.empty());
    CHECK(c._firmwareVersion.empty());
    CHECK(c._realOutputs == 4);
    CHECK(c._outputPixels.size() == 16);
    CHECK(c._outputPixels[0] == 0);
}

// _bankVoltage is a vector<uint8_t> but the packet carries 16 bit values, so anything above 255 is truncated.
TEST_CASE("Pixlite16 V4 keeps bank voltages above 255", "[misc][vendors][pixlite]") {
    auto buf = MakeV4("PixLite 16", true, 500);
    Pixlite16::Config c;
    REQUIRE(PixliteAccess::ParseV4Config(buf.data(), c));
    REQUIRE(c._bankVoltage.size() == 2);
    CHECK((int)c._bankVoltage[0] == 500);
}
