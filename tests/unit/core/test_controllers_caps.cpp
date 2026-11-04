#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "controllers/ControllerCaps.h"
#include "outputs/Controller.h"
#include "support/TempDir.h"
#include "utils/FileUtils.h"

namespace {

const char* kFixture = R"(<Vendor Name="AcmeLights">
    <AbstractVariant Name="Base">
        <SupportsUpload/>
        <SupportsAutoLayout/>
        <MaxInputUniverses>96</MaxInputUniverses>
        <MaxSerialPortChannels>512</MaxSerialPortChannels>
        <SerialProtocols>
            <Protocol>dmx</Protocol>
            <Protocol>Renard</Protocol>
        </SerialProtocols>
        <PixelProtocols>
            <Protocol>WS2811</Protocol>
            <Protocol>apa102</Protocol>
        </PixelProtocols>
    </AbstractVariant>
    <AbstractVariant Name="V2" Base="AcmeLights:Base">
        <MaxPixelPortChannels>2040</MaxPixelPortChannels>
        <InputProtocols>
            <Protocol>e131</Protocol>
            <Protocol>artnet</Protocol>
        </InputProtocols>
        <FPS40Pixels>680</FPS40Pixels>
    </AbstractVariant>
    <Controller Name="Pix4">
        <Variant Name="Standard" ID="acme-pix4" Base="AcmeLights:V2">
            <MaxPixelPort>4</MaxPixelPort>
            <MaxSerialPort>1</MaxSerialPort>
            <SupportsSmartRemotes>3</SupportsSmartRemotes>
            <SmartRemoteTypes><Type>a</Type><Type>b</Type></SmartRemoteTypes>
            <AltNames><AltName>OldPix4</AltName></AltNames>
        </Variant>
        <Variant Name="Expanded" ID="acme-pix4x" Base="AcmeLights:V2">
            <MaxPixelPort>12</MaxPixelPort>
            <MaxSerialPort>2</MaxSerialPort>
            <MaxPixelPortChannels>3000</MaxPixelPortChannels>
            <AltVariantNames><AltVariantName>Big</AltVariantName></AltVariantNames>
            <SupportsExtraSerialPorts/>
            <PixelProtocolGroups><Group>ws2811,apa102</Group><Group>tm1814</Group></PixelProtocolGroups>
            <PixelProtocols><Protocol>ws2811</Protocol><Protocol>apa102</Protocol><Protocol>tm1814</Protocol></PixelProtocols>
            <fppCapeVersions>2,3</fppCapeVersions>
        </Variant>
    </Controller>
    <Controller Name="Single">
        <Variant Name="" Base="AcmeLights:Base">
            <MaxPixelPort>1</MaxPixelPort>
            <InputProtocols><Protocol>dmx</Protocol></InputProtocols>
            <PreferredInputProtocol>dmx</PreferredInputProtocol>
            <PreferredState>Inactive</PreferredState>
            <DisableMonitoring/>
            <ExtraProperties>
                <Property name="mode" label="Mode"><Type>Enum</Type><Default>a</Default><Values><Value>a</Value><Value>b</Value></Values></Property>
                <Property name="note" label="Note"><Default>hi</Default></Property>
            </ExtraProperties>
        </Variant>
    </Controller>
</Vendor>)";

struct CapsFixture {
    TempDir dir;
    std::string prevResources;

    CapsFixture() {
        prevResources = FileUtils::GetResourcesDir();
        std::filesystem::create_directories(dir.path / "controllers");
        dir.Write("controllers/acme.xcontroller", kFixture);
        ControllerCaps::UnloadControllers();
        FileUtils::SetResourcesDir(dir.path.string());
        ControllerCaps::LoadControllers();
    }
    ~CapsFixture() {
        ControllerCaps::UnloadControllers();
        FileUtils::SetResourcesDir(prevResources);
    }
};

bool Has(const std::list<std::string>& l, const std::string& v) {
    return std::find(l.begin(), l.end(), v) != l.end();
}

} // namespace

TEST_CASE("ControllerCaps lookup by vendor model and variant", "[controllers]") {
    CapsFixture f;
    auto* c = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Standard");
    REQUIRE(c != nullptr);
    CHECK(c->GetVendor() == "AcmeLights");
    CHECK(c->GetModel() == "Pix4");
    CHECK(c->GetVariantName() == "Standard");
    CHECK(c->GetID() == "acme-pix4");

    auto* x = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Expanded");
    REQUIRE(x != nullptr);
    CHECK(x != c);
    CHECK(x->GetID() == "acme-pix4x");

    CHECK(ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Nope") == nullptr);
    CHECK(ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "") == nullptr);
    CHECK(ControllerCaps::GetControllerConfig("AcmeLights", "Missing", "Standard") == nullptr);
    CHECK(ControllerCaps::GetControllerConfig("Nobody", "Pix4", "Standard") == nullptr);
}

