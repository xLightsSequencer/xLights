// UDController / UDControllerPort / UDControllerPortModel built from real models on a
// controller: port assignment, start channels, virtual strings, smart remotes and the
// Check() rules every vendor upload runs first.
#ifndef _WIN32

#include "test_fpp3_common.h"

#include <catch2/matchers/catch_matchers_string.hpp>

#include "controllers/ExportSettings.h"

using namespace fpp3;
using Catch::Matchers::ContainsSubstring;

namespace {

// Limits for Check(): every rule the generic checks know about, spread over variants.
const char* kRuleCaps = R"(<Vendor Name="Rules">
    <AbstractVariant Name="Base">
        <SupportsUpload/>
        <InputProtocols><Protocol>e131</Protocol><Protocol>ddp</Protocol></InputProtocols>
        <PixelProtocols><Protocol>ws2811</Protocol><Protocol>apa102</Protocol></PixelProtocols>
        <SerialProtocols><Protocol>dmx</Protocol><Protocol>LOR Enhanced</Protocol></SerialProtocols>
        <MaxPixelPortChannels>300</MaxPixelPortChannels>
        <MaxSerialPortChannels>100</MaxSerialPortChannels>
    </AbstractVariant>
    <Controller Name="Strict">
        <Variant Name="" ID="Strict" Base="Rules:Base">
            <MaxPixelPort>4</MaxPixelPort>
            <MaxSerialPort>1</MaxSerialPort>
            <MaxInputUniverseChannels>400</MaxInputUniverseChannels>
            <MinInputUniverseChannels>100</MinInputUniverseChannels>
            <AllInputUniversesMustBeSameSize/>
            <AllInputUniversesMustBe510/>
            <UniversesMustBeInNumericalOrder/>
            <UniversesMustBeSequential/>
            <MaxStartNulls>2</MaxStartNulls>
            <MaxEndNulls>1</MaxEndNulls>
            <MaxGroup>3</MaxGroup>
            <MinGroup>2</MinGroup>
            <MaxZigZag>4</MaxZigZag>
        </Variant>
    </Controller>
    <Controller Name="Smart">
        <Variant Name="" ID="Smart" Base="Rules:Base">
            <MaxPixelPort>8</MaxPixelPort>
            <MaxSerialPort>0</MaxSerialPort>
            <SupportsVirtualStrings/>
            <SupportsSmartRemotes>3</SupportsSmartRemotes>
            <AllSmartRemoteTypesPerPortMustBeSame/>
            <SupportsMultipleSimultaneousInputProtocols/>
            <SmartRemoteTypes><Type>falcon_v1</Type><Type>falcon_v2</Type></SmartRemoteTypes>
        </Variant>
    </Controller>
    <Controller Name="Banked">
        <Variant Name="" ID="Banked" Base="Rules:Base">
            <MaxPixelPort>4</MaxPixelPort>
            <MaxSerialPort>0</MaxSerialPort>
            <NumberOfBanks>2</NumberOfBanks>
            <BankSize>2</BankSize>
            <SupportsUniversePerString/>
            <SupportsVirtualStrings/>
            <SupportsPixelPortGrouping/>
            <MergeConsecutiveVirtualStrings/>
        </Variant>
    </Controller>
    <Controller Name="Grouped">
        <Variant Name="" ID="Grouped" Base="Rules:Base">
            <MaxPixelPort>4</MaxPixelPort>
            <MaxSerialPort>0</MaxSerialPort>
            <SupportsMultipleSimultaneousOutputProtocols/>
            <PixelProtocolGroups><Group>ws2811</Group><Group>apa102</Group></PixelProtocolGroups>
        </Variant>
    </Controller>
    <Controller Name="Bare">
        <Variant Name="" ID="Bare">
            <MaxPixelPort>0</MaxPixelPort>
            <MaxSerialPort>0</MaxSerialPort>
            <InputProtocols><Protocol>ddp</Protocol></InputProtocols>
        </Variant>
    </Controller>
</Vendor>)";

// A show on a controller using kRuleCaps.
struct RuleShow {
    VendorCapsFixture caps{ kRuleCaps };
    OutputManager om;
    ModelManager mm{ &om };
    ControllerEthernet* ctl = nullptr;

    explicit RuleShow(const std::string& model, const std::string& protocol = OUTPUT_DDP) {
        InitLoggers();
        ctl = new ControllerEthernet(&om, false);
        ctl->SetName("Ctl");
        ctl->SetIP("10.0.0.5");
        ctl->SetProtocol(protocol);
        ctl->SetVendor("Rules");
        ctl->SetModel(model);
        ctl->SetVariant("");
        ctl->SetAutoLayout(false);
        if (protocol == OUTPUT_DDP) {
            ctl->GetOutputs().front()->SetChannels(3000);
        }
        om.AddController(ctl);
        om.SomethingChanged();
    }
    ControllerCaps* Caps() const { return ControllerCaps::GetControllerConfig(ctl); }
    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m != nullptr);
        return m;
    }
    Model* OnPort(const std::string& name, const std::string& protocol, int port, int nodes, const std::string& sc, const std::string& extraCC = "") {
        return Add("<model name=\"" + name + "\" DisplayAs=\"Single Line\" parm1=\"1\" parm2=\"" + std::to_string(nodes) +
                   "\" parm3=\"1\" StringType=\"RGB Nodes\" StartChannel=\"" + sc + "\" Controller=\"Ctl\"><ControllerConnection Protocol=\"" + protocol +
                   "\" Port=\"" + std::to_string(port) + "\" " + extraCC + "/></model>");
    }
    void Recalc() {
        om.SomethingChanged();
        mm.RecalcStartChannels();
    }
};

std::string CheckText(UDController& cud, const ControllerCaps* caps, bool* ok = nullptr) {
    std::string res;
    bool r = cud.Check(caps, res);
    if (ok) *ok = r;
    return res;
}

} // namespace

