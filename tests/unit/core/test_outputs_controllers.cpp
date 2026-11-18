#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <sstream>
#include <string>

#include <pugixml.hpp>

#include "outputs/ArtNetOutput.h"
#include "outputs/Controller.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/ControllerNull.h"
#include "outputs/ControllerSerial.h"
#include "outputs/DDPOutput.h"
#include "outputs/E131Output.h"
#include "outputs/NullOutput.h"
#include "outputs/OutputManager.h"
#include "outputs/SerialOutput.h"

namespace {

std::string Print(pugi::xml_node n) {
    std::ostringstream ss;
    n.print(ss, "", pugi::format_raw);
    return ss.str();
}

std::string SaveToString(Controller& c) {
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("root");
    c.Save(root);
    return Print(root.first_child());
}

std::unique_ptr<Controller> Load(OutputManager& om, const std::string& xml) {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(xml.c_str()));
    return std::unique_ptr<Controller>(Controller::Create(&om, doc.first_child(), ""));
}

const char* kEthXml = R"(<Controller Id="5" Name="Roof" Description="  the roof  " Type="Ethernet" Protocol="E131" IP="10.1.2.3" Priority="80" ActiveState="Active" Vendor="FooVendor" Model="BarModel" Variant="Baz" AutoSize="1" AutoLayout="1" SuppressDuplicates="1" Monitor="0">
    <network NetworkType="E131" ComPort="10.1.2.3" BaudRate="1" MaxChannels="510"/>
    <network NetworkType="E131" ComPort="10.1.2.3" BaudRate="2" MaxChannels="510"/>
    <network NetworkType="E131" ComPort="10.1.2.3" BaudRate="3" MaxChannels="100"/>
    <ExtraProperty name="k" value="v"/>
</Controller>)";

const char* kEthPlain = R"(<Controller Id="5" Name="Roof" Type="Ethernet" Protocol="E131" IP="10.1.2.3" ActiveState="Active">
    <network NetworkType="E131" ComPort="10.1.2.3" BaudRate="1" MaxChannels="510"/>
    <network NetworkType="E131" ComPort="10.1.2.3" BaudRate="2" MaxChannels="510"/>
</Controller>)";

} // namespace

TEST_CASE("Controller active state encoding", "[outputs]") {
    CHECK(Controller::EncodeActiveState("Active") == Controller::ACTIVESTATE::ACTIVE);
    CHECK(Controller::EncodeActiveState("Inactive") == Controller::ACTIVESTATE::INACTIVE);
    CHECK(Controller::EncodeActiveState("xLights Only") == Controller::ACTIVESTATE::ACTIVEINXLIGHTSONLY);
    CHECK(Controller::EncodeActiveState("garbage") == Controller::ACTIVESTATE::ACTIVE);
    CHECK(Controller::EncodeActiveState("") == Controller::ACTIVESTATE::ACTIVE);
    CHECK(Controller::DecodeActiveState(Controller::ACTIVESTATE::ACTIVE) == "Active");
    CHECK(Controller::DecodeActiveState(Controller::ACTIVESTATE::INACTIVE) == "Inactive");
    CHECK(Controller::DecodeActiveState(Controller::ACTIVESTATE::ACTIVEINXLIGHTSONLY) == "xLights Only");
}

TEST_CASE("Controller::Create selects the controller class", "[outputs]") {
    OutputManager om;
    auto eth = Load(om, R"(<Controller Name="a" Type="Ethernet" Protocol="E131" ActiveState="Inactive"><network NetworkType="E131" ComPort="1.1.1.1" BaudRate="1" MaxChannels="10"/></Controller>)");
    auto nul = Load(om, R"(<Controller Name="b" Type="Null" ActiveState="Inactive"><network NetworkType="NULL" Id="3" MaxChannels="10"/></Controller>)");
    auto ser = Load(om, R"(<Controller Name="c" Type="Serial" Protocol="DMX" Port="COM1" Speed="250000" ActiveState="Inactive"><network NetworkType="DMX" Id="1" ComPort="COM1" BaudRate="250000" MaxChannels="512"/></Controller>)");
    REQUIRE(eth != nullptr);
    REQUIRE(nul != nullptr);
    REQUIRE(ser != nullptr);
    CHECK(eth->GetType() == CONTROLLER_ETHERNET);
    CHECK(nul->GetType() == CONTROLLER_NULL);
    CHECK(ser->GetType() == CONTROLLER_SERIAL);
    CHECK(dynamic_cast<ControllerEthernet*>(eth.get()) != nullptr);
    CHECK(dynamic_cast<ControllerNull*>(nul.get()) != nullptr);
    CHECK(dynamic_cast<ControllerSerial*>(ser.get()) != nullptr);
}

TEST_CASE("Controller::Create ignores unknown controller types", "[outputs]") {
    OutputManager om;
    CHECK(Load(om, R"(<Controller Name="x" Type="Teleporter"/>)") == nullptr);
    CHECK(Load(om, R"(<Controller Name="x"/>)") == nullptr);
}

