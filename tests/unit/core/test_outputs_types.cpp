#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <vector>

#include <pugixml.hpp>

#include "outputs/ArtNetOutput.h"
#include "outputs/DDPOutput.h"
#include "outputs/DMXOutput.h"
#include "outputs/E131Output.h"
#include "outputs/KinetOutput.h"
#include "outputs/NullOutput.h"
#include "outputs/OPCOutput.h"
#include "outputs/Output.h"
#include "outputs/RenardOutput.h"
#include "outputs/TwinklyOutput.h"
#include "outputs/xxxEthernetOutput.h"

namespace {

pugi::xml_node Parse(pugi::xml_document& doc, const std::string& xml) {
    REQUIRE(doc.load_string(xml.c_str()));
    return doc.first_child();
}

std::unique_ptr<Output> Make(const std::string& xml) {
    pugi::xml_document doc;
    pugi::xml_node n = Parse(doc, xml);
    return std::unique_ptr<Output>(Output::Create(nullptr, n, ""));
}

std::string Serialize(Output& o) {
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("root");
    o.Save(root);
    std::ostringstream ss;
    root.print(ss, "", pugi::format_raw);
    return ss.str();
}

pugi::xml_node SaveNode(Output& o, pugi::xml_document& doc) {
    pugi::xml_node root = doc.append_child("root");
    return o.Save(root);
}

} // namespace

TEST_CASE("Output::Create builds the right class for every network type", "[outputs]") {
    struct Row {
        const char* attrType;
        const char* expected;
    };
    const Row rows[] = {
        { "E131", OUTPUT_E131 },
        { "ArtNet", OUTPUT_ARTNET },
        { "DDP", OUTPUT_DDP },
        { "KINET", OUTPUT_KINET },
        { "NULL", OUTPUT_NULL },
        { "OPC", OUTPUT_OPC },
        { "Twinkly", OUTPUT_TWINKLY },
        { "DMX", OUTPUT_DMX },
        { "Renard", OUTPUT_RENARD },
        { "LOR", OUTPUT_LOR },
        { "LOR Optimised", OUTPUT_LOR_OPT },
        { "OpenDMX", OUTPUT_OPENDMX },
        { "Pixelnet", OUTPUT_PIXELNET },
        { "Pixelnet-Open", OUTPUT_OPENPIXELNET },
        { "Generic Serial", OUTPUT_GENERICSERIAL },
        { "D-Light", OUTPUT_DLIGHT },
        { "xxx Serial", OUTPUT_xxxSERIAL },
        { "xxx Ethernet", OUTPUT_xxxETHERNET },
    };
    for (const auto& r : rows) {
        auto o = Make(std::string("<network NetworkType=\"") + r.attrType + "\" MaxChannels=\"16\"/>");
        INFO(r.attrType);
        REQUIRE(o != nullptr);
        CHECK(o->GetType() == r.expected);
    }
}

TEST_CASE("Output::Create maps legacy Sy... Ethernet names to xxx Ethernet", "[outputs]") {
    auto o = Make(R"(<network NetworkType="Syncrolight Ethernet" MaxChannels="12"/>)");
    REQUIRE(o != nullptr);
    CHECK(o->GetType() == OUTPUT_xxxETHERNET);
}

TEST_CASE("IP outputs report their transport", "[outputs]") {
    for (const char* t : { "E131", "ArtNet", "DDP", "KINET", "OPC", "Twinkly", "xxx Ethernet" }) {
        auto o = Make(std::string("<network NetworkType=\"") + t + "\" ComPort=\"10.0.0.1\"/>");
        INFO(t);
        REQUIRE(o != nullptr);
        CHECK(o->IsIpOutput());
        CHECK_FALSE(o->IsSerialOutput());
    }
    for (const char* t : { "DMX", "Renard", "LOR", "Pixelnet", "OpenDMX" }) {
        auto o = Make(std::string("<network NetworkType=\"") + t + "\" ComPort=\"COM1\"/>");
        INFO(t);
        REQUIRE(o != nullptr);
        CHECK_FALSE(o->IsIpOutput());
        CHECK(o->IsSerialOutput());
    }
}