TEST_CASE("UDController puts models on the ports their connection names", "[fpp3][uploaddata]") {
    FppShow s;
    Model* a = s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    Model* b = s.OnPort("B", "ws2811", 1, 5, "!Ctl:31");
    Model* c = s.OnPort("C", "ws2811", 3, 20, "!Ctl:46");
    s.Recalc();

    UDController cud(s.ctl, &s.om, &s.mm, false);
    CHECK(cud.GetMaxPixelPort() == 3);
    CHECK(cud.HasPixelPort(1));
    CHECK_FALSE(cud.HasPixelPort(2));
    CHECK(cud.HasPixelPort(3));
    CHECK(cud.HasModels());
    CHECK(cud.IsValid());
    CHECK(cud.GetNoConnectionModels().empty());
    CHECK(cud.GetFirstOutput() == s.ctl->GetFirstOutput());

    UDControllerPort* p1 = cud.GetControllerPixelPort(1);
    REQUIRE(p1->GetModelCount() == 2);
    CHECK(p1->GetStartChannel() == 1);
    CHECK(p1->GetEndChannel() == 45);
    CHECK(p1->Channels() == 45);
    CHECK(p1->Pixels() == 15);
    CHECK(p1->GetPortName() == "A");
    CHECK(p1->GetFirstModel()->GetModel() == a);
    CHECK(p1->GetLastModel()->GetModel() == b);
    CHECK(p1->GetModelAfter(a) == b);
    CHECK(p1->GetModelAfter(b) == nullptr);
    CHECK(p1->GetModelBefore(b) == a);
    CHECK(p1->GetModelBefore(a) == nullptr);
    CHECK(p1->GetUniverse() == 1); // the DDP output's id
    CHECK(p1->GetUniverseStartChannel() == 1);
    CHECK(cud.GetModelAfter(a) == b);
    CHECK(cud.GetPortContainingModel(c) == cud.GetControllerPixelPort(3));
    CHECK(cud.GetControllerPortModel("B", 0)->GetStartChannel() == 31);
    CHECK(cud.GetControllerPortModel("nope", 0) == nullptr);
    CHECK(cud.GetMaxPixelPortChannels() == 60);

    auto* pm = p1->GetFirstModel();
    CHECK(pm->GetName() == "A");
    CHECK(pm->GetLabel() == "A");
    CHECK(pm->Channels() == 30);
    CHECK(pm->GetChannelsPerPixel() == 3);
    CHECK(pm->GetLightsPerNode() == 1);
    CHECK(pm->IsFirstModelString());
    CHECK(pm->GetSmartRemote() == 0);
    CHECK(pm->GetSmartRemoteLetter() == ' ');
    CHECK(pm->GetProtocol() == "ws2811");
}

TEST_CASE("UDController ExportAsJSON describes every pixel port", "[fpp3][uploaddata]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1", "", "", 1, "RGB Nodes", "Description=\"the arch\"");
    s.OnPort("B", "ws2811", 2, 4, "!Ctl:31");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    auto j = nlohmann::json::parse(cud.ExportAsJSON());
    REQUIRE(j["pixelports"].size() == 2);
    auto p1 = j["pixelports"][0];
    CHECK(p1["port"] == 1);
    CHECK(p1["startchannel"] == 1);
    CHECK(p1["channels"] == 30);
    CHECK(p1["pixels"] == 10);
    REQUIRE(p1["models"].size() == 1);
    CHECK(p1["models"][0]["name"] == "A");
    CHECK(p1["models"][0]["description"] == "the arch");
    CHECK(p1["models"][0]["pixels"] == 10);
    CHECK(j["pixelports"][1]["startchannel"] == 31);
    CHECK(j["serialports"].empty());
    CHECK(j["virtualmatrixports"].empty());
}

TEST_CASE("UDController ExportAsJSON with a matrix only on port 2 is valid json", "[fpp3]") {
    FppShow s("TestPanels");
    s.Add(R"(<model name="M" DisplayAs="Matrix" parm1="4" parm2="4" parm3="1" StringType="RGB Nodes" StartChannel="!Ctl:1" Controller="Ctl"><ControllerConnection Protocol="Virtual Matrix" Port="2"/></model>)");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    CHECK(nlohmann::json::accept(cud.ExportAsJSON()));
}

TEST_CASE("UDController splits a multi string model over consecutive ports", "[fpp3][uploaddata]") {
    FppShow s;
    Model* m = s.OnPort("Tree", "ws2811", 2, 5, "!Ctl:1", "", "", 3);
    s.Recalc();
    REQUIRE(m->GetNumPhysicalStrings() == 3);
    UDController cud(s.ctl, &s.om, &s.mm, false);
    CHECK(cud.GetMaxPixelPort() == 4);
    for (int i = 0; i < 3; i++) {
        INFO("string " << i);
        UDControllerPort* p = cud.GetControllerPixelPort(2 + i);
        REQUIRE(p->GetModelCount() == 1);
        auto* pm = p->GetFirstModel();
        CHECK(pm->GetString() == i);
        CHECK(pm->GetName() == "Tree-str-" + std::to_string(i + 1));
        CHECK(pm->GetStartChannel() == 1 + i * 15);
        CHECK(pm->Channels() == 15);
        CHECK(pm->IsFirstModelString() == (i == 0));
    }
    CHECK(cud.GetControllerPortModel("Tree", 1)->GetStartChannel() == 16);
}

TEST_CASE("UDController lists models with no usable connection separately", "[fpp3][uploaddata]") {
    FppShow s;
    Model* none = s.Add(R"(<model name="NoPort" DisplayAs="Single Line" parm1="1" parm2="5" parm3="1" StringType="RGB Nodes" StartChannel="!Ctl:1" Controller="Ctl"/>)");
    s.OnPort("Other", "ws2811", 1, 5, "!Ctl:100");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    REQUIRE(cud.GetNoConnectionModels().size() == 1);
    CHECK(cud.GetNoConnectionModels().front() == none);
    CHECK(cud.GetMaxPixelPort() == 1);
}