TEST_CASE("Ethernet controller loads its configuration", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    auto* e = dynamic_cast<ControllerEthernet*>(c.get());
    REQUIRE(e != nullptr);
    CHECK(e->GetName() == "Roof");
    CHECK(e->GetDescription() == "the roof");
    CHECK(e->GetProtocol() == OUTPUT_E131);
    CHECK(e->GetIP() == "10.1.2.3");
    CHECK(e->GetPriority() == 80);
    CHECK(e->GetVendor() == "FooVendor");
    CHECK(e->GetModel() == "BarModel");
    CHECK(e->GetVariant() == "Baz");
    CHECK(e->GetVMV() == "FooVendor BarModel - Baz");
    CHECK(e->IsActive());
    CHECK(e->IsAutoLayout());
    CHECK(e->IsSuppressDuplicateFrames());
    CHECK_FALSE(e->IsMonitoring());
    CHECK(e->GetOutputCount() == 3);
    CHECK(e->GetChannels() == 1120);
    CHECK(e->GetExtraProperty("k") == "v");
    CHECK(e->GetExtraProperty("missing", "dflt") == "dflt");
}

TEST_CASE("A freshly loaded plain controller is not dirty", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthPlain);
    REQUIRE(c != nullptr);
    CHECK_FALSE(c->IsDirty());
}

TEST_CASE("Ethernet controller Save and reload is stable", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    REQUIRE(c != nullptr);
    const std::string first = SaveToString(*c);
    pugi::xml_document doc;
    REQUIRE(doc.load_string(first.c_str()));
    OutputManager om2;
    std::unique_ptr<Controller> again(Controller::Create(&om2, doc.first_child(), ""));
    REQUIRE(again != nullptr);
    CHECK(SaveToString(*again) == first);
}

TEST_CASE("Ethernet controller Save writes the documented attributes", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    pugi::xml_document doc;
    pugi::xml_node n = c->Save(doc);
    CHECK(std::string(n.name()) == "Controller");
    CHECK(n.attribute("Id").as_int() == 5);
    CHECK(std::string(n.attribute("Type").as_string()) == "Ethernet");
    CHECK(std::string(n.attribute("Protocol").as_string()) == "E131");
    CHECK(std::string(n.attribute("IP").as_string()) == "10.1.2.3");
    CHECK(std::string(n.attribute("ActiveState").as_string()) == "Active");
    CHECK(std::string(n.attribute("SuppressDuplicates").as_string()) == "1");
    CHECK(std::string(n.attribute("Monitor").as_string()) == "0");
    CHECK(std::string(n.attribute("AutoSize").as_string()) == "1");
    CHECK(n.attribute("FullxLightsControl").empty());
    int networks = 0;
    for (auto child : n.children("network")) {
        (void)child;
        ++networks;
    }
    CHECK(networks == 3);
    CHECK(std::string(n.child("ExtraProperty").attribute("name").as_string()) == "k");
}

TEST_CASE("Controller legacy Active attribute", "[outputs]") {
    OutputManager om;
    auto off = Load(om, R"(<Controller Name="a" Active="0" Type="Null"><network NetworkType="NULL" Id="1" MaxChannels="5"/></Controller>)");
    auto on = Load(om, R"(<Controller Name="b" Active="1" Type="Null"><network NetworkType="NULL" Id="2" MaxChannels="5"/></Controller>)");
    CHECK_FALSE(off->IsActive());
    CHECK(on->IsActive());
}

TEST_CASE("Inactive controller disables its outputs", "[outputs]") {
    OutputManager om;
    auto c = Load(om, R"(<Controller Name="a" ActiveState="Inactive" Type="Ethernet" Protocol="E131" IP="1.2.3.4"><network NetworkType="E131" ComPort="1.2.3.4" BaudRate="1" MaxChannels="10"/></Controller>)");
    REQUIRE(c != nullptr);
    CHECK_FALSE(c->IsActive());
    int32_t start = 1;
    int nulls = 0;
    c->SetTransientData(start, nulls);
    CHECK_FALSE(c->IsEnabled());
    c->SetActive("Active");
    CHECK(c->IsActive());
    CHECK(c->IsEnabled());
    CHECK(c->GetLongDescription().rfind("INACTIVE", 0) == std::string::npos);
    c->SetActive("Inactive");
    CHECK(c->GetLongDescription().rfind("INACTIVE ", 0) == 0);
    CHECK_FALSE(c->GetFirstOutput()->IsEnabled());
}

TEST_CASE("xLights only controllers follow the application flag", "[outputs]") {
    OutputManager om;
    auto c = Load(om, R"(<Controller Name="a" ActiveState="xLights Only" Type="Null"><network NetworkType="NULL" Id="1" MaxChannels="5"/></Controller>)");
    const bool prev = IsxLights();
    SetIsxLights(true);
    CHECK(c->IsActive());
    SetIsxLights(false);
    CHECK_FALSE(c->IsActive());
    SetIsxLights(prev);
}