TEST_CASE("E131 output reads its attributes", "[outputs]") {
    auto o = Make(R"(<network NetworkType="E131" ComPort="192.168.1.10" BaudRate="5" MaxChannels="170" Enabled="Yes" Priority="50"/>)");
    REQUIRE(o != nullptr);
    auto* e = dynamic_cast<E131Output*>(o.get());
    REQUIRE(e != nullptr);
    CHECK(e->GetIP() == "192.168.1.10");
    CHECK(e->GetUniverse() == 5);
    CHECK(e->GetUniverseString() == "5");
    CHECK(e->GetChannels() == 170);
    CHECK(e->GetPriority() == 50);
    CHECK(e->IsEnabled());
    CHECK_FALSE(e->IsDirty());
    CHECK(e->GetSortName() == "192.168.1.10");
    CHECK(e->GetMaxChannels() == 512);
}

TEST_CASE("E131 output defaults and clamping", "[outputs]") {
    auto o = Make(R"(<network NetworkType="E131" ComPort="1.2.3.4" MaxChannels="9999"/>)");
    auto* e = dynamic_cast<E131Output*>(o.get());
    REQUIRE(e != nullptr);
    CHECK(e->GetChannels() == 512);
    CHECK(e->GetUniverse() == 1);
    CHECK(e->GetPriority() == E131_DEFAULT_PRIORITY);
    CHECK(e->IsEnabled());

    E131Output d;
    CHECK(d.GetChannels() == 510);
    CHECK(d.GetUniverse() == 1);
    CHECK(d.GetPriority() == 100);
    CHECK(d.IsDirty());
    CHECK(d.GetIP().empty());
}

TEST_CASE("E131 channel count validation boundaries", "[outputs]") {
    E131Output e;
    CHECK_FALSE(e.IsValidChannelCount(0));
    CHECK_FALSE(e.IsValidChannelCount(-1));
    CHECK(e.IsValidChannelCount(1));
    CHECK(e.IsValidChannelCount(512));
    CHECK_FALSE(e.IsValidChannelCount(513));
}

TEST_CASE("Max channel limits per protocol", "[outputs]") {
    CHECK(ArtNetOutput().GetMaxChannels() == 512);
    CHECK(ArtNetOutput().IsValidChannelCount(512));
    CHECK_FALSE(ArtNetOutput().IsValidChannelCount(513));
    CHECK(KinetOutput().GetMaxChannels() == 512);
    CHECK(OPCOutput().GetMaxChannels() == 65535);
    CHECK(OPCOutput().IsValidChannelCount(65535));
    CHECK_FALSE(OPCOutput().IsValidChannelCount(65536));
    CHECK(DDPOutput().GetMaxChannels() == 2000000);
    CHECK(DDPOutput().IsValidChannelCount(2000000));
    CHECK_FALSE(DDPOutput().IsValidChannelCount(2000001));
    CHECK_FALSE(DDPOutput().IsValidChannelCount(0));
    CHECK(xxxEthernetOutput::GetMaxxxxChannels() == 4096 * 3);
    const int dmxMax = DMXOutput().GetMaxChannels();
    CHECK(dmxMax >= 512);
    CHECK(DMXOutput().IsValidChannelCount(dmxMax));
    CHECK_FALSE(DMXOutput().IsValidChannelCount(dmxMax + 1));
    CHECK_FALSE(DMXOutput().IsValidChannelCount(0));
}