TEST_CASE("UDController ignores models outside the controller's channels", "[fpp3][uploaddata]") {
    FppShow s("TestCape", OUTPUT_DDP, 60);
    s.OnPort("In", "ws2811", 1, 10, "!Ctl:1");
    s.OnPort("Out", "ws2811", 2, 10, "100");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    CHECK(cud.GetMaxPixelPort() == 1);
    CHECK(cud.GetControllerPortModel("Out", 0) == nullptr);
}

TEST_CASE("UDControllerPort drops overlapping models, optionally unplugging them", "[fpp3][uploaddata]") {
    FppShow s;
    Model* big = s.OnPort("Big", "ws2811", 1, 10, "!Ctl:1");
    Model* inside = s.OnPort("Inside", "ws2811", 1, 2, "!Ctl:4");
    Model* partial = s.OnPort("Partial", "ws2811", 1, 10, "!Ctl:25");
    s.Recalc();
    {
        UDController cud(s.ctl, &s.om, &s.mm, false);
        auto models = cud.GetControllerPixelPort(1)->GetModels();
        REQUIRE(models.size() == 1);
        CHECK(models.front()->GetModel() == big);
        CHECK(inside->GetControllerPort() == 1);
    }
    UDController cud(s.ctl, &s.om, &s.mm, true);
    CHECK(cud.GetControllerPixelPort(1)->GetModelCount() == 1);
    CHECK(inside->GetControllerPort() == 0);
    CHECK(partial->GetControllerPort() == 0);
    CHECK(big->GetControllerPort() == 1);
}

TEST_CASE("UDControllerPort keeps the longer of two models that start together", "[fpp3][uploaddata]") {
    FppShow s;
    s.OnPort("Short", "ws2811", 1, 2, "!Ctl:1");
    Model* lng = s.OnPort("Long", "ws2811", 1, 10, "!Ctl:1");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    auto models = cud.GetControllerPixelPort(1)->GetModels();
    REQUIRE(models.size() == 1);
    CHECK(models.front()->GetModel() == lng);
}

TEST_CASE("UDControllerPort serial start channel accounts for the DMX channel", "[fpp3][uploaddata]") {
    FppShow s;
    Model* d = s.OnPort("Dmx", "dmx", 1, 2, "!Ctl:105", "channel=\"5\"");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    CHECK(cud.GetMaxSerialPort() == 1);
    CHECK(cud.HasSerialPort(1));
    CHECK(cud.IsListedSerialPort(1));
    CHECK(cud.IsListedSerialPort(2));  // the cape has two serial ports
    CHECK_FALSE(cud.IsListedSerialPort(3));
    UDControllerPort* p = cud.GetControllerSerialPort(1);
    auto* pm = p->GetFirstModel();
    CHECK(pm->GetDMXChannelOffset() == 5);
    CHECK(pm->GetStartChannel() == 105);
    CHECK(p->GetStartChannel() == 101);
    CHECK(p->GetUniverse() == 1);
    CHECK(p->GetUniverseStartChannel() == 101);
    CHECK_FALSE(p->IsPixelProtocol());
    CHECK(d->IsSerialProtocol());
    auto j = nlohmann::json::parse(p->ExportAsJSON());
    CHECK(j["startchannel"] == 101);
    CHECK_FALSE(j.contains("pixels"));
    CHECK_FALSE(j["models"][0].contains("pixels"));
}

TEST_CASE("Virtual strings merge plain neighbours and split on controller properties", "[fpp3][uploaddata]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.OnPort("B", "ws2811", 1, 10, "!Ctl:31");
    s.OnPort("C", "ws2811", 1, 10, "!Ctl:61", "Brightness=\"50\" gamma=\"2.2\" colorOrder=\"GRB\" Reverse=\"1\" groupCount=\"2\"");
    s.OnPort("D", "ws2811", 1, 10, "!Ctl:91", "nullNodes=\"3\" endNullNodes=\"2\" ZigZag=\"5\" ts=\"7\"");
    s.OnPort("E", "ws2811", 1, 10, "!Ctl:200");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    UDControllerPort* p = cud.GetControllerPixelPort(1);

    p->CreateVirtualStrings(true);
    REQUIRE(p->GetVirtualStringCount() == 4);
    auto* ab = p->GetVirtualString(0);
    CHECK(ab->_description == "A");
    CHECK(ab->_startChannel == 1);
    CHECK(ab->_endChannel == 60);
    CHECK(ab->Channels() == 60);
    CHECK(ab->_models.size() == 2);
    CHECK_FALSE(ab->_brightnessSet);
    CHECK(ab->_channelsPerPixel == 3);
    CHECK(ab->_index == 0);

    auto* c = p->GetVirtualString(1);
    CHECK(c->_description == "C");
    CHECK(c->_brightnessSet);
    CHECK(c->_brightness == 50);
    CHECK(c->_gammaSet);
    CHECK(c->_gamma == 2.2f);
    CHECK(c->_colourOrderSet);
    CHECK(c->_colourOrder == "GRB");
    CHECK(c->_reverseSet);
    CHECK(c->_reverse == "Reverse");
    CHECK(c->_groupCountSet);
    CHECK(c->_groupCount == 2);
    CHECK_FALSE(c->_zigZagSet);

    auto* d = p->GetVirtualString(2);
    CHECK(d->_description == "D");
    CHECK(d->_startNullPixelsSet);
    CHECK(d->_startNullPixels == 3);
    CHECK(d->_endNullPixelsSet);
    CHECK(d->_endNullPixels == 2);
    CHECK(d->_zigZagSet);
    CHECK(d->_zigZag == 5);
    CHECK(d->_tsSet);
    CHECK(d->_ts == 7);

    // a gap in the channels always starts a new string
    auto* e = p->GetVirtualString(3);
    CHECK(e->_description == "E");
    CHECK(e->_startChannel == 200);
    CHECK(e->_index == 3);
    CHECK(p->GetRealVirtualStringCount() == 4);
    CHECK(p->GetModelCount(0) == 4);
    CHECK(p->Channels() == 150);

    p->CreateVirtualStrings(false);
    CHECK(p->GetVirtualStringCount() == 5);
    CHECK(p->GetVirtualString(1)->_description == "B");
    CHECK(p->GetVirtualString(5) == nullptr);
}