TEST_CASE("Default ethernet controller", "[outputs]") {
    OutputManager om;
    ControllerEthernet c(&om);
    CHECK(c.GetProtocol() == OUTPUT_E131);
    CHECK(c.GetOutputCount() == 1);
    CHECK(c.GetName() == "Ethernet_");
    CHECK(c.GetType() == "Ethernet");
    CHECK(c.IsManaged());
    CHECK(c.GetId() == -1);
    ControllerEthernet dup(&om, true);
    CHECK_FALSE(dup.IsManaged());
    ControllerEthernet noMgr(nullptr);
    CHECK(noMgr.GetName() == "Dummy");
}

TEST_CASE("Ethernet IP is cleaned and cascades to outputs", "[outputs]") {
    OutputManager om;
    ControllerEthernet c(&om);
    c.SetIP("010.000.000.007");
    CHECK(c.GetIP() == "10.0.0.7");
    CHECK(c.GetFirstOutput()->GetIP() == "10.0.0.7");
    CHECK(c.GetColumn2Label() == "10.0.0.7");
    CHECK(c.IsDirty());
    CHECK(c.CanPing());
    c.SetIP("MULTICAST");
    CHECK_FALSE(c.CanPing());
}

TEST_CASE("Ethernet FPP proxy ignores the controller's own address", "[outputs]") {
    OutputManager om;
    ControllerEthernet c(&om);
    c.SetIP("10.0.0.7");
    c.SetFPPProxy("10.0.0.7");
    CHECK(c.GetControllerFPPProxy().empty());
    c.SetFPPProxy("10.0.0.9");
    CHECK(c.GetControllerFPPProxy() == "10.0.0.9");
    CHECK(c.GetFPPProxy() == "10.0.0.9");
    CHECK(c.GetFirstOutput()->GetFPPProxyIP() == "10.0.0.9");
    CHECK(c.GetFirstOutput()->IsUsingFPPProxy());
}

TEST_CASE("Ethernet controller falls back to the global FPP proxy", "[outputs]") {
    OutputManager om;
    ControllerEthernet c(&om);
    c.SetIP("10.0.0.7");
    CHECK(c.GetFPPProxy().empty());
    om.SetGlobalFPPProxy("10.0.0.50");
    CHECK(c.GetFPPProxy() == "10.0.0.50");
    ControllerEthernet proxyItself(&om);
    proxyItself.SetIP("10.0.0.50");
    CHECK(proxyItself.GetFPPProxy().empty());
}

TEST_CASE("Ethernet proxyable protocols", "[outputs]") {
    OutputManager om;
    ControllerEthernet c(&om);
    CHECK(c.IsFPPProxyable());
    c.SetProtocol(OUTPUT_ARTNET);
    CHECK_FALSE(c.IsFPPProxyable());
    c.SetProtocol(OUTPUT_DDP);
    CHECK(c.IsFPPProxyable());
    c.SetProtocol(OUTPUT_PLAYER_ONLY);
    CHECK(c.IsFPPProxyable());
}

TEST_CASE("Ethernet id rules depend on the protocol", "[outputs]") {
    OutputManager om;
    ControllerEthernet c(&om);
    CHECK_FALSE(c.IsNeedsId());
    CHECK(c.GetId() == -1);
    c.SetProtocol(OUTPUT_DDP);
    CHECK(c.IsNeedsId());
    CHECK(c.GetId() != -1);
    c.SetId(77);
    CHECK(c.GetId() == 77);
    CHECK(dynamic_cast<DDPOutput*>(c.GetFirstOutput())->GetId() == 77);
}

TEST_CASE("Changing protocol replaces the outputs and keeps the layout", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    auto* e = dynamic_cast<ControllerEthernet*>(c.get());
    REQUIRE(e != nullptr);
    e->SetProtocol(OUTPUT_ARTNET);
    CHECK(e->GetProtocol() == OUTPUT_ARTNET);
    REQUIRE(e->GetOutputCount() == 3);
    for (const auto& o : e->GetOutputs()) {
        CHECK(o->GetType() == OUTPUT_ARTNET);
        CHECK(o->GetIP() == "10.1.2.3");
    }
    CHECK(e->GetOutput(0)->GetUniverse() == 1);
    CHECK(e->GetOutput(1)->GetUniverse() == 2);
    CHECK(e->GetOutput(2)->GetChannels() == 100);

    e->SetProtocol(OUTPUT_DDP);
    REQUIRE(e->GetOutputCount() == 1);
    CHECK(e->GetFirstOutput()->GetType() == OUTPUT_DDP);
    CHECK(e->GetChannels() == 1120);
    CHECK(e->GetFirstOutput()->GetIP() == "10.1.2.3");
    int32_t start = 1;
    int nulls = 0;
    e->SetTransientData(start, nulls);
    CHECK(e->GetFirstOutput()->IsSuppressDuplicateFrames());
}