TEST_CASE("ControllerCaps single variant models match an empty variant", "[controllers]") {
    CapsFixture f;
    auto* c = ControllerCaps::GetControllerConfig("AcmeLights", "Single", "");
    REQUIRE(c != nullptr);
    CHECK(c->GetVariantName().empty());
    CHECK(c->GetPreferredInputProtocol() == "dmx");
    CHECK(c->GetPreferredState() == "Inactive");
    CHECK(c->DisableMonitoring());
}

TEST_CASE("ControllerCaps inherits settings from abstract variants", "[controllers]") {
    CapsFixture f;
    auto* c = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Standard");
    REQUIRE(c != nullptr);
    CHECK(c->SupportsUpload());
    CHECK(c->SupportsAutoLayout());
    CHECK_FALSE(c->SupportsAutoUpload());
    CHECK_FALSE(c->SupportsFullxLightsControl());
    CHECK(c->GetMaxInputE131Universes() == 96);
    CHECK(c->GetMaxSerialPortChannels() == 512);
    CHECK(c->GetMaxPixelPortChannels() == 2040);
    CHECK(c->GetMaxPixelPort() == 4);
    CHECK(c->GetMaxSerialPort() == 1);
    CHECK(c->GetMaxPixelsAt40FPS() == 680);
    CHECK(c->GetMaxPixelsAt40FPS_SR() == 680);
}

TEST_CASE("ControllerCaps variant values override the abstract base", "[controllers]") {
    CapsFixture f;
    auto* x = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Expanded");
    REQUIRE(x != nullptr);
    CHECK(x->GetMaxPixelPortChannels() == 3000);
    CHECK(x->GetMaxPixelPort() == 12);
}

TEST_CASE("ControllerCaps defaults for absent settings", "[controllers]") {
    CapsFixture f;
    auto* c = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Standard");
    REQUIRE(c != nullptr);
    CHECK(c->GetMaxDDPChannels() == 0);
    CHECK(c->GetMaxInputUniverseChannels() == 512);
    CHECK(c->GetMinInputUniverseChannels() == 1);
    CHECK(c->GetMaxPacing() == 0);
    CHECK(c->GetNumberOfBanks() == 1);
    CHECK(c->GetBankSize() == 16);
    CHECK(c->GetMaxStartNullPixels() == -1);
    CHECK(c->GetMaxEndNullPixels() == -1);
    CHECK(c->GetMaxGroupPixels() == -1);
    CHECK(c->GetMinGroupPixels() == -1);
    CHECK(c->GetMaxZigZagPixels() == -1);
    CHECK(c->GetMaxPWMPort() == 0);
    CHECK(c->GetMaxVirtualMatrixPort() == 0);
    CHECK(c->GetMaxLEDPanelMatrixPort() == 0);
    CHECK_FALSE(c->SupportsLEDPanelMatrix());
    CHECK_FALSE(c->SupportsVirtualMatrix());
    CHECK_FALSE(c->IsPlayerOnly());
    CHECK_FALSE(c->NoWebUI());
}

TEST_CASE("ControllerCaps protocol lists and validators", "[controllers]") {
    CapsFixture f;
    auto* c = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Standard");
    REQUIRE(c != nullptr);
    CHECK(c->GetInputProtocols() == std::vector<std::string>{ "e131", "artnet" });
    CHECK(c->GetPixelProtocols() == std::vector<std::string>{ "WS2811", "apa102" });
    CHECK(c->GetSerialProtocols() == std::vector<std::string>{ "dmx", "Renard" });
    CHECK(c->GetAllProtocols() == std::vector<std::string>{ "WS2811", "apa102", "dmx", "Renard" });

    CHECK(c->IsValidPixelProtocol("ws2811"));
    CHECK(c->IsValidPixelProtocol("WS2811"));
    CHECK_FALSE(c->IsValidPixelProtocol("tm1814"));
    CHECK(c->IsValidSerialProtocol("renard"));
    CHECK_FALSE(c->IsValidSerialProtocol("lor"));
    CHECK(c->IsValidInputProtocol("E131"));
    CHECK_FALSE(c->IsValidInputProtocol("ddp"));
    CHECK(c->SupportsEthernetInputProtols());
    CHECK_FALSE(c->SupportsSerialInputProtols());
    CHECK_FALSE(c->IsSerialController());
}

TEST_CASE("ControllerCaps serial only controller", "[controllers]") {
    CapsFixture f;
    auto* c = ControllerCaps::GetControllerConfig("AcmeLights", "Single", "");
    REQUIRE(c != nullptr);
    CHECK(c->SupportsSerialInputProtols());
    CHECK_FALSE(c->SupportsEthernetInputProtols());
    CHECK(c->IsSerialController());
}