TEST_CASE("Virtual strings: a single channel model first on a port takes the port's pixel size", "[fpp3][uploaddata]") {
    FppShow s;
    s.OnPort("Single", "ws2811", 1, 6, "!Ctl:1", "", "", 1, "Single Color White");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    UDControllerPort* p = cud.GetControllerPixelPort(1);
    p->CreateVirtualStrings(true, true);
    CHECK(p->GetVirtualString(0)->_channelsPerPixel == 3);
    p->CreateVirtualStrings(true, false);
    CHECK(p->GetVirtualString(0)->_channelsPerPixel == 1);
}

TEST_CASE("Group count multiplies the effective channels", "[fpp3][uploaddata]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1", "groupCount=\"3\"");
    s.OnPort("B", "ws2811", 1, 10, "!Ctl:31");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    UDControllerPort* p = cud.GetControllerPixelPort(1);
    p->CreateVirtualStrings(false);
    // Falcon carries a group count on to the models after it, others do not
    CHECK(p->EffectiveChannels("Falcon") == 90 + 90);
    CHECK(p->EffectiveChannels("FPP") == 90 + 30);
}

TEST_CASE("Smart remotes create dummy strings for skipped remotes", "[fpp3][uploaddata]") {
    FppShow s;
    s.OnPort("R1", "ws2811", 1, 10, "!Ctl:1", "SmartRemote=\"1\" SmartRemoteType=\"falcon_v1\"");
    s.OnPort("R3", "ws2811", 1, 10, "!Ctl:31", "SmartRemote=\"3\" SmartRemoteType=\"falcon_v1\"");
    s.OnPort("R3b", "ws2811", 1, 10, "!Ctl:61", "SmartRemote=\"3\" SmartRemoteType=\"falcon_v1\"");
    s.OnPort("P2", "ws2811", 2, 10, "!Ctl:91", "SmartRemote=\"2\" SmartRemoteType=\"falcon_v1\"");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    UDControllerPort* p = cud.GetControllerPixelPort(1);
    CHECK(p->AtLeastOneModelIsUsingSmartRemote());
    CHECK_FALSE(p->AtLeastOneModelIsNotUsingSmartRemote());
    CHECK(p->GetSmartRemoteCount() == 3);
    CHECK(p->AllSmartRemoteTypesSame());
    CHECK(p->GetSmartRemoteType(3) == "falcon_v1");
    CHECK(p->GetSmartRemoteType(2).empty());
    CHECK(p->GetFirstModel(3)->GetName() == "R3");
    CHECK(p->GetFirstModel(2) == nullptr);
    CHECK(p->GetFirstModel()->GetSmartRemoteLetter() == 'A');
    // smart remote models are counted by their own channels, not the span
    CHECK(p->Channels() == 90);

    p->CreateVirtualStrings(true);
    REQUIRE(p->GetVirtualStringCount() == 3);
    auto* r1 = p->GetVirtualString(0);
    auto* dummy = p->GetVirtualString(1);
    auto* r3 = p->GetVirtualString(2);
    CHECK(r1->_smartRemote == 1);
    CHECK(r1->_index == 0);
    CHECK(dummy->_isDummy);
    CHECK(dummy->_smartRemote == 2);
    CHECK(dummy->_description == "DUMMY");
    CHECK(dummy->_index == 0);
    CHECK(r3->_smartRemote == 3);
    CHECK(r3->_models.size() == 2);
    CHECK(r3->_index == 0);
    CHECK(p->GetRealVirtualStringCount() == 2);
    CHECK(p->GetModelCount(2) == 0);
    CHECK(p->GetModelCount(3) == 1);
    CHECK(p->CountEmptySmartRemotesBefore(3) == 1);
    CHECK(p->CountEmptySmartRemotesBefore(1) == 0);

    CHECK(cud.GetSmartRemoteCount(1) == 3);
    CHECK(cud.GetSmartRemoteCount(4) == 3); // same block of 4
    CHECK(cud.GetSmartRemoteCount(5) == 0);
    cud.TagSmartRemotePorts();
    CHECK(cud.GetControllerPixelPort(4)->IsSmartRemotePort());
    CHECK_FALSE(cud.HasPixelPort(5));

    auto csv = p->ExportAsCSV(ExportSettings::SETTINGS_NONE, 100.0f);
    REQUIRE(csv.size() == 4);
    CHECK(csv[1] == "Remote A:R1");
    CHECK(csv[2] == "Remote C:R3");
    auto j = nlohmann::json::parse(p->ExportAsJSON());
    CHECK(j["models"][1]["smartremote"] == "C");
}

TEST_CASE("Smart remotes without merging open a dummy for a skipped first remote", "[fpp3][uploaddata]") {
    FppShow s;
    s.OnPort("R2", "ws2811", 1, 10, "!Ctl:1", "SmartRemote=\"2\"");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    UDControllerPort* p = cud.GetControllerPixelPort(1);
    p->CreateVirtualStrings(false);
    REQUIRE(p->GetVirtualStringCount() == 2);
    CHECK(p->GetVirtualString(0)->_isDummy);
    CHECK(p->GetVirtualString(0)->_smartRemote == 1);
    CHECK(p->GetVirtualString(1)->_smartRemote == 2);
}

