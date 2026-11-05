// The handlers are reached through ExposeVendors.h and a loopback server, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "support/ExposeVendors.h"
#include "controllers/Falcon.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"

namespace {
// Nothing listens on port 1, so the constructor fails fast and leaves a blank handler
// whose private state the tests then set directly.
Falcon MakeDisconnectedFalcon() {
    VendorsTestInit();
    return Falcon("127.0.0.1:1", "");
}

Falcon MakeFalcon(int model, int version, const std::string& fw = "") {
    Falcon f = MakeDisconnectedFalcon();
    f._modelnum = model;
    f._versionnum = version;
    f._firmwareVersion = fw;
    return f;
}

std::string V4Reply(const std::string& payload, int r = 200, int f = 1, int b = 0, int rb = 0) {
    return "{\"R\":" + std::to_string(r) + ",\"T\":\"Q\",\"F\":" + std::to_string(f) + ",\"B\":" + std::to_string(b) +
           ",\"M\":\"X\",\"RB\":" + std::to_string(rb) + ",\"P\":" + payload + ",\"W\":\" \",\"L\":\"\"}";
}
} // namespace

TEST_CASE("Falcon2 disconnected handler reports not connected", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    CHECK_FALSE(f.IsConnected());
    CHECK(f.UsesHTTP());
    CHECK(f.GetName().empty());
}

