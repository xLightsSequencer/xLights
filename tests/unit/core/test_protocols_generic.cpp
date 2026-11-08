#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "../support/ProtocolAccess.h"

using namespace protocols_test;

namespace {

template <class T>
std::string SaveOf(T& o) {
    return SaveToString(o);
}

} // namespace

// ---------------------------------------------------------------------------------------------
// Properties every serial output type shares
// ---------------------------------------------------------------------------------------------

#define PT_SERIAL_TYPES RenardOutput, PixelNetOutput, OpenPixelNetOutput, DMXOutput, OpenDMXOutput, GenericSerialOutput, xxxSerialOutput, LOROutput, LOROptimisedOutput

TEMPLATE_TEST_CASE("Serial output default state", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    CHECK(o.IsSerialOutput());
    CHECK_FALSE(o.IsIpOutput());
    CHECK(o.IsEnabled());
    CHECK(o.IsDirty());
    CHECK(o.GetStartChannel() == -1);
    CHECK(o.GetCommPort() == "NotConnected");
    CHECK(o.IsOk());
    CHECK(o.TxEmpty());
    CHECK(o.TxNonEmptyCount() == 0);
    CHECK_FALSE(o.IsUsingFPPProxy());
    CHECK_FALSE(o.IsSuppressDuplicateFrames());
}

TEMPLATE_TEST_CASE("Serial output channel count validity", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    const int32_t max = o.GetMaxChannels();
    CHECK(max > 0);
    CHECK_FALSE(o.IsValidChannelCount(0));
    CHECK_FALSE(o.IsValidChannelCount(-1));
    CHECK(o.IsValidChannelCount(1));
    CHECK(o.IsValidChannelCount(max));
    CHECK_FALSE(o.IsValidChannelCount(max + 1));
    CHECK_FALSE(o.IsValidChannelCount(2147483647));
    CHECK_FALSE(o.IsValidChannelCount(-2147483647 - 1));
}

TEMPLATE_TEST_CASE("Serial output setters track dirty state", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    o.ClearDirty();
    o.SetCommPort("NotConnected");
    CHECK_FALSE(o.IsDirty());
    o.SetCommPort("COM9");
    CHECK(o.IsDirty());
    o.ClearDirty();
    o.SetChannels(o.GetChannels());
    CHECK_FALSE(o.IsDirty());
    o.SetChannels(3);
    CHECK(o.IsDirty());
    o.ClearDirty();
    o.Enable(true);
    CHECK_FALSE(o.IsDirty());
    o.Enable(false);
    CHECK(o.IsDirty());
    o.ClearDirty();
    o.SetSuppressDuplicateFrames(false);
    CHECK_FALSE(o.IsDirty());
    o.SetSuppressDuplicateFrames(true);
    CHECK(o.IsDirty());
    o.ClearDirty();
    o.SetId(0);
    CHECK_FALSE(o.IsDirty());
    o.SetId(4);
    CHECK(o.IsDirty());
    CHECK(o.GetId() == 4);
    CHECK(o.GetUniverse() == 4);
}

TEMPLATE_TEST_CASE("Serial output Save writes the common attributes", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    o.SetCommPort("COM7");
    o.SetId(3);
    o.Enable(false);
    auto s = SaveOf(o);
    CHECK(s.find("NetworkType=\"" + o.GetType() + "\"") != std::string::npos);
    CHECK(s.find("ComPort=\"COM7\"") != std::string::npos);
    CHECK(s.find("Id=\"3\"") != std::string::npos);
    CHECK(s.find("Enabled=\"No\"") != std::string::npos);
    CHECK(s.find("MaxChannels=\"") != std::string::npos);
    CHECK(s.find("BaudRate=\"") != std::string::npos);
    CHECK_FALSE(o.IsDirty());
}

TEMPLATE_TEST_CASE("Serial output Save omits an empty comm port", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    o.SetCommPort("");
    CHECK(SaveOf(o).find("ComPort") == std::string::npos);
}

TEMPLATE_TEST_CASE("Serial output reload keeps the comm port and id", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    o.SetCommPort("/dev/ttyUSB3");
    o.SetId(9);
    pugi::xml_document doc;
    auto root = doc.append_child("root");
    o.Save(root);
    TestType again(root.first_child());
    CHECK(again.GetCommPort() == "/dev/ttyUSB3");
    CHECK(again.GetId() == 9);
    CHECK_FALSE(again.IsDirty());
}

TEMPLATE_TEST_CASE("Serial output Open of a disabled output succeeds without a port", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    o.Enable(false);
    o.SetCommPort("/definitely/not/a/port");
    CHECK(o.Open());
    o.Close();
}

TEMPLATE_TEST_CASE("Serial output Open of an empty comm port fails", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    o.SetCommPort("");
    CHECK_FALSE(o.Open());
    o.Close();
}