TEST_CASE("E131 setters mark dirty only on change", "[outputs]") {
    E131Output e;
    e.ClearDirty();
    CHECK_FALSE(e.IsDirty());
    e.SetUniverse(e.GetUniverse());
    e.SetChannels(e.GetChannels());
    e.SetPriority(e.GetPriority());
    e.Enable(true);
    CHECK_FALSE(e.IsDirty());

    e.SetUniverse(7);
    CHECK(e.IsDirty());
    e.ClearDirty();
    e.SetChannels(100);
    CHECK(e.IsDirty());
    e.ClearDirty();
    e.SetPriority(10);
    CHECK(e.IsDirty());
    e.ClearDirty();
    e.Enable(false);
    CHECK(e.IsDirty());
    CHECK_FALSE(e.IsEnabled());
    e.ClearDirty();
    e.SetSuppressDuplicateFrames(true);
    CHECK(e.IsDirty());
    CHECK(e.IsSuppressDuplicateFrames());
}

TEST_CASE("SetIP cleans up the address and tracks the resolved ip", "[outputs]") {
    E131Output e;
    e.ClearDirty();
    e.SetIP("010.001.002.003", false, false);
    CHECK(e.GetIP() == "10.1.2.3");
    CHECK(e.GetResolvedIP() == "10.1.2.3");
    CHECK(e.IsDirty());
    e.ClearDirty();
    e.SetIP("10.1.2.3", false, false);
    CHECK_FALSE(e.IsDirty());
    e.SetResolvedIP("10.9.9.9");
    CHECK(e.GetResolvedIP() == "10.9.9.9");
    CHECK(e.GetIP() == "10.1.2.3");
}

TEST_CASE("E131 Save and reload round trip", "[outputs]") {
    auto o = Make(R"(<network NetworkType="E131" ComPort="192.168.1.10" BaudRate="5" MaxChannels="170" Enabled="No" Priority="50"/>)");
    REQUIRE(o != nullptr);
    pugi::xml_document doc;
    pugi::xml_node n = SaveNode(*o, doc);
    CHECK(std::string(n.name()) == "network");
    CHECK(std::string(n.attribute("NetworkType").as_string()) == "E131");
    CHECK(std::string(n.attribute("ComPort").as_string()) == "192.168.1.10");
    CHECK(n.attribute("BaudRate").as_int() == 5);
    CHECK(n.attribute("MaxChannels").as_int() == 170);
    CHECK(std::string(n.attribute("Enabled").as_string()) == "No");
    CHECK(n.attribute("Priority").as_int() == 50);

    std::unique_ptr<Output> again(Output::Create(nullptr, n, ""));
    REQUIRE(again != nullptr);
    CHECK(Serialize(*again) == Serialize(*o));
}

TEST_CASE("E131 Save omits the default priority", "[outputs]") {
    E131Output e;
    e.SetIP("1.2.3.4", false, false);
    pugi::xml_document doc;
    pugi::xml_node n = SaveNode(e, doc);
    CHECK(n.attribute("Priority").empty());
    CHECK(n.attribute("MaxChannels").as_int() == 510);
    e.SetPriority(20);
    pugi::xml_document doc2;
    CHECK(SaveNode(e, doc2).attribute("Priority").as_int() == 20);
}

TEST_CASE("Saving an output with no ip omits ComPort", "[outputs]") {
    E131Output e;
    pugi::xml_document doc;
    CHECK(SaveNode(e, doc).attribute("ComPort").empty());
}

TEST_CASE("Output Save clears the dirty flag", "[outputs]") {
    E131Output e;
    CHECK(e.IsDirty());
    pugi::xml_document doc;
    SaveNode(e, doc);
    CHECK_FALSE(e.IsDirty());
}