TEST_CASE("Port amps follow per model brightness", "[fpp3][uploaddata]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.OnPort("B", "ws2811", 1, 10, "!Ctl:31", "Brightness=\"50\"");
    s.OnPort("C", "ws2811", 1, 10, "!Ctl:61");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    UDControllerPort* p = cud.GetControllerPixelPort(1);
    auto models = p->GetModels();
    auto it = models.begin();
    auto* a = *it++;
    auto* b = *it++;
    auto* c = *it++;
    float const full = a->GetAmps(100);
    CHECK(full > 0.0f);
    CHECK(b->GetAmps(100) == full / 2);
    // a brightness carries on to the models after it
    CHECK(p->GetAmps(100) == full + full / 2 + full / 2);
    CHECK(p->GetAmps(c, 100) == full / 2);
    CHECK(p->GetAmps(a, 100) == full);
    CHECK(p->GetAmps(nullptr, 100) == 0.0f);
    CHECK(p->GetAmps(100, 0) == p->GetAmps(100));
    CHECK(p->GetAmps(100, 1) == 0.0f);
    UDControllerPort serial("Serial", 1, "dmx");
    CHECK(serial.GetAmps(100) == 0.0f);
}

TEST_CASE("UDController ExportAsCSV lays out ports and models with the chosen columns", "[fpp3][uploaddata]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1", "", "", 1, "RGB Nodes", "Description=\"desc\"");
    s.OnPort("B", "ws2811", 1, 5, "!Ctl:31");
    s.OnPort("D", "dmx", 2, 4, "!Ctl:100");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    ExportSettings::SETTINGS all = ExportSettings::SETTINGS_PORT_ABSADDRESS;
    for (auto f : { ExportSettings::SETTINGS_PORT_UNIADDRESS, ExportSettings::SETTINGS_PORT_CHANNELS, ExportSettings::SETTINGS_PORT_PIXELS,
                    ExportSettings::SETTINGS_MODEL_DESCRIPTIONS, ExportSettings::SETTINGS_MODEL_ABSADDRESS, ExportSettings::SETTINGS_MODEL_UNIADDRESS,
                    ExportSettings::SETTINGS_MODEL_CHANNELS, ExportSettings::SETTINGS_MODEL_PIXELS }) {
        all |= f;
    }
    int columns = 0;
    auto lines = cud.ExportAsCSV(all, 100.0f, columns);
    CHECK(columns == 2);
    REQUIRE(lines.size() == 4); // header, pixel port 1, serial ports 1 and 2
    CHECK(lines[0] == std::vector<std::string>{ "Output", "Model 1", "Model 2" });
    CHECK(lines[1][0] == "Pixel Port 1(SC:1)(UNI:#1:1)(CHANS:45)(PIX:15)");
    CHECK(lines[1][1] == "A(DESC:desc)(SC:1)(UNI:#1:1)(CHANS:30)(PIX:10)");
    CHECK(lines[1][2] == "B(SC:31)(UNI:#1:31)(CHANS:15)(PIX:5)");
    CHECK(lines[2] == std::vector<std::string>{ "Serial Port 1" });
    CHECK(lines[3][0] == "Serial Port 2(SC:100)(UNI:#1:100)(CHANS:12)");
    CHECK(lines[3][1] == "D(SC:100)(UNI:#1:100)(CHANS:12)");
}

TEST_CASE("UDController renames, re-protocols and clears models on its ports", "[fpp3][uploaddata]") {
    FppShow s;
    Model* a = s.OnPort("A", "ws2811", 1, 10, "!Ctl:1", "SmartRemote=\"1\"");
    Model* b = s.OnPort("B", "tm1814", 2, 10, "!Ctl:31", "", ">A");
    Model* d = s.OnPort("D", "renard", 1, 4, "!Ctl:100");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);

    // b chains on a model of another port, so it is put back at the start of its port
    CHECK(cud.SetAllModelsToControllerName("Ctl2"));
    CHECK(a->GetControllerName() == "Ctl2");
    CHECK(d->GetControllerName() == "Ctl2");
    CHECK(b->GetModelChain().empty());
    CHECK_FALSE(cud.SetAllModelsToControllerName("Ctl2"));

    CHECK(cud.SetAllModelsToValidProtocols({ "ws2811", "apa102" }, { "dmx" }, true));
    CHECK(a->GetControllerProtocol() == "ws2811");
    CHECK(b->GetControllerProtocol() == "ws2811");
    CHECK(d->GetControllerProtocol() == "dmx");
    CHECK_FALSE(cud.SetAllModelsToValidProtocols({ "ws2811" }, { "dmx" }, true));

    cud.ClearSmartRemoteOnAllModels();
    CHECK(a->GetSmartRemote() == 0);
}

TEST_CASE("UDControllerPort chains every model on a port to the one before it", "[fpp3][uploaddata]") {
    FppShow s;
    Model* a = s.OnPort("A", "ws2811", 1, 10, "!Ctl:1", "", ">Z");
    Model* b = s.OnPort("B", "ws2811", 1, 10, "!Ctl:31");
    Model* c = s.OnPort("C", "ws2811", 1, 10, "!Ctl:61", "SmartRemote=\"1\"");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    UDControllerPort* p = cud.GetControllerPixelPort(1);
    CHECK(p->EnsureAllModelsAreChained());
    CHECK(a->GetModelChain().empty());
    CHECK(b->GetModelChain() == ">A");
    CHECK(c->GetModelChain().empty()); // first on its smart remote
    CHECK_FALSE(p->EnsureAllModelsAreChained());
}

TEST_CASE("UDControllerPort protocol fallback picks the first protocol when none fit", "[fpp3][uploaddata]") {
    FppShow s;
    Model* a = s.OnPort("A", "lor", 1, 4, "!Ctl:1");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    UDControllerPort* p = cud.GetControllerSerialPort(1);
    CHECK_FALSE(p->SetAllModelsToValidProtocols({}, ""));
    CHECK(p->SetAllModelsToValidProtocols({ "dmx", "renard" }, ""));
    CHECK(a->GetControllerProtocol() == "dmx");
    CHECK(p->SetAllModelsToValidProtocols({ "dmx", "renard" }, "renard"));
    CHECK(a->GetControllerProtocol() == "renard");
}