TEST_CASE("ControllerCaps smart remote information", "[controllers]") {
    CapsFixture f;
    auto* c = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Standard");
    REQUIRE(c != nullptr);
    CHECK(c->SupportsSmartRemotes());
    CHECK(c->GetSmartRemoteCount() == 3);
    CHECK(c->GetSmartRemoteTypes() == std::vector<std::string>{ "a", "b" });

    auto* x = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Expanded");
    REQUIRE(x != nullptr);
    CHECK_FALSE(x->SupportsSmartRemotes());
    CHECK(x->GetSmartRemoteTypes() == std::vector<std::string>{ "" });
}

TEST_CASE("ControllerCaps pixel protocol groups restrict mixing", "[controllers]") {
    CapsFixture f;
    auto* x = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Expanded");
    REQUIRE(x != nullptr);
    CHECK(x->GetPixelProtocolGroups().size() == 2);
    CHECK(x->ArePixelProtocolsCompatible("ws2811", "APA102"));
    CHECK_FALSE(x->ArePixelProtocolsCompatible("ws2811", "tm1814"));
    CHECK(x->ArePixelProtocolsCompatible("ws2811", "ws2811"));
    CHECK(x->ArePixelProtocolsCompatible("", "tm1814"));
    CHECK(x->ArePixelProtocolsCompatible("ws2811", "unknown"));

    auto* c = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Standard");
    REQUIRE(c != nullptr);
    CHECK(c->GetPixelProtocolGroups().empty());
    CHECK(c->ArePixelProtocolsCompatible("ws2811", "apa102"));
}

TEST_CASE("ControllerCaps alternative names", "[controllers]") {
    CapsFixture f;
    auto* byOld = ControllerCaps::GetControllerConfigByAlternateName("AcmeLights", "OldPix4", "Standard");
    REQUIRE(byOld != nullptr);
    CHECK(byOld->GetModel() == "Pix4");
    CHECK(byOld->GetVariantName() == "Standard");
    CHECK(ControllerCaps::GetControllerConfigByAlternateName("AcmeLights", "OldPix4", "Other") == nullptr);
    CHECK(ControllerCaps::GetControllerConfigByAlternateName("Nobody", "OldPix4", "Standard") == nullptr);

    auto* becameVariant = ControllerCaps::GetControllerConfigByAlternateName("AcmeLights", "Expanded", "");
    REQUIRE(becameVariant != nullptr);
    CHECK(becameVariant->GetVariantName() == "Expanded");
}

TEST_CASE("ControllerCaps renamed variants still resolve", "[controllers]") {
    CapsFixture f;
    auto* c = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Big");
    REQUIRE(c != nullptr);
    CHECK(c->GetVariantName() == "Expanded");
    CHECK(c->GetAlternativeVariantNames() == std::vector<std::string>{ "Big" });
}

TEST_CASE("ControllerCaps lookups by id model and vendor", "[controllers]") {
    CapsFixture f;
    auto* byId = ControllerCaps::GetControllerConfigByID("acme-pix4x");
    REQUIRE(byId != nullptr);
    CHECK(byId->GetVariantName() == "Expanded");
    CHECK(ControllerCaps::GetControllerConfigByID("") == nullptr);
    CHECK(ControllerCaps::GetControllerConfigByID("missing") == nullptr);

    auto* byModel = ControllerCaps::GetControllerConfigByModel("Pix4", "Standard");
    REQUIRE(byModel != nullptr);
    CHECK(byModel->GetVendor() == "AcmeLights");
    CHECK(ControllerCaps::GetControllerConfigByModel("Nope", "Standard") == nullptr);

    CHECK(ControllerCaps::GetControllerConfigByVendor("AcmeLights") != nullptr);
    CHECK(ControllerCaps::GetControllerConfigByVendor("Nobody") == nullptr);
}

TEST_CASE("ControllerCaps FPP cape version matching", "[controllers]") {
    CapsFixture f;
    auto* x = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Expanded");
    REQUIRE(x != nullptr);
    CHECK(x->MatchesFPPCapeVersion("2.0"));
    CHECK(x->MatchesFPPCapeVersion("3.5.1"));
    CHECK_FALSE(x->MatchesFPPCapeVersion("4.0"));
    CHECK_FALSE(x->MatchesFPPCapeVersion(""));
    auto* c = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Standard");
    CHECK_FALSE(c->MatchesFPPCapeVersion("2.0"));

    CHECK(ControllerCaps::GetControllerConfigByIDAndCapeVersion("acme-pix4x", "3.0") == x);
    CHECK(ControllerCaps::GetControllerConfigByIDAndCapeVersion("acme-pix4x", "9.0") == nullptr);
    CHECK(ControllerCaps::GetControllerConfigByIDAndCapeVersion("", "3.0") == nullptr);
}