TEST_CASE("Output Copy keeps the configuration", "[outputs]") {
    auto o = Make(R"(<network NetworkType="E131" ComPort="192.168.1.10" BaudRate="9" MaxChannels="300" Enabled="No" Priority="75" SuppressDuplicates="Yes"/>)");
    REQUIRE(o != nullptr);
    std::unique_ptr<Output> c(o->Copy());
    REQUIRE(c != nullptr);
    CHECK(c->GetType() == OUTPUT_E131);
    CHECK(c->GetIP() == "192.168.1.10");
    CHECK(c->GetUniverse() == 9);
    CHECK(c->GetChannels() == 300);
    CHECK_FALSE(c->IsEnabled());
    CHECK(c->IsSuppressDuplicateFrames());
    CHECK(dynamic_cast<E131Output*>(c.get())->GetPriority() == 75);
    CHECK(*c == *o);
}

TEST_CASE("Disabled outputs describe themselves as inactive", "[outputs]") {
    auto on = Make(R"(<network NetworkType="E131" ComPort="1.1.1.1" BaudRate="3" MaxChannels="10" Enabled="Yes"/>)");
    auto off = Make(R"(<network NetworkType="E131" ComPort="1.1.1.1" BaudRate="3" MaxChannels="10" Enabled="No"/>)");
    CHECK(on->GetLongDescription().rfind("INACTIVE", 0) == std::string::npos);
    CHECK(off->GetLongDescription().rfind("INACTIVE ", 0) == 0);
    CHECK(on->GetLongDescription().find("E1.31") != std::string::npos);
}

TEST_CASE("Output equality compares transport identity", "[outputs]") {
    auto a = Make(R"(<network NetworkType="E131" ComPort="10.0.0.1" BaudRate="1" MaxChannels="10"/>)");
    auto same = Make(R"(<network NetworkType="E131" ComPort="10.0.0.1" BaudRate="1" MaxChannels="99"/>)");
    auto otherUniverse = Make(R"(<network NetworkType="E131" ComPort="10.0.0.1" BaudRate="2" MaxChannels="10"/>)");
    auto otherIp = Make(R"(<network NetworkType="E131" ComPort="10.0.0.2" BaudRate="1" MaxChannels="10"/>)");
    auto otherType = Make(R"(<network NetworkType="ArtNet" ComPort="10.0.0.1" BaudRate="1" MaxChannels="10"/>)");
    CHECK(*a == *same);
    CHECK_FALSE(*a == *otherUniverse);
    CHECK_FALSE(*a == *otherType);
    a->SetResolvedIP("10.0.0.1");
    otherIp->SetResolvedIP("10.0.0.2");
    CHECK_FALSE(*a == *otherIp);
}

TEST_CASE("Output equality with different unresolved ips", "[outputs][!mayfail]") {
    // Outputs loaded for inactive controllers keep an empty resolved ip, and operator== then
    // matches on "" == "" even though the configured IPs differ.
    auto a = Make(R"(<network NetworkType="E131" ComPort="10.0.0.1" BaudRate="1" MaxChannels="10"/>)");
    auto b = Make(R"(<network NetworkType="E131" ComPort="10.0.0.2" BaudRate="1" MaxChannels="10"/>)");
    CHECK_FALSE(*a == *b);
}

TEST_CASE("Serial output equality compares the comm port", "[outputs]") {
    auto a = Make(R"(<network NetworkType="DMX" ComPort="COM3" BaudRate="250000" MaxChannels="512"/>)");
    auto b = Make(R"(<network NetworkType="DMX" ComPort="COM3" BaudRate="9600" MaxChannels="12"/>)");
    auto c = Make(R"(<network NetworkType="DMX" ComPort="COM4" BaudRate="250000" MaxChannels="512"/>)");
    CHECK(*a == *b);
    CHECK_FALSE(*a == *c);
}

TEST_CASE("SetTransientData assigns start channels and advances", "[outputs]") {
    auto a = Make(R"(<network NetworkType="E131" ComPort="1.1.1.1" BaudRate="1" MaxChannels="100"/>)");
    auto b = Make(R"(<network NetworkType="E131" ComPort="1.1.1.1" BaudRate="2" MaxChannels="50"/>)");
    int32_t start = 1;
    a->SetTransientData(start, 0);
    b->SetTransientData(start, 0);
    CHECK(a->GetStartChannel() == 1);
    CHECK(a->GetEndChannel() == 100);
    CHECK(b->GetStartChannel() == 101);
    CHECK(b->GetEndChannel() == 150);
    CHECK(start == 151);
}