TEST_CASE("UDController Check without rules fails", "[fpp3][uploaddata][check]") {
    FppShow s;
    UDController cud(s.ctl, &s.om, &s.mm, false);
    std::string res;
    CHECK_FALSE(cud.Check(nullptr, res));
    CHECK(res == "No rules to evaluate.");
}

TEST_CASE("UDController Check accepts a clean layout", "[fpp3][uploaddata][check]") {
    FppShow s;
    s.OnPort("A", "ws2811", 1, 10, "!Ctl:1");
    s.OnPort("B", "ws2811", 2, 10, "!Ctl:31");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    bool ok = false;
    auto res = CheckText(cud, s.Caps(), &ok);
    CHECK(ok);
    CHECK(res.empty());
}

TEST_CASE("UDController Check reports ports, sizes and per model limits", "[fpp3][uploaddata][check]") {
    RuleShow s("Strict");
    s.OnPort("Long", "ws2811", 1, 120, "!Ctl:1");                    // 360 channels > 300
    s.OnPort("Gap", "ws2811", 2, 2, "!Ctl:400");
    s.OnPort("Gap2", "ws2811", 2, 2, "!Ctl:420");                    // gap on the same port
    s.OnPort("Far", "ws2811", 6, 2, "!Ctl:500");                     // port past the last one
    s.OnPort("Nulls", "ws2811", 3, 2, "!Ctl:600", "nullNodes=\"5\" endNullNodes=\"4\" groupCount=\"7\" ZigZag=\"9\"");
    s.OnPort("TooFew", "ws2811", 4, 2, "!Ctl:700", "groupCount=\"1\"");
    s.OnPort("Smart", "ws2811", 4, 2, "!Ctl:710", "SmartRemote=\"1\"");
    s.OnPort("BigDmx", "dmx", 1, 50, "!Ctl:800");                    // 150 channels > 100
    s.OnPort("Ser2", "dmx", 2, 2, "!Ctl:1000");                      // serial port 2 does not exist
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    bool ok = true;
    auto res = CheckText(cud, s.Caps(), &ok);
    INFO(res);
    CHECK_FALSE(ok);
    CHECK(UDController::IsError(res));
    CHECK_THAT(res, ContainsSubstring("ERR: Pixel port 1 has 120 nodes allocated but maximum is 100."));
    CHECK_THAT(res, ContainsSubstring("WARN: Gap in models on pixel port 2 channel 405 to 420."));
    CHECK_THAT(res, ContainsSubstring("ERR: Pixel port 6 is not valid on this controller. Valid ports 1-4."));
    CHECK_THAT(res, ContainsSubstring("ERR: Model Nulls has too many start NULL pixels : 5. Maximum 2."));
    CHECK_THAT(res, ContainsSubstring("ERR: Model Nulls has too many end NULL pixels : 4. Maximum 1."));
    CHECK_THAT(res, ContainsSubstring("ERR: Model Nulls has too many grouped pixels : 7. Maximum 3."));
    CHECK_THAT(res, ContainsSubstring("ERR: Model TooFew has too few grouped pixels : 1. Minimum 2."));
    CHECK_THAT(res, ContainsSubstring("ERR: Model Nulls has too many zig zagged pixels : 9. Maximum 4."));
    CHECK_THAT(res, ContainsSubstring("ERR: Port 4 has model Smart with smart remote set ... but this controller does not support smart remotes."));
    CHECK_THAT(res, ContainsSubstring("ERR: Model Smart has invalid smart remote A with cascade of 1."));
    CHECK_THAT(res, ContainsSubstring("ERR: Serial port 1 has 150 channels allocated but maximum is 100."));
    CHECK_THAT(res, ContainsSubstring("ERR: Serial port 2 is not valid on this controller. Valid ports 1-1."));
    CHECK(res.find("ERR: DDP is not a protocol this controller supports.") == std::string::npos);
}

TEST_CASE("UDController Check reports protocol mismatches", "[fpp3][uploaddata][check]") {
    RuleShow s("Strict");
    s.OnPort("A", "ws2811", 1, 2, "!Ctl:1");
    s.OnPort("B", "apa102", 2, 2, "!Ctl:10");
    s.OnPort("C", "tm1814", 3, 2, "!Ctl:20");
    s.OnPort("D", "renard", 1, 2, "!Ctl:30");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    // mix protocols on one port after the port took its protocol from the first model
    cud.GetControllerPixelPort(1)->GetFirstModel()->GetModel()->SetControllerProtocol("apa102");
    auto res = CheckText(cud, s.Caps());
    INFO(res);
    CHECK_THAT(res, ContainsSubstring("ERR: Invalid protocol on pixel port 3: tm1814"));
    CHECK_THAT(res, ContainsSubstring("ERR: Invalid protocol on serial port 1: renard"));
    CHECK_THAT(res, ContainsSubstring("ERR: Pixel ports only support a single protocol at a time. ws2811 and apa102 found."));
}

TEST_CASE("UDController Check reports a model whose protocol differs from its port", "[fpp3][uploaddata][check]") {
    RuleShow s("Strict");
    Model* a = s.OnPort("A", "ws2811", 1, 2, "!Ctl:1");
    s.OnPort("B", "ws2811", 1, 2, "!Ctl:7");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    (void)a;
    s.mm["B"]->SetControllerProtocol("apa102");
    cud.Rescan(false);
    auto res = CheckText(cud, s.Caps());
    INFO(res);
    CHECK_THAT(res, ContainsSubstring("ERR: Model B on pixel port 1 has protocol apa102 but port protocol has been set to ws2811."));
}

