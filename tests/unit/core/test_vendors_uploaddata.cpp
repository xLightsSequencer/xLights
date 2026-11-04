#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "controllers/ControllerCaps.h"
#include "controllers/ControllerUploadData.h"
#include "controllers/ExportSettings.h"
#include "models/Pixels.h"
#include "support/TempDir.h"
#include "utils/FileUtils.h"

namespace {

const char* kCaps = R"(<Vendor Name="AcmeLights">
    <AbstractVariant Name="Base">
        <SupportsUpload/>
        <MaxInputUniverses>96</MaxInputUniverses>
        <MaxSerialPortChannels>512</MaxSerialPortChannels>
        <SerialProtocols><Protocol>dmx</Protocol></SerialProtocols>
        <PixelProtocols><Protocol>ws2811</Protocol><Protocol>apa102</Protocol></PixelProtocols>
        <MaxPixelPortChannels>900</MaxPixelPortChannels>
    </AbstractVariant>
    <Controller Name="Pix4">
        <Variant Name="Standard" ID="acme-pix4" Base="AcmeLights:Base">
            <MaxPixelPort>4</MaxPixelPort>
            <MaxSerialPort>1</MaxSerialPort>
        </Variant>
    </Controller>
</Vendor>)";

struct CapsFixture {
    TempDir dir;
    std::string prev;
    CapsFixture() {
        prev = FileUtils::GetResourcesDir();
        std::filesystem::create_directories(dir.path / "controllers");
        dir.Write("controllers/acme.xcontroller", kCaps);
        ControllerCaps::UnloadControllers();
        FileUtils::SetResourcesDir(dir.path.string());
        ControllerCaps::LoadControllers();
    }
    ~CapsFixture() {
        ControllerCaps::UnloadControllers();
        FileUtils::SetResourcesDir(prev);
    }
};

bool Has(const std::vector<std::string>& v, const std::string& s) {
    return std::find(v.begin(), v.end(), s) != v.end();
}

} // namespace

TEST_CASE("IsPixelProtocol is case insensitive and rejects serial names", "[misc][vendors][pixels]") {
    CHECK(IsPixelProtocol("ws2811"));
    CHECK(IsPixelProtocol("WS2811"));
    CHECK(IsPixelProtocol("Apa102"));
    CHECK(IsPixelProtocol("tm1814"));
    CHECK_FALSE(IsPixelProtocol("dmx"));
    CHECK_FALSE(IsPixelProtocol("renard"));
    CHECK_FALSE(IsPixelProtocol("not a protocol"));
    CHECK_FALSE(IsPixelProtocol("ws2811 "));
}

TEST_CASE("IsSerialProtocol is case insensitive and rejects pixel names", "[misc][vendors][pixels]") {
    CHECK(IsSerialProtocol("dmx"));
    CHECK(IsSerialProtocol("DMX"));
    CHECK(IsSerialProtocol("Renard"));
    CHECK(IsSerialProtocol("LOR Enhanced"));
    CHECK(IsSerialProtocol("pixelnet-lynx"));
    CHECK_FALSE(IsSerialProtocol("ws2811"));
    CHECK_FALSE(IsSerialProtocol(""));
}

TEST_CASE("Matrix and PWM protocol predicates are exact", "[misc][vendors][pixels]") {
    CHECK(IsMatrixProtocol("Virtual Matrix"));
    CHECK(IsMatrixProtocol(PROTOCOL_LED_PANEL_MATRIX));
    CHECK(IsMatrixProtocol(PROTOCOL_LED_PANEL_MATRIX_CAPE));
    CHECK(IsMatrixProtocol(PROTOCOL_LED_PANEL_MATRIX_COLORLIGHT));
    CHECK_FALSE(IsMatrixProtocol("virtual matrix"));
    CHECK_FALSE(IsMatrixProtocol("ws2811"));
    CHECK(IsVirtualMatrixProtocol("Virtual Matrix"));
    CHECK_FALSE(IsVirtualMatrixProtocol(PROTOCOL_LED_PANEL_MATRIX));
    CHECK(IsLEDPanelMatrixProtocol(PROTOCOL_LED_PANEL_MATRIX_CAPE));
    CHECK_FALSE(IsLEDPanelMatrixProtocol("Virtual Matrix"));
    CHECK(IsPWMProtocol("PWM"));
    CHECK_FALSE(IsPWMProtocol("pwm"));
}