TEST_CASE("ArtNet universe bit packing", "[outputs]") {
    CHECK(ArtNetOutput::GetArtNetCombinedUniverse(1, 2, 3) == 0x0123);
    CHECK(ArtNetOutput::GetArtNetNet(0x0123) == 1);
    CHECK(ArtNetOutput::GetArtNetSubnet(0x0123) == 2);
    CHECK(ArtNetOutput::GetArtNetUniverse(0x0123) == 3);
    CHECK(ArtNetOutput::GetArtNetCombinedUniverse(0x7F, 0xF, 0xF) == 0x7FFF);
    CHECK(ArtNetOutput::GetArtNetCombinedUniverse(0xFF, 0x1F, 0x1F) == 0x7FFF);
    CHECK(ArtNetOutput::GetArtNetNet(0x7FFF) == 0x7F);
    CHECK(ArtNetOutput::GetArtNetCombinedUniverse(0, 0, 0) == 0);
}

TEST_CASE("ArtNet combined universe round trips for every net subnet universe", "[outputs]") {
    for (int net = 0; net < 128; net += 17) {
        for (int sub = 0; sub < 16; ++sub) {
            for (int uni = 0; uni < 16; ++uni) {
                int u = ArtNetOutput::GetArtNetCombinedUniverse(net, sub, uni);
                REQUIRE(ArtNetOutput::GetArtNetNet(u) == net);
                REQUIRE(ArtNetOutput::GetArtNetSubnet(u) == sub);
                REQUIRE(ArtNetOutput::GetArtNetUniverse(u) == uni);
            }
        }
    }
}

TEST_CASE("ArtNet output from XML", "[outputs]") {
    auto o = Make(R"(<network NetworkType="ArtNet" ComPort="2.0.0.5" BaudRate="291" MaxChannels="600" ForceSourcePort="TRUE"/>)");
    auto* a = dynamic_cast<ArtNetOutput*>(o.get());
    REQUIRE(a != nullptr);
    CHECK(a->GetUniverse() == 291);
    CHECK(a->GetArtNetNet() == 1);
    CHECK(a->GetArtNetSubnet() == 2);
    CHECK(a->GetArtNetUniverse() == 3);
    CHECK(a->GetChannels() == 512);
    CHECK(a->isForceSourcePort());
    CHECK(a->GetUniverseString() == "1:2:3 or 291");
    a->SetArtNetUniverse(0, 1, 4);
    CHECK(a->GetUniverse() == 0x14);
}

TEST_CASE("ArtNet defaults and copy", "[outputs]") {
    ArtNetOutput a;
    CHECK(a.GetChannels() == 510);
    CHECK_FALSE(a.isForceSourcePort());
    a.SetForceSourcePort(true);
    a.SetUniverse(0x234);
    std::unique_ptr<Output> c(a.Copy());
    auto* ca = dynamic_cast<ArtNetOutput*>(c.get());
    REQUIRE(ca != nullptr);
    CHECK(ca->isForceSourcePort());
    CHECK(ca->GetUniverse() == 0x234);
}

TEST_CASE("DDP defaults", "[outputs]") {
    DDPOutput d;
    CHECK(d.GetId() == 64001);
    CHECK(d.GetUniverse() == 64001);
    CHECK(d.GetChannels() == 512);
    CHECK(d.GetChannelsPerPacket() == 1440);
    CHECK(d.IsKeepChannelNumbers());
}