TEST_CASE("ControllerCaps extra serial ports", "[controllers]") {
    CapsFixture f;
    auto* x = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Expanded");
    REQUIRE(x != nullptr);
    CHECK(x->SupportsExtraSerialPorts());
    CHECK(x->IsValidSerialPort(1));
    CHECK(x->IsValidSerialPort(2));
    CHECK_FALSE(x->IsValidSerialPort(3));
    CHECK_FALSE(x->IsValidSerialPort(0));
    CHECK(x->IsValidSerialPort(ControllerCaps::FIRST_EXTRA_SERIAL_PORT));
    CHECK(x->IsExtraSerialPort(ControllerCaps::FIRST_EXTRA_SERIAL_PORT + 11));
    CHECK_FALSE(x->IsExtraSerialPort(ControllerCaps::FIRST_EXTRA_SERIAL_PORT + 12));
    CHECK(x->GetExtraSerialPorts().size() == 12);
    CHECK(x->GetSerialPortDevice(ControllerCaps::FIRST_EXTRA_SERIAL_PORT) == "ttyUSB0");
    CHECK(x->GetSerialPortDevice(ControllerCaps::FIRST_EXTRA_SERIAL_PORT + 8) == "ttyACM0");
    CHECK(x->GetSerialPortLabel(ControllerCaps::FIRST_EXTRA_SERIAL_PORT + 1) == "ttyUSB1");
    CHECK(x->GetSerialPortLabel(1) == "Serial Port 1");
    CHECK(x->GetMaxSerialPortChannels(ControllerCaps::FIRST_EXTRA_SERIAL_PORT) == 512);
    CHECK(x->GetMaxSerialPortChannels(1) == 512);

    auto* c = ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Standard");
    CHECK_FALSE(c->SupportsExtraSerialPorts());
    CHECK(c->GetExtraSerialPorts().empty());
    CHECK_FALSE(c->IsValidSerialPort(ControllerCaps::FIRST_EXTRA_SERIAL_PORT));
}

TEST_CASE("ControllerCaps extra property definitions", "[controllers]") {
    CapsFixture f;
    auto* c = ControllerCaps::GetControllerConfig("AcmeLights", "Single", "");
    REQUIRE(c != nullptr);
    auto defs = c->GetExtraPropertyDefs();
    REQUIRE(defs.size() == 2);
    CHECK(defs[0].name == "mode");
    CHECK(defs[0].label == "Mode");
    CHECK(defs[0].type == "Enum");
    CHECK(defs[0].defaultValue == "a");
    CHECK(defs[0].values == std::vector<std::string>{ "a", "b" });
    CHECK(defs[1].name == "note");
    CHECK(defs[1].type == "String");
    CHECK(defs[1].defaultValue == "hi");
    CHECK(defs[1].values.empty());
}

TEST_CASE("ControllerCaps vendor model and variant listings", "[controllers]") {
    CapsFixture f;
    auto vendors = ControllerCaps::GetVendors(CONTROLLER_ETHERNET);
    CHECK(Has(vendors, ""));
    CHECK(Has(vendors, "AcmeLights"));

    auto models = ControllerCaps::GetModels(CONTROLLER_ETHERNET, "AcmeLights");
    CHECK(Has(models, "Pix4"));
    CHECK_FALSE(Has(models, "Single"));
    auto serialModels = ControllerCaps::GetModels(CONTROLLER_SERIAL, "AcmeLights");
    CHECK(Has(serialModels, "Single"));
    CHECK_FALSE(Has(serialModels, "Pix4"));

    auto variants = ControllerCaps::GetVariants(CONTROLLER_ETHERNET, "AcmeLights", "Pix4");
    CHECK(variants == std::list<std::string>{ "Standard", "Expanded" });
    CHECK(ControllerCaps::GetVariants(CONTROLLER_ETHERNET, "AcmeLights", "Missing") == std::list<std::string>{ "" });
}

TEST_CASE("ControllerCaps with no controllers folder finds nothing", "[controllers]") {
    TempDir empty;
    const std::string prev = FileUtils::GetResourcesDir();
    ControllerCaps::UnloadControllers();
    FileUtils::SetResourcesDir(empty.path.string());
    CHECK(ControllerCaps::GetControllerConfig("AcmeLights", "Pix4", "Standard") == nullptr);
    CHECK(ControllerCaps::GetControllerConfigByID("acme-pix4") == nullptr);
    CHECK(ControllerCaps::GetControllerConfig(nullptr) == nullptr);
    CHECK(ControllerCaps::GetVendors(CONTROLLER_ETHERNET) == std::list<std::string>{ "" });
    ControllerCaps::UnloadControllers();
    FileUtils::SetResourcesDir(prev);
}