TEMPLATE_TEST_CASE("Serial output Open of a port that does not exist fails", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    o.SetChannels(4);
    o.SetCommPort("/definitely/not/a/port");
    CHECK_FALSE(o.Open());
    CHECK_FALSE(o.IsOk());
    o.EndFrame(0);
    o.Close();
}

TEMPLATE_TEST_CASE("Serial output Open and Close of NotConnected", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    o.SetChannels(4);
    o.SetCommPort("NotConnected");
    CHECK(o.Open());
    CHECK(o.IsOk());
    o.EndFrame(0);
    o.Close();
    o.Close();
}

TEMPLATE_TEST_CASE("Serial output description lists the channel range", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    o.SetChannels(8);
    int32_t start = 101;
    o.SetTransientData(start, 0);
    auto d = o.GetLongDescription();
    CHECK(d.find("[1-8]") != std::string::npos);
    CHECK(d.find("(101-108)") != std::string::npos);
    CHECK(d.rfind("INACTIVE", 0) != 0);
    o.Enable(false);
    CHECK(o.GetLongDescription().rfind("INACTIVE ", 0) == 0);
}

TEMPLATE_TEST_CASE("Serial output sort name is the port", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    o.SetCommPort("COM12");
    CHECK(o.GetSortName() == "COM12");
}

TEMPLATE_TEST_CASE("Serial output SetTransientData advances", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType a, b;
    a.SetChannels(10);
    b.SetChannels(20);
    int32_t start = 5;
    a.SetTransientData(start, 0);
    b.SetTransientData(start, 0);
    CHECK(a.GetStartChannel() == 5);
    CHECK(a.GetEndChannel() == 14);
    CHECK(b.GetStartChannel() == 15);
    CHECK(b.GetEndChannel() == 34);
    CHECK(start == 35);
}

TEMPLATE_TEST_CASE("Serial output copy is the same type and port", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    o.SetCommPort("COM4");
    o.SetChannels(6);
    std::unique_ptr<Output> c(o.Copy());
    REQUIRE(c != nullptr);
    CHECK(c->GetType() == o.GetType());
    CHECK(c->GetCommPort() == "COM4");
    // xxx Serial derives its channel count from the device layout, which a default object does not have
    if (o.GetType() != OUTPUT_xxxSERIAL) CHECK(c->GetChannels() == 6);
    CHECK(c.get() != &o);
    CHECK(*c == o);
}

TEMPLATE_TEST_CASE("Serial output StartFrame stores the timer and respects enabled", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    o.SetCommPort("NotConnected");
    o.SetChannels(4);
    o.Open();
    o.StartFrame(1234);
    CHECK(o.GetTimer() == 1234);
    o.Enable(false);
    o.StartFrame(9999);
    CHECK(o.GetTimer() == 1234);
    o.Close();
}

TEMPLATE_TEST_CASE("Serial output baud rate falls back to 115200 when unset", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    TestType o;
    PT(o, out_baud) = 0;
    CHECK(o.GetBaudRate() == 115200);
}

TEMPLATE_TEST_CASE("Serial output n/a baud rate round trips", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    pugi::xml_document doc;
    auto n = ParseNode(doc, "<network ComPort=\"COM1\" BaudRate=\"n/a\" Id=\"1\" MaxChannels=\"8\"/>");
    TestType o(n);
    PT(o, out_baud) = 0;
    CHECK(SaveOf(o).find("BaudRate=\"n/a\"") != std::string::npos);
}

TEMPLATE_TEST_CASE("Serial output missing attributes give defaults", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    pugi::xml_document doc;
    auto n = ParseNode(doc, "<network/>");
    TestType o(n);
    CHECK(o.GetCommPort() == "");
    CHECK(o.GetId() == 0);
    CHECK(o.IsEnabled());
    CHECK_FALSE(o.IsDirty());
}

TEMPLATE_TEST_CASE("Serial output tolerates a garbage id and baud rate", "[protocols][generic][serial]", PT_SERIAL_TYPES) {
    pugi::xml_document doc;
    auto n = ParseNode(doc, "<network ComPort=\"COM1\" BaudRate=\"fast\" Id=\"x\" MaxChannels=\"y\"/>");
    TestType o(n);
    CHECK(o.GetId() == 0);
    if (o.GetType() != OUTPUT_xxxSERIAL) CHECK(o.GetChannels() == 0);
    CHECK(o.GetCommPort() == "COM1");
}

// ---------------------------------------------------------------------------------------------
// Properties every IP output type shares
// ---------------------------------------------------------------------------------------------