TEST_CASE("Ethernet SetChannelSize adds and removes universes", "[outputs]") {
    OutputManager om;
    ControllerEthernet c(&om);
    c.SetIP("10.0.0.5");
    REQUIRE(c.GetOutputCount() == 1);
    CHECK(c.GetChannels() == 510);

    CHECK(c.SetChannelSize(1200));
    REQUIRE(c.GetOutputCount() == 3);
    int expectedUniverse = 1;
    for (const auto& o : c.GetOutputs()) {
        CHECK(o->GetUniverse() == expectedUniverse++);
        CHECK(o->GetChannels() == 510);
        CHECK(o->GetIP() == "10.0.0.5");
    }

    CHECK(c.SetChannelSize(300));
    CHECK(c.GetOutputCount() == 1);
    CHECK(c.AllSameSize());
}

TEST_CASE("Ethernet SetChannelSize on single output protocols resizes the output", "[outputs]") {
    OutputManager om;
    ControllerEthernet c(&om);
    c.SetProtocol(OUTPUT_DDP);
    CHECK(c.SetChannelSize(9000));
    CHECK(c.GetOutputCount() == 1);
    CHECK(c.GetChannels() == 9000);
}

TEST_CASE("Ethernet SetChannelSize with no outputs fails", "[outputs]") {
    OutputManager om;
    ControllerEthernet c(&om);
    c.ClearOutputs();
    CHECK_FALSE(c.SetChannelSize(100));
    CHECK(c.GetOutputCount() == 0);
    CHECK(c.GetChannels() == 0);
    CHECK(c.GetStartChannel() == 0);
    CHECK(c.GetEndChannel() == 0);
}

TEST_CASE("Ethernet AddOutput and RemoveTrailingOutputs", "[outputs]") {
    OutputManager om;
    ControllerEthernet c(&om);
    c.AddOutput();
    c.AddOutput();
    CHECK(c.GetOutputCount() == 3);
    c.RemoveTrailingOutputs(1);
    CHECK(c.GetOutputCount() == 1);
    c.RemoveTrailingOutputs(5);
    CHECK(c.GetOutputCount() == 1);
}

TEST_CASE("Ethernet AllSameSize detects mixed sizes", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    auto* e = dynamic_cast<ControllerEthernet*>(c.get());
    CHECK_FALSE(e->AllSameSize());
    e->GetOutput(2)->SetChannels(510);
    CHECK(e->AllSameSize());
}

TEST_CASE("Controller channel ranges after SomethingChanged", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    int32_t start = 1;
    int nulls = 0;
    c->SetTransientData(start, nulls);
    CHECK(start == 1121);
    CHECK(c->GetStartChannel() == 1);
    CHECK(c->GetEndChannel() == 1120);
    CHECK(c->GetOutput(1)->GetStartChannel() == 511);
    CHECK(c->GetOutput(2)->GetStartChannel() == 1021);

    int32_t rel = 0;
    Output* o = c->GetOutput(520, rel);
    REQUIRE(o != nullptr);
    CHECK(o->GetUniverse() == 2);
    CHECK(rel == 10);
    CHECK(c->GetOutput(1121, rel) == nullptr);
    CHECK(c->GetOutput(0, rel) == nullptr);
}

TEST_CASE("Controller GetOutput by index", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    CHECK(c->GetOutput(0)->GetUniverse() == 1);
    CHECK(c->GetOutput(2)->GetUniverse() == 3);
    CHECK(c->GetOutput(-1) == nullptr);
    CHECK(c->GetOutput(100) == nullptr);
}

TEST_CASE("Controller ContainsChannels treats its range as 1-based inclusive", "[outputs]") {
    // The controller spans channels 1-1120 (510 + 510 + 100). Both edges count.
    OutputManager om;
    auto c = Load(om, kEthXml);
    int32_t start = 1;
    int nulls = 0;
    c->SetTransientData(start, nulls);
    REQUIRE(c->GetStartChannel() == 1);
    REQUIRE(c->GetEndChannel() == 1120);
    CHECK(c->ContainsChannels(1, 10));
    CHECK(c->ContainsChannels(1000, 2000));
    CHECK(c->ContainsChannels(1120, 1120));  // last channel only
    CHECK(c->ContainsChannels(1, 1));        // first channel only
    CHECK_FALSE(c->ContainsChannels(1121, 1200));
}

TEST_CASE("Controller ContainsChannels edges with a controller that does not start at 1", "[outputs]") {
    // Callers holding a model's 0-based GetFirstChannel()/GetLastChannel() must add 1.
    // With the controller at 101-1220, a model on 0-based 0..100 ends on channel 101
    // (the controller's first) and one starting at 0-based 1220 is channel 1221 (past it).
    OutputManager om;
    auto c = Load(om, kEthXml);
    int32_t start = 101;
    int nulls = 0;
    c->SetTransientData(start, nulls);
    REQUIRE(c->GetStartChannel() == 101);
    REQUIRE(c->GetEndChannel() == 1220);
    CHECK_FALSE(c->ContainsChannels(1, 100));
    CHECK(c->ContainsChannels(0 + 1, 100 + 1));
    CHECK(c->ContainsChannels(1220, 1300));
    CHECK_FALSE(c->ContainsChannels(1220 + 1, 1300 + 1));
}