TEST_CASE("UDController Check universe size rules", "[fpp3][uploaddata][check]") {
    RuleShow s("Strict", OUTPUT_E131);
    // universes 1 (510), 3 (50) and 2 (510): too small, out of order, not sequential, not all 510
    s.ctl->GetOutputs().front()->SetUniverse(1);
    s.ctl->AddOutput();
    s.ctl->GetOutputs().back()->SetUniverse(3);
    s.ctl->GetOutputs().back()->SetChannels(50);
    s.ctl->AddOutput();
    s.ctl->GetOutputs().back()->SetUniverse(2);
    s.om.SomethingChanged();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    auto res = CheckText(cud, s.Caps());
    INFO(res);
    CHECK_THAT(res, ContainsSubstring("ERR: Controller limits universe sizes to max of 400 but you are trying to use 510. Universe 1."));
    CHECK_THAT(res, ContainsSubstring("ERR: Controller limits universe sizes to min of 100 but you are trying to use 50. Universe 3."));
    CHECK_THAT(res, ContainsSubstring("ERR: All universes must be the same size. 510 and 50 found."));
    CHECK_THAT(res, ContainsSubstring("ERR: All universes must be 510 channels. 50 found."));
    CHECK_THAT(res, ContainsSubstring("ERR: All universes must be in numerical order. 2 followed 3."));
    CHECK_THAT(res, ContainsSubstring("ERR: All universes must be sequential. 3 followed 1."));
}

TEST_CASE("UDController Check smart remote block rules", "[fpp3][uploaddata][check]") {
    RuleShow s("Smart");
    s.OnPort("A", "ws2811", 1, 2, "!Ctl:1", "SmartRemote=\"1\" SmartRemoteType=\"falcon_v1\"");
    s.OnPort("A2", "ws2811", 1, 2, "!Ctl:7", "SmartRemote=\"1\" SmartRemoteType=\"falcon_v2\"");
    s.OnPort("B", "ws2811", 2, 2, "!Ctl:20");
    s.OnPort("C", "ws2811", 3, 2, "!Ctl:40", "SmartRemote=\"2\" SmartRemoteType=\"falcon_v2\"");
    s.OnPort("Mix", "ws2811", 3, 2, "!Ctl:50");
    s.OnPort("Deep", "ws2811", 5, 2, "!Ctl:60", "SmartRemote=\"3\" SRMaxCascade=\"2\"");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    auto res = CheckText(cud, s.Caps());
    INFO(res);
    CHECK_THAT(res, ContainsSubstring("ERR: Pixel port 1 has different types of smart remotes types for A."));
    CHECK_THAT(res, ContainsSubstring("ERR: Pixel port 2 has a model configured not on a smart remote but this block of 4 ports has at least one model that is configured as on a smart remote."));
    CHECK_THAT(res, ContainsSubstring("ERR: Pixel Port:3 SRID:B has different smart remotes types for this block of 4 ports."));
    CHECK_THAT(res, ContainsSubstring("ERR: Pixel port 1 has a models not configured all as the same smart remote type."));
    CHECK_THAT(res, ContainsSubstring("ERR: Port 3 has a mix of models with smart and non smart remotes. This is not supported."));
    CHECK_THAT(res, ContainsSubstring("ERR: Model Deep has invalid smart remote C with cascade of 2."));
    CHECK(res.find("WARN: Gap in models on pixel port 1 smart remote A channel 6 to 7.") == std::string::npos);
}

TEST_CASE("UDController Check gap warning on a smart remote", "[fpp3][uploaddata][check]") {
    RuleShow s("Smart");
    s.OnPort("A", "ws2811", 1, 2, "!Ctl:1", "SmartRemote=\"1\"");
    s.OnPort("B", "ws2811", 1, 2, "!Ctl:20", "SmartRemote=\"1\"");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    auto res = CheckText(cud, s.Caps());
    CHECK_THAT(res, ContainsSubstring("WARN: Gap in models on pixel port 1 smart remote A channel 6 to 20."));
}

TEST_CASE("UDController Check bank and universe per string rules", "[fpp3][uploaddata][check]") {
    RuleShow s("Banked");
    s.OnPort("A", "ws2811", 1, 80, "!Ctl:1");  // 240 in bank 1
    s.OnPort("B", "ws2811", 3, 60, "!Ctl:300", "colorOrder=\"RGBW\""); // 180 in bank 2
    s.Recalc();
    s.ctl->SetUniversePerString(true);
    UDController cud(s.ctl, &s.om, &s.mm, false);
    auto res = CheckText(cud, s.Caps());
    INFO(res);
    CHECK_THAT(res, ContainsSubstring("ERR: Controllers 'Bank' channel count [420 (140)] is over the maximum [300 (100)]."));
    CHECK_THAT(res, ContainsSubstring(" Bank 1 - Port 1 [240 (80)],  Bank 2 - Port 3 [180 (60)]"));
    CHECK_THAT(res, ContainsSubstring("ERR: Controller does NOT Support Color Order RGBW."));
    CHECK(res.find("ERR: Controller only support a single input protocol at a time.") == std::string::npos);
    // universe per string separates the strings, so the port counts each model on its own
    CHECK(cud.GetControllerPixelPort(1)->GetVirtualStringCount() == 1);
}

TEST_CASE("UDController Check grouping beyond the port size", "[fpp3][uploaddata][check]") {
    RuleShow s("Banked");
    s.OnPort("A", "ws2811", 1, 40, "!Ctl:1", "groupCount=\"3\""); // 120 channels, 360 effective
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    auto res = CheckText(cud, s.Caps());
    CHECK_THAT(res, ContainsSubstring("ERR: Pixel port 1 has 120 nodes allocated (as a result of grouping) but maximum is 100."));
}