TEST_CASE("LED panel matrix protocol list keeps the generic one first", "[misc][vendors][pixels]") {
    auto l = GetAllLEDPanelMatrixProtocols();
    REQUIRE(l.size() == 3);
    CHECK(l[0] == PROTOCOL_LED_PANEL_MATRIX);
    for (auto const& p : l) {
        CHECK(IsLEDPanelMatrixProtocol(p));
    }
}

TEST_CASE("IsLOREnhancedProtocol ignores case", "[misc][vendors][pixels]") {
    CHECK(IsLOREnhancedProtocol("LOR Enhanced"));
    CHECK(IsLOREnhancedProtocol("lor enhanced"));
    CHECK_FALSE(IsLOREnhancedProtocol("LOR"));
    CHECK_FALSE(IsLOREnhancedProtocol(""));
}

TEST_CASE("LOR enhanced channel maths maps channels to units and pixels", "[misc][vendors][pixels]") {
    CHECK(LOREnhancedUnitFromChannel(1) == 1);
    CHECK(LOREnhancedUnitFromChannel(510) == 1);
    CHECK(LOREnhancedUnitFromChannel(511) == 2);
    CHECK(LOREnhancedUnitFromChannel(LOR_ENHANCED_MAX_CHANNELS) == LOR_ENHANCED_MAX_UNIT);
    CHECK(LOREnhancedUnitFromChannel(LOR_ENHANCED_MAX_CHANNELS + 1) == LOR_ENHANCED_MAX_UNIT + 1);
    CHECK(LOREnhancedPixelFromChannel(1) == 1);
    CHECK(LOREnhancedPixelFromChannel(3) == 1);
    CHECK(LOREnhancedPixelFromChannel(4) == 2);
    CHECK(LOREnhancedPixelFromChannel(510) == LOR_ENHANCED_PIXELS_PER_UNIT);
    CHECK(LOREnhancedPixelFromChannel(511) == 1);
}

TEST_CASE("LOR enhanced channel maths clamps nonsense channels", "[misc][vendors][pixels]") {
    CHECK(LOREnhancedUnitFromChannel(0) == 1);
    CHECK(LOREnhancedUnitFromChannel(-100) == 1);
    CHECK(LOREnhancedPixelFromChannel(0) == 1);
    CHECK(LOREnhancedPixelFromChannel(-7) == 1);
}

TEST_CASE("LOREnhancedChannel inverts unit and pixel", "[misc][vendors][pixels]") {
    for (int unit : { 1, 2, 17, 240 }) {
        for (int pixel : { 1, 2, 85, 170 }) {
            int ch = LOREnhancedChannel(unit, pixel);
            CHECK(LOREnhancedUnitFromChannel(ch) == unit);
            CHECK(LOREnhancedPixelFromChannel(ch) == pixel);
        }
    }
    CHECK(LOREnhancedChannel(1, 1) == 1);
    CHECK(LOREnhancedChannel(2, 1) == LOR_ENHANCED_CHANNELS_PER_UNIT + 1);
}

TEST_CASE("GetChannelsPerPixel by chip family", "[misc][vendors][pixels]") {
    CHECK(GetChannelsPerPixel("ws2811") == 3);
    CHECK(GetChannelsPerPixel("apa102") == 3);
    CHECK(GetChannelsPerPixel("") == 3);
    CHECK(GetChannelsPerPixel("unknown") == 3);
    for (auto p : { "apa109", "sk6812rgbw", "sk6818", "sm16704", "sm16804", "ucs2904", "ws2814", "dmx512p-4", "my9291", "tm1814", "tm1814a", "ucs8904", "ucs8904 (16)" }) {
        INFO(p);
        CHECK(GetChannelsPerPixel(p) == 4);
    }
    CHECK(GetChannelsPerPixel("sm16825") == 5);
    CHECK(GetChannelsPerPixel("sm16825 (16)") == 5);
}

// IsPixelProtocol and the protocol tables are lower case and case insensitive; this lookup is not.
TEST_CASE("GetChannelsPerPixel is case insensitive", "[misc][vendors][pixels][!mayfail]") {
    CHECK(GetChannelsPerPixel("TM1814") == 4);
    CHECK(GetChannelsPerPixel("SM16825") == 5);
}