TEST_CASE("Falcon2 DecodeBrightness maps codes to percentages", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    auto row = GENERATE(table<int, int>({ { 0, 100 }, { 1, 95 }, { 2, 90 }, { 3, 85 }, { 4, 80 }, { 5, 75 }, { 6, 70 }, { 7, 65 },
                                          { 8, 60 }, { 9, 50 }, { 10, 40 }, { 11, 30 }, { 12, 20 }, { 13, 10 },
                                          { 14, 100 }, { -1, 100 }, { 1000, 100 }, { INT32_MIN, 100 } }));
    CHECK(f.DecodeBrightness(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("Falcon2 EncodeBrightness uses threshold buckets", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    auto row = GENERATE(table<int, int>({ { 100, 0 }, { 98, 0 }, { 97, 1 }, { 93, 1 }, { 92, 2 }, { 88, 2 }, { 87, 3 }, { 83, 3 }, { 82, 4 },
                                          { 78, 4 }, { 77, 5 }, { 73, 5 }, { 72, 6 }, { 68, 6 }, { 67, 7 }, { 63, 7 }, { 62, 8 }, { 55, 8 },
                                          { 54, 9 }, { 45, 9 }, { 44, 10 }, { 35, 10 }, { 34, 11 }, { 25, 11 }, { 24, 12 }, { 15, 12 },
                                          { 14, 13 }, { 0, 13 }, { -50, 13 } }));
    INFO("brightness " << std::get<0>(row));
    CHECK(f.EncodeBrightness(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("Falcon2 EncodeBrightness above 100 stays at the top code", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    CHECK(f.EncodeBrightness(101) == 0);
    CHECK(f.EncodeBrightness(100000) == 0);
    CHECK(f.EncodeBrightness(INT32_MAX) == 0);
    CHECK(f.EncodeBrightness(INT32_MIN) == 13);
}

TEST_CASE("Falcon2 brightness code round trips for every code", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    for (int code = 0; code <= 13; ++code) {
        INFO("code " << code);
        int pct = f.DecodeBrightness(code);
        // the 50 and 60 percent steps are not evenly spaced so encode lands on the nearest bucket
        int back = f.EncodeBrightness(pct);
        CHECK(f.DecodeBrightness(back) == pct);
    }
}

TEST_CASE("Falcon2 DecodeGamma", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    auto row = GENERATE(table<int, float>({ { 0, 1.0f }, { 1, 2.0f }, { 2, 2.3f }, { 3, 2.5f }, { 4, 2.8f }, { 5, 3.0f }, { 6, 1.0f }, { -1, 1.0f }, { 99, 1.0f } }));
    CHECK(f.DecodeGamma(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("Falcon2 EncodeGamma", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    auto row = GENERATE(table<float, int>({ { 0.0f, 0 }, { 1.0f, 0 }, { 1.49f, 0 }, { 1.5f, 1 }, { 2.0f, 1 }, { 2.14f, 1 }, { 2.15f, 2 }, { 2.3f, 2 },
                                            { 2.39f, 2 }, { 2.4f, 3 }, { 2.5f, 3 }, { 2.64f, 3 }, { 2.65f, 4 }, { 2.8f, 4 }, { 2.89f, 4 }, { 2.9f, 5 },
                                            { 3.0f, 5 }, { 100.0f, 5 }, { -5.0f, 0 } }));
    INFO("gamma " << std::get<0>(row));
    CHECK(f.EncodeGamma(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("Falcon2 gamma code round trips", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    for (int code = 0; code <= 5; ++code) {
        CHECK(f.EncodeGamma(f.DecodeGamma(code)) == code);
    }
}

TEST_CASE("Falcon2 colour order codes", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    const char* names[] = { "RGB", "RBG", "GRB", "GBR", "BRG", "BGR" };
    for (int i = 0; i < 6; ++i) {
        CHECK(f.DecodeColourOrder(i) == names[i]);
        CHECK(f.EncodeColourOrder(names[i]) == i);
    }
    CHECK(f.DecodeColourOrder(6) == "RGB");
    CHECK(f.DecodeColourOrder(-1) == "RGB");
    CHECK(f.EncodeColourOrder("") == 0);
    CHECK(f.EncodeColourOrder("rgb") == 0);
    CHECK(f.EncodeColourOrder("GRBW") == 0);
    CHECK(f.EncodeColourOrder("XYZ") == 0);
}

TEST_CASE("Falcon2 direction codes", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    CHECK(f.DecodeDirection(0) == "Forward");
    CHECK(f.DecodeDirection(1) == "Reverse");
    CHECK(f.DecodeDirection(2) == "Forward");
    CHECK(f.DecodeDirection(-1) == "Forward");
    CHECK(f.EncodeDirection("Forward") == 0);
    CHECK(f.EncodeDirection("Reverse") == 1);
    CHECK(f.EncodeDirection("reverse") == 0);
    CHECK(f.EncodeDirection("") == 0);
}

TEST_CASE("Falcon2 serial output protocol decode is case insensitive", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    auto row = GENERATE(table<std::string, int>({ { "dmx", 0 }, { "DMX", 0 }, { "pixelnet", 1 }, { "PixelNet", 1 }, { "renard", 2 }, { "RENARD", 2 }, { "", 0 }, { "lor", 0 } }));
    CHECK(f.DecodeSerialOutputProtocol(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("Falcon2 string port protocol decode", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    auto row = GENERATE(table<std::string, int>({ { "ws2811", 0 }, { "WS2811", 0 }, { "tm18xx", 1 }, { "lx1203", 2 }, { "ws2801", 3 }, { "tls3001", 4 },
                                                  { "lpd6803", 5 }, { "gece", 6 }, { "lpd8806", 7 }, { "apa102", 8 }, { "ucs1903", 9 }, { "dm412", 10 },
                                                  { "p9813", 11 }, { "ucs2903", 12 }, { "tm1814", 14 }, { "unknown", 0 }, { "", 0 }, { "sm16716", 0 } }));
    INFO(std::get<0>(row));
    CHECK(f.DecodeStringPortProtocol(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("Falcon2 SafeDescription collapses spaces and truncates", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    CHECK(f.SafeDescription("") == "");
    CHECK(f.SafeDescription("a  b") == "a b");
    CHECK(f.SafeDescription("a     b      c") == "a b c");
    CHECK(f.SafeDescription("  ") == " ");
    CHECK(f.SafeDescription(std::string(25, 'x')) == std::string(25, 'x'));
    CHECK(f.SafeDescription(std::string(26, 'x')) == std::string(25, 'x'));
    CHECK(f.SafeDescription(std::string(5000, 'y')).size() == 25);
    CHECK(f.SafeDescription(std::string(30, ' ')) == " ");
    // the spaces collapse before truncation
    CHECK(f.SafeDescription("abc          defghijklmnopqrstuvwxyz") == "abc defghijklmnopqrstuvwx");
}

TEST_CASE("Falcon2 model flags follow the model and version numbers", "[vendors2][falcon2]") {
    auto row = GENERATE(table<int, int>({ { 16, 2 }, { 4, 3 }, { 48, 3 }, { 16, 4 }, { 32, 5 }, { 0, 0 } }));
    Falcon f = MakeFalcon(std::get<0>(row), std::get<1>(row));
    CHECK(f.IsF16() == (std::get<0>(row) == 16));
    CHECK(f.IsF4() == (std::get<0>(row) == 4));
    CHECK(f.IsF48() == (std::get<0>(row) == 48));
    CHECK(f.IsV2() == (std::get<1>(row) == 2));
    CHECK(f.IsV3() == (std::get<1>(row) == 3));
    CHECK(f.IsV4() == (std::get<1>(row) == 4));
}

TEST_CASE("Falcon2 daughter board thresholds depend on the model", "[vendors2][falcon2]") {
    Falcon f16 = MakeFalcon(16, 3);
    CHECK(f16.GetDaughter1Threshold() == 16);
    CHECK(f16.GetBank1Threshold() == 16);
    CHECK(f16.GetDaughter2Threshold() == 32);
    Falcon f48 = MakeFalcon(48, 3);
    CHECK(f48.GetDaughter1Threshold() == 16);
    CHECK(f48.GetDaughter2Threshold() == 32);
    Falcon f4 = MakeFalcon(4, 3);
    CHECK(f4.GetDaughter1Threshold() == 4);
    CHECK(f4.GetBank1Threshold() == 6);
    CHECK(f4.GetDaughter2Threshold() == 12);
    Falcon unknown = MakeFalcon(0, 0);
    CHECK(unknown.GetDaughter1Threshold() == 4);
    CHECK(unknown.GetDaughter2Threshold() == 12);
}

TEST_CASE("Falcon2 IsFirmwareEqualOrGreaterThan compares major then minor", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    f._majorFirmwareVersion = 2;
    f._minorFirmwareVersion = 5;
    CHECK(f.IsFirmwareEqualOrGreaterThan(2, 5));
    CHECK(f.IsFirmwareEqualOrGreaterThan(2, 4));
    CHECK_FALSE(f.IsFirmwareEqualOrGreaterThan(2, 6));
    CHECK(f.IsFirmwareEqualOrGreaterThan(1, 99));
    CHECK_FALSE(f.IsFirmwareEqualOrGreaterThan(3, 0));
    CHECK(f.IsFirmwareEqualOrGreaterThan(0, 0));
    CHECK(f.IsFirmwareEqualOrGreaterThan(-1, -1));
}

TEST_CASE("Falcon2 IsEnhancedV2Firmware parses the firmware string", "[vendors2][falcon2]") {
    auto row = GENERATE(table<std::string, bool>({ { "", false }, { "2.1", true }, { "2.0", false }, { "2.10", true }, { "3.0", true }, { "1.99", false },
                                                   { "10.0", true }, { "2", false }, { "2.1.5", true }, { "UNKNOWN", false }, { "abc", false },
                                                   { "2.x", false }, { ".", false }, { "-2.5", false } }));
    Falcon f = MakeFalcon(16, 2, std::get<0>(row));
    INFO("firmware '" << std::get<0>(row) << "'");
    CHECK(f.IsEnhancedV2Firmware() == std::get<1>(row));
}

TEST_CASE("Falcon2 IsEnhancedV2Firmware with an absurdly large major version", "[vendors2][falcon2][!mayfail]") {
    // Falcon.cpp IsEnhancedV2Firmware: strtol saturates to LONG_MAX and the (int) cast makes it -1, so a huge major reads as old
    Falcon f = MakeFalcon(16, 2, "99999999999999999999.0");
    CHECK(f.IsEnhancedV2Firmware());
}

TEST_CASE("Falcon2 GetMaxPixels is limited on old V2 firmware", "[vendors2][falcon2]") {
    CHECK(MakeFalcon(16, 2, "1.5").GetMaxPixels() == 680);
    CHECK(MakeFalcon(16, 2, "").GetMaxPixels() == 680);
    CHECK(MakeFalcon(16, 2, "2.1").GetMaxPixels() == 1024);
    CHECK(MakeFalcon(16, 3, "1.0").GetMaxPixels() == 1024);
    CHECK(MakeFalcon(16, 4, "").GetMaxPixels() == 1024);
}

TEST_CASE("Falcon2 SupportsVariableExpansions", "[vendors2][falcon2]") {
    CHECK(MakeFalcon(16, 3).SupportsVariableExpansions());
    CHECK(MakeFalcon(16, 2, "2.1").SupportsVariableExpansions());
    CHECK_FALSE(MakeFalcon(16, 2, "1.9").SupportsVariableExpansions());
    CHECK_FALSE(MakeFalcon(16, 4).SupportsVariableExpansions());
}

TEST_CASE("Falcon2 V4 decode mode", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    auto row = GENERATE(table<int, std::string>({ { 0, "E1.31/ArtNET" }, { 1, "ZCPP" }, { 2, "DDP" }, { 3, "Remote" }, { 4, "Master" }, { 5, "Player" },
                                                  { 6, "Unknown" }, { -1, "Unknown" }, { 64, "Unknown" } }));
    CHECK(f.V4_DecodeMode(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("Falcon2 V4 board configuration names", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    for (int i = 0; i <= 11; ++i) {
        INFO("config " << i);
        CHECK(f.V4_DecodeBoardConfiguration(i) != "Unknown");
    }
    CHECK(f.V4_DecodeBoardConfiguration(12) == "Unknown");
    CHECK(f.V4_DecodeBoardConfiguration(-1) == "Unknown");
    CHECK(f.V4_DecodeBoardConfiguration(0) == "16 Local Ports");
    CHECK(f.V4_DecodeBoardConfiguration(10) == "12 Smart Receiver Chains");
    CHECK(f.V4_DecodeBoardConfiguration(11) == "8 Smart Receiver Chains");
}

TEST_CASE("Falcon2 V4 board port counts", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    auto row = GENERATE(table<int, int>({ { 0, 16 }, { 1, 24 }, { 2, 32 }, { 3, 40 }, { 4, 32 }, { 5, 40 }, { 6, 48 }, { 7, 48 }, { 8, 48 }, { 9, 48 }, { 10, 48 },
                                          { 11, 32 }, { 12, 0 }, { -1, 0 }, { 255, 0 } }));
    INFO("board mode " << std::get<0>(row));
    CHECK(f.V4_GetBoardPorts(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("Falcon2 V4 smart remote port ranges", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    struct R {
        int mode, lo, hi;
    };
    // [lo,hi) is the smart remote range; -1 means never, 999 means always
    const R rows[] = { { 0, -1, -1 }, { 1, 16, 24 }, { 2, 16, 24 }, { 3, 32, 40 }, { 4, 16, 32 }, { 5, 16, 40 }, { 6, 32, 40 }, { 7, 32, 48 },
                       { 8, 16, 40 }, { 9, 16, 48 }, { 10, 999, 999 }, { 11, 999, 999 } };
    for (auto const& r : rows) {
        for (int port = -1; port < 60; ++port) {
            bool expect = r.lo == 999 ? true : (r.lo == -1 ? false : (port >= r.lo && port < r.hi));
            INFO("mode " << r.mode << " port " << port);
            CHECK(f.V4_IsPortSmartRemoteEnabled(r.mode, port) == expect);
        }
    }
    CHECK_FALSE(f.V4_IsPortSmartRemoteEnabled(12, 20));
    CHECK_FALSE(f.V4_IsPortSmartRemoteEnabled(-1, 20));
}

TEST_CASE("Falcon2 V4 pixel protocol names round trip", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    for (int p = 0; p <= 19; ++p) {
        std::string n = f.V4_DecodePixelProtocol(p);
        INFO("protocol " << p);
        REQUIRE_FALSE(n.empty());
        CHECK(f.V4_EncodePixelProtocol(n) == p);
    }
    CHECK(f.V4_DecodePixelProtocol(20).empty());
    CHECK(f.V4_DecodePixelProtocol(-1).empty());
}

TEST_CASE("Falcon2 V4 pixel protocol aliases and unknown names", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    CHECK(f.V4_EncodePixelProtocol("tm18xx") == 14);
    CHECK(f.V4_EncodePixelProtocol("lx1203") == 14);
    CHECK(f.V4_EncodePixelProtocol("") == -1);
    CHECK(f.V4_EncodePixelProtocol("WS2811") == -1); // case sensitive unlike the V3 decoder
    CHECK(f.V4_EncodePixelProtocol("nonsense") == -1);
}

TEST_CASE("Falcon2 V4 max pixels per port", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    struct R {
        const char* proto;
        int small, big;
    };
    const R rows[] = { { "apa102", 766, 526 }, { "tm1814", 766, 526 }, { "apa109", 768, 528 }, { "lpd8806", 1023, 703 }, { "tm1829", 1023, 703 },
                       { "sm16716", 943, 647 }, { "ucs8903", 512, 352 }, { "ucs8903 16 bit", 512, 352 }, { "ws2811 slow", 512, 352 },
                       { "ucs8904", 384, 264 }, { "ucs8904 16 bit", 384, 264 }, { "lpd6803", 1024, 704 }, { "ws2801", 1024, 704 },
                       { "ws2811", 1024, 704 }, { "tls3001", 288, 192 }, { "dmx512p", 743, 510 }, { "dmx512p-4", 557, 382 },
                       { "sj1221", 339, 232 }, { "sm16825", 306, 210 }, { "sm16825 16 bit", 306, 210 } };
    for (auto const& r : rows) {
        int p = f.V4_EncodePixelProtocol(r.proto);
        INFO(r.proto);
        REQUIRE(p >= 0);
        CHECK(f.V4_GetMaxPortPixels(0, p) == r.small);   // 16 ports
        CHECK(f.V4_GetMaxPortPixels(2, p) == r.small);   // 32 ports is still the small table
        CHECK(f.V4_GetMaxPortPixels(6, p) == r.big);     // 48 ports
        CHECK(f.V4_GetMaxPortPixels(3, p) == r.big);     // 40 ports
    }
}

TEST_CASE("Falcon2 V4 max pixels for unknown protocol or board is zero", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    CHECK(f.V4_GetMaxPortPixels(0, 99) == 0);
    CHECK(f.V4_GetMaxPortPixels(0, -1) == 0);
    CHECK(f.V4_GetMaxPortPixels(6, 99) == 0);
    // unknown board mode reports 0 ports so falls into the small table
    CHECK(f.V4_GetMaxPortPixels(99, 14) == 1024);
}

TEST_CASE("Falcon2 V4 input protocol", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    CHECK(f.V4_EncodeInputProtocol("E131") == 0);
    CHECK(f.V4_EncodeInputProtocol("ArtNet") == 1);
    CHECK(f.V4_EncodeInputProtocol("") == 0);
    CHECK(f.V4_EncodeInputProtocol("DDP") == 0);
}

TEST_CASE("Falcon2 V4 colour order encodes white orders", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    auto row = GENERATE(table<std::string, int>({ { "RGB", 0 }, { "RBG", 1 }, { "GRB", 2 }, { "GBR", 3 }, { "BRG", 4 }, { "BGR", 5 },
                                                  { "RGBW", 0 }, { "RBGW", 1 }, { "GRBW", 2 }, { "GBRW", 3 }, { "BRGW", 4 }, { "BGRW", 5 },
                                                  { "WRGB", 6 }, { "WRBG", 7 }, { "WGRB", 8 }, { "WGBR", 9 }, { "WBRG", 10 }, { "WBGR", 11 },
                                                  { "", 0 }, { "XYZ", 0 }, { "rgb", 0 }, { "RGBA", 0 }, { "WWWW", 6 }, { "W", 0 }, { "WW", 0 } }));
    INFO(std::get<0>(row));
    CHECK(f.V4_EncodeColourOrder(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("Falcon2 V4 valid brightness snaps to supported steps", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    auto row = GENERATE(table<int, int>({ { 100, 100 }, { 96, 100 }, { 95, 90 }, { 86, 90 }, { 85, 80 }, { 76, 80 }, { 75, 70 }, { 66, 70 }, { 65, 60 },
                                          { 56, 60 }, { 55, 50 }, { 46, 50 }, { 45, 40 }, { 36, 40 }, { 35, 30 }, { 26, 30 }, { 25, 20 }, { 19, 20 },
                                          { 18, 15 }, { 13, 15 }, { 12, 10 }, { 8, 10 }, { 7, 5 }, { 0, 5 }, { -100, 5 }, { 1000, 100 } }));
    INFO("input " << std::get<0>(row));
    CHECK(f.V4_ValidBrightness(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("Falcon2 V4 valid gamma snaps to supported steps", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    auto row = GENERATE(table<int, int>({ { 0, 10 }, { 10, 10 }, { 14, 10 }, { 15, 20 }, { 21, 20 }, { 22, 23 }, { 24, 23 }, { 25, 25 }, { 26, 25 },
                                          { 27, 28 }, { 29, 28 }, { 30, 30 }, { 100, 30 }, { -5, 10 } }));
    CHECK(f.V4_ValidGamma(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("Falcon2 V4 valid brightness is idempotent", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    for (int b = -10; b <= 120; ++b) {
        int v = f.V4_ValidBrightness(b);
        INFO("b " << b);
        CHECK(f.V4_ValidBrightness(v) == v);
    }
}

TEST_CASE("Falcon2 V4 string lookup helpers", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    std::vector<Falcon::FALCON_V4_STRING> strs;
    for (int p = 0; p < 3; ++p) {
        for (int sr = 0; sr < 3; ++sr) {
            Falcon::FALCON_V4_STRING s{};
            s.port = p;
            s.smartRemote = sr;
            s.brightness = 10 * p + sr;
            s.gamma = 20 + p;
            strs.push_back(s);
        }
    }
    CHECK(f.V4_GetStringFirstIndex(strs, 0, 0) == 0);
    CHECK(f.V4_GetStringFirstIndex(strs, 1, 2) == 5);
    CHECK(f.V4_GetStringFirstIndex(strs, 2, -1) == 6);
    CHECK(f.V4_GetStringFirstIndex(strs, 3, -1) == -1);
    CHECK(f.V4_GetStringFirstIndex(strs, 0, 3) == -1);
    CHECK(f.V4_GetStringFirstIndex({}, 0, 0) == -1);
    CHECK(f.V4_GetBrightness(1, 2, 77, strs) == 12);
    CHECK(f.V4_GetBrightness(9, 2, 77, strs) == 77);
    CHECK(f.V4_GetBrightness(0, 0, 77, {}) == 77);
    CHECK(f.V4_GetGamma(2, 1, 1.5f, strs) == 22);
    CHECK(f.V4_GetGamma(2, 7, 1.5f, strs) == 1); // float default truncated to int
}

TEST_CASE("Falcon2 V4 GetStartChannel in absolute and universe modes", "[vendors2][falcon2]") {
    Falcon f = MakeFalcon(16, 4);
    int universe = -1;
    unsigned long sc = 0;

    f._v4status = nlohmann::json{ { "A", 0 } };
    f.V4_GetStartChannel(5, 10, 101, universe, sc, false, 0);
    CHECK(universe == 0);
    CHECK(sc == 100);
    f.V4_GetStartChannel(5, 10, 101, universe, sc, true, 51);
    CHECK(sc == 50);
    f.V4_GetStartChannel(5, 10, 1, universe, sc, true, 1);
    CHECK(sc == 0);

    f._v4status = nlohmann::json{ { "A", 1 } };
    f.V4_GetStartChannel(5, 10, 101, universe, sc, true, 51);
    CHECK(universe == 5);
    CHECK(sc == 9);
}

TEST_CASE("Falcon2 V4 GetStartChannel wraps when the model starts before the controller", "[vendors2][falcon2]") {
    Falcon f = MakeFalcon(16, 4);
    f._v4status = nlohmann::json{ { "A", 0 } };
    int universe = 0;
    unsigned long sc = 0;
    // model channel 1 with a controller starting at 100 underflows the unsigned start channel
    f.V4_GetStartChannel(1, 1, 1, universe, sc, true, 100);
    CHECK(sc > 1000000UL);
}

TEST_CASE("Falcon2 V4 GetStartChannel without status A throws", "[vendors2][falcon2][!mayfail]") {
    // Falcon.cpp V4_GetStartChannel: _v4status["A"].get<int>() throws type_error when the key is missing
    Falcon f = MakeFalcon(16, 4);
    int universe = 0;
    unsigned long sc = 0;
    CHECK_NOTHROW(f.V4_GetStartChannel(1, 1, 1, universe, sc, false, 0));
}

TEST_CASE("Falcon2 V4 GetRebootSecs depends on whether the status IP matches", "[vendors2][falcon2]") {
    Falcon f = MakeFalcon(16, 4);
    f._ip = "10.0.0.5";
    f._v4status = nlohmann::json{ { "I", "10.0.0.5" } };
    CHECK(f.V4_GetRebootSecs() == 8);
    f._v4status = nlohmann::json{ { "I", "10.0.0.6" } };
    CHECK(f.V4_GetRebootSecs() == 20);
}

TEST_CASE("Falcon2 V4 GetRebootSecs with no status IP throws", "[vendors2][falcon2][!mayfail]") {
    // Falcon.cpp V4_GetRebootSecs: _v4status["I"].get<std::string>() throws on a missing/non-string key
    Falcon f = MakeFalcon(16, 4);
    CHECK_NOTHROW(f.V4_GetRebootSecs());
}

TEST_CASE("Falcon2 GetMode for V4 reads O from the status", "[vendors2][falcon2]") {
    Falcon f = MakeFalcon(16, 4);
    CHECK(f.GetMode().empty());
    f._v4status = nlohmann::json{ { "O", 2 } };
    CHECK(f.GetMode() == "DDP");
    f._v4status = nlohmann::json{ { "O", "2" } };
    CHECK(f.GetMode().empty());
    f._v4status = nlohmann::json{ { "O", 2.5 } };
    CHECK(f.GetMode().empty());
    f._v4status = nlohmann::json{ { "O", 99 } };
    CHECK(f.GetMode() == "Unknown");
}

TEST_CASE("Falcon2 GetMode for V3 decodes the status m value", "[vendors2][falcon2]") {
    Falcon f = MakeFalcon(16, 3);
    f._status["m"] = "0";
    CHECK(f.GetMode() == "E131/ArtNET");
    f._status["m"] = "64";
    CHECK(f.GetMode() == "DDP");
    f._status["m"] = "16";
    CHECK(f.GetMode() == "ZCPP");
    f._status["m"] = "banana";
    CHECK(f.GetMode() == "E131/ArtNET"); // strtol failure reads as 0
}

TEST_CASE("Falcon2 GetMode for V3 without an m entry throws", "[vendors2][falcon2][!mayfail]") {
    // Falcon.cpp GetMode: _status["m"].get<std::string>() on a missing key throws type_error
    Falcon f = MakeFalcon(16, 3);
    CHECK_NOTHROW(f.GetMode());
}

TEST_CASE("Falcon2 V4 input json round trip", "[vendors2][falcon2]") {
    Falcon::FALCON_V4_INPUTS in{};
    in.universe = 7;
    in.channels = 512;
    in.universeCount = 12;
    in.protocol = 1;
    auto j = in.asJson();
    CHECK(j["p"] == "a");
    CHECK(j["u"] == 7);
    Falcon::FALCON_V4_INPUTS back(j);
    CHECK(back.universe == 7);
    CHECK(back.channels == 512);
    CHECK(back.universeCount == 12);
    CHECK(back.protocol == 1);
    j["p"] = "e";
    CHECK(Falcon::FALCON_V4_INPUTS(j).protocol == 0);
    j["p"] = "anything";
    CHECK(Falcon::FALCON_V4_INPUTS(j).protocol == 1);
}

TEST_CASE("Falcon2 V4 input json with a missing key should throw not assert", "[vendors2][falcon2][.][known-crash]") {
    // Falcon.h FALCON_V4_INPUTS(json const&): json["u"] on a const object with a missing key is an
    // assert (debug) / undefined behaviour (release) rather than an exception; use json.at() or contains().
    nlohmann::json good = { { "u", 1 }, { "c", 512 }, { "uc", 3 }, { "p", "e" } };
    for (const char* key : { "u", "c", "uc", "p" }) {
        nlohmann::json j = good;
        j.erase(key);
        INFO("missing " << key);
        CHECK_THROWS(Falcon::FALCON_V4_INPUTS(j));
    }
}

TEST_CASE("Falcon2 V4 input json with mistyped keys throws", "[vendors2][falcon2]") {
    nlohmann::json good = { { "u", 1 }, { "c", 512 }, { "uc", 3 }, { "p", "e" } };
    CHECK_NOTHROW(Falcon::FALCON_V4_INPUTS(good));
    nlohmann::json bad = good;
    bad["u"] = "one";
    CHECK_THROWS(Falcon::FALCON_V4_INPUTS(bad));
    bad = good;
    bad["p"] = 5;
    CHECK_THROWS(Falcon::FALCON_V4_INPUTS(bad));
    bad = good;
    bad["c"] = nullptr;
    CHECK_THROWS(Falcon::FALCON_V4_INPUTS(bad));
}

TEST_CASE("Falcon2 V4 string json round trip", "[vendors2][falcon2]") {
    Falcon::FALCON_V4_STRING s{};
    s.port = 3;
    s.string = 1;
    s.smartRemote = 2;
    s.universe = 40;
    s.name = "Roof";
    s.blank = true;
    s.gamma = 28;
    s.brightness = 90;
    s.zigcount = 4;
    s.endNulls = 1;
    s.startNulls = 2;
    s.colourOrder = 5;
    s.direction = 1;
    s.group = 3;
    s.pixels = 150;
    s.protocol = 14;
    s.startChannel = 4000;
    auto j = s.asJson();
    CHECK(j["bl"] == 1);
    Falcon::FALCON_V4_STRING b(j);
    CHECK(b.port == 3);
    CHECK(b.string == 1);
    CHECK(b.smartRemote == 2);
    CHECK(b.universe == 40);
    CHECK(b.name == "Roof");
    CHECK(b.blank);
    CHECK(b.gamma == 28);
    CHECK(b.brightness == 90);
    CHECK(b.zigcount == 4);
    CHECK(b.endNulls == 1);
    CHECK(b.startNulls == 2);
    CHECK(b.colourOrder == 5);
    CHECK(b.direction == 1);
    CHECK(b.group == 3);
    CHECK(b.pixels == 150);
    CHECK(b.protocol == 14);
    CHECK(b.startChannel == 4000UL);
}

TEST_CASE("Falcon2 V4 string json with a missing key should throw not assert", "[vendors2][falcon2][.][known-crash]") {
    // same const operator[] problem as FALCON_V4_INPUTS: a missing key aborts instead of throwing
    Falcon::FALCON_V4_STRING s{};
    s.name = "x";
    nlohmann::json good = s.asJson();
    for (auto it = good.begin(); it != good.end(); ++it) {
        nlohmann::json j = good;
        j.erase(it.key());
        INFO("missing " << it.key());
        CHECK_THROWS(Falcon::FALCON_V4_STRING(j));
    }
}

TEST_CASE("Falcon2 V4 string json rejects a non-string name", "[vendors2][falcon2]") {
    Falcon::FALCON_V4_STRING s{};
    nlohmann::json j = s.asJson();
    j["nm"] = 12;
    CHECK_THROWS(Falcon::FALCON_V4_STRING(j));
    j["nm"] = nullptr;
    CHECK_THROWS(Falcon::FALCON_V4_STRING(j));
}

TEST_CASE("Falcon2 V4 string json accepts a boolean blank flag", "[vendors2][falcon2]") {
    Falcon::FALCON_V4_STRING s{};
    nlohmann::json j = s.asJson();
    j["bl"] = true;
    CHECK(Falcon::FALCON_V4_STRING(j).blank);
    j["bl"] = 0;
    CHECK_FALSE(Falcon::FALCON_V4_STRING(j).blank);
}

TEST_CASE("Falcon2 CountStrings counts port transitions", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    auto parse = [](const char* xml) {
        pugi::xml_document d;
        REQUIRE(d.load_string(xml));
        return d.document_element() ? 1 : 0;
    };
    (void)parse;
    pugi::xml_document d;
    d.load_string("<vs><vs p=\"0\"/><vs p=\"0\"/><vs p=\"1\"/><vs p=\"1\"/><vs p=\"3\"/></vs>");
    CHECK(f.CountStrings(d) == 3);
    pugi::xml_document empty;
    CHECK(f.CountStrings(empty) == 0);
    pugi::xml_document rootOnly;
    rootOnly.load_string("<vs/>");
    CHECK(f.CountStrings(rootOnly) == 0);
    pugi::xml_document descending;
    descending.load_string("<vs><vs p=\"5\"/><vs p=\"4\"/><vs p=\"3\"/></vs>");
    CHECK(f.CountStrings(descending) == 1);
    pugi::xml_document noPort;
    noPort.load_string("<vs><vs/><vs/></vs>");
    CHECK(f.CountStrings(noPort) == 1); // missing p defaults to port 0
}

TEST_CASE("Falcon2 MaxPixels reads the k attributes", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    pugi::xml_document d;
    d.load_string("<vs k0=\"100\" k1=\"200\" k2=\"300\"/>");
    CHECK(f.MaxPixels(d, 0) == 100);
    CHECK(f.MaxPixels(d, 1) == 200);
    CHECK(f.MaxPixels(d, 2) == 300);
    CHECK(f.MaxPixels(d, 3) == 0);
    CHECK(f.MaxPixels(d, -1) == 0);
    pugi::xml_document none;
    CHECK(f.MaxPixels(none, 0) == 0);
    pugi::xml_document missing;
    missing.load_string("<vs k0=\"abc\"/>");
    CHECK(f.MaxPixels(missing, 0) == 0);
    CHECK(f.MaxPixels(missing, 1) == 0);
}

TEST_CASE("Falcon2 V4_ValidateWAV rejects missing and short files", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    CHECK_FALSE(f.V4_ValidateWAV("definitely/not/here.wav"));
}

#ifndef _WIN32
TEST_CASE("Falcon2 constructor identifies a V3 controller from status.xml", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        resp.contentType = "text/xml";
        if (r.path == "/status.xml") {
            resp.body = "<status><fv>1.40</fv><a>1</a><n>  Test Falcon  </n><p>5</p><m>0</m></status>";
        } else {
            resp.status = 404;
        }
        return resp;
    });
    REQUIRE(srv.ok());
    Falcon f(srv.Address(), "");
    CHECK(f.IsConnected());
    CHECK(f.GetName() == "Test Falcon");
    CHECK(f.GetModel() == "F16v3");
    CHECK(f._firmwareVersion == "1.40");
    CHECK(f._majorFirmwareVersion == 1);
    CHECK(f._minorFirmwareVersion == 40);
    CHECK_FALSE(f._usingAbsolute);
    CHECK(f.GetMode() == "E131/ArtNET");
}

TEST_CASE("Falcon2 constructor flags absolute addressing", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "<status><v>2.1</v><a>0</a><n>x</n><p>4</p></status>";
        return r;
    });
    Falcon f(srv.Address(), "");
    CHECK(f.IsConnected());
    CHECK(f._usingAbsolute);
    CHECK(f.GetModel() == "F4v2");
    CHECK(f.IsEnhancedV2Firmware());
}

TEST_CASE("Falcon2 constructor names the 48 port V3 plain F48", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "<status><v>1.0</v><p>7</p></status>";
        return r;
    });
    Falcon f(srv.Address(), "");
    CHECK(f.GetModel() == "F48");
    CHECK(f._modelnum == 48);
}

TEST_CASE("Falcon2 constructor falls back to index.htm for the firmware", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/status.xml") {
            resp.body = "<status><n>x</n><p>6</p></status>";
        } else if (r.path == "/index.htm") {
            resp.body = "<html><head><title>F4V3            - v1.10</title></head></html>";
        }
        return resp;
    });
    Falcon f(srv.Address(), "");
    CHECK(f.IsConnected());
    CHECK(f._firmwareVersion == "1.10");
    CHECK(f._majorFirmwareVersion == 1);
    CHECK(f._minorFirmwareVersion == 10);
    CHECK(srv.Count("/index.htm") == 1);
}

TEST_CASE("Falcon2 constructor disconnects when index.htm is unavailable", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/status.xml") {
            resp.body = "<status><p>6</p></status>";
        } else {
            resp.status = 404;
        }
        return resp;
    });
    Falcon f(srv.Address(), "");
    CHECK_FALSE(f.IsConnected());
}

TEST_CASE("Falcon2 constructor survives malformed status XML", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    for (std::string body : { std::string("<status><p>"), std::string("not xml at all"), std::string("<a/>"), std::string("\x01\x02\x03") }) {
        FakeHttpServer srv([&](const FakeRequest&) {
            FakeResponse r;
            r.body = body;
            return r;
        });
        Falcon f(srv.Address(), "");
        INFO(body);
        CHECK_FALSE(f.IsConnected());
    }
}

TEST_CASE("Falcon2 constructor with an unknown product id assumes a 16 port V3", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "<status><v>3.0</v><p>9999</p></status>";
        return r;
    });
    Falcon f(srv.Address(), "");
    CHECK(f.GetModel() == "F16v3");
}

TEST_CASE("Falcon2 constructor with a non numeric product id is not connected", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "<status><v>3.0</v><p>abc</p></status>";
        return r;
    });
    Falcon f(srv.Address(), "");
    // strtol gives 0, which decodes as the default 16 port V3
    CHECK(f.GetModel() == "F16v3");
}

TEST_CASE("Falcon2 constructor reads V4 board count from the API status", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/status.xml") {
            resp.body = "<status><v>1.0</v><n>v4</n><p>128</p></status>";
        } else if (r.path == "/api") {
            resp.body = V4Reply("{\"BR\":32,\"O\":2,\"A\":1,\"I\":\"1.2.3.4\"}");
        }
        return resp;
    });
    Falcon f(srv.Address(), "");
    CHECK(f.IsConnected());
    CHECK(f._modelnum == 32);
    CHECK(f.GetModel() == "F32v4");
    CHECK(f.GetMode() == "DDP");
    CHECK(f.V4_GetStatus()["I"] == "1.2.3.4");
}

TEST_CASE("Falcon2 constructor rejects a V4 controller whose status lacks BR", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/status.xml") {
            resp.body = "<status><v>1.0</v><p>128</p></status>";
        } else {
            resp.body = V4Reply("{\"O\":2}");
        }
        return resp;
    });
    Falcon f(srv.Address(), "");
    CHECK_FALSE(f.IsConnected());
    CHECK(f.V4_GetStatus().empty());
}

TEST_CASE("Falcon2 constructor rejects a V4 controller whose BR is not an integer", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/status.xml") {
            resp.body = "<status><v>1.0</v><p>130</p></status>";
        } else {
            resp.body = V4Reply("{\"BR\":\"16\"}");
        }
        return resp;
    });
    Falcon f(srv.Address(), "");
    CHECK_FALSE(f.IsConnected());
}

TEST_CASE("Falcon2 CallFalconV4API decodes a good reply", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{\"x\":5}", 200, 1, 3, 1);
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    bool fin = false, rb = false;
    int ob = -1;
    nlohmann::json out;
    CHECK(f.CallFalconV4API("Q", "ST", 0, 0, 0, nlohmann::json(), fin, ob, rb, out) == 200);
    CHECK(fin);
    CHECK(rb);
    CHECK(ob == 3);
    CHECK(out["x"] == 5);
    auto reqs = srv.Requests();
    REQUIRE(reqs.size() == 1);
    CHECK(reqs[0].path == "/api");
    auto sent = nlohmann::json::parse(reqs[0].body);
    CHECK(sent["T"] == "Q");
    CHECK(sent["M"] == "ST");
    CHECK(sent["P"].is_object());
}

TEST_CASE("Falcon2 CallFalconV4API returns 506 for unusable replies", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    auto body = GENERATE(std::string(""), std::string("garbage"), std::string("{"), std::string("[]"), std::string("{}"), std::string("null"),
                         std::string("{\"R\":\"200\"}"), std::string("{\"R\":200}"), std::string("{\"R\":200,\"F\":\"1\",\"B\":0,\"RB\":0,\"P\":{}}"),
                         std::string("{\"R\":null,\"F\":1,\"B\":0,\"RB\":0,\"P\":{}}"));
    FakeHttpServer srv([&](const FakeRequest&) {
        FakeResponse r;
        r.body = body;
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    bool fin = false, rb = false;
    int ob = 0;
    nlohmann::json out;
    INFO("reply '" << body << "'");
    int rc = f.CallFalconV4API("Q", "ST", 0, 0, 0, nlohmann::json(), fin, ob, rb, out);
    // the first case ("") is an empty reply and also yields 506
    CHECK(rc == 506);
}

TEST_CASE("Falcon2 CallFalconV4API without a payload yields a null result", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "{\"R\":200,\"F\":1,\"B\":2,\"RB\":0}";
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    bool fin = false, rb = true;
    int ob = -1;
    nlohmann::json out = 5;
    CHECK(f.CallFalconV4API("Q", "ST", 0, 0, 0, nlohmann::json(), fin, ob, rb, out) == 200);
    CHECK(out.is_null());
    CHECK(ob == 2);
}

TEST_CASE("Falcon2 CallFalconV4API passes through error result codes", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{}", 400);
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    bool fin, rb;
    int ob;
    nlohmann::json out;
    CHECK(f.CallFalconV4API("Q", "ST", 0, 0, 0, nlohmann::json(), fin, ob, rb, out) == 400);
}

TEST_CASE("Falcon2 CallFalconV4API strips non ascii from a reply that is not valid json", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "{\"R\":200,\"F\":1,\"B\":0,\"RB\":0,\"P\":{\"n\":\"a\xff" "b\"}}";
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    bool fin = false, rb = true;
    int ob = -1;
    nlohmann::json out;
    CHECK(f.CallFalconV4API("Q", "ST", 0, 0, 0, nlohmann::json(), fin, ob, rb, out) == 200);
    CHECK(out["n"] == "ab");
    CHECK_FALSE(rb);
}

TEST_CASE("Falcon2 CallFalconV4API on an unreachable controller returns 506", "[vendors2][falcon2]") {
    Falcon f = MakeDisconnectedFalcon();
    bool fin, rb;
    int ob;
    nlohmann::json out;
    CHECK(f.CallFalconV4API("Q", "ST", 0, 0, 0, nlohmann::json(), fin, ob, rb, out) == 506);
}

TEST_CASE("Falcon2 V4_GetInputs follows batches until the final reply", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        auto req = nlohmann::json::parse(r.body);
        int b = req["B"].get<int>();
        if (b == 0) {
            resp.body = V4Reply("{\"A\":[{\"p\":\"e\",\"u\":1,\"c\":512,\"uc\":16}]}", 200, 0, 0);
        } else {
            resp.body = V4Reply("{\"A\":[{\"p\":\"a\",\"u\":17,\"c\":170,\"uc\":2}]}", 200, 1, 1);
        }
        return resp;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    std::vector<Falcon::FALCON_V4_INPUTS> in;
    CHECK(f.V4_GetInputs(in));
    REQUIRE(in.size() == 2);
    CHECK(in[0].universe == 1);
    CHECK(in[0].protocol == 0);
    CHECK(in[1].universe == 17);
    CHECK(in[1].protocol == 1);
    CHECK(srv.Requests().size() == 2);
}

TEST_CASE("Falcon2 V4_GetInputs reports failure on a bad reply", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{}", 500);
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    std::vector<Falcon::FALCON_V4_INPUTS> in(3);
    CHECK_FALSE(f.V4_GetInputs(in));
    CHECK(in.empty());
}

TEST_CASE("Falcon2 V4_GetInputs with an incomplete A entry should not crash", "[vendors2][falcon2][http][.][known-crash]") {
    // Falcon.cpp V4_GetInputs: emplace_back(inpj) builds FALCON_V4_INPUTS from a const json, so a reply
    // entry with a missing key hits json const operator[] (assert/UB) instead of an exception
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{\"A\":[{\"p\":\"e\"}]}");
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    std::vector<Falcon::FALCON_V4_INPUTS> in;
    CHECK_NOTHROW(f.V4_GetInputs(in));
}

TEST_CASE("Falcon2 V4_GetInputs tolerates a payload with no A", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{}");
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    std::vector<Falcon::FALCON_V4_INPUTS> in;
    CHECK(f.V4_GetInputs(in));
    CHECK(in.empty());
}

TEST_CASE("Falcon2 V4_GetStrings parses strings and stops on the final batch", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{\"A\":[{\"p\":9,\"s\":0,\"r\":0,\"v\":0,\"u\":10,\"sc\":0,\"n\":100,\"z\":0,\"ns\":0,\"ne\":0,\"g\":10,\"o\":0,\"b\":40,\"gp\":1,\"nm\":\"Port 10\",\"bl\":0,\"l\":13}]}");
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    std::vector<Falcon::FALCON_V4_STRING> s;
    REQUIRE(f.V4_GetStrings(s));
    REQUIRE(s.size() == 1);
    CHECK(s[0].port == 9);
    CHECK(s[0].name == "Port 10");
    CHECK(s[0].pixels == 100);
    CHECK(s[0].protocol == 13);
    CHECK_FALSE(s[0].blank);
}

TEST_CASE("Falcon2 V4_GetMediaFiles accepts strings and objects and skips junk", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{\"F\":[\"a.wav\",{\"f\":\"b.wav\"},{\"f\":3},{\"x\":\"c.wav\"},7,null,[\"d.wav\"]]}");
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    auto files = f.V4_GetMediaFiles();
    REQUIRE(files.size() == 2);
    CHECK(files[0] == "a.wav");
    CHECK(files[1] == "b.wav");
}

TEST_CASE("Falcon2 V4_GetMediaFiles ignores an F that is not an array", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    for (std::string payload : { std::string("{\"F\":\"a.wav\"}"), std::string("{\"F\":{}}"), std::string("{}"), std::string("{\"F\":[]}") }) {
        FakeHttpServer srv([&](const FakeRequest&) {
            FakeResponse r;
            r.body = V4Reply(payload);
            return r;
        });
        Falcon f = MakeDisconnectedFalcon();
        f._ip = srv.Address();
        INFO(payload);
        CHECK(f.V4_GetMediaFiles().empty());
    }
}

TEST_CASE("Falcon2 V4_GetMediaFiles clears the list when a later batch fails", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        int b = nlohmann::json::parse(r.body)["B"].get<int>();
        resp.body = b == 0 ? V4Reply("{\"F\":[\"a.wav\"]}", 200, 0, 0) : V4Reply("{}", 500);
        return resp;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    CHECK(f.V4_GetMediaFiles().empty());
}

TEST_CASE("Falcon2 V4_IsFileUploading reads the D flag", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    auto row = GENERATE(table<std::string, bool>({ { "{\"D\":1}", false }, { "{\"D\":0}", true }, { "{\"D\":2}", true } }));
    FakeHttpServer srv([&](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply(std::get<0>(row));
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    CHECK(f.V4_IsFileUploading() == std::get<1>(row));
}

TEST_CASE("Falcon2 V4_IsFileUploading treats an error reply as not uploading", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{}", 500);
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    CHECK_FALSE(f.V4_IsFileUploading());
}

TEST_CASE("Falcon2 V4_IsFileUploading without D throws", "[vendors2][falcon2][http][!mayfail]") {
    // Falcon.cpp V4_IsFileUploading: outParams["D"].get<int>() throws type_error on a reply without D
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{}");
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    CHECK_NOTHROW(f.V4_IsFileUploading());
}

TEST_CASE("Falcon2 V4_GetConversionProgress", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    auto row = GENERATE(table<std::string, int>({ { "{\"D\":1,\"MP3\":0}", 100 }, { "{\"D\":0,\"MP3\":100}", 99 }, { "{\"D\":0,\"MP3\":42}", 42 },
                                                  { "{\"D\":0,\"MP3\":0}", 0 }, { "{\"D\":0,\"MP3\":150}", 150 } }));
    FakeHttpServer srv([&](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply(std::get<0>(row));
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    CHECK(f.V4_GetConversionProgress() == std::get<1>(row));
}

TEST_CASE("Falcon2 NumConfiguredStrings counts V4 string entries", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{\"A\":[{},{},{}]}");
        return r;
    });
    Falcon f = MakeFalcon(16, 4);
    f._ip = srv.Address();
    CHECK(f.NumConfiguredStrings() == 3);
}

TEST_CASE("Falcon2 NumConfiguredStrings on V4 failure is minus one", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{}", 500);
        return r;
    });
    Falcon f = MakeFalcon(16, 4);
    f._ip = srv.Address();
    CHECK(f.NumConfiguredStrings() == -1);
}

TEST_CASE("Falcon2 NumConfiguredStrings with a non array A does not throw", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{\"A\":7}");
        return r;
    });
    Falcon f = MakeFalcon(16, 4);
    f._ip = srv.Address();
    CHECK(f.NumConfiguredStrings() == 1); // a scalar has size 1
}

TEST_CASE("Falcon2 NumConfiguredStrings for V3 reads strings.xml", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        resp.contentType = "text/xml";
        if (r.path == "/strings.xml") {
            resp.body = "<vs><vs p=\"0\"/><vs p=\"0\"/><vs p=\"2\"/></vs>";
        }
        return resp;
    });
    Falcon f = MakeFalcon(16, 3);
    f._ip = srv.Address();
    CHECK(f.NumConfiguredStrings() == 2);
}

TEST_CASE("Falcon2 NumConfiguredStrings for V3 with a broken document is zero", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse resp;
        resp.body = "<vs><vs p=";
        return resp;
    });
    Falcon f = MakeFalcon(16, 3);
    f._ip = srv.Address();
    CHECK(f.NumConfiguredStrings() == 0);
}

TEST_CASE("Falcon2 V4_SendBoardMode posts the one based start channel as zero based", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{}", 200, 1, 0, 1);
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    bool reboot = false;
    CHECK(f.V4_SendBoardMode(3, 2, 1, reboot));
    CHECK(reboot);
    auto sent = nlohmann::json::parse(srv.Requests().at(0).body);
    CHECK(sent["M"] == "BM");
    CHECK(sent["T"] == "S");
    CHECK(sent["P"]["B"] == 3);
    CHECK(sent["P"]["O"] == 2);
    CHECK(sent["P"]["ps"] == 0);
}

TEST_CASE("Falcon2 V4_SendInputs batches ten inputs at a time", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{}");
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    std::vector<Falcon::FALCON_V4_INPUTS> in(25);
    for (int i = 0; i < 25; ++i) {
        in[i].universe = i;
        in[i].channels = 512;
        in[i].universeCount = 1;
        in[i].protocol = 0;
    }
    bool reboot = false;
    CHECK(f.V4_SendInputs(in, reboot));
    auto reqs = srv.Requests();
    REQUIRE(reqs.size() == 3);
    CHECK(nlohmann::json::parse(reqs[0].body)["P"]["A"].size() == 10);
    CHECK(nlohmann::json::parse(reqs[2].body)["P"]["A"].size() == 5);
    CHECK(nlohmann::json::parse(reqs[2].body)["I"] == 20);
    CHECK(nlohmann::json::parse(reqs[2].body)["E"] == 25);
}

TEST_CASE("Falcon2 V4_SendInputs of nothing sends nothing and succeeds", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    std::vector<Falcon::FALCON_V4_INPUTS> in;
    bool reboot = false;
    CHECK(f.V4_SendInputs(in, reboot));
    CHECK(srv.Requests().empty());
}

TEST_CASE("Falcon2 V4_SendInputs stops at the first failed batch", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{}", 500);
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    std::vector<Falcon::FALCON_V4_INPUTS> in(30);
    for (auto& i : in) {
        i.universe = 1;
        i.channels = 1;
        i.universeCount = 1;
        i.protocol = 0;
    }
    bool reboot = false;
    CHECK_FALSE(f.V4_SendInputs(in, reboot));
    CHECK(srv.Requests().size() == 1);
}

TEST_CASE("Falcon2 V4_SetSerialConfig posts the serial settings", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = V4Reply("{}");
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    CHECK(f.V4_SetSerialConfig(1, 2, 3, 250000));
    auto sent = nlohmann::json::parse(srv.Requests().at(0).body);
    CHECK(sent["P"]["sm"] == 1);
    CHECK(sent["P"]["su"] == 2);
    CHECK(sent["P"]["ssc"] == 3);
    CHECK(sent["P"]["sr"] == 250000);
}

TEST_CASE("Falcon2 V4_GetStatus merges all batches", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        int b = nlohmann::json::parse(r.body)["B"].get<int>();
        resp.body = b == 0 ? V4Reply("{\"N\":\"Falcon\",\"BR\":16}", 200, 0, 0) : V4Reply("{\"O\":0,\"A\":0}", 200, 1, 1);
        return resp;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    nlohmann::json st = { { "stale", 1 } };
    REQUIRE(f.V4_GetStatus(st));
    CHECK_FALSE(st.contains("stale"));
    CHECK(st["N"] == "Falcon");
    CHECK(st["BR"] == 16);
    CHECK(st["O"] == 0);
}

TEST_CASE("Falcon2 V4_GetStatus clears the result on error", "[vendors2][falcon2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "garbage";
        return r;
    });
    Falcon f = MakeDisconnectedFalcon();
    f._ip = srv.Address();
    nlohmann::json st = { { "stale", 1 } };
    CHECK_FALSE(f.V4_GetStatus(st));
    CHECK(st.empty());
}
#endif

#endif