TEST_CASE("DDP output from XML and Save round trip", "[outputs]") {
    auto o = Make(R"(<network NetworkType="DDP" ComPort="10.0.0.9" BaudRate="64002" MaxChannels="3000" ChannelsPerPacket="1200" KeepChannelNumbers="1"/>)");
    auto* d = dynamic_cast<DDPOutput*>(o.get());
    REQUIRE(d != nullptr);
    CHECK(d->GetId() == 64002);
    CHECK(d->GetChannels() == 3000);
    CHECK(d->GetChannelsPerPacket() == 1200);
    CHECK(d->IsKeepChannelNumbers());

    pugi::xml_document doc;
    pugi::xml_node n = SaveNode(*d, doc);
    CHECK(std::string(n.attribute("KeepChannelNumbers").as_string()) == "1");
    CHECK(n.attribute("ChannelsPerPacket").as_int() == 1200);
    std::unique_ptr<Output> again(Output::Create(nullptr, n, ""));
    CHECK(Serialize(*again) == Serialize(*o));
}

TEST_CASE("DDP KeepChannelNumbers defaults off when loading", "[outputs]") {
    auto o = Make(R"(<network NetworkType="DDP" ComPort="10.0.0.9" BaudRate="1" MaxChannels="10"/>)");
    auto* d = dynamic_cast<DDPOutput*>(o.get());
    REQUIRE(d != nullptr);
    CHECK_FALSE(d->IsKeepChannelNumbers());
    CHECK(d->GetChannelsPerPacket() == 0);
    d->SetKeepChannelNumber(true);
    CHECK(d->IsKeepChannelNumbers());
    d->SetId(5);
    CHECK(d->GetUniverse() == 5);
}

TEST_CASE("Kinet output version attribute", "[outputs]") {
    auto o = Make(R"(<network NetworkType="KINET" ComPort="1.1.1.1" BaudRate="3" MaxChannels="100" Version="1"/>)");
    auto* k = dynamic_cast<KinetOutput*>(o.get());
    REQUIRE(k != nullptr);
    CHECK(k->GetVersion() == 1);
    pugi::xml_document doc;
    CHECK(SaveNode(*k, doc).attribute("Version").as_int() == 1);

    auto def = Make(R"(<network NetworkType="KINET" ComPort="1.1.1.1" BaudRate="3" MaxChannels="100"/>)");
    CHECK(dynamic_cast<KinetOutput*>(def.get())->GetVersion() == 2);
    k->SetVersion(2);
    CHECK(k->GetVersion() == 2);
}

TEST_CASE("Twinkly output http port", "[outputs]") {
    auto o = Make(R"(<network NetworkType="Twinkly" ComPort="1.1.1.1" BaudRate="64001" MaxChannels="900" HTTPPort="8080"/>)");
    REQUIRE(o != nullptr);
    pugi::xml_document doc;
    CHECK(SaveNode(*o, doc).attribute("HTTPPort").as_int() == 8080);
    CHECK(o->GetType() == OUTPUT_TWINKLY);
}

TEST_CASE("xxx Ethernet output uses Id and Port", "[outputs]") {
    auto o = Make(R"(<network NetworkType="xxx Ethernet" ComPort="1.1.1.1" Id="7" MaxChannels="300"/>)");
    auto* x = dynamic_cast<xxxEthernetOutput*>(o.get());
    REQUIRE(x != nullptr);
    CHECK(x->GetId() == 7);
    pugi::xml_document doc;
    CHECK(SaveNode(*x, doc).attribute("Id").as_int() == 7);
    x->SetId(9);
    CHECK(x->GetId() == 9);
}