TEST_CASE("IsArtificialPixelType lists only synthetic group names", "[misc][vendors][pixels]") {
    for (auto p : { "tm18xx", "ws281x", "ws2811v1.4", "ws2811 slow", "gs820x", "rgb+2", "dmx512p", "dmx512p-4", "rgbw+2", "gs8206/8", "sm16825 (16)" }) {
        CHECK(IsArtificialPixelType(p));
    }
    CHECK_FALSE(IsArtificialPixelType("ws2811"));
    CHECK_FALSE(IsArtificialPixelType("WS281X"));
    CHECK_FALSE(IsArtificialPixelType(""));
}

TEST_CASE("GetAllPixelTypes filters artificial, serial and matrices on request", "[misc][vendors][pixels]") {
    auto all = GetAllPixelTypes(true, true, true);
    auto real = GetAllPixelTypes(false, false, false);
    CHECK(Has(all, "ws2811"));
    CHECK(Has(all, "tm18xx"));
    CHECK(Has(all, "dmx"));
    CHECK(Has(all, "Virtual Matrix"));
    CHECK(Has(all, "PWM"));
    CHECK(Has(real, "ws2811"));
    CHECK_FALSE(Has(real, "tm18xx"));
    CHECK_FALSE(Has(real, "dmx"));
    CHECK_FALSE(Has(real, "Virtual Matrix"));
    CHECK(Has(real, "PWM"));
    CHECK(real.size() < all.size());
    for (auto const& p : real) {
        CHECK_FALSE(IsArtificialPixelType(p));
    }
}

TEST_CASE("GetAllPixelTypes default arguments exclude serial but include matrices", "[misc][vendors][pixels]") {
    auto d = GetAllPixelTypes();
    CHECK_FALSE(Has(d, "renard"));
    CHECK(Has(d, PROTOCOL_LED_PANEL_MATRIX));
    CHECK(Has(d, "tm18xx"));
}

TEST_CASE("GetAllPixelTypes restricted to a controller keeps only known protocols", "[misc][vendors][pixels]") {
    auto r = GetAllPixelTypes({ "ws2811", "WS2812", "bogus", "apa102", "" }, false, true, true);
    CHECK(Has(r, "ws2811"));
    CHECK(Has(r, "WS2812"));
    CHECK(Has(r, "apa102"));
    CHECK_FALSE(Has(r, "bogus"));
    CHECK(GetAllPixelTypes({}, false, true, true).empty());
}

TEST_CASE("GetAllSerialTypes lists every serial protocol", "[misc][vendors][pixels]") {
    auto s = GetAllSerialTypes();
    for (auto p : { "dmx", "dmx512", "dmx-open", "opendmx", "dmx-pro", "lor", "lor enhanced", "renard", "genericserial", "pixelnet", "pixelnet-lynx", "pixelnet-open" }) {
        CHECK(Has(s, p));
    }
    CHECK_FALSE(Has(s, "ws2811"));
    auto f = GetAllSerialTypes({ "dmx", "Renard", "nope", "lor" });
    CHECK(Has(f, "dmx"));
    CHECK(Has(f, "Renard"));
    CHECK(Has(f, "lor"));
    CHECK_FALSE(Has(f, "nope"));
}

TEST_CASE("ChooseBestControllerSerial prefers exact then equivalent names", "[misc][vendors][pixels]") {
    CHECK(ChooseBestControllerSerial({ "dmx", "renard" }, "renard") == "renard");
    CHECK(ChooseBestControllerSerial({ "dmx512" }, "dmx") == "dmx512");
    CHECK(ChooseBestControllerSerial({ "opendmx" }, "dmx-open") == "opendmx");
    CHECK(ChooseBestControllerSerial({ "renard" }, "dmx") == "");
    CHECK(ChooseBestControllerSerial({}, "dmx") == "");
    CHECK(ChooseBestControllerSerial({ "dmx" }, "DMX") == "dmx");
}

TEST_CASE("ChooseBestControllerPixel prefers exact then group equivalents", "[misc][vendors][pixels]") {
    CHECK(ChooseBestControllerPixel({ "ws2811", "apa102" }, "apa102") == "apa102");
    CHECK(ChooseBestControllerPixel({ "ws2811" }, "ws2812") == "ws2811");
    CHECK(ChooseBestControllerPixel({ "ws2811" }, "tm1804") == "ws2811");
    CHECK(ChooseBestControllerPixel({ "apa102" }, "sk9822") == "apa102");
    CHECK(ChooseBestControllerPixel({ "ws2801" }, "ws2803") == "ws2801");
    CHECK(ChooseBestControllerPixel({ "ws2801" }, "apa102") == "");
    CHECK(ChooseBestControllerPixel({}, "ws2811") == "");
    CHECK(ChooseBestControllerPixel({ "ws2811" }, "unknownchip") == "");
}