TEST_CASE("UDController Check protocol groups", "[fpp3][uploaddata][check]") {
    RuleShow s("Grouped");
    s.OnPort("A", "ws2811", 1, 2, "!Ctl:1");
    s.OnPort("B", "apa102", 2, 2, "!Ctl:10");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    auto res = CheckText(cud, s.Caps());
    CHECK_THAT(res, ContainsSubstring("ERR: Pixel port 2 uses apa102 which cannot be used on the same controller as ws2811."));
}

TEST_CASE("UDController Check port types the controller lacks", "[fpp3][uploaddata][check]") {
    RuleShow s("Bare");
    s.OnPort("P", "ws2811", 1, 2, "!Ctl:1");
    s.OnPort("S", "dmx", 1, 2, "!Ctl:10");
    s.Add(R"(<model name="VM" DisplayAs="Matrix" parm1="2" parm2="2" parm3="1" StringType="RGB Nodes" StartChannel="!Ctl:20" Controller="Ctl"><ControllerConnection Protocol="Virtual Matrix" Port="1"/></model>)");
    s.Add(R"(<model name="LP" DisplayAs="Matrix" parm1="2" parm2="2" parm3="1" StringType="RGB Nodes" StartChannel="!Ctl:40" Controller="Ctl"><ControllerConnection Protocol="LED Panel Matrix" Port="1"/></model>)");
    s.Add(R"(<model name="PW" DisplayAs="Single Line" parm1="1" parm2="2" parm3="1" StringType="Single Color White" StartChannel="!Ctl:60" Controller="Ctl"><ControllerConnection Protocol="PWM" Port="1"/></model>)");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    CHECK(cud.GetMaxVirtualMatrixPort() == 1);
    CHECK(cud.GetMaxLEDPanelMatrixPort() == 1);
    CHECK(cud.HasVirtualMatrixPort(1));
    CHECK(cud.HasLEDPanelMatrixPort(1));
    CHECK_FALSE(cud.HasPWMPort(1)); // the controller has no PWM ports, so nothing is placed
    CHECK(cud.GetMaxPWMPort() == 0);
    auto res = CheckText(cud, s.Caps());
    INFO(res);
    CHECK_THAT(res, ContainsSubstring("ERR: Attempt to upload pixel port 1 but this controller does not support pixel ports."));
    CHECK_THAT(res, ContainsSubstring("ERR: Attempt to upload serial port 1 but this controller does not support serial ports."));
    CHECK_THAT(res, ContainsSubstring("ERR: Attempt to upload Virtual Matrix but this controller does not support Virtual Matrices."));
    CHECK_THAT(res, ContainsSubstring("ERR: Attempt to upload LED Panel Matrix but this controller does not support LED Panel Matrices."));
    int columns = 0;
    auto lines = cud.ExportAsCSV(ExportSettings::SETTINGS_NONE, 100.0f, columns);
    CHECK(lines.size() == 1 + 1 + 1 + 1 + 1); // header, pixel, serial, vm, panel
}

TEST_CASE("UDController places PWM outputs on consecutive ports", "[fpp3][uploaddata]") {
    FppShow s("TestHat");
    Model* m = s.Add(R"(<model name="PW" DisplayAs="Single Line" parm1="1" parm2="2" parm3="1" StringType="Node Single Color" StartChannel="!Ctl:10" Controller="Ctl"><ControllerConnection Protocol="PWM" Port="2"/></model>)");
    s.Recalc();
    REQUIRE(m->IsPWMProtocol());
    auto outs = m->GetPWMOutputs();
    REQUIRE(outs.size() == 2);
    UDController cud(s.ctl, &s.om, &s.mm, false);
    CHECK(cud.GetMaxPWMPort() == 3);
    auto* p2 = cud.GetControllerPWMPort(2)->GetFirstModel();
    auto* p3 = cud.GetControllerPWMPort(3)->GetFirstModel();
    REQUIRE(p2 != nullptr);
    REQUIRE(p3 != nullptr);
    CHECK(p2->GetPWMProperties().type == 0);
    CHECK(p2->GetStartChannel() == outs[0].startChannel);
    CHECK(p3->GetStartChannel() == outs[1].startChannel);
    CHECK(p2->GetLabel() == outs[0].label);
    CHECK(p2->GetName() == "PW");
    CHECK(p2->GetBrightness(7) == outs[0].brightness);
    CHECK(p2->GetGamma(1) == outs[0].gamma);
    std::string res;
    CHECK(cud.Check(s.Caps(), res));
}

TEST_CASE("UDController LOR enhanced serial ports check unit overlap", "[fpp3][uploaddata][check]") {
    RuleShow s("Strict");
    // two strings: the second one starts on the next unit ID, at port channel 511
    s.Add(R"(<model name="A" DisplayAs="Single Line" parm1="2" parm2="10" parm3="1" StringType="RGB Nodes" StartChannel="!Ctl:1" Controller="Ctl"><ControllerConnection Protocol="LOR Enhanced" Port="1"/></model>)");
    s.OnPort("B", "LOR Enhanced", 1, 4, "!Ctl:511");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    REQUIRE(cud.GetControllerSerialPort(1)->GetModelCount() == 2);
    auto res = CheckText(cud, s.Caps());
    INFO(res);
    CHECK_THAT(res, ContainsSubstring("ERR: Models A and B on serial port 1 both use pixel 1 of LOR unit ID 2."));
    CHECK(res.find("channels allocated but maximum") == std::string::npos);
}

TEST_CASE("UDControllerPortModel warns about channels the controller does not send", "[fpp3][uploaddata][check]") {
    FppShow s("TestCape", OUTPUT_DDP, 40);
    s.OnPort("Over", "ws2811", 1, 10, "!Ctl:31");
    s.Recalc();
    UDController cud(s.ctl, &s.om, &s.mm, false);
    auto res = CheckText(cud, s.Caps());
    CHECK_THAT(res, ContainsSubstring("WARN: Model Over uses channels not being sent to this controller."));
}

#endif