TEST_CASE("Controller dirty tracking includes outputs", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthPlain);
    CHECK_FALSE(c->IsDirty());
    c->GetFirstOutput()->SetChannels(77);
    CHECK(c->IsDirty());
    c->ClearDirty();
    CHECK_FALSE(c->IsDirty());
    c->SetDescription("changed");
    CHECK(c->IsDirty());
    c->ClearDirty();
    c->SetDescription("changed");
    CHECK_FALSE(c->IsDirty());
    c->SetName("other");
    CHECK(c->IsDirty());
}

TEST_CASE("Controller setters cascade suppress duplicate frames", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    c->SetSuppressDuplicateFrames(false);
    for (const auto& o : c->GetOutputs()) CHECK_FALSE(o->IsSuppressDuplicateFrames());
    c->SetSuppressDuplicateFrames(true);
    for (const auto& o : c->GetOutputs()) CHECK(o->IsSuppressDuplicateFrames());
}

TEST_CASE("Controller Enable toggles every output", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    c->Enable(false);
    CHECK_FALSE(c->IsEnabled());
    c->GetOutput(1)->Enable(true);
    CHECK(c->IsEnabled());
}

TEST_CASE("Controller temp disable only applies where supported", "[outputs]") {
    OutputManager om;
    auto eth = Load(om, kEthXml);
    CHECK(eth->CanTempDisable());
    eth->TempDisable(true);
    CHECK(eth->IsTempDisable());
    for (const auto& o : eth->GetOutputs()) CHECK(o->IsTempDisable());
    CHECK(eth->GetPingDescription().find("(Down)") != std::string::npos);

    auto nul = Load(om, R"(<Controller Name="n" Type="Null"><network NetworkType="NULL" Id="1" MaxChannels="5"/></Controller>)");
    CHECK_FALSE(nul->CanTempDisable());
    nul->TempDisable(true);
    CHECK_FALSE(nul->IsTempDisable());
}

TEST_CASE("Ethernet descriptions and labels", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    int32_t start = 1;
    int nulls = 0;
    c->SetTransientData(start, nulls);
    CHECK(c->GetLongDescription() == "Roof E131 10.1.2.3 (1-1120) the roof");
    CHECK(c->GetShortDescription() == "Roof E131 10.1.2.3 the roof");
    CHECK(c->GetColumn1Label() == "E131");
    CHECK(c->GetColumn3Label() == "1-3");
    CHECK(c->GetColumn4Label() == "1120 [1-1120]");
    CHECK(c->GetColumn5Label() == "FooVendor");
    CHECK(c->GetColumn6Label() == "BarModel");
    CHECK(c->GetColumn7Label() == "Baz");
    CHECK(c->GetColumn8Label() == "Active");
    CHECK(c->GetColumn11Label() == "the roof");
    CHECK(c->GetColumn13Label().empty());
    CHECK(c->GetColumn14Label().empty());
    CHECK(c->GetColumn17Label().empty());
    CHECK(c->GetSortName() == "Roof");
}

TEST_CASE("Ethernet export row", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    int32_t start = 1;
    int nulls = 0;
    c->SetTransientData(start, nulls);
    const std::string row = c->GetExport();
    CHECK(row.rfind("Roof,1,1120,FooVendor BarModel - Baz,E131,10.1.2.3,,,\"the roof\",1-3,1120,", 0) == 0);
    CHECK(row.find("SuppressDuplicates") != std::string::npos);
    CHECK(row.find("AutoLayout") != std::string::npos);
}

TEST_CASE("Ethernet channel mapping text", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    int32_t start = 1;
    int nulls = 0;
    c->SetTransientData(start, nulls);
    const std::string m = c->GetChannelMapping(520);
    CHECK(m.find("Type: E131") != std::string::npos);
    CHECK(m.find("Name: Roof") != std::string::npos);
    CHECK(m.find("IP: 10.1.2.3") != std::string::npos);
    CHECK(m.find("Universe: 2") != std::string::npos);
    CHECK(m.find("Channel: 10") != std::string::npos);
    CHECK(c->GetChannelMapping(5000).find("INVALID") != std::string::npos);
}

TEST_CASE("Controller JSON data", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    int32_t start = 1;
    int nulls = 0;
    c->SetTransientData(start, nulls);
    const std::string j = c->GetJSONData();
    CHECK(j.find("\"name\":\"Roof\"") != std::string::npos);
    CHECK(j.find("\"type\":\"Ethernet\"") != std::string::npos);
    CHECK(j.find("\"protocol\":\"E131\"") != std::string::npos);
    CHECK(j.find("\"startchannel\":1,") != std::string::npos);
    CHECK(j.find("\"channels\":1120") != std::string::npos);
    CHECK(j.find("\"ip\":\"10.1.2.3\"") != std::string::npos);
    CHECK(j.find("\"active\":true") != std::string::npos);
    CHECK(j.find("\"controllercap\":{}") != std::string::npos);
    CHECK(c->GetCapJSONData() == "{}");
}