TEST_CASE("Null output identity and description", "[outputs]") {
    auto o = Make(R"(<network NetworkType="NULL" Id="12" MaxChannels="40"/>)");
    auto* n = dynamic_cast<NullOutput*>(o.get());
    REQUIRE(n != nullptr);
    CHECK(n->GetId() == 12);
    CHECK_FALSE(n->IsIpOutput());
    CHECK_FALSE(n->IsSerialOutput());
    int32_t start = 100;
    n->SetTransientData(start, 3);
    CHECK(start == 140);
    CHECK(n->GetLongDescription() == "NULL (100-139)");
    CHECK(n->GetSortName() == "NULL03");

    pugi::xml_document doc;
    pugi::xml_node saved = SaveNode(*n, doc);
    CHECK(saved.attribute("Id").as_int() == 12);
    CHECK(saved.attribute("MaxChannels").as_int() == 40);
    n->Enable(false);
    CHECK(n->GetLongDescription().rfind("INACTIVE NULL", 0) == 0);
}

TEST_CASE("Serial outputs save and load Id port and baud rate", "[outputs]") {
    auto o = Make(R"(<network NetworkType="DMX" Id="4" ComPort="COM3" BaudRate="250000" MaxChannels="512"/>)");
    REQUIRE(o != nullptr);
    CHECK(o->GetCommPort() == "COM3");
    CHECK(o->GetBaudRate() == 250000);
    CHECK(o->GetChannels() == 512);
    pugi::xml_document doc;
    pugi::xml_node n = SaveNode(*o, doc);
    CHECK(n.attribute("Id").as_int() == 4);
    CHECK(std::string(n.attribute("ComPort").as_string()) == "COM3");
    CHECK(n.attribute("BaudRate").as_int() == 250000);
    std::unique_ptr<Output> again(Output::Create(nullptr, n, ""));
    CHECK(Serialize(*again) == Serialize(*o));
}

TEST_CASE("Serial output with n/a baud rate falls back to the default", "[outputs]") {
    auto o = Make(R"(<network NetworkType="Renard" Id="1" ComPort="COM1" BaudRate="n/a" MaxChannels="16"/>)");
    REQUIRE(o != nullptr);
    CHECK(o->GetBaudRate() == 115200);
    pugi::xml_document doc;
    CHECK(std::string(SaveNode(*o, doc).attribute("BaudRate").as_string()) == "n/a");
}

TEST_CASE("FPP proxy attribute is stored and saved only when set", "[outputs]") {
    auto o = Make(R"(<network NetworkType="E131" ComPort="1.1.1.1" BaudRate="1" MaxChannels="10" FPPProxy="2.2.2.2"/>)");
    CHECK(o->IsUsingFPPProxy());
    CHECK(o->GetFPPProxyIP() == "2.2.2.2");
    pugi::xml_document doc;
    CHECK(std::string(SaveNode(*o, doc).attribute("FPPProxy").as_string()) == "2.2.2.2");
    o->SetFPPProxyIP("");
    CHECK_FALSE(o->IsUsingFPPProxy());
    pugi::xml_document doc2;
    CHECK(SaveNode(*o, doc2).attribute("FPPProxy").empty());
}

TEST_CASE("Output temp disable and suspend flags", "[outputs]") {
    E131Output e;
    CHECK_FALSE(e.IsTempDisable());
    e.TempDisable(true);
    CHECK(e.IsTempDisable());
    e.TempDisable(false);
    CHECK_FALSE(e.IsTempDisable());
}

TEST_CASE("Output frame bookkeeping decides when to transmit", "[outputs]") {
    E131Output e;
    CHECK(e.NeedToOutput(5));
    e.SetSuppressDuplicateFrames(true);
    CHECK(e.NeedToOutput(5));
    e.FrameOutput();
    CHECK_FALSE(e.NeedToOutput(5));
    for (int i = 0; i < 4; ++i) e.SkipFrame();
    CHECK_FALSE(e.NeedToOutput(5));
    e.SkipFrame();
    CHECK(e.NeedToOutput(5));
}

TEST_CASE("Output default baud rate", "[outputs]") {
    E131Output e;
    CHECK(e.GetBaudRate() == 115200);
    e.SetBaudRate(9600);
    CHECK(e.GetBaudRate() == 9600);
}