TEST_CASE("ChooseBestControllerPixel exact upper case match is returned as is", "[misc][vendors][pixels]") {
    CHECK(ChooseBestControllerPixel({ "WS2811" }, "WS2811") == "WS2811");
}

// The lower case retry in ChooseBestControllerPixel calls ChooseBestControllerSerial, so an upper case
// name that needs the group lookup finds nothing.
TEST_CASE("ChooseBestControllerPixel retries a mixed case name against the pixel groups", "[misc][vendors][pixels][!mayfail]") {
    CHECK(ChooseBestControllerPixel({ "ws2811" }, "WS2812") == "ws2811");
}

TEST_CASE("UDController IsError looks for the ERR prefix anywhere", "[misc][vendors][uploaddata]") {
    CHECK(UDController::IsError("ERR: something"));
    CHECK(UDController::IsError("WARN: a\nERR: b\n"));
    CHECK_FALSE(UDController::IsError("WARN: only a warning"));
    CHECK_FALSE(UDController::IsError(""));
    CHECK_FALSE(UDController::IsError("err: lower case"));
}

TEST_CASE("UDControllerPort starts valid and empty", "[misc][vendors][uploaddata]") {
    UDControllerPort p("Pixel", 3, "ws2811");
    CHECK(p.GetPort() == 3);
    CHECK(p.GetType() == "Pixel");
    CHECK(p.GetProtocol() == "ws2811");
    CHECK(p.IsPixelProtocol());
    CHECK(p.IsValid());
    CHECK(p.GetInvalidReason().empty());
    CHECK(p.GetModelCount() == 0);
    CHECK(p.GetVirtualStringCount() == 0);
    CHECK(p.GetRealVirtualStringCount() == 0);
    CHECK(p.GetFirstModel() == nullptr);
    CHECK(p.GetLastModel() == nullptr);
    CHECK(p.GetVirtualString(0) == nullptr);
    CHECK(p.GetVirtualString(-1) == nullptr);
    CHECK(p.GetStartChannel() == -1);
    CHECK(p.GetEndChannel() == -1);
    CHECK(p.GetUniverse() == -1);
    CHECK(p.GetUniverseStartChannel() == -1);
    CHECK(p.Channels() == 0);
    CHECK(p.Pixels() == 0);
    CHECK(p.GetPortName().empty());
    CHECK(p.GetAmps(100) == 0.0f);
    CHECK(p.GetSmartRemoteCount() == 0);
    CHECK_FALSE(p.AtLeastOneModelIsUsingSmartRemote());
    CHECK_FALSE(p.AtLeastOneModelIsNotUsingSmartRemote());
    CHECK(p.AllSmartRemoteTypesSame());
    CHECK(p.GetSmartRemoteType(1).empty());
    CHECK_FALSE(p.IsSmartRemotePort());
    p.TagSmartRemotePort();
    CHECK(p.IsSmartRemotePort());
    p.SetInvalid();
    CHECK_FALSE(p.IsValid());
}

TEST_CASE("UDControllerPort serial port with no models reports unset values", "[misc][vendors][uploaddata]") {
    UDControllerPort p("Serial", 1, "dmx");
    CHECK_FALSE(p.IsPixelProtocol());
    CHECK(p.GetUniverse() == -1);
    CHECK(p.GetStartChannel() == -1);
    CHECK(p.Channels() == 0);
}

TEST_CASE("UDControllerPort empty virtual string handling", "[misc][vendors][uploaddata]") {
    UDControllerPort p("Pixel", 1, "ws2811");
    p.CreateVirtualStrings(true);
    p.CreateVirtualStrings(false, false);
    CHECK(p.GetVirtualStringCount() == 0);
    CHECK(p.EffectiveChannels("Falcon") == 0);
    CHECK(p.GetModelCount(1) == 0);
    p.SetSeparateUniverses(true);
    p.SetPackedStrings(true);
    CHECK(p.Channels() == 0);
    CHECK(p.ClearSmartRemoteOnAllModels() == false);
    CHECK(p.EnsureAllModelsAreChained() == false);
    CHECK(p.SetAllModelsToControllerName("x") == false);
    CHECK(p.SetAllModelsToValidProtocols({ "ws2811" }, "") == false);
}