TEST_CASE("Controller JSON data escapes quotes in names", "[outputs]") {
    OutputManager om;
    auto c = Load(om, R"(<Controller Name="Say &quot;hi&quot;" Type="Null"><network NetworkType="NULL" Id="1" MaxChannels="5"/></Controller>)");
    CHECK(c->GetJSONData().find("\"name\":\"Say \\\"hi\\\"\"") != std::string::npos);
}

TEST_CASE("Ethernet Copy is independent but equal", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    std::unique_ptr<Controller> copy(c->Copy(&om));
    REQUIRE(copy != nullptr);
    CHECK(SaveToString(*copy) == SaveToString(*c));
    CHECK(*copy == *c);
    copy->GetFirstOutput()->SetChannels(1);
    CHECK(c->GetFirstOutput()->GetChannels() == 510);
    CHECK(copy->GetOutputCount() == c->GetOutputCount());
}

TEST_CASE("Controller UpdateFrom reports changes", "[outputs]") {
    OutputManager om;
    auto a = Load(om, kEthPlain);
    auto b = Load(om, kEthPlain);
    CHECK_FALSE(a->UpdateFrom(b.get()));
    CHECK_FALSE(a->IsDirty());

    dynamic_cast<ControllerEthernet*>(b.get())->SetIP("10.9.9.9");
    CHECK(a->UpdateFrom(b.get()));
    CHECK(a->GetIP() == "10.9.9.9");
    CHECK_FALSE(a->UpdateFrom(b.get()));
}

TEST_CASE("Ethernet UpdateFrom marks the controller dirty", "[outputs]") {
    // ControllerEthernet::UpdateFrom only sets its own fields; _dirty is set solely by the base
    // class when a base field differs, so an IP-only change is never flagged for saving.
    OutputManager om;
    auto a = Load(om, kEthPlain);
    auto b = Load(om, kEthPlain);
    dynamic_cast<ControllerEthernet*>(b.get())->SetIP("10.9.9.9");
    CHECK(a->UpdateFrom(b.get()));
    CHECK(a->IsDirty());
}

TEST_CASE("Controller UpdateFrom copies outputs when they differ", "[outputs]") {
    OutputManager om;
    auto a = Load(om, kEthXml);
    auto b = Load(om, kEthXml);
    b->GetOutput(2)->SetChannels(50);
    CHECK(a->UpdateFrom(b.get()));
    CHECK(a->GetChannels() == 510 + 510 + 50);
    b->DeleteAllOutputs();
    CHECK(a->UpdateFrom(b.get()));
    CHECK(a->GetOutputCount() == 0);
}

TEST_CASE("Controller vendor model variant setters", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    c->SetVendor("A");
    c->SetModel("");
    c->SetVariant("");
    CHECK(c->GetVMV() == "A");
    c->SetModel("M");
    CHECK(c->GetVMV() == "A M");
    c->SetVariant("V");
    CHECK(c->GetVMV() == "A M - V");
}

TEST_CASE("Ethernet SetPriority cascades to E131 outputs", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    auto* e = dynamic_cast<ControllerEthernet*>(c.get());
    e->SetPriority(120);
    for (const auto& o : e->GetOutputs()) {
        CHECK(dynamic_cast<E131Output*>(o)->GetPriority() == 120);
    }
}

TEST_CASE("Controller GetId and unique ids come from the manager", "[outputs]") {
    OutputManager om;
    auto a = Load(om, R"(<Controller Id="3" Name="a" Type="Null"><network NetworkType="NULL" Id="3" MaxChannels="5"/></Controller>)");
    CHECK(a->GetId() == 3);
    om.AddController(a.release());
    ControllerNull fresh(&om);
    CHECK(fresh.GetId() == 1);
    fresh.SetId(3);
    fresh.EnsureUniqueId();
    CHECK(fresh.GetId() == 1);
}

TEST_CASE("EnsureUniqueId keeps the output id in step", "[outputs]") {
    // Controller::EnsureUniqueId assigns _id directly instead of calling the virtual SetId, so
    // the Null/DDP/serial output keeps the stale id.
    OutputManager om;
    auto a = Load(om, R"(<Controller Id="1" Name="a" Type="Null"><network NetworkType="NULL" Id="1" MaxChannels="5"/></Controller>)");
    om.AddController(a.release());
    ControllerNull fresh(&om);
    fresh.SetId(1);
    fresh.EnsureUniqueId();
    CHECK(fresh.GetId() != 1);
    CHECK(dynamic_cast<NullOutput*>(fresh.GetFirstOutput())->GetId() == fresh.GetId());
}