#define PT_IP_TYPES E131Output, ArtNetOutput, DDPOutput, KinetOutput, OPCOutput, xxxEthernetOutput, TwinklyOutput
// Twinkly opens an HTTP session to the device, so the open/close tests leave it out
#define PT_IP_OPEN_TYPES E131Output, ArtNetOutput, DDPOutput, KinetOutput, OPCOutput, xxxEthernetOutput

TEMPLATE_TEST_CASE("IP output default state", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    CHECK(o.IsIpOutput());
    CHECK_FALSE(o.IsSerialOutput());
    CHECK(o.IsEnabled());
    CHECK(o.GetIP() == "");
    CHECK(o.GetResolvedIP() == "");
    CHECK(o.GetStartChannel() == -1);
    CHECK(o.IsOk());
    CHECK_FALSE(o.IsUsingFPPProxy());
    CHECK_FALSE(o.IsUsingForceLocalIP());
    CHECK(o.GetForceLocalIPToUse() == "");
}

TEMPLATE_TEST_CASE("IP output channel count validity", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    const int32_t max = o.GetMaxChannels();
    CHECK(max >= 512);
    CHECK_FALSE(o.IsValidChannelCount(0));
    CHECK_FALSE(o.IsValidChannelCount(-1));
    CHECK(o.IsValidChannelCount(1));
    CHECK(o.IsValidChannelCount(max));
    CHECK_FALSE(o.IsValidChannelCount(max + 1));
    CHECK_FALSE(o.IsValidChannelCount(-2147483647 - 1));
}

TEMPLATE_TEST_CASE("IP output SetIP cleans the address and tracks dirty", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    o.ClearDirty();
    o.SetIP("", false, false);
    CHECK_FALSE(o.IsDirty());
    o.SetIP("  10.1.2.3  ", false, false);
    CHECK(o.GetIP() == "10.1.2.3");
    CHECK(o.IsDirty());
    o.ClearDirty();
    o.SetIP("10.1.2.3", false, false);
    CHECK_FALSE(o.IsDirty());
    CHECK(o.GetResolvedIP() == "10.1.2.3");
}

TEMPLATE_TEST_CASE("IP output Save omits ComPort when no address is set", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    auto s = SaveOf(o);
    CHECK(s.find("ComPort") == std::string::npos);
    CHECK(s.find("NetworkType=\"" + o.GetType() + "\"") != std::string::npos);
    o.SetIP("1.2.3.4", false, false);
    CHECK(SaveOf(o).find("ComPort=\"1.2.3.4\"") != std::string::npos);
}

TEMPLATE_TEST_CASE("IP output Save stores the universe in BaudRate", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    o.SetUniverse(321);
    CHECK(SaveOf(o).find("BaudRate=\"321\"") != std::string::npos);
}

TEMPLATE_TEST_CASE("IP output Save writes Enabled and clears dirty", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    CHECK(SaveOf(o).find("Enabled=\"Yes\"") != std::string::npos);
    o.Enable(false);
    CHECK(o.IsDirty());
    CHECK(SaveOf(o).find("Enabled=\"No\"") != std::string::npos);
    CHECK_FALSE(o.IsDirty());
}

TEMPLATE_TEST_CASE("IP output Save writes the proxy only when set", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    CHECK(SaveOf(o).find("FPPProxy") == std::string::npos);
    o.SetFPPProxyIP("9.8.7.6");
    CHECK(o.IsUsingFPPProxy());
    CHECK(SaveOf(o).find("FPPProxy=\"9.8.7.6\"") != std::string::npos);
    o.SetFPPProxyIP("");
    CHECK_FALSE(o.IsUsingFPPProxy());
    CHECK(SaveOf(o).find("FPPProxy") == std::string::npos);
}

TEMPLATE_TEST_CASE("IP output sort name is the address", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    o.SetIP("9.9.9.9", false, false);
    CHECK(o.GetSortName() == "9.9.9.9");
}

TEMPLATE_TEST_CASE("IP output SetTransientData advances", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType a, b;
    a.SetChannels(10);
    b.SetChannels(20);
    int32_t start = 5;
    a.SetTransientData(start, 0);
    b.SetTransientData(start, 0);
    CHECK(a.GetStartChannel() == 5);
    CHECK(a.GetEndChannel() == 14);
    CHECK(b.GetStartChannel() == 15);
    CHECK(start == 35);
}

TEMPLATE_TEST_CASE("IP output copy keeps address universe and size", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    o.SetIP("4.3.2.1", false, false);
    o.SetUniverse(12);
    o.SetChannels(77);
    o.SetSuppressDuplicateFrames(true);
    o.Enable(false);
    std::unique_ptr<Output> c(o.Copy());
    REQUIRE(c != nullptr);
    CHECK(c->GetType() == o.GetType());
    CHECK(c->GetIP() == "4.3.2.1");
    CHECK(c->GetResolvedIP() == "4.3.2.1");
    CHECK(c->GetUniverse() == 12);
    CHECK(c->GetChannels() == 77);
    CHECK(c->IsSuppressDuplicateFrames());
    CHECK_FALSE(c->IsEnabled());
    CHECK(*c == o);
}