TEST_CASE("UDControllerPort JSON export of an empty port", "[misc][vendors][uploaddata]") {
    UDControllerPort p("Pixel", 3, "ws2811");
    CHECK(p.ExportAsJSON() == "{\"port\":3,\"startchannel\":-1,\"universe\":-1,\"universestartchannel\":-1,\"channels\":0,\"models\":[]}");
    UDControllerPort s("Serial", 12, "dmx");
    CHECK(s.ExportAsJSON().rfind("{\"port\":12,", 0) == 0);
}

TEST_CASE("UDControllerPort CSV export of an empty port is just the port label", "[misc][vendors][uploaddata]") {
    UDControllerPort p("Pixel", 2, "ws2811");
    auto none = p.ExportAsCSV(ExportSettings::SETTINGS_NONE, 100.0f);
    REQUIRE(none.size() == 1);
    CHECK(none[0] == "Pixel Port 2");

    ExportSettings::SETTINGS all = ExportSettings::SETTINGS_PORT_ABSADDRESS;
    all |= ExportSettings::SETTINGS_PORT_UNIADDRESS;
    all |= ExportSettings::SETTINGS_PORT_CHANNELS;
    all |= ExportSettings::SETTINGS_PORT_PIXELS;
    auto some = p.ExportAsCSV(all, 100.0f);
    REQUIRE(some.size() == 1);
    CHECK(some[0] == "Pixel Port 2");

    auto cur = p.ExportAsCSV(ExportSettings::SETTINGS_PORT_CURRENT, 100.0f);
    REQUIRE(cur.size() == 1);
    CHECK(cur[0] == "Pixel Port 2(CUR:0.00A)");
    UDControllerPort s("Serial", 1, "dmx");
    CHECK(s.ExportAsCSV(ExportSettings::SETTINGS_PORT_CURRENT, 100.0f)[0] == "Serial Port 1");
}

TEST_CASE("ExportSettings flags combine with bit or", "[misc][vendors][uploaddata]") {
    using namespace ExportSettings;
    SETTINGS s = SETTINGS_NONE;
    s |= SETTINGS_PORT_PIXELS;
    s |= SETTINGS_MODEL_CURRENT;
    CHECK((s & SETTINGS_PORT_PIXELS) != 0);
    CHECK((s & SETTINGS_MODEL_CURRENT) != 0);
    CHECK((s & SETTINGS_PORT_CHANNELS) == 0);
    CHECK((SETTINGS_PORT_ABSADDRESS | SETTINGS_PORT_UNIADDRESS) == 3);
}

TEST_CASE("UDControllerPort Check validates the protocol against the controller", "[misc][vendors][uploaddata]") {
    CapsFixture f;
    auto* rules = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Standard");
    REQUIRE(rules != nullptr);

    UDControllerPort good("Pixel", 1, "ws2811");
    std::string res;
    CHECK(good.Check(nullptr, true, rules, res));
    CHECK(res.empty());

    UDControllerPort bad("Pixel", 2, "tm1814");
    std::string res2;
    CHECK_FALSE(bad.Check(nullptr, true, rules, res2));
    CHECK(res2.find("ERR: Invalid protocol on pixel port 2: tm1814") != std::string::npos);
    CHECK(UDController::IsError(res2));
}

TEST_CASE("UDControllerPort Check validates serial protocols", "[misc][vendors][uploaddata]") {
    CapsFixture f;
    auto* rules = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Standard");
    REQUIRE(rules != nullptr);

    UDControllerPort good("Serial", 1, "dmx");
    std::string res;
    CHECK(good.Check(nullptr, false, rules, res));

    UDControllerPort bad("Serial", 1, "renard");
    std::string res2;
    CHECK_FALSE(bad.Check(nullptr, false, rules, res2));
    CHECK(res2.find("ERR: Invalid protocol on serial port 1: renard") != std::string::npos);
}

TEST_CASE("Every listed pixel type is accepted as a pixel protocol", "[misc][vendors][pixels]") {
    for (auto const& p : GetAllPixelTypes(false, true, false)) {
        INFO(p);
        if (p.empty() || p == "PWM") {
            continue;
        }
        CHECK(IsPixelProtocol(p));
    }
}