TEST_CASE("Null controller", "[outputs]") {
    OutputManager om;
    ControllerNull c(&om);
    CHECK(c.GetName() == "Null_");
    CHECK(c.GetOutputCount() == 1);
    CHECK(c.GetFirstOutput()->GetType() == OUTPUT_NULL);
    c.GetFirstOutput()->SetChannels(25);
    c.SetId(42);
    CHECK(c.GetId() == 42);
    CHECK(dynamic_cast<NullOutput*>(c.GetFirstOutput())->GetId() == 42);
    int32_t start = 10;
    int nulls = 0;
    c.SetTransientData(start, nulls);
    CHECK(start == 35);
    CHECK(c.GetLongDescription() == "Null_  NULL (10-34) ");
    CHECK(c.GetShortDescription() == "Null_  NULL");
    CHECK(c.GetChannelMapping(12).find("Channel: 3") != std::string::npos);
    CHECK(c.GetExport().rfind("Null_,10,34,", 0) == 0);
}

TEST_CASE("Null controller XML round trip", "[outputs]") {
    OutputManager om;
    auto c = Load(om, R"(<Controller Id="9" Name="Gap" Description="spacer" Type="Null" ActiveState="Inactive"><network NetworkType="NULL" Id="9" MaxChannels="30" Enabled="No"/></Controller>)");
    REQUIRE(c != nullptr);
    CHECK(c->GetChannels() == 30);
    CHECK_FALSE(c->IsActive());
    CHECK(c->GetLongDescription().rfind("INACTIVE Gap", 0) == 0);
    const std::string once = SaveToString(*c);
    pugi::xml_document doc;
    REQUIRE(doc.load_string(once.c_str()));
    OutputManager om2;
    std::unique_ptr<Controller> again(Controller::Create(&om2, doc.first_child(), ""));
    CHECK(SaveToString(*again) == once);
}

TEST_CASE("Serial controller encodes prefix and postfix escapes", "[outputs]") {
    OutputManager om;
    auto c = Load(om, R"(<Controller Id="1" Name="s" Type="Serial" Protocol="DMX" Port="COM3" Speed="250000" ActiveState="Inactive"><network NetworkType="DMX" Id="1" ComPort="COM3" BaudRate="250000" MaxChannels="512"/></Controller>)");
    auto* s = dynamic_cast<ControllerSerial*>(c.get());
    REQUIRE(s != nullptr);
    CHECK(s->Encode("abc") == std::vector<uint8_t>{ 'a', 'b', 'c' });
    CHECK(s->Encode("\\n\\r\\t") == std::vector<uint8_t>{ '\n', '\r', '\t' });
    CHECK(s->Encode("\\\\") == std::vector<uint8_t>{ '\\' });
    CHECK(s->Encode("\\x01\\xFf") == std::vector<uint8_t>{ 0x01, 0xFF });
    CHECK(s->Encode("a\\xZ1").size() == 2);
    CHECK(s->Encode("\\x4").empty());
    CHECK(s->Encode("\\q").empty());
    CHECK(s->Encode("").empty());
}

TEST_CASE("Serial controller loads and saves its settings", "[outputs]") {
    OutputManager om;
    auto c = Load(om, R"(<Controller Id="1" Name="s" Type="Serial" Protocol="DMX" Port="COM3" Speed="250000" Prefix="\x01" Postfix="\x02\x03" ActiveState="Inactive"><network NetworkType="DMX" Id="1" ComPort="COM3" BaudRate="250000" MaxChannels="512"/></Controller>)");
    auto* s = dynamic_cast<ControllerSerial*>(c.get());
    REQUIRE(s != nullptr);
    CHECK(s->GetPort() == "COM3");
    CHECK(s->GetSpeed() == 250000);
    CHECK(s->GetProtocol() == OUTPUT_DMX);
    CHECK(s->GetPreFix() == std::vector<uint8_t>{ 0x01 });
    CHECK(s->GetPostFix() == std::vector<uint8_t>{ 0x02, 0x03 });
    CHECK(s->GetSaveablePreFix() == "\\x01");
    CHECK(s->GetColumn2Label() == "COM3:250000");
    CHECK(s->GetColumn1Label() == OUTPUT_DMX);
    CHECK_FALSE(s->IsManaged());
    CHECK(s->GetIP().empty());
    CHECK(s->GetSerialOutput() != nullptr);
    CHECK(s->GetChannels() == 512);

    const std::string once = SaveToString(*c);
    pugi::xml_document doc;
    REQUIRE(doc.load_string(once.c_str()));
    OutputManager om2;
    std::unique_ptr<Controller> again(Controller::Create(&om2, doc.first_child(), ""));
    CHECK(SaveToString(*again) == once);
}

TEST_CASE("Serial controller setters update the output", "[outputs]") {
    OutputManager om;
    auto c = Load(om, R"(<Controller Id="1" Name="s" Type="Serial" Protocol="DMX" Port="COM3" Speed="250000" ActiveState="Inactive"><network NetworkType="DMX" Id="1" ComPort="COM3" BaudRate="250000" MaxChannels="512"/></Controller>)");
    auto* s = dynamic_cast<ControllerSerial*>(c.get());
    REQUIRE(s != nullptr);
    s->SetPort("COM9");
    CHECK(s->GetPort() == "COM9");
    CHECK(s->GetFirstOutput()->GetCommPort() == "COM9");
    s->SetSpeed(9600);
    CHECK(s->GetFirstOutput()->GetBaudRate() == 250000);
    s->SetChannels(64);
    CHECK(s->GetChannels() == 64);
    CHECK(s->IsDirty());
}