TEMPLATE_TEST_CASE("IP output equality needs the same universe and address", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType a, b;
    a.SetIP("1.1.1.1", false, false);
    b.SetIP("1.1.1.1", false, false);
    a.SetUniverse(1);
    b.SetUniverse(1);
    CHECK(a == b);
    b.SetUniverse(2);
    CHECK_FALSE(a == b);
    b.SetUniverse(1);
    b.SetIP("1.1.1.2", false, false);
    CHECK_FALSE(a == b);
}

TEMPLATE_TEST_CASE("IP output Open fails without an address", "[protocols][generic][ip]", PT_IP_OPEN_TYPES) {
    TestType o;
    o.SetChannels(10);
    CHECK_FALSE(o.Open());
    o.Close();
}

TEMPLATE_TEST_CASE("IP output Open of a disabled output succeeds", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    o.Enable(false);
    CHECK(o.Open());
    o.Close();
}

TEMPLATE_TEST_CASE("IP output temp disable and suspend flags", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    CHECK_FALSE(o.IsTempDisable());
    o.TempDisable(true);
    CHECK(o.IsTempDisable());
    o.TempDisable(false);
    CHECK_FALSE(o.IsTempDisable());
    o.Suspend(true);
    o.Suspend(false);
    SUCCEED();
}

TEMPLATE_TEST_CASE("IP output frame skipping obeys the suppress limit", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    CHECK(o.NeedToOutput(5));
    o.SetSuppressDuplicateFrames(true);
    CHECK(o.NeedToOutput(5));
    for (int i = 0; i < 4; i++) o.SkipFrame();
    PT(o, out_skipped) = 0;
    for (int i = 0; i < 4; i++) {
        o.SkipFrame();
        CHECK_FALSE(o.NeedToOutput(5));
    }
    o.SkipFrame();
    CHECK(o.NeedToOutput(5));
    CHECK(o.NeedToOutput(0));
}

TEMPLATE_TEST_CASE("IP output force local ip falls back to the global value", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    o.SetGlobalForceLocalIP("not an ip");
    CHECK(o.GetForceLocalIPToUse() == "");
    o.SetForceLocalIP("not an ip either");
    CHECK(o.GetForceLocalIPToUse() == "");
    CHECK(o.IsUsingForceLocalIP());
}

TEMPLATE_TEST_CASE("IP output universe string is the universe unless the protocol decorates it", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    o.SetUniverse(77);
    auto s = o.GetUniverseString();
    CHECK(s.find("77") != std::string::npos);
}

TEMPLATE_TEST_CASE("IP output SetResolvedIP does not mark the output dirty", "[protocols][generic][ip]", PT_IP_TYPES) {
    TestType o;
    o.ClearDirty();
    o.SetResolvedIP("5.5.5.5");
    CHECK(o.GetResolvedIP() == "5.5.5.5");
    CHECK_FALSE(o.IsDirty());
    CHECK(o.GetIP() == "");
}

TEMPLATE_TEST_CASE("IP output xml with no attributes", "[protocols][generic][ip]", PT_IP_TYPES) {
    pugi::xml_document doc;
    auto n = ParseNode(doc, "<network/>");
    TestType o(n, false);
    CHECK(o.GetIP() == "");
    CHECK(o.GetUniverse() >= 0);
    CHECK(o.IsEnabled());
    CHECK(o.GetChannels() == 0);
}

TEMPLATE_TEST_CASE("IP output xml only resolves the address for active controllers", "[protocols][generic][ip]", PT_IP_TYPES) {
    pugi::xml_document doc;
    auto n = ParseNode(doc, "<network ComPort=\"192.168.9.9\" BaudRate=\"2\" MaxChannels=\"10\"/>");
    TestType inactive(n, false);
    CHECK(inactive.GetResolvedIP() == "");
    TestType active(n, true);
    CHECK(active.GetResolvedIP() == "192.168.9.9");
    CHECK(active.GetIP() == "192.168.9.9");
}

TEMPLATE_TEST_CASE("IP output channel count survives garbage text", "[protocols][generic][ip]", PT_IP_TYPES) {
    const char* v = GENERATE("", "x", "12x", "1 2", "+5");
    pugi::xml_document doc;
    auto n = ParseNode(doc, std::string("<network ComPort=\"1.2.3.4\" BaudRate=\"2\" MaxChannels=\"") + v + "\"/>");
    TestType o(n, false);
    CHECK(o.GetChannels() >= 0);
    CHECK(o.GetChannels() <= 65535);
}