TEST_CASE("Serial controller protocol change swaps the output type", "[outputs]") {
    OutputManager om;
    auto c = Load(om, R"(<Controller Id="1" Name="s" Type="Serial" Protocol="DMX" Port="COM3" Speed="250000" ActiveState="Inactive"><network NetworkType="DMX" Id="1" ComPort="COM3" BaudRate="250000" MaxChannels="100"/></Controller>)");
    auto* s = dynamic_cast<ControllerSerial*>(c.get());
    REQUIRE(s != nullptr);
    s->SetProtocol(OUTPUT_RENARD);
    CHECK(s->GetProtocol() == OUTPUT_RENARD);
    CHECK(s->GetSerialOutput()->GetType() == OUTPUT_RENARD);
    CHECK(s->IsDirty());
    CHECK(s->SupportsAutoSize());
    s->SetProtocol(OUTPUT_xxxSERIAL);
    CHECK_FALSE(s->SupportsAutoSize());
}

TEST_CASE("Serial controller Copy and UpdateFrom", "[outputs]") {
    OutputManager om;
    const char* xml = R"(<Controller Id="1" Name="s" Type="Serial" Protocol="DMX" Port="COM3" Speed="250000" ActiveState="Inactive"><network NetworkType="DMX" Id="1" ComPort="COM3" BaudRate="250000" MaxChannels="100"/></Controller>)";
    auto a = Load(om, xml);
    auto b = Load(om, xml);
    CHECK_FALSE(a->UpdateFrom(b.get()));
    dynamic_cast<ControllerSerial*>(b.get())->SetPort("COM7");
    CHECK(a->UpdateFrom(b.get()));
    CHECK(dynamic_cast<ControllerSerial*>(a.get())->GetPort() == "COM7");

    std::unique_ptr<Controller> copy(a->Copy(&om));
    REQUIRE(copy != nullptr);
    CHECK(copy->GetName() == "s");
    CHECK(copy->GetChannels() == 100);
}

TEST_CASE("Serial controller Copy keeps the port and speed", "[outputs]") {
    // The copy constructor calls SetPort/SetSpeed/SetPrefix before _serialOutput is assigned,
    // and all of them do nothing while it is null, so the copy has an empty port and speed 0.
    OutputManager om;
    auto a = Load(om, R"(<Controller Id="1" Name="s" Type="Serial" Protocol="DMX" Port="COM3" Speed="250000" Prefix="\x01" ActiveState="Inactive"><network NetworkType="DMX" Id="1" ComPort="COM3" BaudRate="250000" MaxChannels="100"/></Controller>)");
    std::unique_ptr<Controller> copy(a->Copy(&om));
    auto* cs = dynamic_cast<ControllerSerial*>(copy.get());
    REQUIRE(cs != nullptr);
    CHECK(cs->GetPort() == "COM3");
    CHECK(cs->GetSpeed() == 250000);
    CHECK(cs->GetSaveablePreFix() == "\\x01");
}

TEST_CASE("Serial controller speed applies to protocols that allow it", "[outputs]") {
    OutputManager om;
    auto c = Load(om, R"(<Controller Id="1" Name="s" Type="Serial" Protocol="Renard" Port="COM3" Speed="19200" ActiveState="Inactive"><network NetworkType="Renard" Id="1" ComPort="COM3" BaudRate="19200" MaxChannels="100"/></Controller>)");
    auto* s = dynamic_cast<ControllerSerial*>(c.get());
    REQUIRE(s != nullptr);
    CHECK(s->GetSpeed() == 19200);
    s->SetSpeed(57600);
    CHECK(s->GetSpeed() == 57600);
    CHECK(s->GetFirstOutput()->GetBaudRate() == 57600);
}

TEST_CASE("Controller vendor model conversion table", "[outputs]") {
    std::string vendor, model, variant;
    Controller::ConvertOldTypeToVendorModel("F16v3", vendor, model, variant);
    CHECK(vendor == "Falcon");
    CHECK(model == "F16V3");
    CHECK(variant == "Two Expansion Boards");
    Controller::ConvertOldTypeToVendorModel("AlphaPix 4", vendor, model, variant);
    CHECK(vendor == "Holiday Coro");
    CHECK(model == "AlphaPix 4");
    CHECK(variant.empty());
    Controller::ConvertOldTypeToVendorModel("Totally Unknown", vendor, model, variant);
    CHECK(vendor.empty());
    CHECK(model.empty());
    CHECK(variant.empty());
}

TEST_CASE("Controller GetOutput rejects an index equal to the size", "[outputs]") {
    OutputManager om;
    auto c = Load(om, kEthXml);
    const int n = c->GetOutputCount();
    REQUIRE(n > 0);
    CHECK(c->GetOutput(n - 1) != nullptr);
    CHECK(c->GetOutput(n) == nullptr);
    CHECK(c->GetOutput(-1) == nullptr);
}
